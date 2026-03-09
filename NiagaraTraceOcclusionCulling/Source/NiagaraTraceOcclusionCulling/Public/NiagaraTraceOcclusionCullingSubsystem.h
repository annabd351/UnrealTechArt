// Demo -- Not for distribution, all rights reserved

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraTraceOcclusionCullingSubsystem.generated.h"

struct FNiagaraTraceOcclusionCullingComponentInfo
{
	FNiagaraTraceOcclusionCullingComponentInfo(const UNiagaraComponent& component)
	{
		initialVisibility = component.GetVisibleFlag();
		initialPaused = component.IsPaused();
		initialLocation = component.GetComponentLocation();
	}

	bool initialVisibility;
	bool initialPaused;
	FVector initialLocation;

	void RestoreState(UNiagaraComponent* niagaraComponentChecked) const;
	bool isOccluded = false;

	// For debug visualization
	FVector traceStart = FVector::ZeroVector;
	FVector traceEnd = FVector::ZeroVector;
};

struct TNiagaraTraceOcclusionCullingComponentPairType : TPair<TWeakObjectPtr<UNiagaraComponent>, TSharedPtr<FNiagaraTraceOcclusionCullingComponentInfo>>
{
	TNiagaraTraceOcclusionCullingComponentPairType(UNiagaraComponent* component, const TSharedPtr<FNiagaraTraceOcclusionCullingComponentInfo>& componentInfoPtr);
	TNiagaraTraceOcclusionCullingComponentPairType() = default;
};

class UNiagaraComponent;

/**
 * Dynamic performance optimization system which "culls" Niagara instances not visible to the player.  "Culling," in this case, is
 * pausing systems and setting visibility to false when occluded.  We determine occlusion through efficient batching of a
 * fixed (small) number of line traces per tick, and a manageably low tick rate.
 *
 * This solves a problem ignored by the base Niagara framework:  Niagara systems simulate even when the player can't see them.
 * In large games, unneeded simulation can eat up substantial CPU and GPU time.  Also, occluded systems still incur emitter update
 * costs, and have a (minor) impact on the frustum culling loop.
 *
 * The use case for this optimization is games with lots of VFX, and lots of occluded sightlines.  It was developed for use in
 * a first-person game set in a dense city.  Lots of buildings, boxes, vehicles, etc. to occlude the player's sightline (and
 * provide cover).  And lots and lots of VFX to provide life.  In this real-world use case, we're able to shave a significant
 * amount of Niagara overhead.
 *
 * **NOTE: This system requires a minor engine change**
 * 
 * This system was designed for a large, existing project with the assumption that we can't ask VFX artists to retrofit a
 * UNiagaraComponent subclass into the game.  The cleanest way to detect creation of VFX elements is to add delegate
 * methods to the engine.
 * 
 * In UNiagaraComponent.h, the definition would looks like
 *  DECLARE_MULTICAST_DELEGATE_OneParam(FOnNiagaraComponentRegistered, UNiagaraComponent*);
 *  DECLARE_MULTICAST_DELEGATE_OneParam(FOnNiagaraComponentUnregistered, UNiagaraComponent*);
 *
 *  NIAGARA_API static FOnNiagaraComponentRegistered OnComponentRegistered;
 *  NIAGARA_API static FOnNiagaraComponentUnregistered OnComponentUnregistered;
 *
 * In UNiagaraComponent.cpp, the implementation is
 *  UNiagaraComponent::FOnNiagaraComponentRegistered UNiagaraComponent::OnComponentRegistered;
 *  UNiagaraComponent::FOnNiagaraComponentUnregistered UNiagaraComponent::OnComponentUnregistered;
 *
 * Add near the end of UNiagaraComponent::OnRegister()
 *  OnComponentRegistered.Broadcast(this);
 *
 * Add near the end of UNiagaraComponent::OnUnregister()
 *  OnComponentUnregistered.Broadcast(this);
 */
UCLASS()
class NIAGARATRACEOCCLUSIONCULLING_API UNiagaraTraceOcclusionCullingSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

#pragma region UWorldSubsystem, FTickableWorldSubsystem

public:
	virtual void Tick(float deltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual void Initialize(FSubsystemCollectionBase& collection) override;
	virtual void Deinitialize() override;
	virtual bool IsTickableWhenPaused() const override;
	virtual bool IsTickable() const override;
	virtual ETickableTickType GetTickableTickType() const override;

#pragma endregion UWorldSubsystem, FTickableWorldSubsystem

#pragma region Constants

	static constexpr float kMaxTickRateLimit = 60.0f;
	static constexpr float kMaxTickRateInitial = 30.0f;

	static constexpr int32 kMaxTracesPerTickLimit = 128;
	static constexpr int32 kMaxTracesPerTickInitial = 64;

	static constexpr float kPlayerViewpointOffsetInitial = 150.f;
	static constexpr int32 kComponentRemovalBatchSizeInitial = 16;

#pragma endregion

	void SetEnabled(const bool enabled);
	bool IsEnabled() const;

	void OnNiagaraComponentRegistered(UNiagaraComponent* niagaraComponent);
	void OnNiagaraComponentUnregistered(UNiagaraComponent* niagaraComponent);

	using ComponentPtr = TWeakObjectPtr<UNiagaraComponent>;
	using ComponentInfoPtr = TSharedPtr<FNiagaraTraceOcclusionCullingComponentInfo>;
	using ComponentPairType = TNiagaraTraceOcclusionCullingComponentPairType;

	void OnTraceComplete(const FTraceHandle& handle, FTraceDatum& datum);

private:
	TArray<TWeakObjectPtr<UNiagaraComponent>> RecentlyRegisteredComponents;
	TSet<TWeakObjectPtr<UNiagaraComponent>> RecentlyUnregisteredComponents;
	TArray<ComponentPairType> Components;
	TSpscQueue<ComponentPairType> ComponentsNeedingUpdate;
	void FlushComponentsNeedingUpdate();

	bool bEnabled = false;
	float AccumulatedTickDelta = 0.0f;

	FVector LastPlayerViewLocation;
	bool IsLastPlayerViewLocationValid = false;
	void ProcessRecentlyRegisteredComponents();
	static bool GetLineTraceEndpoints(const UNiagaraComponent* niagaraComponentChecked, const FVector& viewLocation,
	                                  float kViewpointOffset, FVector& traceStart, FVector& traceEnd);
	void DispatchLineTraces(const FVector& viewLocation, UWorld* worldChecked);
	void UpdateComponents();
	int32 NextComponentIndexToTrace = 0;
	FTraceDelegate TraceDelegate;

	void CleanComponents();
	int32 NextComponentIndexToClean = 0;

	void ResetOcclusionCullingDebug();
	void RegisterConsoleCommands();
	void DrawDebugInfo(const UWorld* worldChecked);

	void RestoreStateAllComponents();

	// Settings
	float MaxTickRate = kMaxTickRateInitial;
	int32 MaxTracesPerTick = kMaxTracesPerTickInitial;
	float PlayerViewpointOffset = kPlayerViewpointOffsetInitial;
	bool ShouldDrawDebugInfo = false;
	int32 ComponentRemovalBatchSize = kComponentRemovalBatchSizeInitial;
};
