// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksEnvironmentLightingTrackComponent.h"

USparksEnvironmentLightingTrackComponent::USparksEnvironmentLightingTrackComponent()
{
#if WITH_EDITORONLY_DATA
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
#else
	PrimaryComponentTick.bCanEverTick = false;
#endif
}

void USparksEnvironmentLightingTrackComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateLightingActor(IsValid(PresetAsset) ? ESparksEnvironmentLightingTrackComponentAction::Add : ESparksEnvironmentLightingTrackComponentAction::None);
}

#if WITH_EDITORONLY_DATA
void USparksEnvironmentLightingTrackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEditorInitPending)
	{
		EditorInitComponent();
		bEditorInitPending = false;
		SetComponentTickEnabled(false);
	}
}

void USparksEnvironmentLightingTrackComponent::EditorInitComponent()
{
	UpdatePresetDelegates();
	UpdateLightingActor(IsValid(PresetAsset) ? ESparksEnvironmentLightingTrackComponentAction::Add : ESparksEnvironmentLightingTrackComponentAction::None);
}

void USparksEnvironmentLightingTrackComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ESparksEnvironmentLightingTrackComponentAction ComponentAction = ESparksEnvironmentLightingTrackComponentAction::Dirty;

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(USparksEnvironmentLightingTrackComponent, PresetAsset))
	{
		UpdatePresetDelegates();
		ComponentAction = IsValid(PresetAsset) ? ESparksEnvironmentLightingTrackComponentAction::Add : ESparksEnvironmentLightingTrackComponentAction::Remove;
	}

	UpdateLightingActor(ComponentAction);
}

void USparksEnvironmentLightingTrackComponent::UpdatePresetDelegates()
{
	if (IsValid(PresetAsset))
	{
		PresetAsset.Get()->OnPresetUpdated.AddUObject(this, &USparksEnvironmentLightingTrackComponent::PresetPropertiesChanged);
	}
}

void USparksEnvironmentLightingTrackComponent::PresetPropertiesChanged()
{
	UpdateLightingActor(ESparksEnvironmentLightingTrackComponentAction::None);
}

#endif

void USparksEnvironmentLightingTrackComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	UpdateLightingActor(ESparksEnvironmentLightingTrackComponentAction::Add);
}

void USparksEnvironmentLightingTrackComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IsValid(PresetAsset))
	{
		PresetAsset->OnPresetUpdated.RemoveAll(this);
	}

	UpdateLightingActor(ESparksEnvironmentLightingTrackComponentAction::Remove);

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}
