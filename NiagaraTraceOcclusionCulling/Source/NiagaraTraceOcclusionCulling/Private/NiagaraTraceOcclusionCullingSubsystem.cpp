// Demo -- Not for distribution, all rights reserved

#include "NiagaraTraceOcclusionCullingSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraTraceOcclusionCullingStats.h"
#include "Kismet/GameplayStatics.h"

//----------------------------------------------------------------------------------------------------
#pragma region TNiagaraTraceOcclusionCullingComponentPairType

TNiagaraTraceOcclusionCullingComponentPairType::TNiagaraTraceOcclusionCullingComponentPairType(UNiagaraComponent* component, const TSharedPtr<FNiagaraTraceOcclusionCullingComponentInfo>& componentInfoPtr)
{
	Key = TWeakObjectPtr(component);
	Value = componentInfoPtr;
}

void FNiagaraTraceOcclusionCullingComponentInfo::RestoreState(UNiagaraComponent* niagaraComponentChecked) const
{
	niagaraComponentChecked->SetVisibility(initialVisibility);
	niagaraComponentChecked->SetPaused(initialPaused);
}

bool operator==(const TNiagaraTraceOcclusionCullingComponentPairType& lhs, const TNiagaraTraceOcclusionCullingComponentPairType& rhs)
{
	return lhs.Key == rhs.Key;
}

#pragma endregion

//----------------------------------------------------------------------------------------------------
#pragma region UWorldSubsystem

TStatId UNiagaraTraceOcclusionCullingSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UNiagaraTraceOcclusionCullingSubsystem, STATGROUP_Tickables);
}

void UNiagaraTraceOcclusionCullingSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);

	RegisterConsoleCommands();

	UNiagaraComponent::OnComponentRegistered.AddUObject(this, &UNiagaraTraceOcclusionCullingSubsystem::OnNiagaraComponentRegistered);
	UNiagaraComponent::OnComponentUnregistered.AddUObject(this, &UNiagaraTraceOcclusionCullingSubsystem::OnNiagaraComponentUnregistered);
	TraceDelegate.BindUObject(this, &UNiagaraTraceOcclusionCullingSubsystem::OnTraceComplete);
}

void UNiagaraTraceOcclusionCullingSubsystem::Deinitialize()
{
	Super::Deinitialize();

	UNiagaraComponent::OnComponentRegistered.RemoveAll(this);
	UNiagaraComponent::OnComponentUnregistered.RemoveAll(this);
	TraceDelegate.Unbind();

	RestoreStateAllComponents();
}

ETickableTickType UNiagaraTraceOcclusionCullingSubsystem::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

bool UNiagaraTraceOcclusionCullingSubsystem::IsTickable() const
{
	return true;
}

bool UNiagaraTraceOcclusionCullingSubsystem::IsTickableWhenPaused() const
{
	return false;
}

#pragma endregion

//----------------------------------------------------------------------------------------------------
#pragma region Tick

void UNiagaraTraceOcclusionCullingSubsystem::Tick(float deltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_NiagaraTraceOcclusionCulling_Tick);
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("UNiagaraTraceOcclusionCullingSubsystem::Tick");

	Super::Tick(deltaTime);

	if (IsEnabled() == false)
	{
		return;
	}

	// Could potentially vary this dynamically.
	check(MaxTickRate > 0.0f);
	const float kTickInterval = 1.0f/MaxTickRate;

	AccumulatedTickDelta += deltaTime;
	if (AccumulatedTickDelta < kTickInterval)
	{
		return;
	}

	AccumulatedTickDelta = 0.0f;

	ProcessRecentlyRegisteredComponents();
	SET_DWORD_STAT(STAT_NiagaraTraceOcclusionCulling_ManagedComponents, Components.Num());

	if (IsEnabled() == false || Components.IsEmpty())
	{
		return;
	}

	// Use info from previous tick line traces
	UpdateComponents();

	const APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	if (UNLIKELY(IsValid(playerController) == false))
	{
		return;
	}

	FVector viewLocation;
	FRotator viewRotation;
	playerController->GetPlayerViewPoint(viewLocation, viewRotation);

	// Early exit if player is stationary and we're not displaying debug locators.
	const bool viewLocationChanged = viewLocation != LastPlayerViewLocation || IsLastPlayerViewLocationValid == false;
	LastPlayerViewLocation = viewLocation;
	IsLastPlayerViewLocationValid = true;
	if (viewLocationChanged == false && ShouldDrawDebugInfo == false)
	{
		if (LIKELY(ComponentsNeedingUpdate.IsEmpty()))
		{
			CleanComponents();
		}
		return;
	}

	UWorld* world = GetWorld();
	if (UNLIKELY(IsValid(world) == false))
	{
		return;
	}

	SET_DWORD_STAT(STAT_NiagaraTraceOcclusionCulling_OccludedComponents, 0);
	DispatchLineTraces(viewLocation, world);

	if (ShouldDrawDebugInfo)
	{
		DrawDebugInfo(world);
	}
}

