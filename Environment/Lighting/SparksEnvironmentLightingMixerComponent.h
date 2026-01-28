// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SparksEnvironmentLightingActor.h"
#include "SparksEnvironmentLightingPresetAsset.h"
#include "GameFramework/Actor.h"
#include "SparksEnvironmentLightingMixerComponent.generated.h"

USTRUCT(BlueprintType)
struct FSparksEnvironmentLightingMixChannel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (UIMin = "0.01", UIMax = "1", ClampMin = "0.01", SliderExponent = 1.0), meta = (EditCondition = "bIsEditable", HideEditConditionToggle))
	float BlendWeight = 1.0;

	// Disable this preset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Mute = false;

	// Use only this and other solo'ed presets.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Solo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bIsEditable", HideEditConditionToggle))
	TObjectPtr<USparksEnvironmentLightingPresetAsset> PresetAsset = nullptr;

#if WITH_EDITORONLY_DATA
	void EditorUpdate(const bool bAnotherChannelIsSoloed)
	{
		bIsEditable = IsInfluencingMix(bAnotherChannelIsSoloed);
	}

private:
	UPROPERTY()
	bool bIsEditable = true;
#endif

public:
	bool IsInfluencingMix(const bool bAnotherChannelIsSoloed) const
	{
		return ((Mute == false && Solo == false && bAnotherChannelIsSoloed == false) || (Mute == false && Solo == true)) && BlendWeight > 0.0;
	}
};

UCLASS(Placeable)
class SPARKSRUNTIME_API USparksEnvironmentLightingMixerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USparksEnvironmentLightingMixerComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sparks Environment Lighting")
	TArray<FSparksEnvironmentLightingMixChannel> MixChannels;

	// Call to apply changes after setting MixChannels.
	UFUNCTION(BlueprintCallable, Category="Sparks Environment Lighting")
	void Update()
	{
		UpdateMix();
		UpdateLightingActor();
	};

	virtual void BeginPlay() override;

	ASparksEnvironmentLightingActor* GetEnvLightingActor() const
	{
		return EnvLightingActor.IsValid() ? EnvLightingActor.Get() : nullptr;
	};

	void SetEnvLightingActor(ASparksEnvironmentLightingActor* NewEnvLightingActor)
	{
		EnvLightingActor = NewEnvLightingActor;

		UpdateMix();
		UpdateLightingActor();
	}

private:
	void UpdateMix();

	// No properties pending at construction -- might not be anything in mix.
	bool bLightActorPropertiesArePending = false;
	FSparksEnvironmentLightingPreset PendingLightActorProperties;
	float PendingLightActorBlendWeightSum = 0.0;
	static constexpr float MinimumBlendWeightSum = 0.01;
	FSparksEnvironmentLightingPreset MinimumBlendWeightDefaultProperties;

	TWeakObjectPtr<ASparksEnvironmentLightingActor> EnvLightingActor = nullptr;

	void UpdateLightingActor()
	{
		if (bLightActorPropertiesArePending == false)
		{
			return;
		}

		if (ASparksEnvironmentLightingActor* LightingActor = GetEnvLightingActor())
		{
			if (PendingLightActorBlendWeightSum > MinimumBlendWeightSum)
			{
				LightingActor->ApplyPresetProperties(&PendingLightActorProperties);
			}
			else
			{
				// Avoid invalid lighting display if all blend weights are zero
				LightingActor->ApplyPresetProperties(&MinimumBlendWeightDefaultProperties);
			}
		}
		bLightActorPropertiesArePending = false;
	}

	void DiscoverLightingActor();

#if WITH_EDITORONLY_DATA
public:
	void PresetPropertiesChanged();
	void EditorInitComponent();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void DestroyComponent(bool bPromoteChildren) override;

private:
	void UpdatePresetDelegates();
	bool bEditorInitPending = true;
#endif
};
