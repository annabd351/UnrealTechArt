// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SparksEnvironmentLightingPresetPropertiesSkyLight.generated.h"

USTRUCT(BlueprintType)
struct FSparksEnvironmentLightingPresetPropertiesSkyLight
{
	GENERATED_BODY()

	FSparksEnvironmentLightingPresetPropertiesSkyLight();

	void ApplyPresetPropertiesToComponent(USkyLightComponent& Component) const;
	void AccumulateMultiplyAdd(const FSparksEnvironmentLightingPresetPropertiesSkyLight& Other, const float Multiplier);
	void ZeroOutProperties();

#pragma region Properties controlled by preset
	/** When enabled, the sky will be captured and convolved to achieve dynamic diffuse and specular environment lighting.
	 * SkyAtmosphere, VolumetricCloud Components as well as sky domes with Sky materials are taken into account. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Light)
	bool bRealTimeCapture;

	/**
	 * Distance from the sky light at which any geometry should be treated as part of the sky.
	 * This is also used by reflection captures, so update reflection captures to see the impact.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Light)
	float SkyDistanceThreshold;

	/**
	 * Total energy that the light emits.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category=Light, meta=(DisplayName = "Intensity", UIMin = "0.0", UIMax = "20.0", ShouldShowInViewport = true))
	float Intensity;

	/**
	 * Filter color of the light.
	 * Note that this can change the light's effective intensity.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category=Light, meta=(HideAlphaChannel, ShouldShowInViewport = true))
	FColor LightColor;

	/**
	 * Scales the indirect lighting contribution from this light.
	 * A value of 0 disables any GI from this light. Default is 1.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category=Light, meta=(UIMin = "0.0", UIMax = "6.0"))
	float IndirectLightingIntensity;

	/** Intensity of the volumetric scattering from this light.  This scales Intensity and LightColor. */
	UPROPERTY(BlueprintReadOnly, interp, Category=Light, meta=(UIMin = "0.25", UIMax = "4.0"))
	float VolumetricScatteringIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Light, AdvancedDisplay)
	FLinearColor LowerHemisphereColor;

	/**
	 * Max distance that the occlusion of one point will affect another.
	 * Higher values increase the cost of Distance Field AO exponentially.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=DistanceFieldAmbientOcclusion, meta=(UIMin = "200", UIMax = "1500"))
	float OcclusionMaxDistance;

	/**
	 * Contrast S-curve applied to the computed AO.  A value of 0 means no contrast increase, 1 is a significant contrast increase.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=DistanceFieldAmbientOcclusion, meta=(UIMin = "0", UIMax = "1", DisplayName = "Occlusion Contrast"))
	float Contrast;

	/**
	 * Exponent applied to the computed AO.  Values lower than 1 brighten occlusion overall without losing contact shadows.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=DistanceFieldAmbientOcclusion, meta=(UIMin = ".6", UIMax = "1.6"))
	float OcclusionExponent;

	/**
	 * Controls the darkest that a fully occluded area can get.  This tends to destroy contact shadows, use Contrast or OcclusionExponent instead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=DistanceFieldAmbientOcclusion, meta=(UIMin = "0", UIMax = "1"))
	float MinOcclusion;

	/** Tint color on occluded areas, artistic control. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=DistanceFieldAmbientOcclusion)
	FColor OcclusionTint;

	/**
     * Whether the cloud should occlude sky contribution within the atmosphere (progressively fading multiple scattering out) or not.
     */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AtmosphereAndCloud)
	uint32 bCloudAmbientOcclusion : 1;

	/**
     * The world space radius of the cloud ambient occlusion map around the camera in kilometers.
     */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AtmosphereAndCloud, meta = (UIMin = "1", ClampMin = "1"))
	float CloudAmbientOcclusionExtent;
#pragma endregion

};