void UNiagaraTraceOcclusionCullingSubsystem::ProcessRecentlyRegisteredComponents()
{
	SCOPE_CYCLE_COUNTER(STAT_NiagaraTraceOcclusionCulling_ProcessRegistered);

	for (int32 index = 0; index < RecentlyRegisteredComponents.Num(); )
	{
		if (RecentlyRegisteredComponents[index].IsValid() == false)
		{
			RecentlyRegisteredComponents.RemoveAtSwap(index);
			continue;
		}

		UNiagaraComponent* niagaraComponent = RecentlyRegisteredComponents[index].IsValid() ? RecentlyRegisteredComponents[index].Get() : nullptr;
		if (niagaraComponent != nullptr && niagaraComponent->GetSystemInstanceController().IsValid())
		{
			RecentlyRegisteredComponents.RemoveAtSwap(index);
			const int32 newElementIndex = Components.Emplace(niagaraComponent, MakeShared<FNiagaraTraceOcclusionCullingComponentInfo>(*niagaraComponent));
		}
		else
		{
			++index;
		}
	}
}

bool UNiagaraTraceOcclusionCullingSubsystem::GetLineTraceEndpoints(const UNiagaraComponent* niagaraComponentChecked, const FVector& viewLocation, const float kViewpointOffset, FVector& traceStart, FVector& traceEnd)
{
	SCOPE_CYCLE_COUNTER(STAT_NiagaraTraceOcclusionCulling_GetLineTraceEndpoints);

	const FVector componentLocation = niagaraComponentChecked->GetComponentLocation();
	const FVector unNormalizedDirection = componentLocation - viewLocation;
	const float length = unNormalizedDirection.Length();
	if (length > SMALL_NUMBER)
	{
		const FVector direction = unNormalizedDirection/length;

		traceStart = componentLocation + FVector(0.0f, 0.0f, viewLocation.Z);
		traceEnd = direction * kViewpointOffset + viewLocation;

		return true;
	}
	return false;
}

void UNiagaraTraceOcclusionCullingSubsystem::DispatchLineTraces(const FVector& viewLocation, UWorld* worldChecked)
{
	SCOPE_CYCLE_COUNTER(STAT_NiagaraTraceOcclusionCulling_DispatchTraces);
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("UNiagaraTraceOcclusionCullingSubsystem::DispatchLineTraces");

	static const FCollisionQueryParams kCollisionQueryParams = FCollisionQueryParams::DefaultQueryParam;
	static const FCollisionResponseParams kCollisionResponseParams = FCollisionResponseParams::DefaultResponseParam;

	if (Components.IsEmpty())
	{
		return;
	}

	check(MaxTracesPerTick >= 0 && MaxTracesPerTick <= kMaxTracesPerTickLimit);

	int32 index = NextComponentIndexToTrace;
	int32 windowCount = 0;
	const float kViewpointOffset = PlayerViewpointOffset;

	// Avoid modulo operation
	const int32 lastValidArrayIndex = Components.Num() - 1;
	for (; windowCount < MaxTracesPerTick; index = index == lastValidArrayIndex ? 0 : index + 1, windowCount++)
	{
		ComponentPairType& elementRef = Components[index];
		check(elementRef.Value.IsValid())

		if (elementRef.Key.IsValid() == false || elementRef.Key->GetSystemInstanceController().IsValid() == false)
		{
			continue;
		}

		UNiagaraComponent* niagaraComponent = elementRef.Key.Get();
		ComponentInfoPtr& componentInfoPtr = elementRef.Value;
		if (GetLineTraceEndpoints(niagaraComponent, viewLocation, kViewpointOffset, componentInfoPtr->traceStart, componentInfoPtr->traceEnd) == false)
		{
			continue;
		}

		INC_DWORD_STAT(STAT_NiagaraTraceOcclusionCulling_ActiveTraces);

		worldChecked->AsyncLineTraceByChannel(
			EAsyncTraceType::Test,
			componentInfoPtr->traceStart,
			componentInfoPtr->traceEnd,
			ECC_Visibility,
			kCollisionQueryParams,
			kCollisionResponseParams,
			&TraceDelegate,
			index
			);
	}

	NextComponentIndexToTrace = index;
}

