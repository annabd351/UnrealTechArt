// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CoreTypes.h"
#include "SparksRichFXWorker.h"
#include "UObject/Object.h"
#include "SparksRichFXWorkerPool.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SparksRichFXWorkerPool, Log, All)

class ASparksRichFXWorkerPoolActor;
class USparksRichFXWorker;

/**
 * FXWorker pool. World-scope.
 */

namespace SparksRichFXWorkerPoolStatics
{
	// TODO: Make CVars

	// Per-worker class settings (there is one sub-pool per worker class)

	// Allocate this many instances at a time
	static constexpr uint16 AllocationBlockCount = 8;

	// Always keep this number of instances alive
	static constexpr uint16 MinInstances = 32;

	// Only allow this many instances -- after this, no more FX workers from this pool until some are released
	static constexpr uint16 MaxInstances = 512;

	// Time interval between sweeps to recycle and request release
	static constexpr double CleanupPeriod = 32.0;

	// Time between sweeps to shrink pool of idle instances
	static constexpr double ShrinkPeriod = 96.0;

	// Allocated instances can remain active for this amount of time, after which they are
	// asked to release
	static constexpr double ActiveLifespan = 8.0;

	// Max time an instance can sit unallocated before being marked for garbage collection
	static constexpr double IdleLifespan = 96.0;

#if UE_BUILD_DEVELOPMENT
	// Generate log messages
	static bool bLogStats = true;

	// Time between log messages
	static constexpr double LogStatsPeriod = 4.0;
#endif
}

USTRUCT()
struct SPARKSRUNTIME_API FSparksRichFXWorkerPoolPerClass
{
	GENERATED_BODY()

	FSparksRichFXWorkerPoolPerClass(const TSubclassOf<USparksRichFXWorker> WorkerClass, ASparksRichFXWorkerPoolActor* OuterChecked, UWorld* WorldChecked);

	// Required for USTRUCT() -- don't use
	FSparksRichFXWorkerPoolPerClass() = default;

	TObjectPtr<USparksRichFXWorker> GetAvailableInstance();
	void Cleanup();
	void Shrink();

	FTimerHandle CleanupTimerHandle;
	FTimerHandle ShrinkTimerHandle;

#if UE_BUILD_DEVELOPMENT
	FTimerHandle LogTimerHandle;
#endif

	FString ToString() const;

private:
	UPROPERTY()
	TArray<TObjectPtr<USparksRichFXWorker>> InstancesAvailable;

	UPROPERTY()
	TArray<TObjectPtr<USparksRichFXWorker>> InstancesAllocated;

	TWeakObjectPtr<ASparksRichFXWorkerPoolActor> OuterChecked;
	TSubclassOf<USparksRichFXWorker> WorkerClassChecked;
	TWeakObjectPtr<UWorld> WorldChecked;

	void AddInstances(const int RequestedCount);

	int GetTotalInstanceCount() const
	{
		return InstancesAvailable.Num() + InstancesAllocated.Num();
	}

	bool ActiveLifespanExpired(const USparksRichFXWorker& Element) const
	{
		return (WorldChecked->GetTimeSeconds() - Element.GetAllocationTime()) > SparksRichFXWorkerPoolStatics::ActiveLifespan;
	}

	bool IdleLifespanExpired(const USparksRichFXWorker& Element) const
	{
		return (WorldChecked->GetTimeSeconds() - Element.GetAllocationTime()) > SparksRichFXWorkerPoolStatics::IdleLifespan;
	}
};

UCLASS()
class SPARKSRUNTIME_API USparksRichFXWorkerPool: public UWorldSubsystem
{
	GENERATED_BODY()

public:

	static ASparksRichFXWorkerPoolActor* GetWorldFXWorkerPool(const UWorld& WorldRef)
	{
		const USparksRichFXWorkerPool* WorldWorkerPool = WorldRef.GetSubsystem<USparksRichFXWorkerPool>();
		if(ensureMsgf(IsValid(WorldWorkerPool), TEXT("WorldWorkerPool invalid")) == false)
		{
			return nullptr;
		}
		return WorldWorkerPool->WorkerPoolActor;
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	UPROPERTY()
	TObjectPtr<ASparksRichFXWorkerPoolActor> WorkerPoolActor;
};

UCLASS()
class SPARKSRUNTIME_API ASparksRichFXWorkerPoolActor : public AActor
{
	GENERATED_BODY()

public:
	USparksRichFXWorker* AllocateFXWorker(const TSubclassOf<USparksRichFXWorker> WorkerClassChecked);

	void OnWorldBeginPlay(UWorld* WorldChecked);

private:
	UPROPERTY()
	TMap<const TSubclassOf<USparksRichFXWorker>, FSparksRichFXWorkerPoolPerClass> InstancePools;
};
