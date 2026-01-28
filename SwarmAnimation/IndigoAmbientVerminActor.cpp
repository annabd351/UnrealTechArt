/* 
 * For portfolio use only.  Do not distribute.
 */

 

#include "IndigoAmbientVerminActor.h"
#include "IndigoAmbientVerminBoundaryComponent.h"
#include "IndigoAmbientVerminProximityComponent.h"
#include "IndigoConfigurableBoundsNiagaraComponent.h"
#include "StudioLoadNotifierSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Characters/IndigoPlayerCharacter.h"
#include "Components/IndigoAbilitySystemComponent.h"
#include "Components/IndigoWeaponComponent.h"

const FName AIndigoAmbientVerminActor::kImpulseTriggeredName(TEXT("ImpulseTriggered"));
const FName AIndigoAmbientVerminActor::kImpulseLocationName(TEXT("ImpulseLocation"));
const FName AIndigoAmbientVerminActor::kImpulseAdditionalForceDirectionName(TEXT("ImpulseAdditionalForceDirection"));
const FName AIndigoAmbientVerminActor::kScaleToZeroName(TEXT("ScaleToZero"));
const FName AIndigoAmbientVerminActor::kPlayerNearbyName(TEXT("PlayerNearby"));
const FName AIndigoAmbientVerminActor::kPlayerPositionName(TEXT("PlayerPosition"));
const FName AIndigoAmbientVerminActor::kCollisionPlaneCenterName(TEXT("CollisionPlaneCenter"));
const FName AIndigoAmbientVerminActor::kCollisionPlaneNormalName(TEXT("CollisionPlaneNormal"));
const FName AIndigoAmbientVerminActor::kBoundaryRadiusName(TEXT("BoundaryRadius"));
const FName AIndigoAmbientVerminActor::kObstacleLocationsArrayName(TEXT("ObstacleLocations"));
const FName AIndigoAmbientVerminActor::kObstacleNormalsArrayName(TEXT("ObstacleNormals"));
const FName AIndigoAmbientVerminActor::kObstacleRadiiArrayName(TEXT("ObstacleRadii"));
const FName AIndigoAmbientVerminActor::kParticleCountName(TEXT("ParticleCount"));
const FName AIndigoAmbientVerminActor::kMeshScaleMinName(TEXT("MeshSizeMin"));
const FName AIndigoAmbientVerminActor::kMeshScaleMaxName(TEXT("MeshSizeMax"));
const FName AIndigoAmbientVerminActor::kFixedSurfaceOffsetName(TEXT("MeshOffset"));
const FName AIndigoAmbientVerminActor::kBoundaryHardnessName(TEXT("BoundaryHardness"));
const FName AIndigoAmbientVerminActor::kTintColorName(TEXT("TintColor"));
const FName AIndigoAmbientVerminActor::kBaseSpeedFactorName(TEXT("BaseSpeedFactor"));
const FName AIndigoAmbientVerminActor::kExcitedSpeedFactorName(TEXT("ExcitedSpeedFactor"));

AIndigoAmbientVerminActor::AIndigoAmbientVerminActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	BoundaryComponent = CreateDefaultSubobject<UIndigoAmbientVerminBoundaryComponent>(TEXT("Bounds"));
	BoundaryComponent->SetupAttachment(RootComponent);
	BoundaryComponent->SetSphereRadius(kInitialBoundaryComponentRadius);
	BoundaryComponent->SetLineThickness(kInitialLineThickness);
	
	NiagaraComponent = CreateDefaultSubobject<UIndigoConfigurableBoundsNiagaraComponent>(TEXT("NiagaraSystem"));
	NiagaraComponent->SetAutoActivate(false);
	NiagaraComponent->SetCastShadow(true);
	NiagaraComponent->SetupAttachment(RootComponent);

	ProximityComponent = CreateDefaultSubobject<UIndigoAmbientVerminProximityComponent>(TEXT("ProximityComponent"));
	ProximityComponent->SetupAttachment(RootComponent);
	ProximityComponent->SetSphereRadius(kInitialProximityComponentRadius);
	ProximityComponent->SetLineThickness(kInitialLineThickness);
}

