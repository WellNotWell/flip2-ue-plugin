#pragma once

#include "CoreMinimal.h"
#include "FlipV2Types.generated.h"

UENUM(BlueprintType)
enum class EFlipDecisionMode : uint8
{
	And        UMETA(DisplayName = "And (strictest)"),
	Or         UMETA(DisplayName = "Or (most lenient)"),
	Similarity UMETA(DisplayName = "Similarity only"),
	Anomaly    UMETA(DisplayName = "Anomaly only")
};

USTRUCT(BlueprintType)
struct FLIPV2PLUGIN_API FFlipOptions
{
	GENERATED_BODY()

	/** Similarity threshold in [0, 1]. Default: 0.95 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Threshold = 0.95f;

	/** Maximum allowed anomalous pixel percentage. Default: 3.0% */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float MaxAnomalies = 3.0f;

	/** Decision mode for match/different verdict */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP")
	EFlipDecisionMode DecisionMode = EFlipDecisionMode::And;

	/** Apply ACES filmic tone-mapping before comparison (for HDR content) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP")
	bool bTonemap = false;

	/** Force HDR evaluation mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP")
	bool bForceHDR = false;

	/** Force LDR evaluation mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP")
	bool bForceLDR = false;

	/** Generate the per-pixel error heatmap texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP")
	bool bGenerateHeatmap = true;

	/** Pixels per degree. When 0, computed from monitor parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP|Monitor", meta = (ClampMin = "0.0"))
	float PPD = 0.0f;

	/** Monitor viewing distance in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP|Monitor")
	float MonitorDistance = 0.7f;

	/** Monitor physical width in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP|Monitor")
	float MonitorWidth = 0.7f;

	/** Monitor horizontal resolution in pixels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FLIP|Monitor")
	int32 MonitorResWidth = 3840;
};

USTRUCT(BlueprintType)
struct FLIPV2PLUGIN_API FFlipResult
{
	GENERATED_BODY()

	/** Similarity percentage (0-100). Higher = more similar */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	float SimilarityPercent = 0.0f;

	/** Mean FLIP error in [0, 1] */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	float MeanError = 0.0f;

	/** Median error (robust central tendency) */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	float MedianError = 0.0f;

	/** Median Absolute Deviation (raw) */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	float MAD = 0.0f;

	/** MAD-based standard deviation estimate (1.4826 * MAD) */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	float StdDev = 0.0f;

	/** Auto-computed per-pixel anomaly threshold */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	float AutoPixelThreshold = 0.0f;

	/** Number of anomalous pixels */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	int32 FailedPixelCount = 0;

	/** Percentage of anomalous pixels */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	float FailedPixelsPercent = 0.0f;

	/** Final verdict: true = Match, false = Different */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	bool bIsMatch = false;

	/** Human-readable verdict reason */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	FString VerdictReason;

	/** Error heatmap texture (nullptr if heatmap generation was disabled) */
	UPROPERTY(BlueprintReadOnly, Category = "FLIP")
	TObjectPtr<UTexture2D> HeatmapTexture = nullptr;
};
