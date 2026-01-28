// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/PostProcessComponent.h"
#include "SparksEnvironmentLightingPresetPropertiesPostProcess.generated.h"

USTRUCT(BlueprintType)
struct FSparksEnvironmentLightingPresetPropertiesPostProcess
{
	GENERATED_BODY()

	void ApplyPresetPropertiesToComponent(UPostProcessComponent& Component) const;

	// These only affect BlendWeight -- the Settings themselves are not weighted here.
	void AccumulateMultiplyAdd(const FSparksEnvironmentLightingPresetPropertiesPostProcess& Other, const float Multiplier);
	void ZeroOutProperties();

#pragma region Properties controlled by preset
	/** 0:no effect, 1:full effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = PostProcessVolume, meta = (UIMin = "0.0", UIMax = "1.0"))
	float BlendWeight = 1.0f;

	/**
      * Priority of this volume. In the case of overlapping volumes the one with the highest priority
      * overrides the lower priority ones. The order is undefined if two or more overlapping volumes have the same priority.
      */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PostProcessVolume)
	float Priority = 1.0f;;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = PostProcessVolume, meta = (ShowOnlyInnerProperties))
	FPostProcessSettings Settings;
#pragma endregion
};
