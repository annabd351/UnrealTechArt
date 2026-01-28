// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksEnvironmentLightingPresetPropertiesVolumetricClouds.h"

#include "SparksEnvironmentLightingPresetPropertyOperators.h"
#include "Components/VolumetricCloudComponent.h"

using namespace SparksEnvironmentLightingPresetPropertyOperators;

FSparksEnvironmentLightingPresetPropertiesVolumetricClouds::FSparksEnvironmentLightingPresetPropertiesVolumetricClouds()
	: LayerBottomAltitude(5.0f),
	  LayerHeight(10.0f),
	  TracingStartMaxDistance(350.0f),
	  GroundAlbedo(FColor(170, 170, 170)),
	  AerialPespectiveRayleighScatteringStartDistance(0.0f),
	  AerialPespectiveRayleighScatteringFadeDistance(0.0f),
	  AerialPespectiveMieScatteringStartDistance(0.0f),
	  AerialPespectiveMieScatteringFadeDistance(0.0f)
{
}

void FSparksEnvironmentLightingPresetPropertiesVolumetricClouds::ApplyPresetPropertiesToComponent(
	UVolumetricCloudComponent& Component) const
{
	Component.AerialPespectiveMieScatteringFadeDistance = AerialPespectiveMieScatteringFadeDistance;
	Component.AerialPespectiveMieScatteringStartDistance = AerialPespectiveMieScatteringStartDistance;
	Component.AerialPespectiveRayleighScatteringFadeDistance = AerialPespectiveRayleighScatteringFadeDistance;
	Component.AerialPespectiveRayleighScatteringStartDistance = AerialPespectiveRayleighScatteringStartDistance;
	Component.GroundAlbedo = GroundAlbedo;
	Component.LayerBottomAltitude = LayerBottomAltitude;
	Component.LayerHeight = LayerHeight;
	Component.TracingStartMaxDistance = TracingStartMaxDistance;
}

void FSparksEnvironmentLightingPresetPropertiesVolumetricClouds::AccumulateMultiplyAdd(
	const FSparksEnvironmentLightingPresetPropertiesVolumetricClouds& Other, const float Multiplier)
{
	AerialPespectiveMieScatteringFadeDistance += Other.AerialPespectiveMieScatteringFadeDistance * Multiplier;
	AerialPespectiveMieScatteringStartDistance += Other.AerialPespectiveMieScatteringStartDistance * Multiplier;
	AerialPespectiveRayleighScatteringFadeDistance += Other.AerialPespectiveRayleighScatteringFadeDistance * Multiplier;
	AerialPespectiveRayleighScatteringStartDistance += Other.AerialPespectiveRayleighScatteringStartDistance * Multiplier;
	GroundAlbedo += MultiplyByScalar(Other.GroundAlbedo, Multiplier);
	LayerBottomAltitude += Other.LayerBottomAltitude * Multiplier;
	LayerHeight += Other.LayerHeight * Multiplier;
	TracingStartMaxDistance += Other.TracingStartMaxDistance * Multiplier;
}

void FSparksEnvironmentLightingPresetPropertiesVolumetricClouds::ZeroOutProperties()
{
	AerialPespectiveMieScatteringFadeDistance = 0.0f;
	AerialPespectiveMieScatteringStartDistance = 0.0f;
	AerialPespectiveRayleighScatteringFadeDistance = 0.0f;
	AerialPespectiveRayleighScatteringStartDistance = 0.0f;
	GroundAlbedo = FColor::Black;
	LayerBottomAltitude = 0.0f;
	LayerHeight = 0.0f;
	TracingStartMaxDistance = 0.0f;
}