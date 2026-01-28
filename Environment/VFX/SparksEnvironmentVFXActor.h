// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "NiagaraActor.h"
#include "Niagara/Public/NiagaraActor.h"
#include "SparksEnvironmentVFXActor.generated.h"

class UBoxComponent;
class UNiagaraComponent;

// Wrapper for persistent, environmental VFX manually placed in levels.

UENUM(BlueprintType, Category = "Environment | VFX")
enum class ESparksEnvironmentVFXLifecycleType : uint8
{
	// Effect is always active; loops continuously without pauses.
	Continuous,

	// Effect activates, plays, deactivates periodically at random intervals.
	Intermittent
};

UENUM(BlueprintType, Category = "Environment | VFX")
enum class ESparksEnvironmentVFXPlacementPolicyType : uint8
{
	// Effect is randomly placed within the volume once, and remains there on each activation.
	Once,

	// Effect is randomly placed at a different position within the volume on each activation.
	PerActivation
};

UCLASS(Blueprintable)
class SPARKSRUNTIME_API ASparksEnvironmentVFXActor : public ANiagaraActor
{
	GENERATED_BODY()

	ASparksEnvironmentVFXActor(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, Category = "Activation/Placement")
	ESparksEnvironmentVFXLifecycleType Lifecycle = ESparksEnvironmentVFXLifecycleType::Continuous;

	// Range of delay between activations.
	UPROPERTY(EditAnywhere, Category = "Activation/Placement",
		meta=(EditCondition = "Lifecycle == ESparksEnvironmentVFXLifecycleType::Intermittent", EditConditionHides))
	float ActivationDelayMin = 0.0f;

	// Range of delay between activations.
	UPROPERTY(EditAnywhere, Category = "Activation/Placement",
		meta=(EditCondition = "Lifecycle == ESparksEnvironmentVFXLifecycleType::Intermittent", EditConditionHides))
	float ActivationDelayMax = 1.0f;

	// Time for which an intermittent effect should be played, then deactivated.
	UPROPERTY(EditAnywhere, Category = "Activation/Placement",
		meta=(EditCondition = "Lifecycle == ESparksEnvironmentVFXLifecycleType::Intermittent", EditConditionHides))
	float Lifespan = 2.0f;

	// Orient forward vector of effect within a cone defined this angle (in degrees).
	UPROPERTY(EditAnywhere, Category = "Activation/Placement")
	float OrientationOffset = 0.0f;

	// If enabled, position effect at a random position within the placement box.
	UPROPERTY(EditAnywhere, Category = "Activation/Placement")
	bool bUsePlacementBox = false;

	UPROPERTY(EditAnywhere, Category = "Activation/Placement",
		meta=(EditCondition = "Lifecycle == ESparksEnvironmentVFXLifecycleType::Intermittent && bUsePlacementBox",
			EditConditionHides))
	ESparksEnvironmentVFXPlacementPolicyType PlacementPolicy = ESparksEnvironmentVFXPlacementPolicyType::Once;

	// Resize to define placement volume.
	TObjectPtr<UBoxComponent> PlacementVolume = nullptr;

	// If enabled, snap effect to surfaces intersecting the placement volume.
	UPROPERTY(EditAnywhere, Category = "Activation/Placement",
		meta=(EditCondition = "bUsePlacementBox", EditConditionHides))
	bool bSnapToSurfaces = false;

	// Enable to set a custom collision profile in for the Surface Collision Profile property; disable to keep default.
	UPROPERTY(EditAnywhere, Category = "Activation/Placement",
		meta=(EditCondition = "bSnapToSurfaces && bUsePlacementBox", EditConditionHides))
	bool bUseCustomCollisionProfile = false;

	// Collision profile of surfaces to check.
	UPROPERTY(EditAnywhere, Category = "Activation/Placement",
		meta=(EditCondition = "bUseCustomCollisionProfile && bSnapToSurfaces && bUsePlacementBox", EditConditionHides))
	FName SurfaceCollisionProfile = UCollisionProfile::BlockAll_ProfileName;

	virtual void PostRegisterAllComponents() override;

private:
	FTimerHandle PhaseTimer;
	bool bWasPreviouslyPlaced = false;

	void ResetEffect();
	void PlaceEffect(UNiagaraComponent* NiagaraCompChecked, const UWorld* WorldChecked);
	void MovePivot(UNiagaraComponent* NiagaraCompChecked, const UWorld* WorldChecked);
	void StartActivationTimer(const UWorld* WorldChecked);
	bool FindSnapLocation(FHitResult& HitResult, const FVector& InitialLocation, const EAxis::Type& Axis, const UWorld* WorldChecked) const;
	void ActivateEffect();
	void DeactivateEffect();
};
