// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksEnvironmentLightingPresetPropertiesPostProcess.h"

void FSparksEnvironmentLightingPresetPropertiesPostProcess::ApplyPresetPropertiesToComponent(
	UPostProcessComponent& Component) const
{
	Component.BlendWeight = BlendWeight;
	Component.Settings = Settings;
	Component.Priority = Priority;
}

void FSparksEnvironmentLightingPresetPropertiesPostProcess::AccumulateMultiplyAdd(
	const FSparksEnvironmentLightingPresetPropertiesPostProcess& Other, const float Multiplier)
{
	BlendWeight += Other.BlendWeight * Multiplier;
	Priority += Other.Priority * Multiplier;

	// Don't need to apply weight to individual properties -- that's already built in.
	Settings = Other.Settings;
}

void FSparksEnvironmentLightingPresetPropertiesPostProcess::ZeroOutProperties()
{
	BlendWeight = 0.0f;
	Priority = 0.0f;

	// Don't need to zero out Settings since we're not summing weighted properties.
}