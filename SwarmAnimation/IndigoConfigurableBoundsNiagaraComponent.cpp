/* 
 * For portfolio use only.  Do not distribute.
 */



#include "IndigoConfigurableBoundsNiagaraComponent.h"

void UIndigoConfigurableBoundsNiagaraComponent::SetLocalBounds(const double Radius)
{
	this->LocalBounds = FBoxSphereBounds(UE::Math::TSphere(FVector(0, 0, 0), Radius)); 
}

void UIndigoConfigurableBoundsNiagaraComponent::SetLocalBounds(const FBox& LocalBoundingBox)
{
	this->LocalBounds = FBoxSphereBounds(LocalBoundingBox);
}

FBoxSphereBounds UIndigoConfigurableBoundsNiagaraComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return LocalBounds.TransformBy(LocalToWorld); 
}
