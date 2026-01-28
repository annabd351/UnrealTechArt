// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SparksRichFXWorker.generated.h"

class USparksRichFXWorker;

enum class SPARKSRUNTIME_API ESparksRichFXWorkerExecutionState
{
	// Single phase: Spawn FX element, animate parameters, remove FX element
	Immediate,

	// Latent, first phase: Spawn FX element, animate fade in parameters
	LatentInitial,

	// Latent, second phase: hold animation state
	LatentHold,

	// Latent, third phase: Animate fade out parameters, remove
	LatentFinal
};

USTRUCT(BlueprintType)
struct SPARKSRUNTIME_API FSparksRichFXWorkerData
{
	GENERATED_BODY()

	virtual ~FSparksRichFXWorkerData() {};

	// Negative means no duration
	float Duration = -1.0;

	virtual bool HasValidProperties() const
	{
		ensureMsgf(false, TEXT("Implementation required"));
		return false;
	}
};

enum class SPARKSRUNTIME_API ESparksRichFXWorkerPoolStatus
{
	// Ready to be allocated
	Available,

	// In use
	Allocated,

	// Has been released by requester.  Will be recycled or destroyed.
	Released,

	// For completeness -- no actual cases identified
	Invalid
};

/**
 * Base class for "FX" workers: Entities which exist to enhance the immersive experience of playing the game.  VFX (particles, post; graphics),
 * SFX (audio effects), and Practical FX (everything else).  FX are rich entities:  they can be composites of multiple
 * things, can be time-based, blended, and/or stacked.
 *
 * They specifically do NOT affect progression, or any in-world property of the game itself;  they are "decoration."
 *
 * Workers are both assets, and stateful, activate objects which execute dynamic aspects of presenting FX (e.g. blending a parameter over time).
 * Multiple worker instances can operate concurrently and independently on the game thread.
 */
UCLASS()
class SPARKSRUNTIME_API USparksRichFXWorker : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	USparksRichFXWorker(const FObjectInitializer& ObjectInitializer);

	virtual bool Activate(const ESparksRichFXWorkerExecutionState InitialState, const FHitResult& ValidHitResult, const AActor* AttachToActor = nullptr, const FName Socket = NAME_None);
	virtual void Deactivate();
	virtual bool Resume();
	virtual void RequestRelease(const double RequestedAtTime);

	virtual void PostInitProperties() override;

#pragma region Getters
	ESparksRichFXWorkerPoolStatus GetStatus() const
	{
		return Status;
	}

	void SetStatus(const ESparksRichFXWorkerPoolStatus& NewStatus)
	{
		Status = NewStatus;
	}

	void SetAllocationTime(const double Time)
	{
		AllocationTime = Time;
	}

	double GetAllocationTime() const
	{
		return AllocationTime;
	}

	ESparksRichFXWorkerExecutionState GetExecutionState() const
	{
		return ExecutionState;
	}

	void SetExecutionState(const ESparksRichFXWorkerExecutionState NewExecutionMode)
	{
		ExecutionState = NewExecutionMode;
	}
#pragma endregion

#pragma region Operators
	bool operator==(const USparksRichFXWorker& Other) const
	{
		return Guid == Other.Guid;
	}

	bool operator!=(const USparksRichFXWorker& Other) const
	{
		return Guid != Other.Guid;
	}
#pragma endregion

#pragma region FTickableGameObject
	virtual void Tick(float DeltaTime) override
	{
		const UWorld* World = GetWorld();
		check(World);

		Update(*World, DeltaTime);
	}

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Conditional;
	}

	virtual bool IsTickable() const override
	{
		return bIsActive && GetExecutionState() != ESparksRichFXWorkerExecutionState::LatentHold;
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(USparksRichFXWorker, STATGROUP_Tickables);
	}

	virtual bool IsTickableWhenPaused() const override
	{
		return false;
	}

	virtual bool IsTickableInEditor() const override
	{
		return true;
	}
#pragma endregion

protected:
	virtual void Update(const UWorld& World, const float DeltaTime) const
	{
		ensureMsgf(false, TEXT("Implementation required"));
	};

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;

	bool bIsActive = false;
	bool bIsAttachedToNonWorldActor = false;

	template <typename WorkerDataType>
	bool ValidateWorldAndData(const WorkerDataType& Data)
	{
		static_assert(std::is_base_of_v<FSparksRichFXWorkerData, WorkerDataType>, "WorkerDataType must be FSparksRichFXWorkerData");

		if (IsValid(GetWorld()) == false)
		{
			return false;
		}

		if (Data.HasValidProperties() == false)
		{
			return false;
		}

		return true;
	}

	void StartTimer(const float Duration)
	{
		const UWorld* World = GetWorld();
		World->GetTimerManager().ClearAllTimersForObject(this);
		TimerHandle.Invalidate();
		if (Duration > 0.0)
		{
			World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Duration, false);
		}
	}

	UPROPERTY()
	TObjectPtr<USceneComponent> FXComponent = nullptr;

private:
	FGuid Guid = FGuid::NewGuid();
	ESparksRichFXWorkerPoolStatus Status = ESparksRichFXWorkerPoolStatus::Invalid;
	double AllocationTime = 0;
	ESparksRichFXWorkerExecutionState ExecutionState = ESparksRichFXWorkerExecutionState::Immediate;
};
