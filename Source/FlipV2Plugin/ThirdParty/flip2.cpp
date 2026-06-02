#include "flip2_api.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "FLIP_2.h"

static float monitorToPPD(float distance, float physWidth, int resWidth) {
    return FLIP::calculatePPD(distance, static_cast<float>(resWidth), physWidth);
}

static float acesTonemap(float x) {
    const float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    return std::max(0.0f, std::min(1.0f, (x * (a * x + b)) / (x * (c * x + d) + e)));
}

static void applyTonemap(FLIP::image<FLIP::color3>& img, int W, int H) {
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            FLIP::color3 c = img.get(x, y);
            c.r = acesTonemap(c.r);
            c.g = acesTonemap(c.g);
            c.b = acesTonemap(c.b);
            img.set(x, y, c);
        }
    }
}

static void jetColor(float t, float& r, float& g, float& b) {
    t = std::max(0.0f, std::min(1.0f, t));
    r = std::min(1.0f, std::max(0.0f, 1.5f - std::abs(4.0f * t - 3.0f)));
    g = std::min(1.0f, std::max(0.0f, 1.5f - std::abs(4.0f * t - 2.0f)));
    b = std::min(1.0f, std::max(0.0f, 1.5f - std::abs(4.0f * t - 1.0f)));
}

FlipResult computeFLIP(const Image& test, const Image& reference, const FlipOptions& opts) {
    if (test.width != reference.width || test.height != reference.height)
        throw std::runtime_error("Image dimensions do not match: test is " + std::to_string(test.width) + "x" +
                                 std::to_string(test.height) + ", reference is " + std::to_string(reference.width) +
                                 "x" + std::to_string(reference.height));

    const int W = test.width;
    const int H = test.height;

    FLIP::image<FLIP::color3> refImg(W, H);
    FLIP::image<FLIP::color3> tstImg(W, H);
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            refImg.set(x, y, FLIP::color3(reference.at(x, y, 0), reference.at(x, y, 1), reference.at(x, y, 2)));
            tstImg.set(x, y, FLIP::color3(test.at(x, y, 0), test.at(x, y, 1), test.at(x, y, 2)));
        }

    const bool tonemapped = opts.tonemap;
    if (tonemapped) {
        applyTonemap(refImg, W, H);
        applyTonemap(tstImg, W, H);
    }

    bool useHDR = false;
    if (tonemapped || opts.ldr) {
        useHDR = false;
    } else if (opts.hdr) {
        useHDR = true;
    } else {
        for (size_t i = 0; i < test.pixels.size() && !useHDR; ++i)
            if (test.pixels[i] > 1.0f + 1e-4f) useHDR = true;
        for (size_t i = 0; i < reference.pixels.size() && !useHDR; ++i)
            if (reference.pixels[i] > 1.0f + 1e-4f) useHDR = true;
    }

    if (!useHDR && !tonemapped) {
        refImg.sRGBToLinearRGB();
        tstImg.sRGBToLinearRGB();
    }

    FLIP::Parameters params;
    params.PPD =
            (opts.ppd > 0.0f) ? opts.ppd : monitorToPPD(opts.monitorDistance, opts.monitorWidth, opts.monitorResWidth);

    FLIP::image<float> errorMap(W, H, 0.0f);
    FLIP::evaluate(refImg, tstImg, useHDR, params, errorMap);

    {
        std::vector<float> luminance(W * H);
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x) {
                FLIP::color3 c = refImg.get(x, y);
                luminance[y * W + x] = 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b;
            }

        auto getLum = [&](int x, int y) -> float {
            x = std::max(0, std::min(W - 1, x));
            y = std::max(0, std::min(H - 1, y));
            return luminance[y * W + x];
        };

        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                float p00 = getLum(x - 1, y - 1), p10 = getLum(x, y - 1), p20 = getLum(x + 1, y - 1);
                float p01 = getLum(x - 1, y), p11 = getLum(x, y), p21 = getLum(x + 1, y);
                float p02 = getLum(x - 1, y + 1), p12 = getLum(x, y + 1), p22 = getLum(x + 1, y + 1);

                float gx = (p20 + 2.0f * p21 + p22) - (p00 + 2.0f * p01 + p02);
                float gy = (p02 + 2.0f * p12 + p22) - (p00 + 2.0f * p10 + p20);
                float sobelMag = std::sqrt(gx * gx + gy * gy);

                float weberContrast = sobelMag / (p11 + 0.05f);
                float maskVal = std::max(0.0f, std::min(0.85f, weberContrast));
                errorMap.set(x, y, errorMap.get(x, y) * (1.0f - maskVal));
            }
        }
    }

    float* errData = errorMap.getHostData();
    const int numPixels = W * H;

    std::vector<float> sorted(errData, errData + numPixels);
    std::nth_element(sorted.begin(), sorted.begin() + numPixels / 2, sorted.end());
    float medianError = sorted[numPixels / 2];

    for (int i = 0; i < numPixels; ++i) sorted[i] = std::abs(errData[i] - medianError);
    std::nth_element(sorted.begin(), sorted.begin() + numPixels / 2, sorted.end());
    float mad = sorted[numPixels / 2];

    float madSigma = 1.4826f * mad;

    double sum = 0.0;
    for (int i = 0; i < numPixels; ++i) sum += errData[i];
    auto meanError = static_cast<float>(sum / numPixels);

    float autoPixelThreshold = std::max(0.05f, medianError + 3.0f * madSigma);

    int failedPixelCount = 0;
    for (int i = 0; i < numPixels; ++i) {
        if (errData[i] > autoPixelThreshold) {
            failedPixelCount++;
        }
    }

    FlipResult result;
    result.meanError = meanError;
    result.similarityPercent = (1.0f - result.meanError) * 100.0f;
    result.medianError = medianError;
    result.mad = mad;
    result.stdDev = madSigma;
    result.autoPixelThreshold = autoPixelThreshold;
    result.failedPixelCount = failedPixelCount;
    result.failedPixelsPercent = (static_cast<float>(failedPixelCount) / static_cast<float>(numPixels)) * 100.0f;

    if (opts.computeErrorMap) {
        result.errorMap.width = W;
        result.errorMap.height = H;
        result.errorMap.pixels.resize(static_cast<size_t>(W) * H * 4);
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x) {
                const float t = errorMap.get(x, y);
                float r, g, b;
                jetColor(t, r, g, b);
                const int i = (y * W + x) * 4;
                result.errorMap.pixels[i + 0] = r;
                result.errorMap.pixels[i + 1] = g;
                result.errorMap.pixels[i + 2] = b;
                result.errorMap.pixels[i + 3] = 1.0f;
            }
    }
    return result;
}
