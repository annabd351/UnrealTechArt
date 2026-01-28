// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksRichFXPostProcessWorker.h"
#include "Components/PostProcessComponent.h"
#include "SparksRuntime/RichFX/SparksRichFXWorkerPool.h"

USparksRichFXPostProcessWorker::USparksRichFXPostProcessWorker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)

{
	FXComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	TimerDelegate.BindUObject(this, &USparksRichFXPostProcessWorker::Deactivate);
}

bool USparksRichFXPostProcessWorker::Activate(const ESparksRichFXWorkerExecutionState InitialState, const FHitResult& ValidHitResult, const AActor* AttachToActor, const FName Socket)
{
	if (ValidateWorldAndData<FSparksRichFXPostProcessWorkerData>(Data) == false)
	{
		return false;
	};

	// Set up MID
	MaterialInstanceDynamic = UMaterialInstanceDynamic::Create(Data.Material, this, *GetName());
	if (IsValid(MaterialInstanceDynamic) == false)
	{
		return false;
	}

	MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeMaterialParamName, 0.0);

	UPostProcessComponent* PostProcessComponent = ExactCast<UPostProcessComponent>(FXComponent);
	PostProcessComponent->Activate(true);
 	PostProcessComponent->bEnabled = true;
 	PostProcessComponent->bUnbound = true;
	PostProcessComponent->Settings.AddBlendable(MaterialInstanceDynamic, 1.0);

	StartTimer(Data.FadeOnDuration);
	return Super::Activate(InitialState, ValidHitResult, AttachToActor, Socket);
}

bool USparksRichFXPostProcessWorker::Resume()
{
	if (ValidateWorldAndData(GetData()) == false)
	{
		return false;
	}

	StartTimer(Data.FadeOffDuration);
	return Super::Resume();
}

void USparksRichFXPostProcessWorker::Deactivate()
{
	UPostProcessComponent* PostProcessComponent = ExactCast<UPostProcessComponent>(FXComponent);
	if (ensureMsgf(IsValid(PostProcessComponent), TEXT("PostProcessComponent invalid at Deactivate()")) == false)
	{
		return;
	}

	if (GetExecutionState() == ESparksRichFXWorkerExecutionState::Immediate || GetExecutionState() == ESparksRichFXWorkerExecutionState::LatentFinal)
	{
		PostProcessComponent->Settings.RemoveBlendable(MaterialInstanceDynamic);
	}

	Super::Deactivate();
}

void USparksRichFXPostProcessWorker::RequestRelease(const double RequestedAtTime)
{
	SetExecutionState(ESparksRichFXWorkerExecutionState::LatentFinal);
	Deactivate();
	SetStatus(ESparksRichFXWorkerPoolStatus::Released);
}

void USparksRichFXPostProcessWorker::Update(const UWorld& World, const float DeltaTime) const
{
	if (IsValid(MaterialInstanceDynamic) == false || Data.HasValidProperties() == false)
	{
		return;
	}

	const float Rate = World.GetTimerManager().GetTimerRate(TimerHandle);
	const float Remaining = World.GetTimerManager().GetTimerElapsed(TimerHandle);
	const float BlendIn = Data.FadeOnCurve ? Data.FadeOnCurve->GetFloatValue(Remaining / Rate) : 0.0;
	const float BlendOut = Data.bUseFadeOff && (Data.FadeOffCurve != nullptr) ? Data.FadeOffCurve->GetFloatValue(Remaining / Rate) : 1.0;

	switch (GetExecutionState()) {
	case ESparksRichFXWorkerExecutionState::Immediate:
	case ESparksRichFXWorkerExecutionState::LatentInitial:
		MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeMaterialParamName, BlendIn);
		break;
	case ESparksRichFXWorkerExecutionState::LatentHold:
		ensureMsgf(false, TEXT("Unreachable case"));
		break;
	case ESparksRichFXWorkerExecutionState::LatentFinal:
		if (Data.bUseFadeOff)
		{
			MaterialInstanceDynamic->SetScalarParameterValue(Data.FadeMaterialParamName, BlendOut);
		}
		break;
	}
}
