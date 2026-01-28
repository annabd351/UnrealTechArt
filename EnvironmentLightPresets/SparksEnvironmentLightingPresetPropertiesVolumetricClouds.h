// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SparksEnvironmentLightingPresetPropertiesVolumetricClouds.generated.h"

class UVolumetricCloudComponent;

USTRUCT(BlueprintType)
struct FSparksEnvironmentLightingPresetPropertiesVolumetricClouds
{
	GENERATED_BODY()

	FSparksEnvironmentLightingPresetPropertiesVolumetricClouds();

	void ApplyPresetPropertiesToComponent(UVolumetricCloudComponent& Component) const;
	void AccumulateMultiplyAdd(const FSparksEnvironmentLightingPresetPropertiesVolumetricClouds& Other, const float Multiplier);
	void ZeroOutProperties();

#pragma region Properties controlled by preset
	/** The altitude at which the cloud layer starts. (kilometers above the ground) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Layer", meta = (UIMin = 0.0f, UIMax = 20.0f, SliderExponent = 2.0))
	float LayerBottomAltitude;

	/** The height of the the cloud layer. (kilometers above the layer bottom altitude) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Layer", meta = (UIMin = 0.1f, UIMax = 20.0f, ClampMin = 0.1, SliderExponent = 2.0))
	float LayerHeight;

	/** The maximum distance of the volumetric surface, i.e. cloud layer upper and lower bound, before which we will accept to start tracing. (kilometers) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Layer", meta = (UIMin = 100.0f, UIMax = 500.0f, ClampMin = 1.0f, SliderExponent = 2.0))
	float TracingStartMaxDistance;

	/**
	 * The ground albedo used to light the cloud from below with respect to the sun light and sky atmosphere.
	 * This is only used by the cloud material when the 'Volumetric Advanced' node have GroundContribution enabled.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Planet", meta = (HideAlphaChannel))
	FColor GroundAlbedo;

	/** Specify the aerial perspective start distance on cloud for Rayleigh scattering only. (kilometers) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Art Direction", meta = (UIMin = 0.0, UIMax = 100.0, ClampMin = 0.0, SliderExponent = 2.0))
	float AerialPespectiveRayleighScatteringStartDistance;

	/** Specify the distance over which the Rayleigh scattering will linearly ramp up to full effect. (kilometers) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Art Direction", meta = (UIMin = 0.0, UIMax = 100.0, ClampMin = 0.0, SliderExponent = 2.0))
	float AerialPespectiveRayleighScatteringFadeDistance;

	/** Specify the aerial perspective start distance on cloud for Mie scattering only. (kilometers) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Art Direction", meta = (UIMin = 0.0, UIMax = 100.0, ClampMin = 0.0, SliderExponent = 2.0))
	float AerialPespectiveMieScatteringStartDistance;

	/** Specify the distance over which the Rayleigh scattering will linearly ramp up to full effect. (kilometers) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Art Direction", meta = (UIMin = 0.0, UIMax = 100.0, ClampMin = 0.0, SliderExponent = 2.0))
	float AerialPespectiveMieScatteringFadeDistance;
#pragma endregion

};
