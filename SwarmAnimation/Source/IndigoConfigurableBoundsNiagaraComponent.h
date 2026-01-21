/* 
 * For portfolio use only.  Do not distribute.
 */


#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "IndigoConfigurableBoundsNiagaraComponent.generated.h"

// Niagara component with configurable bounds.

// Use case:  Niagara systems often contain particle instances which spill out beyond the component bounds.
// If the component bounds are outside the camera frustum, the entire system is culled and the particles appear to
// pop off.  This class allows the bounds to be set programmatically to prevent this happening.

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class Indigo_API UIndigoConfigurableBoundsNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	void SetLocalBounds(const double Radius);
	void SetLocalBounds(const FBox& LocalBoundingBox);
	
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

private:
	FBoxSphereBounds LocalBounds;
};
