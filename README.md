# FLIP v2 — Perceptual Image Comparison Plugin for Unreal Engine
[![Main framework](https://img.shields.io/badge/main_framework-gpu--agnostic--testing--framework-311B92?logo=github)](https://github.com/WellNotWell/gpu-agnostic-testing-framework)
![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-6A1B9A?logo=unrealengine&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows-4A148C?logo=windows)
[![License](https://img.shields.io/badge/license-MIT-388E3C)](LICENSE)

An Unreal Engine 5.7 plugin that brings the [FLIP v2](https://github.com/WellNotWell/gpu-agnostic-testing-framework) perceptual image difference metric into the editor and runtime. Compare textures and render targets the way humans perceive differences.

## Features

- **Three comparison modes** — Texture vs Texture, RenderTarget vs RenderTarget, RenderTarget vs Texture
- **Per-pixel error heatmap** — generated as a `UTexture2D`, ready to display on UI or in-scene materials
- **LDR and HDR support** — optional ACES tone-mapping for HDR content
- **Configurable thresholds** — similarity percentage, anomaly limits, four decision modes
- **Monitor-aware PPD** — pixels-per-degree computed from viewing distance, monitor size, and resolution
- **Blueprint-friendly** — all functions exposed as Blueprint nodes, no C++ required
- **Subsystem with caching** — `UFlipV2Subsystem` stores the last result for retrieval in subsequent frames

## Installation

1. Clone this repository into your project's `Plugins/` directory **with submodules**:
   ```bash
   cd YourProject/Plugins
   git clone --recurse-submodules https://github.com/WellNotWell/flip2-ue-plugin.git FlipV2
   ```
   If you cloned without `--recurse-submodules`, initialize it afterwards:
   ```bash
   cd YourProject/Plugins/FlipV2
   git submodule update --init --recursive
   ```
2. Regenerate project files (right-click `.uproject` → Generate Visual Studio project files).
3. Build the project. The plugin is enabled by default.

The canonical FLIP v2 implementation (`flip2.cpp`, `FLIP_2.h`, `flip2_api.h`, `image.h`) lives in the [`gpu-agnostic-testing-framework`](https://github.com/WellNotWell/gpu-agnostic-testing-framework) repository and is pulled in as a git submodule under `ThirdParty/flip-framework/`. The plugin and the standalone CLI tool build from the same sources.

## Architecture

```
Plugins/FlipV2/
├── FlipV2.uplugin
├── ThirdParty/
│   └── flip-framework/                      ← submodule (canonical sources)
│       ├── flip_tool_2/src/
│       │   ├── FLIP_2.h
│       │   ├── flip2.cpp
│       │   └── flip2_api.h
│       └── flip_image/image.h
└── Source/FlipV2Plugin/
    ├── FlipV2Plugin.Build.cs                ← adds include paths into the submodule
    ├── Private/
    │   ├── Flip2Bridge.cpp                  ← trampoline TU: #undef PI/MAX/MIN/min/max → #include "flip2.cpp"
    │   └── ... (plugin sources)
    └── Public/...
```

The canonical `FLIP_2.h` uses identifiers like `PI`, `MIN`, `MAX`, `min`, `max` that collide with macros from `Windows.h` / UE headers. Rather than forking `FLIP_2.h`, the plugin compiles the upstream `flip2.cpp` through a thin trampoline (`Flip2Bridge.cpp`) that neutralizes the macros before the include. The framework sources stay untouched.

## Quick Start (Blueprints)

### Basic comparison

1. In any Blueprint graph, add the **Compare Textures (FLIP v2)** node
2. Connect a **Make FlipOptions** node to the `Options` pin (required — it's passed by reference)
3. Wire two `UTexture2D` assets to `Test Texture` and `Reference Texture`
4. Pass the `Return Value` to **Get FLIP Result Summary** → **Print String**

```
Event BeginPlay
    → Compare Textures (FLIP v2)
        ├── Test Texture: [your test image]
        ├── Reference Texture: [your reference image]
        └── Options: Make FlipOptions (defaults are fine)
    → Get FLIP Result Summary
    → Print String
```

### Quick match check

Use **Are Textures Matching? (FLIP v2)** for a simple `true`/`false` answer with default settings (95% similarity threshold, max 3% anomalies).

## Blueprint Nodes

| Node                                 | Description                                                      |
|--------------------------------------|------------------------------------------------------------------|
| **Compare Textures (FLIP v2)**       | Compare two `UTexture2D` assets                                  |
| **Compare Render Targets (FLIP v2)** | Compare two `UTextureRenderTarget2D` (e.g. SceneCapture outputs) |
| **Compare RT vs Texture (FLIP v2)**  | Compare a live render target against a saved reference texture   |
| **Are Textures Matching? (FLIP v2)** | Quick `bool` check with default settings                         |
| **Get FLIP Result Summary**          | Format `FFlipResult` into a human-readable string                |

## When to Use Which Function

| Scenario                                                 | Function                       |
|----------------------------------------------------------|--------------------------------|
| Compare two imported images from Content Browser         | `CompareTextures`              |
| Compare two live SceneCapture outputs in real time       | `CompareRenderTargets`         |
| Compare live rendering against a saved golden screenshot | `CompareRenderTargetVsTexture` |
| Just need a quick yes/no answer                          | `AreTexturesMatching`          |

## Options (FFlipOptions)

| Parameter          | Default  | Description                                                             |
|--------------------|----------|-------------------------------------------------------------------------|
| `Threshold`        | 0.95     | Similarity threshold (0–1). 0.95 = images must be 95% similar           |
| `MaxAnomalies`     | 3.0      | Maximum allowed anomalous pixels (%)                                    |
| `DecisionMode`     | And      | `And` (both criteria), `Or` (either), `Similarity only`, `Anomaly only` |
| `bTonemap`         | false    | Apply ACES filmic tone-mapping before comparison (for HDR)              |
| `bForceHDR`        | false    | Force HDR evaluation mode                                               |
| `bForceLDR`        | false    | Force LDR evaluation mode                                               |
| `bGenerateHeatmap` | true     | Generate per-pixel error heatmap texture                                |
| `PPD`              | 0 (auto) | Pixels per degree. 0 = computed from monitor parameters                 |
| `MonitorDistance`  | 0.7 m    | Viewing distance to monitor                                             |
| `MonitorWidth`     | 0.7 m    | Physical monitor width                                                  |
| `MonitorResWidth`  | 3840     | Monitor horizontal resolution                                           |

## Result (FFlipResult)

| Field                 | Description                                                               |
|-----------------------|---------------------------------------------------------------------------|
| `SimilarityPercent`   | Similarity 0–100% (100 = identical)                                       |
| `MeanError`           | Mean FLIP error (0–1)                                                     |
| `MedianError`         | Median error (robust to outliers)                                         |
| `MAD`                 | Median Absolute Deviation                                                 |
| `StdDev`              | MAD-based standard deviation (1.4826 * MAD)                               |
| `FailedPixelCount`    | Number of anomalous pixels                                                |
| `FailedPixelsPercent` | Percentage of anomalous pixels                                            |
| `bIsMatch`            | Final verdict: `true` = Match, `false` = Different                        |
| `VerdictReason`       | Human-readable explanation of the verdict                                 |
| `HeatmapTexture`      | Per-pixel error heatmap as `UTexture2D` (blue = similar, red = different) |

## Displaying the Heatmap

The heatmap is not displayed automatically. To visualize it:

1. Create a **Material** with a `Texture Parameter` named "Heatmap"
2. After comparison, use **Create Dynamic Material Instance** on a plane or UI widget
3. Call **Set Texture Parameter Value** with the `HeatmapTexture` from the result

## Using the Subsystem

`UFlipV2Subsystem` is a `GameInstanceSubsystem` — it is created automatically. Access it in Blueprints via **Get FlipV2 Subsystem**. It provides the same comparison functions but caches the last result, so you can call `GetLastResult()` in subsequent frames (e.g. to display the heatmap on a UI widget after a comparison).

## Requirements

- Unreal Engine 5.7+
- Windows (tested on Win64 with NVIDIA RTX 3050 Ti)

## License

MIT License. See [LICENSE](LICENSE) for details.

This plugin implements the enhanced FLIP v2 metric developed as part of a thesis project, building upon the original [FLIP](https://github.com/NVlabs/flip) by NVIDIA (BSD-3-Clause).

## Author

Olesia Grediushko — [GitHub](https://github.com/WellNotWell)
