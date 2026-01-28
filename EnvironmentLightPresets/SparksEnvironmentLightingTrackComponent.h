// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SparksRuntime/Environment/Lighting/SparksEnvironmentLightingActor.h"
#include "SparksEnvironmentLightingTrackComponent.generated.h"

enum class ESparksEnvironmentLightingTrackComponentAction
{
	Add,
	Remove,
	Dirty,
	None
};

class ASparksEnvironmentLightingActor;
class USparksEnvironmentLightingPresetAsset;

// One of two blending components used with SparksEnvironmentLightingActor.  This variant is compatible with the sequencer, with each
// component managing a single preset.  You can multiple instances of this class in the lighting actor.
//
// The other version, USparksEnvironmentLightingMixerComponent, blends multiple presets but cannot
// be used with the Sequencer.  Use only a single instance of that class with the lighting actor.
//
// Don't mix blending component types in a lighting actor -- use one type or the other.
UCLASS(meta=(BlueprintSpawnableComponent))
class SPARKSRUNTIME_API USparksEnvironmentLightingTrackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USparksEnvironmentLightingTrackComponent();

	UPROPERTY(EditAnywhere, Interp, BlueprintReadWrite, meta = (UIMin = "0.01", UIMax = "1", ClampMin = "0.01", SliderExponent = 1.0))
	float BlendWeight = 1.0;

	UPROPERTY(EditAnywhere, Interp, BlueprintReadWrite)
	TObjectPtr<USparksEnvironmentLightingPresetAsset> PresetAsset = nullptr;

protected:
	virtual void BeginPlay() override;

public:
	ASparksEnvironmentLightingActor* GetLightingActor() const
	{
		return Cast<ASparksEnvironmentLightingActor>(GetOwner());
	}

	virtual void OnComponentCreated() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

private:
	void UpdateLightingActor(const ESparksEnvironmentLightingTrackComponentAction ComponentAction)
	{
		ASparksEnvironmentLightingActor* LightingActor = GetLightingActor();
		if (!GetLightingActor())
		{
			return;
		}

		switch (ComponentAction)
		{
		case ESparksEnvironmentLightingTrackComponentAction::Add:
			LightingActor->TrackComponentAdded(this);
			LightingActor->TrackComponentDirty();
			break;
		case ESparksEnvironmentLightingTrackComponentAction::Remove:
			LightingActor->TrackComponentRemoved(this);
			LightingActor->TrackComponentDirty();
			break;
		case ESparksEnvironmentLightingTrackComponentAction::Dirty:
			LightingActor->TrackComponentDirty();
			break;
		case ESparksEnvironmentLightingTrackComponentAction::None:
			break;
		}
	}

#if WITH_EDITORONLY_DATA
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void PresetPropertiesChanged();
	void EditorInitComponent();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

private:
	void UpdatePresetDelegates();
	bool bEditorInitPending = true;
#endif
};