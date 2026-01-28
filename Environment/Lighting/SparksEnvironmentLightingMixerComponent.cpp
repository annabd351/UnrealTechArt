// © 2024 Jar of Sparks, All Rights Reserved

#include "SparksEnvironmentLightingMixerComponent.h"

USparksEnvironmentLightingMixerComponent::USparksEnvironmentLightingMixerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// This is a preset with all values set to minimum
	// Used to avoid an ugly edge case when all values are zero
	FSparksEnvironmentLightingPreset ZeroedPreset;
	FSparksEnvironmentLightingPreset DefaultPreset;
	ZeroedPreset.ZeroOutProperties();
	ZeroedPreset.AccumulateMultiplyAdd(DefaultPreset, MinimumBlendWeightSum);
	MinimumBlendWeightDefaultProperties = ZeroedPreset;
}

void USparksEnvironmentLightingMixerComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateMix();
	UpdateLightingActor();
}

void USparksEnvironmentLightingMixerComponent::DiscoverLightingActor()
{
	EnvLightingActor = Cast<ASparksEnvironmentLightingActor>(GetOwner());
}

#if WITH_EDITORONLY_DATA
void USparksEnvironmentLightingMixerComponent::EditorInitComponent()
{
	DiscoverLightingActor();

	UpdatePresetDelegates();
	UpdateMix();
	UpdateLightingActor();
}
#endif

void USparksEnvironmentLightingMixerComponent::UpdateMix()
{
	if (MixChannels.IsEmpty())
	{
		return;
	}

	// Anything to update?
	bool bHasValidAssets = false;
	bool bAnyChannelIsSoloed = false;
	for (const FSparksEnvironmentLightingMixChannel& MixChannel : MixChannels)
	{
		if (IsValid(MixChannel.PresetAsset))
		{
			bHasValidAssets = true;
		}
		bAnyChannelIsSoloed |= MixChannel.Solo;
	}

#if WITH_EDITORONLY_DATA
	for (FSparksEnvironmentLightingMixChannel& MixChannel : MixChannels)
	{
		MixChannel.EditorUpdate(bAnyChannelIsSoloed);
	}
#endif

	if (bHasValidAssets == false)
	{
		return;
	}

	// Start the update
	PendingLightActorProperties.ZeroOutProperties();
	PendingLightActorBlendWeightSum = 0.0;
	bLightActorPropertiesArePending = true;
	for (const FSparksEnvironmentLightingMixChannel& MixChannel : MixChannels)
	{
		if (IsValid(MixChannel.PresetAsset) && MixChannel.IsInfluencingMix(bAnyChannelIsSoloed))
		{
			PendingLightActorProperties.AccumulateMultiplyAdd(MixChannel.PresetAsset->Preset, MixChannel.BlendWeight);
			PendingLightActorBlendWeightSum += MixChannel.BlendWeight;
		}
	}
}

#if WITH_EDITORONLY_DATA
void USparksEnvironmentLightingMixerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if ((PropertyChangedEvent.ChangeType & EPropertyChangeType::ArrayClear ||
		PropertyChangedEvent.ChangeType & EPropertyChangeType::ArrayRemove) &&
		MixChannels.IsEmpty())
	{
		// No presets. Set lighting actor to defaults to prevent view being completely unlit.
		PendingLightActorProperties = FSparksEnvironmentLightingPreset();
		bLightActorPropertiesArePending = true;
	}

	UpdatePresetDelegates();
	UpdateMix();
	UpdateLightingActor();
}

void USparksEnvironmentLightingMixerComponent::UpdatePresetDelegates()
{
	for (const FSparksEnvironmentLightingMixChannel& MixChannel : MixChannels)
	{
		if (IsValid(MixChannel.PresetAsset) && MixChannel.PresetAsset->OnPresetUpdated.IsBoundToObject(this) == false)
		{
			MixChannel.PresetAsset->OnPresetUpdated.AddUObject(this, &USparksEnvironmentLightingMixerComponent::PresetPropertiesChanged);
		}
	}

	// NOTE: this object is never removed from the OnPresetUpdated. This is valid, since multicast delegates
	// are designed to handle stale references;  however, might be some future reason to clean up.
}

void USparksEnvironmentLightingMixerComponent::PresetPropertiesChanged()
{
	UpdateMix();
	UpdateLightingActor();
}

void USparksEnvironmentLightingMixerComponent::DestroyComponent(bool bPromoteChildren)
{
	if (EnvLightingActor.IsValid())
	{
		EnvLightingActor->ResetToDefaults();
	}

	Super::DestroyComponent(bPromoteChildren);
}
#endif
