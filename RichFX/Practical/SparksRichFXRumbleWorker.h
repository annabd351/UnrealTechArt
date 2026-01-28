// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SparksRuntime/RichFX/SparksRichFXWorker.h"
#include "SparksRichFXRumbleWorker.generated.h"

/**
 * STUB
 *
 * Worker for rumble FX.
 *
 */
USTRUCT()
struct SPARKSRUNTIME_API FSparksRichFXRumbleWorkerData
{
	GENERATED_BODY()

	bool HasValidProperties() const
	{
		return true;
	}
};

UCLASS()
class SPARKSRUNTIME_API USparksRichFXRumbleWorker : public USparksRichFXWorker
{
	GENERATED_BODY()

public:

	// Stub
	void ActivateStatic(const FSparksRichFXRumbleWorkerData& ValidData, const FHitResult& ValidHitResult) const
	{
	}
};
