// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksEnvironmentLightingPresetPropertiesSkyAtmosphere.h"

#include "SparksEnvironmentLightingPresetPropertyOperators.h"
#include "Components/SkyAtmosphereComponent.h"

using namespace SparksEnvironmentLightingPresetPropertyOperators;

FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere::FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere()
{
	// All distance here are in kilometer and scattering/absorptions coefficient in 1/kilometers.
	constexpr float EarthBottomRadius = 6360.0f;
	constexpr float EarthTopRadius = 6420.0f;
	constexpr float EarthRayleighScaleHeight = 8.0f;
	constexpr float EarthMieScaleHeight = 1.2f;

	// Default: Earth like atmosphere
	BottomRadius = EarthBottomRadius;
	AtmosphereHeight = EarthTopRadius - EarthBottomRadius;
	GroundAlbedo = FColor(170, 170, 170); // 170 => 0.4f linear

	// Float to a u8 rgb + float length can lose some precision but it is better UI wise.
	constexpr FLinearColor RayleightScatteringRaw = FLinearColor(0.005802f, 0.013558f, 0.033100f);
	RayleighScattering = RayleightScatteringRaw * (1.0f / RayleightScatteringRaw.B);
	RayleighScatteringScale = RayleightScatteringRaw.B;
	RayleighExponentialDistribution = EarthRayleighScaleHeight;

	MieScattering = FColor::White;
	MieScatteringScale = 0.003996f;
	MieAbsorption = FColor::White;
	MieAbsorptionScale = 0.000444f;
	MieAnisotropy = 0.8f;
	MieExponentialDistribution = EarthMieScaleHeight;
	SkyLuminanceFactor = FLinearColor(FLinearColor::White);
	MultiScatteringFactor = 1.0f;
	AerialPespectiveViewDistanceScale = 1.0f;
	HeightFogContribution = 1.0f;
	TransmittanceMinLightElevationAngle = -90.0f;
	AerialPerspectiveStartDepth = 0.1f;

	// Absorption tent distribution representing ozone distribution in Earth atmosphere.
	const FLinearColor OtherAbsorptionRaw = FLinearColor(0.000650f, 0.001881f, 0.000085f);
	OtherAbsorptionScale = OtherAbsorptionRaw.G;
	OtherAbsorption = OtherAbsorptionRaw * (1.0f / OtherAbsorptionRaw.G);
}

void FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere::ApplyPresetPropertiesToComponent(USkyAtmosphereComponent& Component) const
{
	Component.AerialPerspectiveStartDepth = AerialPerspectiveStartDepth;
	Component.AerialPespectiveViewDistanceScale = AerialPespectiveViewDistanceScale;
	Component.AtmosphereHeight = AtmosphereHeight;
	Component.BottomRadius = BottomRadius;
	Component.GroundAlbedo = GroundAlbedo;
	Component.HeightFogContribution = HeightFogContribution;
	Component.MieAbsorption = MieAbsorption;
	Component.MieAbsorptionScale = MieAbsorptionScale;
	Component.MieAnisotropy = MieAnisotropy;
	Component.MieExponentialDistribution = MieExponentialDistribution;
	Component.MieScattering = MieScattering;
	Component.MieScatteringScale = MieScatteringScale;
	Component.MultiScatteringFactor = MultiScatteringFactor;
	Component.RayleighExponentialDistribution = RayleighExponentialDistribution;
	Component.RayleighScattering = RayleighScattering;
	Component.RayleighScatteringScale = RayleighScatteringScale;
	Component.SkyLuminanceFactor = SkyLuminanceFactor;
	Component.TransmittanceMinLightElevationAngle = TransmittanceMinLightElevationAngle;
	Component.OtherAbsorptionScale = OtherAbsorptionScale;
	Component.OtherAbsorption = OtherAbsorption;
}

void FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere::AccumulateMultiplyAdd(const FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere& Other,
	const float Multiplier)
{
	AerialPerspectiveStartDepth += Other.AerialPerspectiveStartDepth * Multiplier;
	AerialPespectiveViewDistanceScale += Other.AerialPespectiveViewDistanceScale * Multiplier;
	AtmosphereHeight += Other.AtmosphereHeight * Multiplier;
	BottomRadius += Other.BottomRadius * Multiplier;
	GroundAlbedo += MultiplyByScalar(Other.GroundAlbedo, Multiplier);
	HeightFogContribution += Other.HeightFogContribution * Multiplier;
	MieAbsorption += Other.MieAbsorption * Multiplier;
	MieAbsorptionScale += Other.MieAbsorptionScale * Multiplier;
	MieAnisotropy += Other.MieAnisotropy * Multiplier;
	MieExponentialDistribution += Other.MieExponentialDistribution * Multiplier;
	MieScattering += Other.MieScattering * Multiplier;
	MieScatteringScale += Other.MieScatteringScale * Multiplier;
	MultiScatteringFactor += Other.MultiScatteringFactor * Multiplier;
	RayleighExponentialDistribution += Other.RayleighExponentialDistribution * Multiplier;
	RayleighScattering += Other.RayleighScattering * Multiplier;
	RayleighScatteringScale += Other.RayleighScatteringScale * Multiplier;
	SkyLuminanceFactor += Other.SkyLuminanceFactor * Multiplier;
	TransmittanceMinLightElevationAngle += Other.TransmittanceMinLightElevationAngle * Multiplier;
	OtherAbsorptionScale += Other.OtherAbsorptionScale;
	OtherAbsorption += Other.OtherAbsorption;
}

void FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere::ZeroOutProperties()
{
	AerialPerspectiveStartDepth = 0.0f;
	AerialPespectiveViewDistanceScale = 0.0f;
	AtmosphereHeight = 0.0f;
	BottomRadius = 0.0f;
	GroundAlbedo = FColor::Black;
	HeightFogContribution = 0.0f;
	MieAbsorption = LinearColorAllZeroes;
	MieAbsorptionScale = 0.0f;
	MieAnisotropy = 0.0f;
	MieExponentialDistribution = 0.0f;
	MieScattering = LinearColorAllZeroes;
	MieScatteringScale = 0.0f;
	MultiScatteringFactor = 0.0f;
	RayleighExponentialDistribution = 0.0f;
	RayleighScattering = LinearColorAllZeroes;
	RayleighScatteringScale = 0.0f;
	SkyLuminanceFactor = LinearColorAllZeroes;
	TransmittanceMinLightElevationAngle = 0.0f;
	OtherAbsorptionScale = 0.0f;
	OtherAbsorption = LinearColorAllZeroes;
}