// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SparksEnvironmentLightingPresetPropertiesDirectionalLight.h"
#include "SparksEnvironmentLightingPresetPropertiesExponentialHeightFog.h"
#include "SparksEnvironmentLightingPresetPropertiesPostProcess.h"
#include "SparksEnvironmentLightingPresetPropertiesSkyAtmosphere.h"
#include "SparksEnvironmentLightingPresetPropertiesSkyLight.h"
#include "SparksEnvironmentLightingPresetPropertiesVolumetricClouds.h"
#include "Engine/DataAsset.h"
#include "SparksEnvironmentLightingPresetAsset.generated.h"

class ASparksEnvironmentLightingActor;
class USparksEnvironmentLightingPresetAsset;

UENUM()
enum class ESparksEnvironmentLightingComponentType
{
	DirectionalLight,
	ExponentialHeightFog,
	SkyAtmosphere,
	SkyLight,
	SkySphere,
	VolumetricClouds,
	PostProcess,
	Unknown
};

DECLARE_MULTICAST_DELEGATE(FSparksEnvironmentLightingPresetUpdateDelegate);

USTRUCT(BlueprintType)
struct FSparksEnvironmentLightingPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSparksEnvironmentLightingPresetPropertiesDirectionalLight DirectionalLightPresetProperties;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSparksEnvironmentLightingPresetPropertiesExponentialHeightFog ExponentialHeightFogPresetProperties;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSparksEnvironmentLightingPresetPropertiesSkyAtmosphere SkyAtmospherePresetProperties;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSparksEnvironmentLightingPresetPropertiesSkyLight SkyLightPresetProperties;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSparksEnvironmentLightingPresetPropertiesVolumetricClouds VolumetricCloudsPresetProperties;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSparksEnvironmentLightingPresetPropertiesPostProcess PostProcessPresetProperties;

	void AccumulateMultiplyAdd(const FSparksEnvironmentLightingPreset& Other, const float Multiplier);
	void ZeroOutProperties();
};

UCLASS(EditInlineNew)
class SPARKSRUNTIME_API USparksEnvironmentLightingPresetAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FSparksEnvironmentLightingPresetUpdateDelegate OnPresetUpdated;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSparksEnvironmentLightingPreset Preset;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
