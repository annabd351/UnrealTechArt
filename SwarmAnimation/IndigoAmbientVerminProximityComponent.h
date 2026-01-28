/* 
 * For portfolio use only.  Do not distribute.
 */
  
#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "IndigoAmbientVerminProximityComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class Indigo_API UIndigoAmbientVerminProximityComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UIndigoAmbientVerminProximityComponent();

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void BeginPlay() override;
};
