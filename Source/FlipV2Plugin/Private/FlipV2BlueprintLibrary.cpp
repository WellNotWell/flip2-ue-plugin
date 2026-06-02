#include "FlipV2BlueprintLibrary.h"
#include "FlipV2ImageUtils.h"

THIRD_PARTY_INCLUDES_START
#include "flip2_api.h"
THIRD_PARTY_INCLUDES_END

/**
 * Map UE FFlipOptions → native FlipOptions and call computeFLIP.
 * Shared helper used by all Compare* functions.
 */
static FFlipResult RunFlipComparison(
	const Image& TestImg,
	const Image& RefImg,
	const FFlipOptions& Options)
{
	FFlipResult Result;

	// Dimension check
	if (TestImg.width != RefImg.width || TestImg.height != RefImg.height)
	{
		Result.VerdictReason = FString::Printf(
			TEXT("Dimension mismatch: test %dx%d vs reference %dx%d"),
			TestImg.width, TestImg.height, RefImg.width, RefImg.height);
		return Result;
	}

	if (TestImg.empty() || RefImg.empty())
	{
		Result.VerdictReason = TEXT("One or both images are empty");
		return Result;
	}

	// Map UE options to native options
	FlipOptions Opts;
	Opts.ppd              = Options.PPD;
	Opts.monitorDistance   = Options.MonitorDistance;
	Opts.monitorWidth     = Options.MonitorWidth;
	Opts.monitorResWidth  = Options.MonitorResWidth;
	Opts.computeErrorMap  = Options.bGenerateHeatmap;
	Opts.hdr              = Options.bForceHDR;
	Opts.ldr              = Options.bForceLDR;
	Opts.tonemap          = Options.bTonemap;

	FlipResult NativeResult = computeFLIP(TestImg, RefImg, Opts);

	// Transfer statistics
	Result.SimilarityPercent  = NativeResult.similarityPercent;
	Result.MeanError          = NativeResult.meanError;
	Result.MedianError        = NativeResult.medianError;
	Result.MAD                = NativeResult.mad;
	Result.StdDev             = NativeResult.stdDev;
	Result.AutoPixelThreshold = NativeResult.autoPixelThreshold;
	Result.FailedPixelCount   = NativeResult.failedPixelCount;
	Result.FailedPixelsPercent = NativeResult.failedPixelsPercent;

	// Decision logic
	const bool bPassBySimilarity = (NativeResult.similarityPercent / 100.0f) >= Options.Threshold;
	const bool bPassByAnomalies  = (NativeResult.failedPixelsPercent <= Options.MaxAnomalies);

	switch (Options.DecisionMode)
	{
	case EFlipDecisionMode::Or:
		Result.bIsMatch = bPassBySimilarity || bPassByAnomalies;
		if (Result.bIsMatch)
		{
			Result.VerdictReason = bPassBySimilarity
				? FString::Printf(TEXT("MATCH (Similarity >= %.1f%%)"), Options.Threshold * 100.0f)
				: FString::Printf(TEXT("MATCH (Anomalies <= %.1f%%)"), Options.MaxAnomalies);
		}
		else
		{
			Result.VerdictReason = TEXT("DIFFERENT (Failed both criteria in OR mode)");
		}
		break;

	case EFlipDecisionMode::And:
		Result.bIsMatch = bPassBySimilarity && bPassByAnomalies;
		if (Result.bIsMatch)
		{
			Result.VerdictReason = TEXT("MATCH (Passed BOTH criteria)");
		}
		else
		{
			Result.VerdictReason = FString::Printf(
				TEXT("DIFFERENT (Similarity %s, Anomalies %s)"),
				bPassBySimilarity ? TEXT("PASS") : TEXT("FAIL"),
				bPassByAnomalies  ? TEXT("PASS") : TEXT("FAIL"));
		}
		break;

	case EFlipDecisionMode::Similarity:
		Result.bIsMatch = bPassBySimilarity;
		Result.VerdictReason = Result.bIsMatch
			? TEXT("MATCH (Similarity mode)")
			: FString::Printf(TEXT("DIFFERENT (Similarity %.1f%% < %.1f%%)"),
				NativeResult.similarityPercent, Options.Threshold * 100.0f);
		break;

	case EFlipDecisionMode::Anomaly:
		Result.bIsMatch = bPassByAnomalies;
		Result.VerdictReason = Result.bIsMatch
			? TEXT("MATCH (Anomaly mode)")
			: FString::Printf(TEXT("DIFFERENT (Anomalies %.1f%% > %.1f%%)"),
				NativeResult.failedPixelsPercent, Options.MaxAnomalies);
		break;
	}

	// Generate heatmap texture if requested
	if (Options.bGenerateHeatmap && !NativeResult.errorMap.empty())
	{
		Result.HeatmapTexture = FlipImageToTexture(NativeResult.errorMap);
	}

	return Result;
}

