// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "UObject/Object.h"
#include "SparksEnvironmentLightingPresetPropertiesExponentialHeightFog.generated.h"

USTRUCT(BlueprintType)
struct FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog
{
	GENERATED_BODY()

	FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog();

	void ApplyPresetPropertiesToComponent(UExponentialHeightFogComponent& Component) const;
	void AccumulateMultiplyAdd(const FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog& Other, const float Multiplier);
	void ZeroOutProperties();

#pragma region Properties controlled by preset

	/** Global density factor. */
	UPROPERTY(BlueprintReadOnly, interp, Category=ExponentialHeightFogComponent, meta=(UIMin = "0", UIMax = ".05"))
	float FogDensity;

	/**
	 * Height density factor, controls how the density increases as height decreases.
	 * Smaller values make the visible transition larger.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category = ExponentialHeightFogComponent, meta = (UIMin = "0.001", UIMax = "2"))
	float FogHeightFalloff;

	/** Settings for the second fog. Setting the density of this to 0 means it doesn't have any influence. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=ExponentialHeightFogComponent)
	FExponentialHeightFogData SecondFogData;

	UPROPERTY(BlueprintReadOnly, interp, Category=ExponentialHeightFogComponent, meta = (DisplayName = "Fog Inscattering Color"))
	FLinearColor FogInscatteringLuminance;

	/** Color used to modulate the SkyAtmosphere component contribution to the non directional component of the fog. Only effective when r.SupportSkyAtmosphereAffectsHeightFog>0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ExponentialHeightFogComponent)
	FLinearColor SkyAtmosphereAmbientContributionColorScale;

	/**
	 * Maximum opacity of the fog.
	 * A value of 1 means the fog can become fully opaque at a distance and replace scene color completely,
	 * A value of 0 means the fog color will not be factored in at all.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category=ExponentialHeightFogComponent, meta=(UIMin = "0", UIMax = "1"))
	float FogMaxOpacity;

	/** Distance from the camera that the fog will start, in world units. */
	UPROPERTY(BlueprintReadOnly, interp, Category=ExponentialHeightFogComponent, meta=(UIMin = "0", UIMax = "5000"))
	float StartDistance;

	/** Scene elements past this distance will not have fog applied.  This is useful for excluding skyboxes which already have fog baked in. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=ExponentialHeightFogComponent, meta=(UIMin = "100000", UIMax = "20000000"))
	float FogCutoffDistance;


	/** Tint color used when InscatteringColorCubemap is specified, for quick edits without having to reimport InscatteringColorCubemap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=InscatteringTexture)
	FLinearColor InscatteringTextureTint;

	/** Distance at which InscatteringColorCubemap should be used directly for the Inscattering Color. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=InscatteringTexture, meta=(UIMin = "1000", UIMax = "1000000"))
	float FullyDirectionalInscatteringColorDistance;

	/** Distance at which only the average color of InscatteringColorCubemap should be used as Inscattering Color. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=InscatteringTexture, meta=(UIMin = "1000", UIMax = "1000000"))
	float NonDirectionalInscatteringColorDistance;

	/**
	 * Controls the size of the directional inscattering cone, which is used to approximate inscattering from a directional light.
	 * Note: there must be a directional light with bUsedAsAtmosphereSunLight enabled for DirectionalInscattering to be used.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category=DirectionalInscattering, meta=(UIMin = "2", UIMax = "64"))
	float DirectionalInscatteringExponent;

	/**
	 * Controls the start distance from the viewer of the directional inscattering, which is used to approximate inscattering from a directional light.
	 * Note: there must be a directional light with bUsedAsAtmosphereSunLight enabled for DirectionalInscattering to be used.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category=DirectionalInscattering)
	float DirectionalInscatteringStartDistance;

	/**
	 * Controls the color of the directional inscattering, which is used to approximate inscattering from a directional light.
	 * Note: there must be a directional light with bUsedAsAtmosphereSunLight enabled for DirectionalInscattering to be used.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category=DirectionalInscattering, meta = (DisplayName = "Directional Inscattering Color"))
	FLinearColor DirectionalInscatteringLuminance;

	/**
	 * Whether to enable Volumetric fog.  Scalability settings control the resolution of the fog simulation.
	 * Note that Volumetric fog currently does not support StartDistance, FogMaxOpacity and FogCutoffDistance.
	 * Volumetric fog also can't match exponential height fog in general as exponential height fog has non-physical behavior.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumetricFog, meta=(DisplayName = "Volumetric Fog"))
	bool bEnableVolumetricFog;

	/**
	 * Controls the scattering phase function - how much incoming light scatters in various directions.
	 * A distribution value of 0 scatters equally in all directions, while .9 scatters predominantly in the light direction.
	 * In order to have visible volumetric fog light shafts from the side, the distribution will need to be closer to 0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumetricFog, meta=(DisplayName = "Scattering Distribution", UIMin = "-.9", UIMax = ".9"))
	float VolumetricFogScatteringDistribution;

	/**
	 * The height fog particle reflectiveness used by volumetric fog.
	 * Water particles in air have an albedo near white, while dust has slightly darker value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumetricFog, meta=(DisplayName = "Albedo"))
	FColor VolumetricFogAlbedo;

	/**
	 * Light emitted by height fog.  This is a density so more light is emitted the further you are looking through the fog.
	 * In most cases skylight is a better choice, however right now volumetric fog does not support precomputed lighting,
	 * So stationary skylights are unshadowed and static skylights don't affect volumetric fog at all.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumetricFog, meta=(DisplayName = "Emissive"))
	FLinearColor VolumetricFogEmissive;

	/** Scales the height fog particle extinction amount used by volumetric fog.  Values larger than 1 cause fog particles everywhere absorb more light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumetricFog, meta=(DisplayName = "Extinction Scale", UIMin = ".1", UIMax = "10"))
	float VolumetricFogExtinctionScale;

	/**
	 * Distance over which volumetric fog should be computed, after the start distance.  Larger values extend the effect into the distance but expose under-sampling artifacts in details.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumetricFog, meta=(DisplayName = "View Distance", UIMin = "1000", UIMax = "10000"))
	float VolumetricFogDistance;

	/**
	 * Distance from the camera that the volumetric fog will start, in world units.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category= VolumetricFog, meta=(DisplayName = "Start Distance", UIMin = "0", UIMax = "5000"))
	float VolumetricFogStartDistance;

	/**
	 * Distance over which volumetric fog will fade in from the start distance.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumetricFog, meta=(DisplayName = "Near Fade In Distance", UIMin = "0", UIMax = "1000"))
	float VolumetricFogNearFadeInDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = VolumetricFog, meta=(DisplayName = "Static Lighting Scattering Intensity", UIMin = "0", UIMax = "10"))
	float VolumetricFogStaticLightingScatteringIntensity;
#pragma endregion

};
