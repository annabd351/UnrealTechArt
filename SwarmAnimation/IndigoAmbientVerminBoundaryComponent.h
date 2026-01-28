/* 
 * For portfolio use only.  Do not distribute.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "IndigoAmbientVerminBoundaryComponent.generated.h"

class AIndigoAmbientVerminActor;

USTRUCT(BlueprintType)
struct FIndigoAmbientVerminObstacle
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vermin", Transient)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vermin", Transient)
	FVector Normal = FVector::UpVector;

	FIndigoAmbientVerminObstacle();
	FIndigoAmbientVerminObstacle(const FVector& Location, const FVector& Normal);

	friend bool operator==(const FIndigoAmbientVerminObstacle& lhs, const FIndigoAmbientVerminObstacle& rhs)
	{
		return lhs.Location.Equals(rhs.Location) && lhs.Normal.Equals(rhs.Normal);
	};
	
	friend bool operator!=(const FIndigoAmbientVerminObstacle& lhs, const FIndigoAmbientVerminObstacle& rhs)
	{
		return !(lhs.Location.Equals(rhs.Location) && lhs.Normal.Equals(rhs.Normal));
	};
	
	friend uint32 GetTypeHash(const FIndigoAmbientVerminObstacle& obstacle)
	{
		return HashCombine(GetTypeHash(obstacle.Location), GetTypeHash(obstacle.Normal));
	};
};

/*
 * Placement helper for vermin.  Displays system boundary, and finds nearby obstacles.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class Indigo_API UIndigoAmbientVerminBoundaryComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	static constexpr float kEditorTickInterval = 1.0 / 60.0;
	static constexpr float kRadialAngleIncrement = PI / 12.0;
	static constexpr float kBoundaryToSweepShapeRatio = 1.0 / 12.0;
	static constexpr float kCollisionPlaneMinNormalVerticality = 0.90;

	static const FColor kPlacementColorGood;
	static const FColor kPlacementColorBad;
	static const FColor kPlacementColorCaution;
	static const FColor kColorMidGray;
	static const FString kUnknownObjectMessageString;
	
	UIndigoAmbientVerminBoundaryComponent();

	// Center of ground plane for the vermin
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vermin", Transient)
	FVector CollisionPlaneCenter = FVector::ZeroVector;

	// Up vector of ground plane
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vermin", Transient)
	FVector CollisionPlaneNormal = FVector::UpVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vermin", Transient)
	bool CollisionPlaneValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vermin", Transient)
	FString CollisionPlaneHitObject = "";
	
	// Approximation of collision obstacles in the environment
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vermin", Transient)
	TSet<FIndigoAmbientVerminObstacle> Obstacles;

	void SetNeedsUpdate(const bool value = true);
	void UpdateBoundaryState();
	
protected:
#if WITH_EDITOR
	virtual void PostEditComponentMove(bool bFinished) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void BeginPlay() override;
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	// Don't update editor on first editor tick -- unnecessarily dirties levels. 
	bool bEditorNeedsUpdate = false;

	// Component initialization which can't happen in constructor
	bool bInitialized = false;
	
	void FindCollisionPlane(const UWorld* worldChecked, AIndigoAmbientVerminActor* owner);
	void FindObstacles(const UWorld* worldChecked, AIndigoAmbientVerminActor* owner);
	void DisplayObstacleLocations(const UWorld* worldChecked) const;
};
