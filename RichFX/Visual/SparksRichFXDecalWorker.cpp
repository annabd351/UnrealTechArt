// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksRichFXDecalWorker.h"
#include "Components/DecalComponent.h"
#include "SparksRuntime/RichFX/SparksRichFXWorkerPool.h"

FSparksRichFXDecalTimeline::FSparksRichFXDecalTimeline(const USparksRichFXDecalWorker* Worker)
{
	bIsValid = false;
	if (IsValid(Worker) == false)
	{
		return;
	}

	const FSparksRichFXDecalWorkerData& WorkerData = Worker->GetData();

	AlphaFadeOn.Min = 0.0;
	AlphaFadeOn.Max = WorkerData.FadeOnDuration;

	const TInterval<float> AlphaHold(AlphaFadeOn.Max + UE_SMALL_NUMBER, WorkerData.Duration - WorkerData.FadeOffDuration);

	AlphaFadeOff.Min = AlphaHold.Max + UE_SMALL_NUMBER;
	AlphaFadeOff.Max = WorkerData.Duration;

	const TInterval<float> EmissiveHold(0.0, WorkerData.EmissiveDuration - WorkerData.EmissiveFadeDuration);

	EmissiveFadeOff.Min = EmissiveHold.Max + UE_SMALL_NUMBER;
	EmissiveFadeOff.Max = WorkerData.EmissiveDuration;

	bIsValid =
		(WorkerData.bUseFadeOn ? AlphaFadeOn.IsValid() : true) &&
		AlphaHold.IsValid() &&
		(WorkerData.bUseFadeOff ? AlphaFadeOff.IsValid() : true) &&
		(WorkerData.bUseEmissive ? EmissiveHold.IsValid() : true) &&
		(WorkerData.bUseEmissive ? EmissiveFadeOff.IsValid() : true) &&
		WorkerData.EmissiveDuration <= WorkerData.Duration;
}

USparksRichFXDecalWorker::USparksRichFXDecalWorker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FXComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	TimerDelegate.BindUObject(this, &USparksRichFXDecalWorker::Deactivate);
}

bool USparksRichFXDecalWorker::Activate(const ESparksRichFXWorkerExecutionState InitialState, const FHitResult& ValidHitResult, const AActor* AttachToActor, const FName Socket)
{
	if (ValidateWorldAndData<FSparksRichFXDecalWorkerData>(GetData()) == false)
	{
		return false;
	}

	// Timing phases
	Timeline = FSparksRichFXDecalTimeline(this);
	if (Timeline.bIsValid == false)
	{
		return false;
	}

	// Set up MID
	if (IsValid(Data.Material))
	{
		// For compatibility with earlier version
		MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(Data.Material, this, *GetName());
	}
	else
	{
		const int RandomMaterialIndex = FMath::RandRange(0, Data.Materials.Num() - 1);
		UMaterialInterface* ChosenMaterial = Data.Materials.IsValidIndex(RandomMaterialIndex) ? Data.Materials[RandomMaterialIndex] : nullptr;
		MaterialInstanceDynamic = IsValid(ChosenMaterial) ? UMaterialInstanceDynamic::Create(ChosenMaterial, this, *GetName()) : nullptr;
	}

	if (IsValid(MaterialInstanceDynamic) == false)
	{
		return false;
	}

	MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeOnMaterialParamName, Data.bUseFadeOn ? 0.0 : 1.0);
	MaterialInstanceDynamic->SetScalarParameterValue(Data.EmissiveFadeDurationMaterialParamName, Data.bUseEmissive ? 1.0 : 0.0);
	MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeOffMaterialParamName, Data.bUseFadeOff ? 1.0 : 0.0);

	// Initialize decal component
	UDecalComponent* DecalComponent = ExactCast<UDecalComponent>(FXComponent);
	DecalComponent->SetMaterial(0, MaterialInstanceDynamic);
	DecalComponent->SortOrder = GetSortOrder();

	// Handle fading in the material -- disable built-in controls
	DecalComponent->FadeStartDelay = 0.0;
	DecalComponent->FadeDuration = 0.0;
	DecalComponent->FadeInDuration = 0.0;
	DecalComponent->FadeInStartDelay = 0.0;
	DecalComponent->bDestroyOwnerAfterFade = false;

	// Size
	DecalComponent->DecalSize = FVector(
		FMath::RandRange(Data.MinSize.X, Data.MaxSize.X),
		FMath::RandRange(Data.MinSize.Y, Data.MaxSize.Y),
		FMath::RandRange(Data.MinSize.Z, Data.MaxSize.Z)
		);
	DecalComponent->SetLifeSpan(0.0);

	// Rotation
	FRotator Rotation = (-ValidHitResult.ImpactNormal).Rotation().Add(-90.0, 180.0, 0.0);
	Rotation.Add(0.0, FMath::RandRange(Data.MinRotation, Data.MaxRotation), 0.0);
	DecalComponent->SetWorldRotation(Rotation);

	DecalComponent->SetWorldLocation(ValidHitResult.ImpactPoint);

	ActivationTime = GetWorld()->GetTimeSeconds();
	StartTimer(Data.Duration);
	return Super::Activate(InitialState, ValidHitResult, AttachToActor, Socket);
}

