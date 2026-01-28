// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SparksRuntime/RichFX/SparksRichFXWorker.h"
#include "NiagaraSystem.h"
#include "SparksRichFXNiagaraWorker.generated.h"

class UNiagaraSystem;

USTRUCT()
struct SPARKSRUNTIME_API FSparksRichFXNiagaraWorkerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

	bool HasValidProperties() const
	{
		return IsValid(NiagaraSystem);
	}
};

UCLASS(BlueprintType)
class SPARKSRUNTIME_API USparksRichFXNiagaraWorker : public USparksRichFXWorker
{
	GENERATED_BODY()

public:

	static void ActivateStatic(const UWorld& World, const FSparksRichFXNiagaraWorkerData& ValidData, const FHitResult& ValidHitResult)
	{
		SpawnInWorld(World, ValidData, ValidHitResult.ImpactPoint, (-ValidHitResult.ImpactNormal).Rotation());
	}

private:
	static void SpawnInWorld(const UWorld& World, const FSparksRichFXNiagaraWorkerData& ValidData, const FVector& Location, const FRotator& Rotation);
};
