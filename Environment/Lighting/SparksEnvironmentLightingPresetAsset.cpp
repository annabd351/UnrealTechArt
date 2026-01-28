// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksEnvironmentLightingPresetAsset.h"

void FSparksEnvironmentLightingPreset::AccumulateMultiplyAdd(const FSparksEnvironmentLightingPreset& Other, const float Multiplier)
{
	// Boolean properties are not "blended" -- they retain the last setting.  Assuming channels are blended in the order in
	// which they are stored in the array, that means the highest number channel controls the setting.

	DirectionalLightPresetProperties.AccumulateMultiplyAdd(Other.DirectionalLightPresetProperties, Multiplier);
	ExponentialHeightFogPresetProperties.AccumulateMultiplyAdd(Other.ExponentialHeightFogPresetProperties, Multiplier);
	SkyAtmospherePresetProperties.AccumulateMultiplyAdd(Other.SkyAtmospherePresetProperties, Multiplier);
	SkyLightPresetProperties.AccumulateMultiplyAdd(Other.SkyLightPresetProperties, Multiplier);
	VolumetricCloudsPresetProperties.AccumulateMultiplyAdd(Other.VolumetricCloudsPresetProperties, Multiplier);
	PostProcessPresetProperties.AccumulateMultiplyAdd(Other.PostProcessPresetProperties, Multiplier);
}

void FSparksEnvironmentLightingPreset::ZeroOutProperties()
{
	// Boolean properties are "zeroed out" to false

	DirectionalLightPresetProperties.ZeroOutProperties();
	ExponentialHeightFogPresetProperties.ZeroOutProperties();
	SkyAtmospherePresetProperties.ZeroOutProperties();
	SkyLightPresetProperties.ZeroOutProperties();
	VolumetricCloudsPresetProperties.ZeroOutProperties();
	PostProcessPresetProperties.ZeroOutProperties();
}

#if WITH_EDITORONLY_DATA
void USparksEnvironmentLightingPresetAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	OnPresetUpdated.Broadcast();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
