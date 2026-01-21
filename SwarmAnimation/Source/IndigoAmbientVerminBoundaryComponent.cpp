/* 
 * For portfolio use only.  Do not distribute.
 */

 
#include "IndigoAmbientVerminBoundaryComponent.h"
#include "IndigoAmbientVerminActor.h"

const FColor UIndigoAmbientVerminBoundaryComponent::kPlacementColorGood = FColor::Cyan;
const FColor UIndigoAmbientVerminBoundaryComponent::kPlacementColorBad = FColor::Red;
const FColor UIndigoAmbientVerminBoundaryComponent::kPlacementColorCaution = FColor::Yellow;
const FColor UIndigoAmbientVerminBoundaryComponent::kColorMidGray = FColor::FromHex(TEXT("888888"));
const FString UIndigoAmbientVerminBoundaryComponent::kUnknownObjectMessageString = FString(TEXT("<object invalid/unknown>"));


FIndigoAmbientVerminObstacle::FIndigoAmbientVerminObstacle()
{
}

FIndigoAmbientVerminObstacle::FIndigoAmbientVerminObstacle(const FVector& Location, const FVector& Normal)
	: Location(Location)
	, Normal(Normal)
{
}


UIndigoAmbientVerminBoundaryComponent::UIndigoAmbientVerminBoundaryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = kEditorTickInterval;
	bTickInEditor = true;
	ShapeColor = kColorMidGray;
	bSelectable = false;
	SetHiddenInGame(true);
	SetCanEverAffectNavigation(false);

	// Disable rotation and translation
	TransformUpdated.AddWeakLambda(this, [this](USceneComponent*, EUpdateTransformFlags, ETeleportType)
	{
		if (IsValid(this))
		{
			SetRelativeLocation(FVector::ZeroVector);
			SetRelativeRotation(FQuat::Identity);
		}
	});
}

#if WITH_EDITOR
void UIndigoAmbientVerminBoundaryComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);
	bEditorNeedsUpdate = true;
}

void UIndigoAmbientVerminBoundaryComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
		
	const AIndigoAmbientVerminActor* owner = Cast<AIndigoAmbientVerminActor>(GetOwner());
	if (IsValid(owner))
	{
		owner->UpdateNiagara();
	}
}
#endif

void UIndigoAmbientVerminBoundaryComponent::BeginPlay()
{
	Super::BeginPlay();
	SetMobility(EComponentMobility::Movable);
}

void UIndigoAmbientVerminBoundaryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	const UWorld* world = GetWorld();
	if (IsValid(world) == false)
	{
		return;
	}

	// Runtime and PIE init
	if (bInitialized == false && world->IsGameWorld())
	{
		bInitialized = true;

		SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetSimulatePhysics(false);

		PrimaryComponentTick.SetTickFunctionEnable(false);
		return;
	}
	
	// In-editor init
	if (bInitialized == false)
	{
		bInitialized = true;

		// This can't happen in constructor -- don't want virtual resolution at compile time
		SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	if (bEditorNeedsUpdate)
	{
		bEditorNeedsUpdate = false;

		UpdateBoundaryState();
		DisplayObstacleLocations(world);
	}
}

void UIndigoAmbientVerminBoundaryComponent::UpdateBoundaryState()
{
	const UWorld* world = GetWorld();
	AIndigoAmbientVerminActor* owner = Cast<AIndigoAmbientVerminActor>(GetOwner());
	if (IsValid(world) == false || IsValid(owner) == false)
	{
		return;
	}

	FindCollisionPlane(world, owner);
	if (CollisionPlaneValid)
	{
		FindObstacles(world, owner);
		ShapeColor = Obstacles.IsEmpty() ? kPlacementColorGood : kPlacementColorCaution;
	}
	else
	{
		ShapeColor = kPlacementColorBad;
	}

	if (world->IsEditorWorld())
	{
		MarkRenderStateDirty();
	}

	owner->UpdateNiagara();
}

