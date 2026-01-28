// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SparksEnvironmentLightingPresetPropertiesDirectionalLight.generated.h"

USTRUCT(BlueprintType)
struct SPARKSRUNTIME_API FSparksEnvironmentLightingPresetPropertiesDirectionalLight
{
	GENERATED_BODY()

	FSparksEnvironmentLightingPresetPropertiesDirectionalLight();

	void ApplyPresetPropertiesToComponent(UDirectionalLightComponent& Component) const;
	void AccumulateMultiplyAdd(const FSparksEnvironmentLightingPresetPropertiesDirectionalLight& Other, const float Multiplier);
	void ZeroOutProperties();

#pragma region Properties controlled by preset
	// Copied from UDirectionalLightComponent, ULightComponent, and ULightComponentBase

	/** Rotation of the component relative to its parent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Transform, meta=(AllowPrivateAccess="true", LinearDeltaSensitivity = "1", Delta = "1.0"))
	FRotator RelativeRotation;

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
	 * Angle subtended by light source in degrees (also known as angular diameter).
	 * Defaults to 0.5357 which is the angle for our sun.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category=Light, meta=(UIMin = "0", UIMax = "5"), DisplayName = "Source Angle")
	float LightSourceAngle;

	/**
	 * Angle subtended by soft light source in degrees.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category=Light, meta=(UIMin = "0", UIMax = "5"), DisplayName = "Source Soft Angle")
	float LightSourceSoftAngle;

	/**
	* Color temperature in Kelvin of the blackbody illuminant.
	* White (D65) is 6500K.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = Light, meta = (UIMin = "1700.0", UIMax = "12000.0", ShouldShowInViewport = true))
	float Temperature;

	/**
	 * Scales the indirect lighting contribution from this light.
	 * A value of 0 disables any GI from this light. Default is 1.
	 */
	UPROPERTY(BlueprintReadOnly, interp, Category=Light, meta=(UIMin = "0.0", UIMax = "6.0"))
	float IndirectLightingIntensity;

	/** Intensity of the volumetric scattering from this light.  This scales Intensity and LightColor. */
	UPROPERTY(BlueprintReadOnly, interp, Category=Light, meta=(UIMin = "0.25", UIMax = "4.0"))
	float VolumetricScatteringIntensity;

	/** Whether to occlude fog and atmosphere inscattering with screenspace blurred occlusion from this light. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category=LightShafts, meta=(DisplayName = "Light Shaft Occlusion"))
	uint32 bEnableLightShaftOcclusion:1;

	/**
	 * Controls how dark the occlusion masking is, a value of 1 results in no darkening term.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category=LightShafts, meta=(UIMin = "0", UIMax = "1"))
	float OcclusionMaskDarkness;

	/**
     * Whether to render light shaft bloom from this light.
     * For directional lights, the color around the light direction will be blurred radially and added back to the scene.
     * for point lights, the color on pixels closer than the light's SourceRadius will be blurred radially and added back to the scene.
     */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=LightShafts, meta=(DisplayName = "Light Shaft Bloom"))
	uint32 bEnableLightShaftBloom : 1;

	/** Scales the additive color. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=LightShafts, meta=(UIMin = "0", UIMax = "10"))
	float BloomScale;

	/** Scene color must be larger than this to create bloom in the light shafts. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=LightShafts, meta=(UIMin = "0", UIMax = "4"))
	float BloomThreshold;

	/** After exposure is applied, scene color brightness larger than BloomMaxBrightness will be rescaled down to BloomMaxBrightness. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=LightShafts, meta=(UIMin = "0", UIMax = "100", SliderExponent = 20.0))
	float BloomMaxBrightness;

	/** Multiplies against scene color to create the bloom color. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=LightShafts)
	FColor BloomTint;

	/**
	 * Scales the lights contribution when scattered in cloud participating media. This can help counter balance the fact that our multiple scattering solution is only an approximation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = AtmosphereAndCloud, meta = (HideAlphaChannel))
	FLinearColor CloudScatteredLuminanceScale;

	/**
	 * A color multiplied with the sun disk luminance.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AtmosphereAndCloud, AdvancedDisplay, meta = (DisplayName = "Atmosphere Sun Disk Color Scale"))
	FLinearColor AtmosphereSunDiskColorScale;

	/**
	 * The overall strength of the cloud shadow, higher value will block more light.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = AtmosphereAndCloud, AdvancedDisplay, meta = (UIMin = "0", UIMax = "1", ClampMin = "0", SliderExponent = 1.0))
	float CloudShadowStrength;
	/**
	 * The strength of the shadow on atmosphere. Disabled when 0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = AtmosphereAndCloud, AdvancedDisplay, meta = (UIMin = "0", UIMax = "1", ClampMin = "0", SliderExponent = 1.0))
	float CloudShadowOnAtmosphereStrength;
	/**
	 * The strength of the shadow on opaque and transparent meshes. Disabled when 0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, interp, Category = AtmosphereAndCloud, AdvancedDisplay, meta = (UIMin = "0", UIMax = "1", ClampMin = "0", SliderExponent = 1.0))
	float CloudShadowOnSurfaceStrength;
	/**
	 * The bias applied to the shadow front depth of the volumetric cloud shadow map.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AtmosphereAndCloud, AdvancedDisplay, meta = (UIMin = "-1", UIMax = "1"))
	float CloudShadowDepthBias;
	/**
	 * The world space radius of the cloud shadow map around the camera in kilometers.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AtmosphereAndCloud, AdvancedDisplay, meta = (UIMin = "1", ClampMin = "1"))
	float CloudShadowExtent;
	/**
	 * Scale the cloud shadow map resolution, base resolution is 512. The resolution is still clamped to 'r.VolumetricCloud.ShadowMap.MaxResolution'.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AtmosphereAndCloud, AdvancedDisplay, meta = (UIMin = "0.25", UIMax = "8", ClampMin = "0.25", SliderExponent = 1.0))
	float CloudShadowMapResolutionScale;
	/**
	 * Scale the shadow map tracing sample count.
	 * The sample count resolution is still clamped according to scalability setting to 'r.VolumetricCloud.ShadowMap.RaySampleMaxCount'.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = AtmosphereAndCloud, AdvancedDisplay, meta = (UIMin = "0.25", UIMax = "8", ClampMin = "0.25", SliderExponent = 1.0))
	float CloudShadowRaySampleCountScale;

#pragma endregion

#pragma region Unused Properties
	// Not needed, but kept for compatibility with prior versions

	/**
      * Multiplier on specular highlights. Use only with great care! Any value besides 1 is not physical!
	  * Can be used to artistically remove highlights mimicking polarizing filters or photo touch up.
	  */
	UPROPERTY()
	float SpecularScale;

	/** Determines how far shadows can be cast, in world units.  Larger values increase the shadowing cost. */
	UPROPERTY()
	float TraceDistance;

	/** Global scale for IES brightness contribution. Only available when "Use IES Brightness" is selected, and a valid IES profile texture is set */
	UPROPERTY()
	float IESBrightnessScale;

	/**
	 * Shadow source angle factor, relative to the light source angle.
	 * Defaults to 1.0 to coincide with light source angle.
	 */
	UPROPERTY()
	float ShadowSourceAngleFactor;
#pragma endregion

};
