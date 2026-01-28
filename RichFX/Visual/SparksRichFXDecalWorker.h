// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SparksRuntime/RichFX/SparksRichFXWorker.h"
#include "Kismet/GameplayStatics.h"
#include "SparksRichFXDecalWorker.generated.h"

class USparksRichFXDecalWorker;

struct SPARKSRUNTIME_API FSparksRichFXDecalTimeline
{
	FSparksRichFXDecalTimeline()
	{
		bIsValid = false;
	}

	FSparksRichFXDecalTimeline(const USparksRichFXDecalWorker* Worker);

	TInterval<float> AlphaFadeOn;
	TInterval<float> AlphaFadeOff;
	TInterval<float> EmissiveFadeOff;

	bool bIsValid;

	static float NormalizedInRange(const TInterval<float>& Interval, const float Value)
	{
		return (Value - Interval.Min)/Interval.Size();
	}

	float GetEmissive(const float ElapsedTime) const
	{
		if (ElapsedTime > EmissiveFadeOff.Max)
		{
			return 0.0;
		}
		return EmissiveFadeOff.Contains(ElapsedTime) ? 1.0 - NormalizedInRange(EmissiveFadeOff, ElapsedTime) : 0.0;
	}

	float GetAlphaFadeOn(const float ElapsedTime) const
	{
		return AlphaFadeOn.Contains(ElapsedTime) ? NormalizedInRange(AlphaFadeOn, ElapsedTime) : 1.0;
	}

	float GetAlphaFadeOff(const float ElapsedTime) const
	{
		return AlphaFadeOff.Contains(ElapsedTime) ? 1.0 - NormalizedInRange(AlphaFadeOff, ElapsedTime) : 1.0;
	}
};

USTRUCT(BlueprintType)
struct SPARKSRUNTIME_API FSparksRichFXDecalWorkerData : public FSparksRichFXWorkerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 5.0;

	// Use min/max size to set a random size range for the decal. Set both to the same value for a fixed size.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MinSize = FVector(300.0,300.0,300.0);

	// Use min/max size to set a random size range for the decal. Set both to the same value for a fixed size.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MaxSize = FVector(300.0,300.0,300.0);

	// Randomly rotate the decal around its apparent normal axis. Set both to the same value for no rotation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinRotation = -180;

	// Randomly rotate the decal around its apparent normal axis. Set both to the same value for no rotation.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRotation = 180;

	// Deprecated property from earlier version
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = "false", EditConditionHides))
	TObjectPtr<UMaterialInterface> Material;

	// If there are multiple materials, one is chosen at random.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UMaterialInterface>> Materials;

	UPROPERTY(VisibleAnywhere)
	bool bUseFadeOn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = bUseFadeOn, EditConditionHides))
	float FadeOnDuration = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = bUseFadeOn, EditConditionHides))
	FName FadeOnMaterialParamName = TEXT("Alpha Fade In");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseEmissive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = bUseEmissive, EditConditionHides))
	float EmissiveDuration = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = bUseEmissive, EditConditionHides))
	float EmissiveFadeDuration = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = bUseEmissive, EditConditionHides))
	FName EmissiveFadeDurationMaterialParamName = TEXT("Emissive Fade Out");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseFadeOff = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = bUseFadeOff, EditConditionHides))
	float FadeOffDuration = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition = bUseFadeOff, EditConditionHides))
	FName FadeOffMaterialParamName = TEXT("Alpha Fade Out");

	// Decals may be faded out (using the FadeOff params) and removed if they are beyond this distance from player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CullDistance = 10000.0;

	// Decals may be faded out (using the FadeOff params) and removed once they've existed for this amount of time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double CullLifespan = 10.0 * 60.0;

	// Decal fade timing params: see https://jarofsparks.atlassian.net/wiki/x/KQDwDg

	virtual bool HasValidProperties() const override
	{
		bool bMaterialsValid = true;
		for (const UMaterialInterface* MaterialInterface : Materials)
		{
			bMaterialsValid = bMaterialsValid && IsValid(MaterialInterface);
		}
		return FMath::IsNearlyZero(Duration) == false && bMaterialsValid;
	}
};

UCLASS(BlueprintType)
class SPARKSRUNTIME_API USparksRichFXDecalWorker : public USparksRichFXWorker
{
	GENERATED_BODY()

public:
	USparksRichFXDecalWorker(const FObjectInitializer& ObjectInitializer);

	void Init(const FSparksRichFXDecalWorkerData& NewData)
	{
		Data = NewData;
		CullDistanceSquared = Data.CullDistance * Data.CullDistance;
	}

	const FSparksRichFXDecalWorkerData& GetData() const
	{
		return Data;
	}

	virtual bool Activate(const ESparksRichFXWorkerExecutionState InitialState, const FHitResult& ValidHitResult, const AActor* AttachToActor = nullptr, const FName Socket = NAME_None) override;
	virtual void Deactivate() override;
	virtual bool Resume() override;
	virtual void RequestRelease(const double RequestedAtTime) override;

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(USparksRichFXDecalWorker, STATGROUP_Tickables);
	}

protected:
	virtual void Update(const UWorld& World, const float DeltaTime) const override;

private:
	FSparksRichFXDecalTimeline Timeline;
	FSparksRichFXDecalWorkerData Data;
	double ActivationTime;
	float CullDistanceSquared;

	bool CullLifespanExpired(const double CurrentTime) const
	{
		return CurrentTime - ActivationTime > Data.CullLifespan;
	}

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstanceDynamic = nullptr;

	// TODO: Algorithm for determining sort order.
	// ReSharper disable once CppMemberFunctionMayBeStatic
	int32 GetSortOrder()
	{
		return 0;
	}

	static bool IsInPlayerCameraFrustum(const UObject* WorldContextObject, const FBoxSphereBounds& Bounds);

	static APawn* GetLocalPlayerCharacter(const UObject* WorldContext)
	{
		const APlayerController* PlayerController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(WorldContext, 0));
		return PlayerController ? PlayerController->GetPawn() : nullptr;
	}
};