void AIndigoAmbientVerminActor::SetNiagaraSystem(UNiagaraSystem* System)
{
	if (IsValid(System) && IsValid(NiagaraComponent))
	{
		NiagaraComponent->SetAsset(System);
	}
}

void AIndigoAmbientVerminActor::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->DeactivateImmediate();
	}

	if (IsValid(ProximityComponent))
	{
		ProximityComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &AIndigoAmbientVerminActor::ProximityComponentBeginOverlap);
		ProximityComponent->OnComponentEndOverlap.AddUniqueDynamic(this, &AIndigoAmbientVerminActor::ProximityComponentEndOverlap);
	}

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->OnDamageTaken.AddUniqueDynamic(this, &AIndigoAmbientVerminActor::DamageTakenCallback);
	}
	
	WorldLoadNotifierSubsystem = UStudioGameInstance::GetWorldSubsystem<UStudioLoadNotifierSubsystem>();
	if (WorldLoadNotifierSubsystem.IsValid() == false)
	{
		return;
	}

	if (WorldLoadNotifierSubsystem->IsLocationLoaded(GetTransform().GetLocation()))
	{
		InitAfterLoad();
	}
	else
	{
		UE_LOG(LogIndigoFauna, Log, TEXT("AIndigoAmbientVerminActor::BeginPlay (%s) -- Location NOT yet loaded.  Registering CallWhenLocationLoaded callback."), *GetName())
		FOnCellLoaded finishedDelegate;
		finishedDelegate.BindDynamic(this, &AIndigoAmbientVerminActor::InitAfterLoad);

		WaitingForLocationLoaded = true;
		WorldLoadNotifierSubsystem->CallWhenLocationLoaded(GetTransform().GetLocation(), finishedDelegate);
	}
}

void AIndigoAmbientVerminActor::InitAfterLoad()
{
	WaitingForLocationLoaded = false;

	if (IsValid(BoundaryComponent) == false)
	{
		return;
	}

	UE_LOG(LogIndigoFauna, Log, TEXT("AIndigoAmbientVerminActor::InitAfterLoad (%s) -- Location loaded."), *GetName());

	BoundaryComponent->UpdateBounds();
	BoundaryComponent->UpdateBoundaryState();
	
	UpdateNiagara();
	InitialProximityCheck();

	if (BoundaryComponent->CollisionPlaneValid == false)
	{
		UE_LOG(LogIndigoFauna, Warning, TEXT("AIndigoAmbientVerminActor::InitAfterLoad (%s) -- Invalid collision plane.  HitResult: Position: %s Normal: %s Actor: %s"),
			*GetName(), *BoundaryComponent->CollisionPlaneCenter.ToString(), *BoundaryComponent->CollisionPlaneNormal.ToString(), *BoundaryComponent->CollisionPlaneHitObject);
	}
}

void AIndigoAmbientVerminActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->DeactivateImmediate();
	}
	
	if (IsValid(ProximityComponent))
	{
		ProximityComponent->OnComponentBeginOverlap.RemoveAll(this);
		ProximityComponent->OnComponentEndOverlap.RemoveAll(this);
	}

	RemoveWeaponFiredCallback(UStatics::GetIndigoPlayerCharacter());

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->OnDamageTaken.RemoveAll(this);
	}

	if (WorldLoadNotifierSubsystem.IsValid())
	{
		WorldLoadNotifierSubsystem->UnregisterCallback(GetTransform().GetLocation());
	}
}

#if WITH_EDITOR
void AIndigoAmbientVerminActor::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.MemberProperty == nullptr)
	{
		return;
	}

	if (IsValid(BoundaryComponent))
	{
		BoundaryComponent->UpdateBounds();
		BoundaryComponent->SetNeedsUpdate();
	}
}
#endif