void UNiagaraTraceOcclusionCullingSubsystem::OnTraceComplete(const FTraceHandle& handle, FTraceDatum& datum)
{
	DEC_DWORD_STAT(STAT_NiagaraTraceOcclusionCulling_ActiveTraces);
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("UNiagaraTraceOcclusionCullingSubsystem::OnTraceComplete");

	const int32 componentIndex = datum.UserData;
	check(Components.IsValidIndex(componentIndex));

	ComponentPairType& componentRef = Components[componentIndex];
	check(componentRef.Value.IsValid());

	const bool isNowOccluded = datum.OutHits.Num() > 0;
	const bool willNeedUpdate = isNowOccluded != componentRef.Value->isOccluded;
	componentRef.Value->isOccluded = isNowOccluded;

	if (willNeedUpdate)
	{
		ComponentsNeedingUpdate.Enqueue(componentRef);
	}

	INC_DWORD_STAT_BY(STAT_NiagaraTraceOcclusionCulling_OccludedComponents, isNowOccluded ? 1 : 0);
}

void UNiagaraTraceOcclusionCullingSubsystem::UpdateComponents()
{
	SCOPE_CYCLE_COUNTER(STAT_NiagaraTraceOcclusionCulling_UpdateComponents);
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("UNiagaraTraceOcclusionCullingSubsystem::UpdateComponents");

	ComponentPairType componentPair;
	while (ComponentsNeedingUpdate.Dequeue(componentPair))
	{
		check(componentPair.Value.IsValid());
		if (componentPair.Key.IsValid() == false)
		{
			continue;
		}

		componentPair.Key->SetVisibility(componentPair.Value->isOccluded == false);
		componentPair.Key->SetPaused(componentPair.Value->isOccluded == true);
	}
}

void UNiagaraTraceOcclusionCullingSubsystem::CleanComponents()
{
	if (Components.IsEmpty())
	{
		return;
	}

	int32 index = NextComponentIndexToClean;
	int32 batchCount = 0;
	for (; batchCount < ComponentRemovalBatchSize && index < Components.Num(); batchCount++)
	{
		ComponentPairType& elementRef = Components[index];
		check(elementRef.Value.IsValid());

		UNiagaraComponent* niagaraComponent = elementRef.Key.IsValid() ? elementRef.Key.Get() : nullptr;

		// Invalid components
		if (niagaraComponent == nullptr)
		{
			Components.RemoveAtSwap(index);
			continue;
		}

		// Unregistered components 
		if (RecentlyUnregisteredComponents.Contains(niagaraComponent))
		{
			RecentlyUnregisteredComponents.Remove(niagaraComponent);
			Components.RemoveAtSwap(index);
			continue;
		}

		// Moving components -- they're probably not a good candidate for culling.
		const bool hasMoved = elementRef.Value->initialLocation != niagaraComponent->GetComponentLocation();
		if (hasMoved)
		{
			elementRef.Value->RestoreState(niagaraComponent);
			Components.RemoveAtSwap(index);
		}
		else
		{
			index = index == Components.Num() - 1 ? 0 : index + 1;
		}
	}

	NextComponentIndexToClean = index;
	NextComponentIndexToTrace = Components.IsEmpty() ? 0 : NextComponentIndexToTrace % Components.Num();
}

#pragma endregion

//----------------------------------------------------------------------------------------------------
#pragma region Start/Stop

void UNiagaraTraceOcclusionCullingSubsystem::SetEnabled(bool enabled)
{
	bEnabled = enabled;
}

bool UNiagaraTraceOcclusionCullingSubsystem::IsEnabled() const
{
	return bEnabled;
}

void UNiagaraTraceOcclusionCullingSubsystem::RestoreStateAllComponents()
{
	// Return Niagara components to their initial, unmanaged state
	for (ComponentPairType& element : Components)
	{
		if (element.Key.IsValid() && element.Value.IsValid())
		{
			element.Value->isOccluded = false;
			UNiagaraComponent* niagaraComponent = element.Key.Get();
			element.Value->RestoreState(niagaraComponent);
		}
	}
}

