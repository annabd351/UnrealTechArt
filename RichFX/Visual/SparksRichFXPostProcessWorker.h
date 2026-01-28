// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/PostProcessComponent.h"
#include "SparksRuntime/RichFX/SparksRichFXWorker.h"
#include "UObject/Object.h"
#include "SparksRichFXPostProcessWorker.generated.h"

class USparksRichFXPostProcessWorker;

USTRUCT(BlueprintType)
struct SPARKSRUNTIME_API FSparksRichFXPostProcessWorkerData : public FSparksRichFXWorkerData
{
	GENERATED_BODY()

	FSparksRichFXPostProcessWorkerData()
	{
	}

	FSparksRichFXPostProcessWorkerData(const TObjectPtr<UMaterialInterface>& Material,
	                               const FName& FadeMaterialParamName,
	                               const float FadeOnDuration,
	                               const TObjectPtr<UCurveFloat>& FadeOnCurve,
	                               const float FadeOffDuration,
	                               const TObjectPtr<UCurveFloat>& FadeOutCurve)
		: Material(Material),
		  FadeMaterialParamName(FadeMaterialParamName),
		  FadeOnDuration(FadeOnDuration),
		  FadeOnCurve(FadeOnCurve),
		  FadeOffDuration(FadeOffDuration),
		  FadeOffCurve(FadeOutCurve)
	{
	}

	FSparksRichFXPostProcessWorkerData(const FSparksRichFXPostProcessWorkerData& Other)
		: FSparksRichFXWorkerData(Other),
		  Material(Other.Material),
		  FadeMaterialParamName(Other.FadeMaterialParamName),
		  FadeOnDuration(Other.FadeOnDuration),
		  FadeOnCurve(Other.FadeOnCurve),
	      FadeOffDuration(Other.FadeOffDuration),
		  FadeOffCurve(Other.FadeOffCurve)
	{
	}

	FSparksRichFXPostProcessWorkerData& operator=(const FSparksRichFXPostProcessWorkerData& Other)
	{
		if (this == &Other)
			return *this;
		FSparksRichFXWorkerData::operator =(Other);
		FadeOnDuration = Other.FadeOnDuration;
		FadeOffDuration = Other.FadeOffDuration;
		Material = Other.Material;
		FadeMaterialParamName = Other.FadeMaterialParamName;
		FadeOnCurve = Other.FadeOnCurve;
		FadeOffCurve = Other.FadeOffCurve;
		return *this;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> Material = nullptr;

	// Material parameter which fades the post effect in or out.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FadeMaterialParamName = TEXT("Alpha");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FadeOnDuration = 5.0;

	// Normalized curve applied to fade IN.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCurveFloat> FadeOnCurve = nullptr;

	// For Constant/Despawn effects, this enables a fade off during Despawn. NOT used for OneShot effects.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseFadeOff = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = bUseFadeOff, EditConditionHides))
	float FadeOffDuration = 5.0;

	// Normalized curve applied to fade OUT.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = bUseFadeOff, EditConditionHides))
	TObjectPtr<UCurveFloat> FadeOffCurve = nullptr;

	virtual bool HasValidProperties() const override
	{
		const bool FadeOnValid = FadeOnDuration > 0.0 ? IsValid(FadeOnCurve) : false;
		const bool FadeOffValid = bUseFadeOff && (FadeOffDuration > 0.0) ? IsValid(FadeOffCurve) : false;
		return (FadeOnValid || FadeOffValid) && IsValid(Material);
	}
};

UCLASS(BlueprintType)
class SPARKSRUNTIME_API USparksRichFXPostProcessWorker : public USparksRichFXWorker
{
	GENERATED_BODY()

public:
	USparksRichFXPostProcessWorker(const FObjectInitializer& ObjectInitializer);

	void Init(const FSparksRichFXPostProcessWorkerData& NewData)
	{
		Data = NewData;
	}

	const FSparksRichFXPostProcessWorkerData& GetData()
	{
		return Data;
	}

	TOptional<FPostProcessSettings> GetPostProcessSettings() const
	{
		if (ensureMsgf(IsValid(FXComponent), TEXT("FXComponent invalid")) == false)
		{
			return TOptional<FPostProcessSettings>();
		}

		const UPostProcessComponent* PostProcessComponent = ExactCast<UPostProcessComponent>(FXComponent);
		TOptional<FPostProcessSettings> ReturnValue;
		if (PostProcessComponent != nullptr)
		{
			ReturnValue.Emplace(PostProcessComponent->Settings);
		}

		return ReturnValue;
	}

	void SetPostProcessSettings(const FPostProcessSettings& NewSettings) const
	{
		ensureMsgf(IsValid(FXComponent), TEXT("FXComponent invalid"));
		UPostProcessComponent* PostProcessComponent = ExactCast<UPostProcessComponent>(FXComponent);
		PostProcessComponent->Settings = NewSettings;
	}

	virtual bool Activate(const ESparksRichFXWorkerExecutionState InitialState, const FHitResult& ValidHitResult = FHitResult(), const AActor* AttachToActor = nullptr, const FName Socket = NAME_None) override;
	virtual bool Resume() override;
	virtual void Deactivate() override;
	virtual void RequestRelease(const double RequestedAtTime) override;

	// Get the dynamic material instance used by this worker. Caution: CAN BE INVALID! Make sure to check.
	UFUNCTION(Blueprintable)
	UMaterialInstanceDynamic* GetMaterialInstanceDynamic() const
	{
		if (IsValid(MaterialInstanceDynamic))
		{
			return MaterialInstanceDynamic;
		}
		return nullptr;
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(USparksRichFXPostProcessWorker, STATGROUP_Tickables);
	}

private:
	FSparksRichFXPostProcessWorkerData Data;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic = nullptr;

	virtual void Update(const UWorld& World, const float DeltaTime) const override;
};