bool USparksRichFXDecalWorker::Resume()
{
	if (ValidateWorldAndData(GetData()) == false)
	{
		return false;
	}

	StartTimer(Data.Duration);
	return Super::Resume();
}

void USparksRichFXDecalWorker::Deactivate()
{
	Super::Deactivate();
}

void USparksRichFXDecalWorker::RequestRelease(const double RequestedAtTime)
{
	// Not active -- release approved
	if (bIsActive == false)
	{
		SetStatus(ESparksRichFXWorkerPoolStatus::Released);
		return;
	}

	// If it's an Immediate in process, just let it finish
	if (GetExecutionState() == ESparksRichFXWorkerExecutionState::Immediate)
	{
		return;
	}

	// Outside camera frustum -- release without fade off
	const bool InFrustum = IsInPlayerCameraFrustum(this, FXComponent->CalcBounds(FXComponent->GetComponentTransform()));
	if (InFrustum == false)
	{
		SetExecutionState(ESparksRichFXWorkerExecutionState::LatentFinal);
		Deactivate();
		return;
	}

	// Decal is in frustum. Check if it's releasable.  If so, fade it off.
	const APawn* Player = GetLocalPlayerCharacter(this);
	if (ensureMsgf(IsValid(Player), TEXT("Player character invalid")))
	{
		return;
	}

	// Outside cull distance -- fade off
	if ((FXComponent->GetComponentLocation() - Player->GetActorLocation()).SquaredLength() > CullDistanceSquared)
	{
		Resume();
		return;
	}

	const UWorld* World = GetWorld();
	check(World);

	// Past lifespan -- fade off
	if (CullLifespanExpired(GetWorld()->GetTimeSeconds()))
	{
		Resume();
	}
}

void USparksRichFXDecalWorker::Update(const UWorld& World, const float DeltaTime) const
{
	if (IsValid(MaterialInstanceDynamic) == false)
	{
		return;
	}

	switch (GetExecutionState())
	{
	case ESparksRichFXWorkerExecutionState::Immediate:
		{
			const float TimeElapsed = World.GetTimerManager().GetTimerElapsed(TimerHandle);
			const float FadeOnValue = Data.bUseFadeOn ? Timeline.GetAlphaFadeOn(TimeElapsed) : 0.0;
			const float FadeOffValue = Data.bUseFadeOff ? Timeline.GetAlphaFadeOff(TimeElapsed) : 0.0;
			const float EmissiveValue = Data.bUseEmissive ? Timeline.GetEmissive(TimeElapsed) : 0.0;

			MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeOnMaterialParamName, FadeOnValue);
			MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeOffMaterialParamName, FadeOffValue);
			MaterialInstanceDynamic->SetScalarParameterValue(Data.EmissiveFadeDurationMaterialParamName, EmissiveValue);
		}
		break;
	case ESparksRichFXWorkerExecutionState::LatentInitial:
		{
			const float TimeElapsed = World.GetTimerManager().GetTimerElapsed(TimerHandle);
			const float FadeOnValue = Data.bUseFadeOn ? Timeline.GetAlphaFadeOn(TimeElapsed) : 0.0;
			const float FadeOffValue = Data.bUseFadeOff ? 1.0 : 0.0;
			const float EmissiveValue = Data.bUseEmissive ? Timeline.GetEmissive(TimeElapsed) : 0.0;

			MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeOnMaterialParamName, FadeOnValue);
			MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeOffMaterialParamName, FadeOffValue);
			MaterialInstanceDynamic->SetScalarParameterValue(Data.EmissiveFadeDurationMaterialParamName, EmissiveValue);
		}
		break;
	case ESparksRichFXWorkerExecutionState::LatentFinal:
		{
			const float TimeElapsed = World.GetTimerManager().GetTimerElapsed(TimerHandle);
			const float FadeOnValue = Data.bUseFadeOn ? 1.0 : 0.0;
			const float FadeOffValue = Data.bUseFadeOff ? Timeline.GetAlphaFadeOff(TimeElapsed) : 0.0;
			constexpr float EmissiveValue = 0.0;

			MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeOnMaterialParamName, FadeOnValue);
			MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeOffMaterialParamName, FadeOffValue);
			MaterialInstanceDynamic->SetScalarParameterValue(Data.EmissiveFadeDurationMaterialParamName, EmissiveValue);
		}
		break;
	case ESparksRichFXWorkerExecutionState::LatentHold:
		ensureMsgf(false, TEXT("Unreachable case"));
		break;
	}
}

bool USparksRichFXDecalWorker::IsInPlayerCameraFrustum(const UObject* WorldContextObject, const FBoxSphereBounds& Bounds)
{
	const ULocalPlayer* LocalPlayer = UGameplayStatics::GetPlayerController(WorldContextObject, 0)->GetLocalPlayer();
	if (IsValid(LocalPlayer) == false)
	{
		return false;
	}

	FSceneViewProjectionData ProjectionData;
	LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, ProjectionData);
	const FMatrix ViewProjMatrix = ProjectionData.ComputeViewProjectionMatrix();
	FConvexVolume Frustum;
	GetViewFrustumBounds(Frustum, ViewProjMatrix, false);

	return Frustum.IntersectBox(Bounds.Origin, Bounds.BoxExtent);
}