#pragma endregion

//----------------------------------------------------------------------------------------------------
#pragma region Registration

void UNiagaraTraceOcclusionCullingSubsystem::OnNiagaraComponentRegistered(UNiagaraComponent* niagaraComponent)
{
	if (IsValid(niagaraComponent))
	{
		RecentlyRegisteredComponents.Emplace(niagaraComponent);
	}
}

void UNiagaraTraceOcclusionCullingSubsystem::OnNiagaraComponentUnregistered(UNiagaraComponent* niagaraComponent)
{
	if (IsValid(niagaraComponent) == false)
	{
		RecentlyUnregisteredComponents.Emplace(niagaraComponent);
	}
}

#pragma endregion

//----------------------------------------------------------------------------------------------------
#pragma region Console

void UNiagaraTraceOcclusionCullingSubsystem::RegisterConsoleCommands()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Niagara.TraceOcclusionCulling.Enable"),
		TEXT("Turn off Niagara system instances the player can't see."),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& args)
		{
			if (IsValid(this) && args.Num() > 0)
			{
				bool previousState = this->IsEnabled();
				this->bEnabled = args[0].ToBool();
				if (this->IsEnabled() == previousState)
				{
					return;
				}

				IsLastPlayerViewLocationValid = false;
				if (this->IsEnabled() == true)
				{
					FlushComponentsNeedingUpdate();
				}
				else
				{
					RestoreStateAllComponents();
				}
			}
		})
	);

	IConsoleManager::Get().RegisterConsoleCommand(
	TEXT("Niagara.TraceOcclusionCulling.MaxTracesPerTick"),
	TEXT("Limit system to this many line traces per tick.  Higher values mean faster Niagara component updates, but more overhead."),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& args)
		{
			if (IsValid(this) && args.Num() > 0)
			{
				const int32 intValue = FCString::Atoi(*args[0]);
				MaxTracesPerTick = FMath::Clamp(intValue, 0, kMaxTracesPerTickLimit);
			}
		})
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Niagara.TraceOcclusionCulling.MaxTickRate"),
		TEXT("Maximum system update rate (FPS). This is the overhead of the optimization system itself."),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& args)
		{
			if (IsValid(this) && args.Num() > 0)
			{
				MaxTickRate = FMath::Clamp(FCString::Atof(*args[0]), 1.0f, kMaxTickRateInitial);
			}
		})
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Niagara.TraceOcclusionCulling.PlayerViewpointOffset"),
		TEXT("When testing occlusion, offset the player's location a little to account for character geometry, weapons, etc."),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& args)
		{
			if (IsValid(this) && args.Num() > 0)
			{
				PlayerViewpointOffset = FCString::Atof(*args[0]);
			}
		})
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Niagara.TraceOcclusionCulling.DrawDebugInfo"),
		TEXT("Display crosshairs at the start and end of occlusion line traces.  Note: this has a performance cost."),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& args)
		{
			if (IsValid(this) && args.Num() > 0)
			{
				ShouldDrawDebugInfo = args[0].ToBool();
			}
		})
	);
}

void UNiagaraTraceOcclusionCullingSubsystem::DrawDebugInfo(const UWorld* worldChecked)
{
	for (const ComponentPairType& element : Components)
	{
		if (element.Value.IsValid() == false)
		{
			return;
		}

		DrawDebugCrosshairs(
			worldChecked,
			element.Value->traceStart,
			FRotator::ZeroRotator,
			50.0,
			element.Value->isOccluded ? FColor::Red : FColor::Green
		);
	}
}

void UNiagaraTraceOcclusionCullingSubsystem::ResetOcclusionCullingDebug()
{
	for (ComponentPairType& element : Components)
	{
		if (element.Key.IsValid() == false)
		{
			continue;
		}

		UNiagaraComponent* niagaraComponent = element.Key.Get();
		niagaraComponent->SetVisibility(true);
		niagaraComponent->SetPaused(false);

		element.Value->traceStart = FVector::ZeroVector;
		element.Value->traceEnd = FVector::ZeroVector;
		element.Value->isOccluded = false;
	}

	LastPlayerViewLocation = FVector::ZeroVector;
}

void UNiagaraTraceOcclusionCullingSubsystem::FlushComponentsNeedingUpdate()
{
	while (ComponentsNeedingUpdate.IsEmpty() == false)
	{
		ComponentsNeedingUpdate.Dequeue();
	}
}

#pragma endregion