void AIndigoAmbientVerminActor::UpdateNiagara() const
{
	if (IsValid(NiagaraComponent) == false || IsValid(BoundaryComponent) == false)
	{
		return;
	}

	NiagaraComponent->DeactivateImmediate();

	if (BoundaryComponent->CollisionPlaneValid == false)
	{
		return;
	}

	NiagaraComponent->SetLocalBounds(BoundaryComponent->GetScaledSphereRadius() * kBoundsScaleApproximationFactor);

	NiagaraComponent->SetVariableBool(kImpulseTriggeredName, false);
	NiagaraComponent->SetVariableBool(kScaleToZeroName, false);
	
	NiagaraComponent->SetWorldLocation(BoundaryComponent->CollisionPlaneCenter);
	NiagaraComponent->SetVectorParameter(kCollisionPlaneCenterName, BoundaryComponent->CollisionPlaneCenter);
	NiagaraComponent->SetVectorParameter(kCollisionPlaneNormalName, BoundaryComponent->CollisionPlaneNormal);
	NiagaraComponent->SetFloatParameter(kBoundaryRadiusName, BoundaryComponent->GetScaledSphereRadius());
	NiagaraComponent->SetIntParameter(kParticleCountName, ParticleCount);
	NiagaraComponent->SetFloatParameter(kMeshScaleMaxName, MeshScaleMax);
	NiagaraComponent->SetFloatParameter(kMeshScaleMinName, MeshScaleMin);
	NiagaraComponent->SetFloatParameter(kFixedSurfaceOffsetName, FixedSurfaceOffset);
	NiagaraComponent->SetFloatParameter(kBoundaryHardnessName, BoundaryHardness);
	NiagaraComponent->SetColorParameter(kTintColorName, TintColor);
	NiagaraComponent->SetFloatParameter(kBaseSpeedFactorName, BaseSpeedFactor);
	NiagaraComponent->SetFloatParameter(kExcitedSpeedFactorName, ExcitedSpeedFactor);
	
	TArray<FVector> obstacleLocations;
	TArray<FVector> obstacleNormals;
	for (const FIndigoAmbientVerminObstacle& obstacle : BoundaryComponent->Obstacles)
	{
		obstacleLocations.Emplace(obstacle.Location);
		obstacleNormals.Emplace(obstacle.Normal);
	}

	TArray<float> uniformRadii;
	uniformRadii.Init(ObstacleCollisionRadius, obstacleLocations.Num());
	
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, kObstacleLocationsArrayName, obstacleLocations);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NiagaraComponent, kObstacleNormalsArrayName, obstacleNormals);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComponent, kObstacleRadiiArrayName, uniformRadii);
	NiagaraComponent->ActivateSystem();
}

void AIndigoAmbientVerminActor::TriggerScatterImpulse(const FVector& location, const FVector& additionalForceDirection)
{
	const UWorld* world = GetWorld();
	if (IsValid(NiagaraComponent) == false || IsValid(world) == false)
	{
		return;
	}

	// Send a pulse to the Niagara system.  System handles this as a one-shot.  This actor
	// handles the cooldown.
	if (bImpulseTriggeredActive == true)
	{
		// Pulse in progress
		return;
	}
	NiagaraComponent->SetVariableBool(kImpulseTriggeredName, true);
	NiagaraComponent->SetVariableVec3(kImpulseLocationName, location);

	// Could also set kImpulseAdditionalForceDirectionName, here, to fine-tune the impulse animation. 

	bImpulseTriggeredActive = true;

	// End the pulse after cooldown
	const FTimerDelegate delegate = FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		if (IsValid(NiagaraComponent))
		{
			NiagaraComponent->SetVariableBool(kImpulseTriggeredName, false);
			bImpulseTriggeredActive = false;
		}
	});
	world->GetTimerManager().SetTimer(ImpulseCooldown, delegate,FMath::Max(ScatterImpulseRetriggerCooldown, KINDA_SMALL_NUMBER),false);
}

void AIndigoAmbientVerminActor::ProximityComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent != ProximityComponent)
	{
		return;
	}

	const AIndigoPlayerCharacter* playerCharacter = Cast<AIndigoPlayerCharacter>(OtherActor);
	UIndigoWeaponComponent* playerWeaponComponent = IsValid(playerCharacter) ? playerCharacter->GetWeaponComponent() : nullptr;
	if (IsValid(playerWeaponComponent))
	{
		playerWeaponComponent->OnWeaponFired.AddUniqueDynamic(this, &AIndigoAmbientVerminActor::WeaponFiredCallback);
	}

	StartPlayerLocationUpdates(playerCharacter);
}

