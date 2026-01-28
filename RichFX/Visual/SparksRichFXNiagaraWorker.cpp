// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksRichFXNiagaraWorker.h"
#include "NiagaraFunctionLibrary.h"

void USparksRichFXNiagaraWorker::SpawnInWorld(const UWorld& World, const FSparksRichFXNiagaraWorkerData& ValidData, const FVector& Location, const FRotator& Rotation)
{
	// This uses Niagara's pooling system.  Another option is to save the component returned here, and make
	// our own pooling system.
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		&World,
		ValidData.NiagaraSystem,
		Location,
		Rotation,
		FVector::OneVector,
		true,
		true,
		ENCPoolMethod::AutoRelease
		);
}
