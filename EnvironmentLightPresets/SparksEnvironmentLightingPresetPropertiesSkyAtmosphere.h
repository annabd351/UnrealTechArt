// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SparksEnvironmentLightingPresetPropertiesSkyAtmosphere.generated.h"

USTRUCT(BlueprintType)
struct FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere
{
	GENERATED_BODY()

	FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere();

	void ApplyPresetPropertiesToComponent(USkyAtmosphereComponent& Component) const;
	void AccumulateMultiplyAdd(const FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere& Other, const float Multiplier);
	void ZeroOutProperties();

#pragma region Properties controlled by preset
	/** The radius in kilometers from the center of the planet to the ground level. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Planet", meta = (DisplayName = "Ground Radius", UIMin = 1.0, UIMax = 7000.0, ClampMin = 0.1, ClampMax = 10000.0, SliderExponent = 5.0))
	float BottomRadius;

	/** The ground albedo that will tint the atmosphere when the sun light will bounce on it. Only taken into account when MultiScattering>0.0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Planet", meta = (HideAlphaChannel))
	FColor GroundAlbedo;

	/** The height of the atmosphere layer above the ground in kilometers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere", meta = (UIMin = 1.0, UIMax = 200.0, ClampMin = 0.1, SliderExponent = 2.0))
	float AtmosphereHeight;

	/** Factor applied to multiple scattering only (after the sun light has bounced around in the atmosphere at least once).
	 * Multiple scattering is evaluated using a dual scattering approach.
	 * A value of 2 is recommended to better represent default atmosphere when r.SkyAtmosphere.MultiScatteringLUT.HighQuality=0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere", meta = (DisplayName = "MultiScattering", UIMin = 0.0, UIMax = 2.0, ClampMin = 0.0, ClampMax = 100.0))
	float MultiScatteringFactor;

	/** Rayleigh scattering coefficient scale.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Rayleigh", meta = (UIMin = 0.0, UIMax = 2.0, ClampMin = 0.0, SliderExponent = 4.0))
	float RayleighScatteringScale;

	/** The Rayleigh scattering coefficients resulting from molecules in the air at an altitude of 0 kilometer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Rayleigh", meta=(HideAlphaChannel))
	FLinearColor RayleighScattering;

	/** The altitude in kilometer at which Rayleigh scattering effect is reduced to 40%.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Rayleigh", meta = (UIMin = 0.01, UIMax = 20.0, ClampMin = 0.001, SliderExponent = 5.0))
	float RayleighExponentialDistribution;

	/** Mie scattering coefficient scale.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Mie", meta = (UIMin = 0.0, UIMax = 5.0, ClampMin = 0.0, SliderExponent = 4.0))
	float MieScatteringScale;

	/** The Mie scattering coefficients resulting from particles in the air at an altitude of 0 kilometer. As it becomes higher, light will be scattered more. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Mie", meta = (HideAlphaChannel))
	FLinearColor MieScattering;

	/** Mie absorption coefficient scale.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Mie", meta = (UIMin = 0.0, UIMax = 5.0, ClampMin = 0.0, SliderExponent = 4.0))
	float MieAbsorptionScale;

	/** The Mie absorption coefficients resulting from particles in the air at an altitude of 0 kilometer. As it becomes higher, light will be absorbed more. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Mie", meta = (HideAlphaChannel))
	FLinearColor MieAbsorption;

	/** A value of 0 mean light is uniformly scattered. A value closer to 1 means lights will scatter more forward, resulting in halos around light sources. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Mie", meta = (UIMin = 0.0, UIMax = 0.999, ClampMin = 0.0, ClampMax = 0.999))
	float MieAnisotropy;

	/** The altitude in kilometer at which Mie effects are reduced to 40%.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Mie", meta = (UIMin = 0.01, UIMax = 10.0, ClampMin = 0.001, SliderExponent = 5.0))
	float MieExponentialDistribution;

	/** Absorption coefficients for another atmosphere layer. Density increase from 0 to 1 between 10 to 25km and decreases from 1 to 0 between 25 to 40km. This approximates ozone molecules distribution in the Earth atmosphere. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Absorption", meta = (DisplayName = "Absorption Scale", UIMin = 0.0, UIMax = 0.2, ClampMin = 0.0, SliderExponent = 3.0))
	float OtherAbsorptionScale;

	/** Absorption coefficients for another atmosphere layer. Density increase from 0 to 1 between 10 to 25km and decreases from 1 to 0 between 25 to 40km. The default values represents ozone molecules absorption in the Earth atmosphere. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Atmosphere - Absorption", meta = (DisplayName = "Absorption", HideAlphaChannel))
	FLinearColor OtherAbsorption;

	/** Scales the luminance of pixels representing the sky. This will impact the captured sky light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Art Direction", meta = (HideAlphaChannel))
	FLinearColor SkyLuminanceFactor;

	/** Makes the aerial perspective look thicker by scaling distances from view to surfaces (opaque and translucent). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Art Direction", meta = (DisplayName = "Aerial Perspective View Distance Scale", UIMin = 0.0, UIMax = 3.0, ClampMin = 0.0, SliderExponent = 2.0))
	float AerialPespectiveViewDistanceScale;

	/** Scale the sky and atmosphere lights contribution to the height fog when SupportSkyAtmosphereAffectsHeightFog project setting is true.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Art Direction", meta = (UIMin = 0.0, UIMax = 1.0, ClampMin = 0.0, SliderExponent = 2.0))
	float HeightFogContribution;

	/** The minimum elevation angle in degree that should be used to evaluate the sun transmittance to the ground. Useful to maintain a visible sun light and shadow on meshes even when the sun has started going below the horizon. This does not affect the aerial perspective.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Art Direction", meta = (UIMin = -90.0, UIMax = 90.0, ClampMin = -90.0f, ClampMax = 90.0f))
	float TransmittanceMinLightElevationAngle;

	/** The distance (kilometers) at which we start evaluating the aerial perspective. Having the aerial perspective starts away from the camera can help with performance: pixels not affected by the aerial perspective will have their computation skipped using early depth test.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = "Art Direction", meta = (UIMin = 0.001f, UIMax = 10.0f, ClampMin = 0.001f))
	float AerialPerspectiveStartDepth;
#pragma endregion

};