void AIndigoAmbientVerminActor::StartPlayerLocationUpdates(const AIndigoPlayerCharacter* playerCharacter)
{
	// Set up player location updates
	if (IsValid(playerCharacter) == false || IsValid(NiagaraComponent) == false)
	{
		return;
	}

	NiagaraComponent->SetVariableBool(kPlayerNearbyName, true);
	NiagaraComponent->SetVariableVec3(kPlayerPositionName, playerCharacter->GetActorLocation());

	// ReSharper disable once CppParameterMayBeConstPtrOrRef
	playerCharacter->GetRootComponent()->TransformUpdated.AddWeakLambda(this, [this, playerCharacter](USceneComponent* sceneComponent, EUpdateTransformFlags updateTransformFlags, ETeleportType teleportType)
	{
		if (IsValid(NiagaraComponent) && IsValid(sceneComponent))
		{
			NiagaraComponent->SetVariableVec3(kPlayerPositionName, sceneComponent->GetComponentLocation());
		}
	});
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AIndigoAmbientVerminActor::ProximityComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OverlappedComponent != ProximityComponent)
	{
		return;
	}

	RemoveWeaponFiredCallback(OtherActor);

	// Stop up player location updates
	const AIndigoPlayerCharacter* playerCharacter = Cast<AIndigoPlayerCharacter>(OtherActor);
	if (IsValid(playerCharacter))
	{
		playerCharacter->GetRootComponent()->TransformUpdated.RemoveAll(this);
	}

	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->SetVariableBool(kPlayerNearbyName, false);
	}
}

void AIndigoAmbientVerminActor::InitialProximityCheck()
{
	// When play starts, proximity state is indeterminate (standard UE behavior).  Force a check.

	const AIndigoPlayerCharacter* playerCharacter = UStatics::GetIndigoPlayerCharacter();
	if (IsValid(playerCharacter) == false || IsValid(ProximityComponent) == false || IsValid(BoundaryComponent) == false)
	{
		return;
	}

	const float playerDistSqToCenter = FVector::DistSquared(playerCharacter->GetActorLocation(), BoundaryComponent->CollisionPlaneCenter);
	const float proximityRadius = ProximityComponent->GetScaledSphereRadius();
	if (playerDistSqToCenter < proximityRadius * proximityRadius)
	{
		StartPlayerLocationUpdates(playerCharacter);
	}		
}

void AIndigoAmbientVerminActor::WeaponFiredCallback()
{
	const AIndigoPlayerCharacter* playerCharacter = UStatics::GetIndigoPlayerCharacter();
	if (IsValid(playerCharacter))
	{
		TriggerScatterImpulse(playerCharacter->GetActorLocation(), FVector::ZeroVector);
	}
}

void AIndigoAmbientVerminActor::RemoveWeaponFiredCallback(AActor* actor) const
{
	const AIndigoPlayerCharacter* playerCharacter = Cast<AIndigoPlayerCharacter>(actor);
	UIndigoWeaponComponent* playerWeaponComponent = IsValid(playerCharacter) ? playerCharacter->GetWeaponComponent() : nullptr;
	if (IsValid(playerWeaponComponent))
	{
		playerWeaponComponent->OnWeaponFired.RemoveAll(this);
	}
}

// Note: This plays the AOE reaction for *any* damage taken. No consideration of health, damage type, weapon, etc. 
void AIndigoAmbientVerminActor::DamageTakenCallback(const FCombatEventData& eventData)
{
	const UWorld* world = GetWorld();
	if (IsValid(NiagaraComponent) == false || IsValid(world) == false || IsValid(BoundaryComponent) == false)
	{
		return;
	}

	const FVector additionalForceDirection = IsValid(BoundaryComponent) ? BoundaryComponent->CollisionPlaneNormal : FVector::ZeroVector;
	TriggerScatterImpulse(eventData.DamageSource, additionalForceDirection);

	// Scale off particles
	NiagaraComponent->SetVariableBool(kScaleToZeroName, true);

	const FTimerDelegate delegate = FTimerDelegate::CreateWeakLambda(this, [this]
	{
		if (IsValid(NiagaraComponent))
		{
			NiagaraComponent->DeactivateImmediate();
		}
	});
	world->GetTimerManager().SetTimer(DeactivateAfterDeathTimer, delegate, kDeactivateOnDeathDelay, false);
}
