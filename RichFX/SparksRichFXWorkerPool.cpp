// © 2024 Jar of Sparks, All Rights Reserved

#include "SparksRichFXWorkerPool.h"
#include "SparksRichFXWorker.h"
#include "Visual/SparksRichFXDecalWorker.h"
#include "Visual/SparksRichFXPostProcessWorker.h"

DEFINE_LOG_CATEGORY(SparksRichFXWorkerPool)

namespace SparksRichFXWorkerPoolTypes
{
	TArray<TSubclassOf<USparksRichFXWorker>> Classes({
		USparksRichFXDecalWorker::StaticClass(),
		USparksRichFXPostProcessWorker::StaticClass()
	});
}

FSparksRichFXWorkerPoolPerClass::FSparksRichFXWorkerPoolPerClass(const TSubclassOf<USparksRichFXWorker> WorkerClass, ASparksRichFXWorkerPoolActor* OuterChecked, UWorld* WorldChecked)
	: OuterChecked(OuterChecked),
	  WorkerClassChecked(WorkerClass),
	  WorldChecked(WorldChecked)
{
	AddInstances(SparksRichFXWorkerPoolStatics::MinInstances);
}

void FSparksRichFXWorkerPoolPerClass::AddInstances(const int RequestedCount)
{
	const int ActualCount = FMath::Max(0, FMath::Min(RequestedCount, static_cast<int>(SparksRichFXWorkerPoolStatics::MaxInstances) - GetTotalInstanceCount()));
	const int ArraySizeAfterAdditions = InstancesAvailable.Num() + ActualCount;
	InstancesAvailable.Reserve(ArraySizeAfterAdditions);
	for (int Index = InstancesAvailable.Num(); Index < ArraySizeAfterAdditions; Index++)
	{
		USparksRichFXWorker* NewWorker = NewObject<USparksRichFXWorker>(Cast<UObject>(OuterChecked), WorkerClassChecked);
		InstancesAvailable.EmplaceAt(Index, NewWorker);
	}
}

TObjectPtr<USparksRichFXWorker> FSparksRichFXWorkerPoolPerClass::GetAvailableInstance()
{
	// At capacity
	if (GetTotalInstanceCount() >= SparksRichFXWorkerPoolStatics::MaxInstances)
	{
		return nullptr;
	}

	// No instances left.  Make more.
	if (InstancesAvailable.IsEmpty())
	{
		AddInstances(SparksRichFXWorkerPoolStatics::AllocationBlockCount);
	}

	USparksRichFXWorker* AllocatedInstance = InstancesAvailable.Pop();
	if (ensureMsgf(IsValid(AllocatedInstance), TEXT("Invalid instance returned from array")) == false)
	{
		return nullptr;
	}

	if (ensureMsgf(AllocatedInstance->GetStatus() == ESparksRichFXWorkerPoolStatus::Available, TEXT("Unavailble instance in available array")) == false)
	{
		return nullptr;
	}

	AllocatedInstance->SetStatus(ESparksRichFXWorkerPoolStatus::Allocated);
	AllocatedInstance->SetAllocationTime(WorldChecked->GetTimeSeconds());

	InstancesAllocated.Add(AllocatedInstance);

	return AllocatedInstance;
}

void FSparksRichFXWorkerPoolPerClass::Cleanup()
{
	// Find released instances and recycle.  Notify expired workers.  Also, weed out any invalid objects.
	const double CurrentTime = WorldChecked->GetTimeSeconds();
	for (int Index = 0; InstancesAllocated.IsValidIndex(Index); )
	{
		USparksRichFXWorker* AllocatedInstance = InstancesAllocated[Index];
		if (ensureMsgf(IsValid(AllocatedInstance) && AllocatedInstance->GetStatus() != ESparksRichFXWorkerPoolStatus::Available,
			TEXT("Corrupted instance: Available in InstancesAllocated")) == false)
		{
			Index++;
			continue;
		};

		if (AllocatedInstance->GetStatus() == ESparksRichFXWorkerPoolStatus::Released)
		{
			InstancesAllocated.RemoveAtSwap(Index);
			AllocatedInstance->SetStatus(ESparksRichFXWorkerPoolStatus::Available);
			AllocatedInstance->SetAllocationTime(CurrentTime);
			InstancesAvailable.Add(AllocatedInstance);
			continue;
		}

		if (ActiveLifespanExpired(*AllocatedInstance))
		{
			AllocatedInstance->RequestRelease(CurrentTime);
		}

		Index++;
	}
}

