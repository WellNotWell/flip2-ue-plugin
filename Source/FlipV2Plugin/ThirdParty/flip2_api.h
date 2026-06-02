#pragma once

#include "image.h"
#include <string>


enum class DecisionMode {
    OR,          // Match if AT LEAST ONE criterion is met
    AND,         // Match if BOTH criteria are met (Default)
    SIMILARITY,  // Evaluate ONLY using the classic similarity threshold
    ANOMALY      // Evaluate ONLY using the statistical anomaly percentage
};

struct FlipResult {
    // Per-pixel FLIP2 error heatmap
    Image errorMap;

    // Mean FLIP2 error across all pixels, in [0, 1]
    float meanError = 0.0f;

    // Similarity
    float similarityPercent = 0.0f;

    // --- Robust statistics (Median + MAD) ---
    float medianError = 0.0f;
    float mad = 0.0f;     // Median Absolute Deviation (raw)
    float stdDev = 0.0f;  // MAD-based σ estimate (1.4826 * MAD)
    float autoPixelThreshold = 0.0f;
    int failedPixelCount = 0;
    float failedPixelsPercent = 0.0f;
};

struct FlipOptions {
    // Pixels-per-degree
    // When <= 0, computed from the monitor parameters below
    float ppd = 0.0f;

    // Monitor parameters — used only when ppd <= 0
    float monitorDistance = 0.7f;  // metres
    float monitorWidth = 0.7f;     // metres
    int monitorResWidth = 3840;    // pixels

    // Produce the per-pixel heatmap
    bool computeErrorMap = false;

    // HDR evaluation mode
    bool hdr = false;

    // LDR evaluation mode
    bool ldr = false;

    // Tone-mapping pre-process: apply ACES filmic tone-mapping to convert
    // HDR linear values to SDR [0,1] before running FLIP, so the comparison
    // reflects what a user sees on a standard SDR display
    bool tonemap = false;
};

FlipResult computeFLIP(const Image& test, const Image& reference, const FlipOptions& opts = FlipOptions{});
