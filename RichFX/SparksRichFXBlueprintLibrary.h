// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SparksRuntime/RichFX/SparksRichFXWorkerPool.h"
#include "SparksRuntime/RichFX/Visual/SparksRichFXDecalWorker.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SparksRichFXBlueprintLibrary.generated.h"

struct FSparksRichFXPostProcessWorkerData;
class USparksRichFXPostProcessWorker;
class USparksRichFXDecalWorker;
struct FSparksRichFXDecalWorkerData;

UCLASS()
class SPARKSRUNTIME_API USparksRichFXBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#pragma region Decals

	// Spawn a short-lived decal
	UFUNCTION(BlueprintCallable, Category = "RichFX")
	static void SpawnFXDecalOneShot(
		const UObject* WorldContextObject,
		const FVector& Location,
		const FVector& Normal,
		const FSparksRichFXDecalWorkerData& Settings,
		const AActor* AttachToActor,
		const FName Socket = NAME_None
	)
	{
		SpawnFXDecalCommon(WorldContextObject, Location, Normal, Settings, AttachToActor, Socket, ESparksRichFXWorkerExecutionState::Immediate);
	};

	// Spawn a decal.  Remains in world until despawned. Animates FadeOn and EmissiveFade parameters. Ignores FadeOff settings. CHECK VALIDITY -- CAN BE INVALID.
	// Pass in attachment actor, or leave empty to spawn in world.
	UFUNCTION(BlueprintCallable, Category = "RichFX")
	static USparksRichFXDecalWorker* SpawnFXDecalConstant(
		const UObject* WorldContextObject,
		const FVector& Location,
		const FVector& Normal,
		const FSparksRichFXDecalWorkerData& Settings,
		const AActor* AttachToActor = nullptr,
		const FName Socket = NAME_None
	)
	{
		return SpawnFXDecalCommon(WorldContextObject, Location, Normal, Settings, AttachToActor, Socket, ESparksRichFXWorkerExecutionState::LatentInitial);
	};

	// Remove a previously spawned decal. Animates FadeOff parameters. Ignores FadeOn and EmissiveFade parameters.
	UFUNCTION(BlueprintCallable, Category = "RichFX")
	static void DespawnFXDecalConstant(USparksRichFXDecalWorker* Worker);

#pragma endregion

#pragma region Post Process

	// Start a short-lived post process using a blend curve. Removed at end of Duration.
	UFUNCTION(BlueprintCallable, Category = "RichFX")
	static void SpawnFXPostProcessOneShot(
		const UObject* WorldContextObject,
		const FSparksRichFXPostProcessWorkerData& Properties
	)
	{
		SpawnFXPostCommon(WorldContextObject, Properties, ESparksRichFXWorkerExecutionState::Immediate);
	};

	// Start a continuing post process. Check returned value, and save it for use in ending the post process.
	UFUNCTION(BlueprintCallable, Category = "RichFX")
	static USparksRichFXPostProcessWorker* SpawnFXPostProcessConstant(
		const UObject* WorldContextObject,
		const FSparksRichFXPostProcessWorkerData& Properties
	)
	{
		return SpawnFXPostCommon(WorldContextObject, Properties, ESparksRichFXWorkerExecutionState::LatentInitial);
	};

	// Pass in previously returned worker, fade off post process.
	UFUNCTION(BlueprintCallable, Category = "RichFX")
	static void DespawnFXPostProcessConstant(USparksRichFXPostProcessWorker* Worker);

	// Get current settings from post VFX worker. Only valid if return value is true!!
	UFUNCTION(BlueprintCallable, Category = "RichFX")
	static bool GetFXPostProcessSettings(const USparksRichFXPostProcessWorker* Worker, FPostProcessSettings& Settings);

	// Update current settings on a post VFX worker. Result indicates success/fail.
	UFUNCTION(BlueprintCallable, Category = "RichFX")
	static bool SetFXPostProcessSettings(const USparksRichFXPostProcessWorker* Worker, const FPostProcessSettings& Settings);

#pragma endregion

private:
	static USparksRichFXDecalWorker* SpawnFXDecalCommon(const UObject* WorldContextObject,
	                                                const FVector& Location,
	                                                const FVector& Normal,
	                                                const FSparksRichFXDecalWorkerData& Properties,
	                                                const AActor* AttachToActor,
	                                                const FName Socket,
	                                                const ESparksRichFXWorkerExecutionState Mode);

	static USparksRichFXPostProcessWorker* SpawnFXPostCommon(const UObject* WorldContextObject,
	                                                     const FSparksRichFXPostProcessWorkerData& Properties,
	                                                     const ESparksRichFXWorkerExecutionState Mode);

	template <typename WorkerType>
	static WorkerType* SpawnFXWorker(const UObject* WorldContextObject)
	{
		const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
		ASparksRichFXWorkerPoolActor* WorkerPool = World ? USparksRichFXWorkerPool::GetWorldFXWorkerPool(*World) : nullptr;
		return WorkerPool ? ExactCast<WorkerType>(WorkerPool->AllocateFXWorker(WorkerType::StaticClass())) : nullptr;
	}
};
