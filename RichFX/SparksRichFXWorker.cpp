// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksRichFXWorker.h"
#include "SparksRichFXWorkerPool.h"

USparksRichFXWorker::USparksRichFXWorker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USparksRichFXWorker::PostInitProperties()
{
	Super::PostInitProperties();

	// FXComponent isn't valid at this point for CDO.  Since we don't need it, workaround.
	if (HasAnyFlags(RF_ClassDefaultObject) == false)
	{
		FXComponent->bAutoActivate = false;
		FXComponent->RegisterComponent();
		FXComponent->SetHiddenInGame(true);
		FXComponent->Deactivate();
	}

	SetStatus(ESparksRichFXWorkerPoolStatus::Available);
}

bool USparksRichFXWorker::Activate(const ESparksRichFXWorkerExecutionState InitialState, const FHitResult& ValidHitResult, const AActor* AttachToActor, const FName Socket)
{
	if (IsValid(AttachToActor))
	{
		FXComponent->AttachToComponent(AttachToActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
		bIsAttachedToNonWorldActor = true;
	}
	else
	{
		USceneComponent* SparksRichFXWorkerPoolActorRootComponent = CastChecked<ASparksRichFXWorkerPoolActor>(GetOuter())->GetRootComponent();
		FXComponent->AttachToComponent(SparksRichFXWorkerPoolActorRootComponent, FAttachmentTransformRules::KeepWorldTransform);
	}
	FXComponent->Activate();
	FXComponent->SetHiddenInGame(false);

	SetExecutionState(InitialState);
	bIsActive = true;
	return true;
};

void USparksRichFXWorker::Deactivate()
{
	switch (GetExecutionState())
	{
	case ESparksRichFXWorkerExecutionState::Immediate:
	case ESparksRichFXWorkerExecutionState::LatentFinal:
		{
			bIsActive = false;
			FXComponent->Deactivate();
			FXComponent->SetHiddenInGame(true);
			SetStatus(ESparksRichFXWorkerPoolStatus::Released);

			if (bIsAttachedToNonWorldActor)
			{
				FXComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				bIsAttachedToNonWorldActor = false;

				const ASparksRichFXWorkerPoolActor* WorkerPoolActor = GetWorld() ? USparksRichFXWorkerPool::GetWorldFXWorkerPool(*GetWorld()) : nullptr;
				if (ensureMsgf(IsValid(WorkerPoolActor), TEXT("WorkerPoolActor invalid")))
				{
					FXComponent->AttachToComponent(WorkerPoolActor->GetRootComponent(),
					                               FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				}
			}
		}
		break;

	case ESparksRichFXWorkerExecutionState::LatentInitial:
		{
			SetExecutionState(ESparksRichFXWorkerExecutionState::LatentHold);
		}
		break;

	case ESparksRichFXWorkerExecutionState::LatentHold:
		{
			SetExecutionState(ESparksRichFXWorkerExecutionState::LatentFinal);
		}
		break;
	}
}

bool USparksRichFXWorker::Resume()
{
	SetExecutionState(ESparksRichFXWorkerExecutionState::LatentFinal);
	return true;
}

void USparksRichFXWorker::RequestRelease(const double RequestedAtTime)
{
	ensureMsgf(false, TEXT("Implementation required"));
}
