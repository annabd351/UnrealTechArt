// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksEnvironmentLightingPresetPropertiesSkyLight.h"

#include "SparksEnvironmentLightingPresetPropertyOperators.h"
#include "Components/SkyLightComponent.h"

using namespace SparksEnvironmentLightingPresetPropertyOperators;

FSparksEnvironmentLightingPresetPropertiesSkyLight::FSparksEnvironmentLightingPresetPropertiesSkyLight()
{
	SkyDistanceThreshold = 150000;
	Intensity = 1.0f;
	LightColor = FColor::White;
	IndirectLightingIntensity = 1.0f;
	VolumetricScatteringIntensity = 1.0f;
	OcclusionMaxDistance = 1000;
	MinOcclusion = 0;
	OcclusionExponent = 1;
	OcclusionTint = FColor::Black;
	LowerHemisphereColor = FLinearColor::Black;
	Contrast = 0.0;
	bCloudAmbientOcclusion = true;
	CloudAmbientOcclusionExtent = 150.0f;
	bRealTimeCapture = true;
}

void FSparksEnvironmentLightingPresetPropertiesSkyLight::ApplyPresetPropertiesToComponent(USkyLightComponent& Component) const
{
	Component.Contrast = Contrast;
	Component.LowerHemisphereColor = LowerHemisphereColor;
	Component.MinOcclusion = MinOcclusion;
	Component.OcclusionExponent = OcclusionExponent;
	Component.OcclusionMaxDistance = OcclusionMaxDistance;
	Component.OcclusionTint = OcclusionTint;
	Component.SkyDistanceThreshold = SkyDistanceThreshold;
	Component.Intensity = Intensity;
	Component.LightColor = LightColor;
	Component.IndirectLightingIntensity = IndirectLightingIntensity;
	Component.VolumetricScatteringIntensity = VolumetricScatteringIntensity;
	Component.bCloudAmbientOcclusion = bCloudAmbientOcclusion;
	Component.CloudAmbientOcclusionExtent = CloudAmbientOcclusionExtent;
	Component.bRealTimeCapture = bRealTimeCapture;
}

void FSparksEnvironmentLightingPresetPropertiesSkyLight::AccumulateMultiplyAdd(const FSparksEnvironmentLightingPresetPropertiesSkyLight& Other,
	const float Multiplier)
{
	Contrast += Other.Contrast * Multiplier;
	LowerHemisphereColor += Other.LowerHemisphereColor * Multiplier;
	MinOcclusion += Other.MinOcclusion * Multiplier;
	OcclusionExponent += Other.OcclusionExponent * Multiplier;
	OcclusionMaxDistance += Other.OcclusionMaxDistance * Multiplier;
	OcclusionTint += MultiplyByScalar(Other.OcclusionTint, Multiplier);
	SkyDistanceThreshold += Other.SkyDistanceThreshold * Multiplier;
	Intensity += Other.Intensity * Multiplier;
	LightColor += MultiplyByScalar(Other.LightColor, Multiplier);
	IndirectLightingIntensity += Other.IndirectLightingIntensity * Multiplier;
	VolumetricScatteringIntensity += Other.VolumetricScatteringIntensity * Multiplier;
	bCloudAmbientOcclusion = Other.bCloudAmbientOcclusion;
	CloudAmbientOcclusionExtent += Other.CloudAmbientOcclusionExtent * Multiplier;
	bRealTimeCapture = Other.bRealTimeCapture;
}

void FSparksEnvironmentLightingPresetPropertiesSkyLight::ZeroOutProperties()
{
	Contrast = 0.0f;
	LowerHemisphereColor = LinearColorAllZeroes;
	MinOcclusion = 0.0f;
	OcclusionExponent = 0.0f;
	OcclusionMaxDistance = 0.0f;
	OcclusionTint = FColor::Black;
	SkyDistanceThreshold = 0.0f;
	Intensity = 0.0f;
	LightColor = FColor::Black;
	IndirectLightingIntensity = 0.0f;
	VolumetricScatteringIntensity = 0.0f;
	bCloudAmbientOcclusion = false;
	CloudAmbientOcclusionExtent = 0.0f;
	bRealTimeCapture = false;
}