// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksEnvironmentLightingPresetPropertiesExponentialHeightFog.h"
#include "SparksEnvironmentLightingPresetPropertyOperators.h"
#include "Components/ExponentialHeightFogComponent.h"

using namespace SparksEnvironmentLightingPresetPropertyOperators;

FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog::FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog()
{
	FogInscatteringLuminance = FLinearColor::Black;
	SkyAtmosphereAmbientContributionColorScale = FLinearColor::White;
	DirectionalInscatteringExponent = 4.0f;
	DirectionalInscatteringStartDistance = 10000.0f;
	DirectionalInscatteringLuminance = FLinearColor::Black;
	InscatteringTextureTint = FLinearColor::White;
	FullyDirectionalInscatteringColorDistance = 100000.0f;
	NonDirectionalInscatteringColorDistance = 1000.0f;
	FogDensity = 0.02f;
	FogHeightFalloff = 0.2f;
	FogMaxOpacity = 1.0f;
	StartDistance = 0.0f;
	FogCutoffDistance = 0;
	bEnableVolumetricFog = true;
	VolumetricFogScatteringDistribution = .2f;
	VolumetricFogAlbedo = FColor::White;
	VolumetricFogExtinctionScale = 1.0f;
	VolumetricFogDistance = 6000.0f;
	VolumetricFogStaticLightingScatteringIntensity = 1;
	VolumetricFogStartDistance = 0.0f;
	VolumetricFogNearFadeInDistance = 0.0f;
	VolumetricFogEmissive = LinearColorAllZeroes;
}

void FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog::ApplyPresetPropertiesToComponent(
	UExponentialHeightFogComponent& Component) const
{
	Component.DirectionalInscatteringExponent = DirectionalInscatteringExponent;
	Component.DirectionalInscatteringLuminance = DirectionalInscatteringLuminance;
	Component.DirectionalInscatteringStartDistance = DirectionalInscatteringStartDistance;
	Component.FogCutoffDistance = FogCutoffDistance;
	Component.FogDensity = FogDensity;
	Component.FogHeightFalloff = FogHeightFalloff;
	Component.FogInscatteringLuminance = FogInscatteringLuminance;
	Component.FogMaxOpacity = FogMaxOpacity;
	Component.FullyDirectionalInscatteringColorDistance = FullyDirectionalInscatteringColorDistance;
	Component.InscatteringTextureTint = InscatteringTextureTint;
	Component.NonDirectionalInscatteringColorDistance = NonDirectionalInscatteringColorDistance;
	Component.SkyAtmosphereAmbientContributionColorScale = SkyAtmosphereAmbientContributionColorScale;
	Component.StartDistance = StartDistance;
	Component.bEnableVolumetricFog = bEnableVolumetricFog;
	Component.VolumetricFogScatteringDistribution = VolumetricFogScatteringDistribution;
	Component.VolumetricFogAlbedo = VolumetricFogAlbedo;
	Component.VolumetricFogExtinctionScale = VolumetricFogExtinctionScale;
	Component.VolumetricFogDistance = VolumetricFogDistance;
	Component.VolumetricFogStaticLightingScatteringIntensity = VolumetricFogStaticLightingScatteringIntensity;
	Component.VolumetricFogStartDistance = VolumetricFogStartDistance;
	Component.VolumetricFogNearFadeInDistance = VolumetricFogNearFadeInDistance;
	Component.VolumetricFogEmissive = VolumetricFogEmissive;
}

void FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog::AccumulateMultiplyAdd(
	const FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog& Other, const float Multiplier)
{
	DirectionalInscatteringExponent += Other.DirectionalInscatteringExponent * Multiplier;
	DirectionalInscatteringLuminance += Other.DirectionalInscatteringLuminance * Multiplier;
	DirectionalInscatteringStartDistance += Other.DirectionalInscatteringStartDistance * Multiplier;
	FogCutoffDistance += Other.FogCutoffDistance * Multiplier;
	FogDensity += Other.FogDensity * Multiplier;
	FogHeightFalloff += Other.FogHeightFalloff * Multiplier;
	FogInscatteringLuminance += Other.FogInscatteringLuminance * Multiplier;
	FogMaxOpacity += Other.FogMaxOpacity * Multiplier;
	FullyDirectionalInscatteringColorDistance += Other.FullyDirectionalInscatteringColorDistance * Multiplier;
	InscatteringTextureTint += Other.InscatteringTextureTint * Multiplier;
	NonDirectionalInscatteringColorDistance += Other.NonDirectionalInscatteringColorDistance * Multiplier;
	SkyAtmosphereAmbientContributionColorScale += Other.SkyAtmosphereAmbientContributionColorScale * Multiplier;
	StartDistance += Other.StartDistance * Multiplier;
	bEnableVolumetricFog = Other.bEnableVolumetricFog;
	VolumetricFogScatteringDistribution += Other.VolumetricFogScatteringDistribution * Multiplier;
	VolumetricFogAlbedo += MultiplyByScalar(Other.VolumetricFogAlbedo, Multiplier);
	VolumetricFogExtinctionScale += Other.VolumetricFogExtinctionScale * Multiplier;
	VolumetricFogDistance += Other.VolumetricFogDistance * Multiplier;
	VolumetricFogStaticLightingScatteringIntensity += Other.VolumetricFogStaticLightingScatteringIntensity * Multiplier;
	VolumetricFogStartDistance += Other.VolumetricFogStartDistance * Multiplier;
	VolumetricFogNearFadeInDistance += Other.VolumetricFogNearFadeInDistance * Multiplier;
	VolumetricFogEmissive += Other.VolumetricFogEmissive * Multiplier;
}

void FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog::ZeroOutProperties()
{
	DirectionalInscatteringExponent = 0.0f;
	DirectionalInscatteringLuminance = LinearColorAllZeroes;
	DirectionalInscatteringStartDistance = 0.0f;
	FogCutoffDistance = 0.0f;
	FogDensity = 0.0f;
	FogHeightFalloff = 0.0f;
	FogInscatteringLuminance = LinearColorAllZeroes;
	FogMaxOpacity = 0.0f;
	FullyDirectionalInscatteringColorDistance = 0.0f;
	InscatteringTextureTint = LinearColorAllZeroes;
	NonDirectionalInscatteringColorDistance = 0.0f;
	SkyAtmosphereAmbientContributionColorScale = LinearColorAllZeroes;
	StartDistance = 0.0f;
	bEnableVolumetricFog = false;
	VolumetricFogScatteringDistribution = 0.0f;
	VolumetricFogAlbedo = FColor::Black;
	VolumetricFogExtinctionScale = 0.0f;
	VolumetricFogDistance = 0.0f;
	VolumetricFogStaticLightingScatteringIntensity = 0.0f;
	VolumetricFogStartDistance = 0.0f;
	VolumetricFogNearFadeInDistance = 0.0f;
	VolumetricFogEmissive = LinearColorAllZeroes;
}