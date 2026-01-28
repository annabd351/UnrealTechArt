// © 2024 Jar of Sparks, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SparksRuntime/RichFX/SparksRichFXWorker.h"
#include "SparksRichFXCameraShakeWorker.generated.h"

/**
 * STUB
 *
 * Worker/asset for camera shake FX.
 *
 */
USTRUCT()
struct SPARKSRUNTIME_API FSparksRichFXCameraShakeData
{
	GENERATED_BODY()

	bool HasValidProperties() const
	{
		return true;
	}
};

UCLASS()
class SPARKSRUNTIME_API USparksRichFXCameraShakeWorker : public USparksRichFXWorker
{
	GENERATED_BODY()

public:

	// TODO: WIP
	void ActivateStatic(const FSparksRichFXCameraShakeData& ValidData, const FHitResult& ValidHitResult) const
	{
	}
};
