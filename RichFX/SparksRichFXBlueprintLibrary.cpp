// © 2024 Jar of Sparks, All Rights Reserved


#include "SparksRichFXBlueprintLibrary.h"
#include "SparksRuntime/RichFX/Visual/SparksRichFXPostProcessWorker.h"

void USparksRichFXBlueprintLibrary::DespawnFXDecalConstant(USparksRichFXDecalWorker* Worker)
{
	if (IsValid(Worker) == false)
	{
		UE_LOG(LogBlueprint, Warning, TEXT("USparksRichFXBlueprintLibrary::DespawnFXDecalConstant - Invalid worker"));
		return;
	}

	Worker->Resume();
}

void USparksRichFXBlueprintLibrary::DespawnFXPostProcessConstant(USparksRichFXPostProcessWorker* Worker)
{
	if (IsValid(Worker) == false)
	{
		UE_LOG(LogBlueprint, Warning, TEXT("USparksRichFXBlueprintLibrary::DespawnFXPostProcessConstant -- Invalid worker"));
		return;
	}

	Worker->Resume();
}

bool USparksRichFXBlueprintLibrary::GetFXPostProcessSettings(const USparksRichFXPostProcessWorker* Worker, FPostProcessSettings& Settings)
{
	if (IsValid(Worker) == false)
	{
		UE_LOG(LogBlueprint, Warning, TEXT("USparksRichFXBlueprintLibrary::GetFXPostProcessSettings -- Invalid worker"))
		return false;
	}

	const TOptional<FPostProcessSettings> RetrievedSettings = Worker->GetPostProcessSettings();
	if (RetrievedSettings.IsSet())
	{
		Settings = RetrievedSettings.GetValue();
		return true;
	}

	return false;
}

bool USparksRichFXBlueprintLibrary::SetFXPostProcessSettings(const USparksRichFXPostProcessWorker* Worker, const FPostProcessSettings& Settings)
{
	if (IsValid(Worker) == false)
	{
		UE_LOG(LogBlueprint, Warning, TEXT("USparksRichFXBlueprintLibrary::SetFXPostProcessSettings -- Invalid worker"))
		return false;
	}

	Worker->SetPostProcessSettings(Settings);
	return true;
}

USparksRichFXDecalWorker* USparksRichFXBlueprintLibrary::SpawnFXDecalCommon(
	const UObject* WorldContextObject,
	const FVector& Location,
	const FVector& Normal,
	const FSparksRichFXDecalWorkerData& Properties,
	const AActor* AttachToActor,
	const FName Socket,
	const ESparksRichFXWorkerExecutionState Mode
)
{
	USparksRichFXDecalWorker* Worker = SpawnFXWorker<USparksRichFXDecalWorker>(WorldContextObject);
	if (IsValid(Worker) == false)
	{
		return nullptr;
	}

	FHitResult HitResult;
	HitResult.ImpactPoint = Location;
	HitResult.ImpactNormal = Normal;

	Worker->Init(Properties);
	Worker->Activate(Mode, HitResult, AttachToActor, Socket);
	return Worker;
}

USparksRichFXPostProcessWorker* USparksRichFXBlueprintLibrary::SpawnFXPostCommon(
	const UObject* WorldContextObject,
	const FSparksRichFXPostProcessWorkerData& Properties,
	const ESparksRichFXWorkerExecutionState Mode
	)
{
	USparksRichFXPostProcessWorker* PostProcessWorker = SpawnFXWorker<USparksRichFXPostProcessWorker>(WorldContextObject);
	if (PostProcessWorker == nullptr)
	{
		return nullptr;
	}

	PostProcessWorker->Init(Properties);
	PostProcessWorker->Activate(Mode);
	return PostProcessWorker;
}
