#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FlipV2Types.h"
#include "FlipV2Subsystem.generated.h"

class UTexture2D;
class UTextureRenderTarget2D;

/**
 * Game Instance Subsystem providing convenient access to FLIP v2 comparison.
 */
UCLASS()
class FLIPV2PLUGIN_API UFlipV2Subsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Compare two UTexture2D assets. Stores the result internally. */
	UFUNCTION(BlueprintCallable, Category = "FLIP v2")
	FFlipResult CompareTextures(
		UTexture2D* TestTexture,
		UTexture2D* ReferenceTexture,
		const FFlipOptions& Options);

	/** Compare two render targets. Stores the result internally. */
	UFUNCTION(BlueprintCallable, Category = "FLIP v2")
	FFlipResult CompareRenderTargets(
		UTextureRenderTarget2D* TestRT,
		UTextureRenderTarget2D* ReferenceRT,
		const FFlipOptions& Options);

	/** Compare a live render target against a saved reference texture. */
	UFUNCTION(BlueprintCallable, Category = "FLIP v2")
	FFlipResult CompareRenderTargetVsTexture(
		UTextureRenderTarget2D* TestRT,
		UTexture2D* ReferenceTexture,
		const FFlipOptions& Options);


	/** Return the result of the most recent comparison. */
	UFUNCTION(BlueprintPure, Category = "FLIP v2")
	const FFlipResult& GetLastResult() const { return LastResult; }

	/** Has at least one comparison been run this session? */
	UFUNCTION(BlueprintPure, Category = "FLIP v2")
	bool HasResult() const { return bHasResult; }

	/** Clear the stored result and free the heatmap texture reference. */
	UFUNCTION(BlueprintCallable, Category = "FLIP v2")
	void ClearResult();

	/** Default options used when callers don't supply their own. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP v2")
	FFlipOptions DefaultOptions;

private:
	/** Cached result of the last comparison. */
	UPROPERTY()
	FFlipResult LastResult;

	/** Whether at least one comparison has been performed. */
	bool bHasResult = false;
};
