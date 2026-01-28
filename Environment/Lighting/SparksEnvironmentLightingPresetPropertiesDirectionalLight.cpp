// © 2024 Jar of Sparks, All Rights Reserved

#include "SparksEnvironmentLightingPresetPropertiesDirectionalLight.h"

#include "SparksEnvironmentLightingPresetPropertyOperators.h"
#include "Components/DirectionalLightComponent.h"

using namespace SparksEnvironmentLightingPresetPropertyOperators;

FSparksEnvironmentLightingPresetPropertiesDirectionalLight::FSparksEnvironmentLightingPresetPropertiesDirectionalLight()
{
	AtmosphereSunDiskColorScale = FLinearColor::White;
	BloomMaxBrightness = 100.0f;
	BloomScale = .2f;
	BloomThreshold = 0;
	BloomTint = FColor::White;
	CloudScatteredLuminanceScale = FLinearColor::White;
	CloudShadowDepthBias = 0.0f;
	CloudShadowExtent = 150.0f;
	CloudShadowMapResolutionScale = 1.0f;
	CloudShadowOnAtmosphereStrength = 1.0f;
	CloudShadowOnSurfaceStrength = 1.0f;
	CloudShadowRaySampleCountScale = 1.0f;
	CloudShadowStrength = 1.0f;
	IESBrightnessScale = 1.0f;
	IndirectLightingIntensity = 1.0f;
	Intensity = 10;
	LightColor = FColor::White;
	LightSourceAngle = 0.5357f;		// Angle of earth's sun
	LightSourceSoftAngle = 0.0f;
	RelativeRotation = FRotator::ZeroRotator;
	ShadowSourceAngleFactor = 1.0f;
	SpecularScale = 1.0f;
	Temperature = 6500.0f;
	TraceDistance = 10000.0f;
	VolumetricScatteringIntensity = 1.0f;
	bEnableLightShaftOcclusion = true;
	bEnableLightShaftBloom = true;
	OcclusionMaskDarkness = 0.05f;
}

void FSparksEnvironmentLightingPresetPropertiesDirectionalLight::ApplyPresetPropertiesToComponent(UDirectionalLightComponent& Component) const
{
	Component.AtmosphereSunDiskColorScale = AtmosphereSunDiskColorScale;
	Component.BloomMaxBrightness = BloomMaxBrightness;
	Component.BloomScale = BloomScale;
	Component.BloomThreshold = BloomThreshold;
	Component.BloomTint = BloomTint;
	Component.CloudScatteredLuminanceScale = CloudScatteredLuminanceScale;
	Component.CloudShadowDepthBias = CloudShadowDepthBias;
	Component.CloudShadowExtent = CloudShadowExtent;
	Component.CloudShadowMapResolutionScale = CloudShadowMapResolutionScale;
	Component.CloudShadowOnAtmosphereStrength = CloudShadowOnAtmosphereStrength;
	Component.CloudShadowOnSurfaceStrength = CloudShadowOnSurfaceStrength;
	Component.CloudShadowRaySampleCountScale = CloudShadowRaySampleCountScale;
	Component.CloudShadowStrength = CloudShadowStrength;
	Component.IESBrightnessScale = IESBrightnessScale;
	Component.IndirectLightingIntensity = IndirectLightingIntensity;
	Component.Intensity = Intensity;
	Component.LightColor = LightColor;
	Component.LightSourceAngle = LightSourceAngle;
	Component.LightSourceSoftAngle = LightSourceSoftAngle;
	Component.SetRelativeRotation(RelativeRotation);
	Component.ShadowSourceAngleFactor = ShadowSourceAngleFactor;
	Component.SpecularScale = SpecularScale;
	Component.Temperature = Temperature;
	Component.TraceDistance = TraceDistance;
	Component.VolumetricScatteringIntensity = VolumetricScatteringIntensity;
	Component.bEnableLightShaftOcclusion = bEnableLightShaftOcclusion;
	Component.bEnableLightShaftBloom = bEnableLightShaftBloom;
	Component.OcclusionMaskDarkness = OcclusionMaskDarkness;
}

