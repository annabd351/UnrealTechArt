// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksEnvironmentVFXActor.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Algo/RandomShuffle.h"

ASparksEnvironmentVFXActor::ASparksEnvironmentVFXActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UNiagaraComponent* NiagaraComp = GetNiagaraComponent();
	if (IsValid(NiagaraComp) == false)
	{
		return;
	}

	PlacementVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("PlacementVolume"));
	PlacementVolume->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	PlacementVolume->CanCharacterStepUpOn = ECB_No;
	PlacementVolume->SetGenerateOverlapEvents(false);
	PlacementVolume->bHiddenInGame = true;
	PlacementVolume->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	RootComponent = PlacementVolume;

	NiagaraComp->SetAutoActivate(false);
	NiagaraComp->SetupAttachment(PlacementVolume);

	// Lock scale -- not relevant for particle systems
	NiagaraComp->TransformUpdated.AddLambda(
		[this](USceneComponent* Component, EUpdateTransformFlags, ETeleportType)
		{
			Component->SetRelativeScale3D(FVector::OneVector);
		});
}

void ASparksEnvironmentVFXActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	bWasPreviouslyPlaced = false;
	ResetEffect();
}

void ASparksEnvironmentVFXActor::ResetEffect()
{
	UNiagaraComponent* NiagaraComp = GetNiagaraComponent();
	const UWorld* World = GetWorld();
	if (IsValid(NiagaraComp) == false || IsValid(World) == false)
	{
		return;
	}

	NiagaraComp->ResetSystem();
	PlacementVolume->SetVisibility(bUsePlacementBox);
	World->GetTimerManager().ClearAllTimersForObject(this);

	PlaceEffect(NiagaraComp, World);
}

void ASparksEnvironmentVFXActor::PlaceEffect(UNiagaraComponent* NiagaraCompChecked, const UWorld* WorldChecked)
{
	// Apply random orientation
	const FVector RandomOrientationWithinCone = FMath::VRandCone(PlacementVolume->GetForwardVector(),
	                                                             FMath::DegreesToRadians(OrientationOffset));

	NiagaraCompChecked->SetRelativeRotation(
		FQuat::FindBetween(PlacementVolume->GetForwardVector(), RandomOrientationWithinCone));

	if (bUsePlacementBox)
	{
		// Are we placing multiple times?
		const bool bMultiplePlacement =
			Lifecycle == ESparksEnvironmentVFXLifecycleType::Intermittent &&
			PlacementPolicy == ESparksEnvironmentVFXPlacementPolicyType::PerActivation;

		if (bMultiplePlacement == true || bWasPreviouslyPlaced == false)
		{
			// We are using multiple placement, and haven't yet placed. Do the random placement by moving the pivot.
			MovePivot(NiagaraCompChecked, WorldChecked);
		}
	}

	if (Lifecycle == ESparksEnvironmentVFXLifecycleType::Continuous)
	{
		ActivateEffect();
	}

	if (Lifecycle == ESparksEnvironmentVFXLifecycleType::Intermittent)
	{
		StartActivationTimer(WorldChecked);
	}
}

void ASparksEnvironmentVFXActor::MovePivot(UNiagaraComponent* NiagaraCompChecked, const UWorld* WorldChecked)
{
	const FTransform PlacementTransform = PlacementVolume->GetComponentTransform();
	const FBox PlacementLocalBox = PlacementVolume->GetLocalBounds().GetBox();
	const FVector RandomLocationInBox =
		PlacementTransform.GetLocation() +
		PlacementTransform.TransformVector(FMath::RandPointInBox(PlacementLocalBox));

	if (bSnapToSurfaces == false)
	{
		NiagaraCompChecked->SetWorldLocation(RandomLocationInBox);
		bWasPreviouslyPlaced = true;
		return;
	}

	TArray<EAxis::Type> Axes = TArray<EAxis::Type>({EAxis::X, EAxis::Y, EAxis::Z});
	Algo::RandomShuffle(Axes);
	for (EAxis::Type Axis : Axes)
	{
		FHitResult HitResult;
		if (FindSnapLocation(HitResult, RandomLocationInBox, Axis, WorldChecked))
		{
			NiagaraCompChecked->SetWorldLocation(HitResult.ImpactPoint);
			bWasPreviouslyPlaced = true;
			return;
		}
	}

	// Nothing to snap to.  Use random position.
	NiagaraCompChecked->SetWorldLocation(RandomLocationInBox);
	bWasPreviouslyPlaced = true;
}

bool ASparksEnvironmentVFXActor::FindSnapLocation(FHitResult& HitResult, const FVector& InitialLocation, const EAxis::Type& Axis, const UWorld* WorldChecked) const
{
	const FTransform PlacementTransform = PlacementVolume->GetComponentTransform();
	const FVector PlacementBoxMin = PlacementTransform.GetLocation() +
		PlacementTransform.TransformVector(PlacementVolume->GetLocalBounds().GetBox().Min);

	const FVector TraceEnd = InitialLocation +
		(PlacementBoxMin - InitialLocation).Dot(PlacementTransform.GetUnitAxis(Axis)) *
		PlacementTransform.GetUnitAxis(Axis);

	const bool Result = WorldChecked->LineTraceSingleByProfile(
		HitResult,
		InitialLocation,
		TraceEnd,
		SurfaceCollisionProfile,
		FCollisionQueryParams::DefaultQueryParam
	);

	return Result && HitResult.IsValidBlockingHit();
}

void ASparksEnvironmentVFXActor::StartActivationTimer(const UWorld* WorldChecked)
{
	const float Delay = FMath::RandRange(FMath::Max(0, ActivationDelayMin), FMath::Max(0, ActivationDelayMax));

	if (FMath::IsNearlyZero(Delay) == false)
	{
		WorldChecked->GetTimerManager().SetTimer(PhaseTimer, this, &ASparksEnvironmentVFXActor::ActivateEffect, Delay,
		                                         false);
	}
	else
	{
		ActivateEffect();
	}
}

void ASparksEnvironmentVFXActor::ActivateEffect()
{
	UNiagaraComponent* NiagaraComp = GetNiagaraComponent();
	const UWorld* World = GetWorld();
	if (IsValid(NiagaraComp) == false || IsValid(World) == false)
	{
		return;
	}

	if (Lifecycle == ESparksEnvironmentVFXLifecycleType::Continuous)
	{
		NiagaraComp->ActivateSystem(true);
	}

	if (Lifecycle == ESparksEnvironmentVFXLifecycleType::Intermittent)
	{
		NiagaraComp->ActivateSystem(true);
		World->GetTimerManager().SetTimer(PhaseTimer, this, &ASparksEnvironmentVFXActor::DeactivateEffect, Lifespan,
		                                  false);
	}
}

void ASparksEnvironmentVFXActor::DeactivateEffect()
{
	UNiagaraComponent* NiagaraComp = GetNiagaraComponent();
	if (IsValid(NiagaraComp) == false)
	{
		return;
	}

	NiagaraComp->Deactivate();
	ResetEffect();
}