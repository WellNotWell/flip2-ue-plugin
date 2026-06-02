#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlipV2Types.h"
#include "FlipV2BlueprintLibrary.generated.h"

class UTexture2D;
class UTextureRenderTarget2D;

/**
 * Blueprint function library for FLIP v2 perceptual image comparison.
 */
UCLASS()
class FLIPV2PLUGIN_API UFlipV2BlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Compare two textures using the FLIP v2 perceptual metric.
	 * Both textures must have the same dimensions.
	 *
	 * @param TestTexture      The image to test
	 * @param ReferenceTexture The ground-truth reference image
	 * @param Options          Comparison options (thresholds, decision mode, etc.)
	 * @return Comparison result with similarity score, statistics, and optional heatmap
	 */
	UFUNCTION(BlueprintCallable, Category = "FLIP v2",
		meta = (DisplayName = "Compare Textures (FLIP v2)"))
	static FFlipResult CompareTextures(
		UTexture2D* TestTexture,
		UTexture2D* ReferenceTexture,
		const FFlipOptions& Options);

	/**
	 * Compare two render targets using the FLIP v2 perceptual metric.
	 * Useful for comparing SceneCapture outputs directly.
	 *
	 * @param TestRT      The render target to test
	 * @param ReferenceRT The ground-truth reference render target
	 * @param Options     Comparison options
	 * @return Comparison result
	 */
	UFUNCTION(BlueprintCallable, Category = "FLIP v2",
		meta = (DisplayName = "Compare Render Targets (FLIP v2)"))
	static FFlipResult CompareRenderTargets(
		UTextureRenderTarget2D* TestRT,
		UTextureRenderTarget2D* ReferenceRT,
		const FFlipOptions& Options);

	/**
	 * Compare a render target against a reference texture.
	 * Useful for comparing live rendering against a saved golden image.
	 *
	 * @param TestRT           The live render target to test
	 * @param ReferenceTexture The saved reference image
	 * @param Options          Comparison options
	 * @return Comparison result
	 */
	UFUNCTION(BlueprintCallable, Category = "FLIP v2",
		meta = (DisplayName = "Compare RT vs Texture (FLIP v2)"))
	static FFlipResult CompareRenderTargetVsTexture(
		UTextureRenderTarget2D* TestRT,
		UTexture2D* ReferenceTexture,
		const FFlipOptions& Options);

	/** Get a human-readable summary string from a FLIP result */
	UFUNCTION(BlueprintPure, Category = "FLIP v2",
		meta = (DisplayName = "Get FLIP Result Summary"))
	static FString GetResultSummary(const FFlipResult& Result);

	/** Quick check: are two textures perceptually matching with default settings? */
	UFUNCTION(BlueprintPure, Category = "FLIP v2",
		meta = (DisplayName = "Are Textures Matching? (FLIP v2)"))
	static bool AreTexturesMatching(
		UTexture2D* TestTexture,
		UTexture2D* ReferenceTexture);
};
