/* 
 * For portfolio use only.  Do not distribute.
 */
 
#pragma once

#include "CoreMinimal.h"
#include "Actors/IndigoAbilityActor.h"
#include "GameFramework/Actor.h"
#include "Templates/TypeHash.h"
#include "IndigoAmbientVerminActor.generated.h"

class UStudioLoadNotifierSubsystem;
class UIndigoConfigurableBoundsNiagaraComponent;
class UNiagaraSystem;
struct FCombatEventData;
class UIndigoAbilitySystemComponent;
class UIndigoAmbientVerminProximityComponent;
class UIndigoAmbientVerminBoundaryComponent;
class UNiagaraComponent;
class AIndigoPlayerCharacter;

/**
 * Base class for low-cost, very minimally interactive, ambient, VAT-based vermin (roaches, rats, etc.).
 *
 * Derived classes specifically do not support:
 * - Collision tracing, navigation
 * - Any sort of general interaction, except the cases specifically defined here
 * - Time manipulation
 *
 * These are very simple, particle-based assets which add life and motion to the environment with very little cost.
 */
UCLASS(Blueprintable)
class Indigo_API AIndigoAmbientVerminActor : public AIndigoAbilityActor
{
	GENERATED_BODY()

public:
	// Niagara parameter names
	static const FName kImpulseTriggeredName;
	static const FName kImpulseLocationName;
	static const FName kImpulseAdditionalForceDirectionName;
	static const FName kScaleToZeroName;
	static const FName kPlayerNearbyName;
	static const FName kPlayerPositionName;
	static const FName kCollisionPlaneCenterName;
	static const FName kCollisionPlaneNormalName;
	static const FName kBoundaryRadiusName;
	static const FName kObstacleLocationsArrayName;
	static const FName kObstacleNormalsArrayName;
	static const FName kObstacleRadiiArrayName;
	static const FName kParticleCountName;
	static const FName kMeshScaleMinName;
	static const FName kMeshScaleMaxName;
	static const FName kFixedSurfaceOffsetName;
	static const FName kBoundaryHardnessName;
	static const FName kTintColorName;
	static const FName kBaseSpeedFactorName;
	static const FName kExcitedSpeedFactorName;
	
	static constexpr float kDeactivateOnDeathDelay = 10.0;
	static constexpr float kInitialProximityComponentRadius = 400.0;
	static constexpr float kBoundsScaleApproximationFactor = 2.0;
	static constexpr float kInitialBoundaryComponentRadius = 250.0;
	static constexpr float kInitialLineThickness = 1.5;

	AIndigoAmbientVerminActor();

	// System detects likely obstacles in the environment.  They are marked with red crosshairs.  This is the collision boundary radius around each obstacle point.   
	UPROPERTY(BlueprintReadWrite, Category = "Vermin")
	float ObstacleCollisionRadius = 1.0;

	// Number of vermin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vermin")
	int32 ParticleCount = 25;

	// Min/max relative scale of instanced meshes.  Distribution is weighted to generate more small instances than large ones.  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vermin", meta=(ClampMin="1.0", ClampMax="10.0"))
	float MeshScaleMin = 2.5;

	// Min/max relative scale of instanced meshes.  Distribution is weighted to generate more small instances than large ones.  
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vermin", meta=(ClampMin="1.0", ClampMax="10.0"))
	float MeshScaleMax = 8.0;

	// Tint the rodents by multiplying this into the material's base color.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vermin")
	FLinearColor TintColor = FLinearColor::White;

	// If vermin are "floating" above the collision surface -- or sinking into it -- adjust this number.  Offsets position along
	// normal direction at which particles are renderer.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vermin")
	float FixedSurfaceOffset = 0.0;

	// "Porousness" of spherical boundary.  If zero, vermin stay within the sphere.  Increase this value to allow them to wander
	// outside the boundary.  Note that vermin can always pass through the boundary to avoid the player or their weapon fire.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vermin")
	float BoundaryHardness = 0.5;

	// Relative speed when vermin are doing nothing in particular.
	// NOTE: Changing this can cause limb movement to be out of sync with root motion.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vermin", AdvancedDisplay)
	float BaseSpeedFactor = 1.0;
	
	// Speed when vermin are being chased, or reacting to weapon fire and AOE damage.
	// NOTE: Changing this can cause limb movement to be out of sync with root motion.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vermin", AdvancedDisplay)
	float ExcitedSpeedFactor = 2.0;

	// Cooldown after player weapon is fired or AOE damage is received.  Inhibit additional triggers.
	UPROPERTY(BlueprintReadWrite, Category = "Vermin")
	float ScatterImpulseRetriggerCooldown = 0.5;
	
	UPROPERTY(VisibleAnywhere, Category = "Vermin")
	TObjectPtr<UIndigoAmbientVerminBoundaryComponent> BoundaryComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Vermin")
	TObjectPtr<UIndigoConfigurableBoundsNiagaraComponent> NiagaraComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Vermin")
	TObjectPtr<UIndigoAmbientVerminProximityComponent> ProximityComponent = nullptr;

	void UpdateNiagara() const;
	
	UFUNCTION()
	void ProximityComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void ProximityComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void WeaponFiredCallback();

	UFUNCTION()
	void DamageTakenCallback(const FCombatEventData& eventData);

	UFUNCTION(BlueprintCallable)
	void SetNiagaraSystem(UNiagaraSystem* System);

	UFUNCTION()
	void InitAfterLoad();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void TriggerScatterImpulse(const FVector& location, const FVector& additionalForceDirection);
	void RemoveWeaponFiredCallback(AActor* actor) const;
	void StartPlayerLocationUpdates(const AIndigoPlayerCharacter* playerCharacter);
	void InitialProximityCheck();
	bool bImpulseTriggeredActive = false;
	FTimerHandle ImpulseCooldown;
	FTimerHandle DeactivateAfterDeathTimer;

	TWeakObjectPtr<UStudioLoadNotifierSubsystem> WorldLoadNotifierSubsystem;

	UPROPERTY(VisibleInstanceOnly, Category = "Debug")
	bool WaitingForLocationLoaded = false;
};
