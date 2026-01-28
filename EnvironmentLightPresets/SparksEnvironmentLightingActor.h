// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SparksEnvironmentLightingPresetAsset.h"
#include "Components/PostProcessComponent.h"
#include "GameFramework/Actor.h"
#include "SparksEnvironmentLightingActor.generated.h"

class USparksEnvironmentLightingTrackComponent;
class USparksEnvironmentLightingMixerComponent;

UCLASS()
class SPARKSRUNTIME_API ASparksEnvironmentLightingActor : public AActor, public FTickableGameObject
{
	GENERATED_BODY()

public:
	ASparksEnvironmentLightingActor();

	UPROPERTY(EditAnywhere)
	TObjectPtr<UDirectionalLightComponent> DirectionalLight = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UExponentialHeightFogComponent> ExponentialHeightFog = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USkyLightComponent> SkyLight = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UVolumetricCloudComponent> VolumetricClouds = nullptr;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UPostProcessComponent> PostProcess = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USparksEnvironmentLightingMixerComponent> PresetMixer = nullptr;

	virtual void BeginPlay() override;

	void ApplyPresetProperties(const FSparksEnvironmentLightingPreset* PresetChecked);

	TSet<USparksEnvironmentLightingTrackComponent*>& GetTrackComponents()
	{
		return TrackComponents;
	};

#pragma region FTickableGameObject
	virtual bool IsTickableInEditor() const override
	{
		return true;
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(ASparksEnvironmentLightingActor, STATGROUP_Tickables);
	}

	virtual void Tick(float DeltaSeconds) override;
#pragma endregion

#if WITH_EDITORONLY_DATA
	FReply ResetToDefaults();

private:
	FTimerDelegate SkyLightToggleDelegate;
	void EditorInit();
	bool bEditorInitPending = true;
#endif

public:
	void TrackComponentAdded(USparksEnvironmentLightingTrackComponent* SequencerTrackComponent);
	void TrackComponentRemoved(const USparksEnvironmentLightingTrackComponent* SequencerTrackComponent);
	void TrackComponentDirty();

private:
	void AddExistingComponents();
	TSet<USparksEnvironmentLightingTrackComponent*> TrackComponents;
	FSparksEnvironmentLightingPreset PresetProperties;
	void UpdatePresetProperties();
};