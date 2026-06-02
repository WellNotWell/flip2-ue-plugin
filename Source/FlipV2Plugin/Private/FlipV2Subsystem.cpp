#include "FlipV2Subsystem.h"
#include "FlipV2BlueprintLibrary.h"

void UFlipV2Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("FlipV2Subsystem initialized"));
}

void UFlipV2Subsystem::Deinitialize()
{
	ClearResult();
	Super::Deinitialize();
}

FFlipResult UFlipV2Subsystem::CompareTextures(
	UTexture2D* TestTexture,
	UTexture2D* ReferenceTexture,
	const FFlipOptions& Options)
{
	LastResult = UFlipV2BlueprintLibrary::CompareTextures(TestTexture, ReferenceTexture, Options);
	bHasResult = true;
	return LastResult;
}

FFlipResult UFlipV2Subsystem::CompareRenderTargets(
	UTextureRenderTarget2D* TestRT,
	UTextureRenderTarget2D* ReferenceRT,
	const FFlipOptions& Options)
{
	LastResult = UFlipV2BlueprintLibrary::CompareRenderTargets(TestRT, ReferenceRT, Options);
	bHasResult = true;
	return LastResult;
}

FFlipResult UFlipV2Subsystem::CompareRenderTargetVsTexture(
	UTextureRenderTarget2D* TestRT,
	UTexture2D* ReferenceTexture,
	const FFlipOptions& Options)
{
	LastResult = UFlipV2BlueprintLibrary::CompareRenderTargetVsTexture(TestRT, ReferenceTexture, Options);
	bHasResult = true;
	return LastResult;
}

void UFlipV2Subsystem::ClearResult()
{
	LastResult = FFlipResult();
	bHasResult = false;
}