void UIndigoAmbientVerminBoundaryComponent::FindCollisionPlane(const UWorld* worldChecked,  AIndigoAmbientVerminActor* owner)
{
	FHitResult hitResult;
	const FVector traceStartLocation = GetComponentLocation();
	const FVector traceEndLocation = traceStartLocation + GetScaledSphereRadius() * FVector::DownVector;
	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	queryParams.AddIgnoredActor(owner);

	bool hitFound = worldChecked->LineTraceSingleByChannel(
		hitResult,
		traceStartLocation,
		traceEndLocation,
		ECC_WorldStatic
	);

	CollisionPlaneCenter = hitResult.ImpactPoint;
	CollisionPlaneNormal = hitResult.ImpactNormal;
	CollisionPlaneValid = 
		hitFound &&
		hitResult.IsValidBlockingHit() &&
		FVector::DotProduct(hitResult.ImpactNormal, FVector::UpVector) > kCollisionPlaneMinNormalVerticality;
	CollisionPlaneHitObject = IsValid(hitResult.GetActor()) ? hitResult.GetActor()->GetName() : kUnknownObjectMessageString;
}

void UIndigoAmbientVerminBoundaryComponent::FindObstacles(const UWorld* worldChecked, AIndigoAmbientVerminActor* owner)
{
	Obstacles.Empty();

	if (CollisionPlaneValid == false)
	{
		return;
	}

	const FCollisionShape kSweepShape = FCollisionShape::MakeSphere(GetScaledSphereRadius() * kBoundaryToSweepShapeRatio);
	const FVector radialVector = FVector::CrossProduct(CollisionPlaneNormal, FVector::XAxisVector).GetSafeNormal() * GetScaledSphereRadius();

	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	queryParams.bTraceComplex = true;
	queryParams.bTraceIntoSubComponents = true;

	TArray<FHitResult> hitResults;

	for (float angle = 0.0; angle < TWO_PI; angle += kRadialAngleIncrement)
	{
		hitResults.Empty();

		const FVector traceVector = radialVector.RotateAngleAxisRad(angle, CollisionPlaneNormal);
		const FVector traceStart = CollisionPlaneCenter;

		const bool hitFound = worldChecked->SweepMultiByChannel(
			hitResults,
			traceStart,
			traceStart + traceVector,
			FQuat::Identity,
			ECC_WorldStatic,
			kSweepShape,
			queryParams
		);

		if (worldChecked->IsGameWorld() == false)
		{
			DrawDebugDirectionalArrow(
				worldChecked,
				traceStart,
				traceStart + traceVector,
				12.0,
				FColor::Cyan,
				false,
				kEditorTickInterval
			);
		}

		if (hitFound == false)
		{
			continue;
		}

		const float radiusSquared = GetScaledSphereRadius() * GetScaledSphereRadius();
		for (const FHitResult& result : hitResults)
		{
			if (result.bStartPenetrating ||
				(result.ImpactPoint - CollisionPlaneCenter).SquaredLength() > radiusSquared)
			{
				continue;
			}

			const FVector location = result.ImpactPoint;
			const FVector normal = result.ImpactNormal;
			Obstacles.Emplace({location, normal});
		}
	}
}

void UIndigoAmbientVerminBoundaryComponent::DisplayObstacleLocations(const UWorld* worldChecked) const
{
	const AIndigoAmbientVerminActor* owner = Cast<AIndigoAmbientVerminActor>(GetOwner());
	if (IsValid(owner) == false)
	{
		return;
	}

	// If collision plane is stale, don't display.  Don't display in-game.
	if (CollisionPlaneValid == false || worldChecked->IsGameWorld() == true) 
	{
		return;
	}

	for (const FIndigoAmbientVerminObstacle& obstacle : Obstacles)
	{
		DrawDebugCrosshairs(
			worldChecked,
			obstacle.Location,
			FRotator::ZeroRotator,
			10.0,
			FColor::Red,
			false,
			kEditorTickInterval
		);
	}
}

void UIndigoAmbientVerminBoundaryComponent::SetNeedsUpdate(const bool value)
{
	bEditorNeedsUpdate = value;
}