FFlipResult UFlipV2BlueprintLibrary::CompareTextures(
	UTexture2D* TestTexture,
	UTexture2D* ReferenceTexture,
	const FFlipOptions& Options)
{
	if (!TestTexture || !ReferenceTexture)
	{
		FFlipResult Result;
		Result.VerdictReason = TEXT("Null texture input");
		UE_LOG(LogTemp, Warning, TEXT("FlipV2::CompareTextures — null texture input"));
		return Result;
	}

	const Image TestImg = TextureToFlipImage(TestTexture);
	const Image RefImg  = TextureToFlipImage(ReferenceTexture);

	return RunFlipComparison(TestImg, RefImg, Options);
}

FFlipResult UFlipV2BlueprintLibrary::CompareRenderTargets(
	UTextureRenderTarget2D* TestRT,
	UTextureRenderTarget2D* ReferenceRT,
	const FFlipOptions& Options)
{
	if (!TestRT || !ReferenceRT)
	{
		FFlipResult Result;
		Result.VerdictReason = TEXT("Null render target input");
		UE_LOG(LogTemp, Warning, TEXT("FlipV2::CompareRenderTargets — null render target input"));
		return Result;
	}

	const Image TestImg = RenderTargetToFlipImage(TestRT);
	const Image RefImg  = RenderTargetToFlipImage(ReferenceRT);

	return RunFlipComparison(TestImg, RefImg, Options);
}

FFlipResult UFlipV2BlueprintLibrary::CompareRenderTargetVsTexture(
	UTextureRenderTarget2D* TestRT,
	UTexture2D* ReferenceTexture,
	const FFlipOptions& Options)
{
	if (!TestRT || !ReferenceTexture)
	{
		FFlipResult Result;
		Result.VerdictReason = TEXT("Null input (render target or texture)");
		UE_LOG(LogTemp, Warning, TEXT("FlipV2::CompareRenderTargetVsTexture — null input"));
		return Result;
	}

	const Image TestImg = RenderTargetToFlipImage(TestRT);
	const Image RefImg  = TextureToFlipImage(ReferenceTexture);

	return RunFlipComparison(TestImg, RefImg, Options);
}

FString UFlipV2BlueprintLibrary::GetResultSummary(const FFlipResult& Result)
{
	return FString::Printf(
		TEXT("Similarity: %.1f%% | Mean: %.4f | Median: %.4f | Anomalies: %.1f%% (%d px) | %s — %s"),
		Result.SimilarityPercent,
		Result.MeanError,
		Result.MedianError,
		Result.FailedPixelsPercent,
		Result.FailedPixelCount,
		Result.bIsMatch ? TEXT("MATCH") : TEXT("DIFFERENT"),
		*Result.VerdictReason);
}

bool UFlipV2BlueprintLibrary::AreTexturesMatching(
	UTexture2D* TestTexture,
	UTexture2D* ReferenceTexture)
{
	FFlipOptions DefaultOptions;
	DefaultOptions.bGenerateHeatmap = false; // skip heatmap for quick check
	const FFlipResult Result = CompareTextures(TestTexture, ReferenceTexture, DefaultOptions);
	return Result.bIsMatch;
}
