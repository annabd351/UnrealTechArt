/* 
 * For portfolio use only.  Do not distribute.
 */

 
#include "IndigoAmbientVerminProximityComponent.h"
#include "IndigoAmbientVerminActor.h"
#include "IndigoAmbientVerminBoundaryComponent.h"

// Sets default values for this component's properties
UIndigoAmbientVerminProximityComponent::UIndigoAmbientVerminProximityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

#if WITH_EDITOR
	// Disable rotation and translation
	TransformUpdated.AddWeakLambda(this, [this](USceneComponent*, EUpdateTransformFlags, ETeleportType)
	{
		if (IsValid(this))
		{
			SetRelativeLocation(FVector::ZeroVector);
			SetRelativeRotation(FQuat::Identity);
		}
	});
#endif
}

#if WITH_EDITOR
void UIndigoAmbientVerminProximityComponent::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const AIndigoAmbientVerminActor* owner = Cast<AIndigoAmbientVerminActor>(GetOwner());
	if (IsValid(owner))
	{
		owner->UpdateNiagara();
	}
}
#endif

void UIndigoAmbientVerminProximityComponent::BeginPlay()
{
	static const FName kCollisionProfileName("OverlapAllDynamic");
	
	Super::BeginPlay();

	SetMobility(EComponentMobility::Movable);
	
	SetHiddenInGame(true);
	SetCollisionProfileName(kCollisionProfileName, true);
	SetGenerateOverlapEvents(true);
	SetSimulatePhysics(false);
}