void FSparksRichFXWorkerPoolPerClass::Shrink()
{
	// Find idle instances to destroy, shrink pool
	for (int Index = 0; InstancesAvailable.IsValidIndex(Index) && InstancesAvailable.Num() >= SparksRichFXWorkerPoolStatics::MinInstances; )
	{
		USparksRichFXWorker* AvailableInstance = InstancesAvailable[Index];
		if (ensureMsgf(IsValid(AvailableInstance) && AvailableInstance->GetStatus() != ESparksRichFXWorkerPoolStatus::Allocated,
			TEXT("Corrupted instance: Allocated in InstancesAvailable")) == false)
		{
			Index++;
			continue;
		}
		if (ensureMsgf(AvailableInstance->GetStatus() != ESparksRichFXWorkerPoolStatus::Released, TEXT("Corrupted instance: Released in InstancesAvailable")) == false)
		{
			Index++;
			continue;
		}

		if (IdleLifespanExpired(*AvailableInstance))
		{
			InstancesAvailable.RemoveAtSwap(Index);
			AvailableInstance->MarkAsGarbage();
			continue;
		}

		Index++;
	}
}

FString FSparksRichFXWorkerPoolPerClass::ToString() const
{
	FString Msg("FSparksRichFXWorkerPoolPerClass");

	const FString ClassName = WorkerClassChecked->GetName();
	const int Available = InstancesAvailable.Num();
	const int Allocated = InstancesAllocated.Num();

	Msg.Appendf(TEXT("(%s) - [%d/%d]"), *ClassName, Available, Allocated);
	return Msg;
}

void USparksRichFXWorkerPool::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	WorkerPoolActor = InWorld.SpawnActor<ASparksRichFXWorkerPoolActor>();

	if (ensureMsgf(IsValid(WorkerPoolActor), TEXT("WorkerPoolActor not valid")))
	{
		WorkerPoolActor->OnWorldBeginPlay(&InWorld);
	}
}

USparksRichFXWorker* ASparksRichFXWorkerPoolActor::AllocateFXWorker(const TSubclassOf<USparksRichFXWorker> WorkerClassChecked)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return nullptr;
	}

	FSparksRichFXWorkerPoolPerClass* InstancePool = InstancePools.Find(WorkerClassChecked);
	if (ensureMsgf(InstancePool != nullptr, TEXT("No pool configured for class %s"), *WorkerClassChecked->GetName()))
	{
		USparksRichFXWorker* Instance = InstancePool->GetAvailableInstance();
		return IsValid(Instance) ? Instance : nullptr;
	}

	return nullptr;
}

void ASparksRichFXWorkerPoolActor::OnWorldBeginPlay(UWorld* WorldChecked)
{
	for (const TSubclassOf<USparksRichFXWorker>& WorkerClass : SparksRichFXWorkerPoolTypes::Classes)
	{
		FSparksRichFXWorkerPoolPerClass& Pool = InstancePools.Emplace(WorkerClass, FSparksRichFXWorkerPoolPerClass(WorkerClass, this, WorldChecked));

		WorldChecked->GetTimerManager().SetTimer(
			Pool.CleanupTimerHandle,
			FTimerDelegate::CreateWeakLambda(this,[&Pool]()
			{
				Pool.Cleanup();
			}),
			SparksRichFXWorkerPoolStatics::CleanupPeriod,
			true
			);

		WorldChecked->GetTimerManager().SetTimer(
		Pool.ShrinkTimerHandle,
		FTimerDelegate::CreateWeakLambda(this,[&Pool]()
		{
			Pool.Shrink();
		}),
		SparksRichFXWorkerPoolStatics::ShrinkPeriod,
		true
		);

#if UE_BUILD_DEVELOPMENT
		if (SparksRichFXWorkerPoolStatics::bLogStats)
		{
			WorldChecked->GetTimerManager().SetTimer(
				Pool.LogTimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [&Pool]()
				{
					UE_LOG(SparksRichFXWorkerPool, Verbose, TEXT("%s"), *Pool.ToString());
				}),
				SparksRichFXWorkerPoolStatics::LogStatsPeriod,
				true
			);
		}
#endif
	}
}
