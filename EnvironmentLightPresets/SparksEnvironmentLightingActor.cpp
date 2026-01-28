// © 2024 Jar of Sparks, All Rights Reserved

#include "SparksEnvironmentLightingActor.h"
#include "SparksEnvironmentLightingMixerComponent.h"
#include "SparksEnvironmentLightingPresetAsset.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "SparksEnvironmentLightingTrackComponent.h"

ASparksEnvironmentLightingActor::ASparksEnvironmentLightingActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
	DirectionalLight->SetupAttachment(RootComponent);
	ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFog"));
	ExponentialHeightFog->SetupAttachment(RootComponent);
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(RootComponent);
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootComponent);
	VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
	VolumetricClouds->SetupAttachment(RootComponent);
	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(RootComponent);
	PostProcess->bEnabled = true;
	PostProcess->bUnbound = true;
	PresetMixer = CreateDefaultSubobject<USparksEnvironmentLightingMixerComponent>(TEXT("PresetMixer"));

	const FSparksEnvironmentLightingPreset DefaultPreset{};
	ApplyPresetProperties(&DefaultPreset);
}

void ASparksEnvironmentLightingActor::BeginPlay()
{
	Super::BeginPlay();

	AddExistingComponents();
}

void ASparksEnvironmentLightingActor::ApplyPresetProperties(const FSparksEnvironmentLightingPreset* PresetChecked)
{
	checkf(PresetChecked, TEXT("ASparksEnvironmentLightingActor::ApplyPresetProperties -- PresetChecked is null"));

	PresetChecked->DirectionalLightPresetProperties.ApplyPresetPropertiesToComponent(*DirectionalLight);
	PresetChecked->ExponentialHeightFogPresetProperties.ApplyPresetPropertiesToComponent(*ExponentialHeightFog);
	PresetChecked->SkyAtmospherePresetProperties.ApplyPresetPropertiesToComponent(*SkyAtmosphere);
	PresetChecked->SkyLightPresetProperties.ApplyPresetPropertiesToComponent(*SkyLight);
	PresetChecked->VolumetricCloudsPresetProperties.ApplyPresetPropertiesToComponent(*VolumetricClouds);
	PresetChecked->PostProcessPresetProperties.ApplyPresetPropertiesToComponent(*PostProcess);

	MarkComponentsRenderStateDirty();
}

void ASparksEnvironmentLightingActor::UpdatePresetProperties()
{
	PresetProperties.ZeroOutProperties();
	for (const USparksEnvironmentLightingTrackComponent* TrackComponent : TrackComponents)
	{
		check(TrackComponent);
		if (TrackComponent->PresetAsset)
		{
			PresetProperties.AccumulateMultiplyAdd(TrackComponent->PresetAsset->Preset, TrackComponent->BlendWeight);
		}
	}

	ApplyPresetProperties(&PresetProperties);
}

void ASparksEnvironmentLightingActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if WITH_EDITORONLY_DATA
	if (bEditorInitPending)
	{
		EditorInit();
		bEditorInitPending = false;
	}
#endif

	if (TrackComponents.IsEmpty())
	{
		return;
	}

	UpdatePresetProperties();
}

#if WITH_EDITORONLY_DATA
void ASparksEnvironmentLightingActor::EditorInit()
{
	if (IsValid(PresetMixer))
	{
		PresetMixer->EditorInitComponent();
	}

	AddExistingComponents();
}

FReply ASparksEnvironmentLightingActor::ResetToDefaults()
{
	const FSparksEnvironmentLightingPreset DefaultPreset{};
	ApplyPresetProperties(&DefaultPreset);

	if (IsValid(GetWorld()) == false)
	{
		return FReply::Unhandled();
	}

	const bool FirstToggleState = !SkyLight->IsRealTimeCaptureEnabled();
	const bool SecondToggleState = !FirstToggleState;
	SkyLight->SetRealTimeCaptureEnabled(FirstToggleState);

	TWeakObjectPtr<USkyLightComponent> WeakSkyLight = SkyLight;
	SkyLightToggleDelegate.BindLambda([WeakSkyLight, SecondToggleState]()
	{
		if (WeakSkyLight.IsValid())
		{
			WeakSkyLight->SetRealTimeCaptureEnabled(SecondToggleState);
		}
	});
	GetWorld()->GetTimerManager().SetTimerForNextTick(SkyLightToggleDelegate);

	return FReply::Handled();
}
#endif

void ASparksEnvironmentLightingActor::AddExistingComponents()
{
	ForEachComponent(false, [&](UActorComponent* Component)
	{
		if (USparksEnvironmentLightingTrackComponent* TrackComponent = Cast<USparksEnvironmentLightingTrackComponent>(Component))
		{
			TrackComponents.Add(TrackComponent);
		}
	});
}

void ASparksEnvironmentLightingActor::TrackComponentAdded(USparksEnvironmentLightingTrackComponent* SequencerTrackComponent)
{
	check(SequencerTrackComponent)
	TrackComponents.Add(SequencerTrackComponent);
}

void ASparksEnvironmentLightingActor::TrackComponentRemoved(const USparksEnvironmentLightingTrackComponent* SequencerTrackComponent)
{
	if (IsValid(SequencerTrackComponent))
	{
		TrackComponents.Remove(SequencerTrackComponent);
	}
}

void ASparksEnvironmentLightingActor::TrackComponentDirty()
{
	UpdatePresetProperties();
}