void FSparksEnvironmentLightingPresetPropertiesDirectionalLight::AccumulateMultiplyAdd(
	const FSparksEnvironmentLightingPresetPropertiesDirectionalLight& Other, const float Multiplier)
{
	AtmosphereSunDiskColorScale += Other.AtmosphereSunDiskColorScale * Multiplier;
	BloomMaxBrightness += Other.BloomMaxBrightness * Multiplier;
	BloomScale += Other.BloomScale * Multiplier;
	BloomThreshold += Other.BloomThreshold * Multiplier;
	BloomTint += MultiplyByScalar(Other.BloomTint, Multiplier);
	CloudScatteredLuminanceScale += Other.CloudScatteredLuminanceScale * Multiplier;
	CloudShadowDepthBias += Other.CloudShadowDepthBias * Multiplier;
	CloudShadowExtent += Other.CloudShadowExtent * Multiplier;
	CloudShadowMapResolutionScale += Other.CloudShadowMapResolutionScale * Multiplier;
	CloudShadowOnAtmosphereStrength += Other.CloudShadowOnAtmosphereStrength * Multiplier;
	CloudShadowOnSurfaceStrength += Other.CloudShadowOnSurfaceStrength * Multiplier;
	CloudShadowRaySampleCountScale += Other.CloudShadowRaySampleCountScale * Multiplier;
	CloudShadowStrength += Other.CloudShadowStrength * Multiplier;
	IESBrightnessScale += Other.IESBrightnessScale * Multiplier;
	IndirectLightingIntensity += Other.IndirectLightingIntensity * Multiplier;
	Intensity += Other.Intensity * Multiplier;
	LightColor += MultiplyByScalar(Other.LightColor, Multiplier);
	LightSourceAngle += Other.LightSourceAngle * Multiplier;
	LightSourceSoftAngle += Other.LightSourceSoftAngle * Multiplier;
	RelativeRotation += Other.RelativeRotation * Multiplier;
	ShadowSourceAngleFactor += Other.ShadowSourceAngleFactor * Multiplier;
	SpecularScale += Other.SpecularScale * Multiplier;
	Temperature += Other.Temperature * Multiplier;
	TraceDistance += Other.TraceDistance * Multiplier;
	VolumetricScatteringIntensity += Other.VolumetricScatteringIntensity * Multiplier;
	bEnableLightShaftOcclusion = Other.bEnableLightShaftOcclusion;
	bEnableLightShaftBloom = Other.bEnableLightShaftBloom;
	OcclusionMaskDarkness = Other.OcclusionMaskDarkness;
}

void FSparksEnvironmentLightingPresetPropertiesDirectionalLight::ZeroOutProperties()
{
	AtmosphereSunDiskColorScale = LinearColorAllZeroes;
	BloomMaxBrightness = 0.0f;
	BloomScale = 0.0f;
	BloomThreshold = 0.0f;
	BloomTint = FColor::Black;
	CloudScatteredLuminanceScale = LinearColorAllZeroes;
	CloudShadowDepthBias = 0.0f;
	CloudShadowExtent = 0.0f;
	CloudShadowMapResolutionScale = 0.0f;
	CloudShadowOnAtmosphereStrength = 0.0f;
	CloudShadowOnSurfaceStrength = 0.0f;
	CloudShadowRaySampleCountScale = 0.0f;
	CloudShadowStrength = 0.0f;
	IESBrightnessScale = 0.0f;
	IndirectLightingIntensity = 0.0f;
	Intensity = 0.0f;
	LightColor = FColor::Black;
	LightSourceAngle = 0.0f;
	LightSourceSoftAngle = 0.0f;
	RelativeRotation = FRotator::ZeroRotator;
	ShadowSourceAngleFactor = 0.0f;
	SpecularScale = 0.0f;
	Temperature = 0.0f;
	TraceDistance = 0.0f;
	VolumetricScatteringIntensity = 0.0f;
	bEnableLightShaftOcclusion = false;
	bEnableLightShaftBloom = false;
	OcclusionMaskDarkness = 0.0f;
}