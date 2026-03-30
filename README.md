# RC-20 Clone

A multi-effect audio plugin inspired by XLN Audio's RC-20 Retro Color, built with [JUCE 8](https://juce.com/) and C++17. Six serial processing modules add analog and vintage character — from subtle coloration at low settings to heavy transformation when pushed hard.

**Signal chain:**  Noise → Wobble → Distortion → Space → Magic → Limit

> **Status:** Phase 1 complete — the plugin loads, passes audio cleanly, and exposes the full parameter tree. DSP for each module is stubbed; implementation is in progress.

---

## Modules

| # | Module       | Description                                           | Modes                                |
|---|-------------|-------------------------------------------------------|--------------------------------------|
| 1 | **Noise**      | Analog background texture (hiss, crackle, hum)       | Tape Hiss · Vinyl Crackle · Electrical Hum |
| 2 | **Wobble**     | Wow & flutter pitch/time instability via modulated delay | Slow · Medium · Fast                |
| 3 | **Distortion** | Nonlinear saturation with 4× oversampling             | Tape · Tube · Transistor             |
| 4 | **Space**      | BBD delay + Schroeder reverb with modulated lines     | Room · Hall · Plate                  |
| 5 | **Magic**      | Lo-fi degradation (bit crush / sample rate reduction) | Bit Crush · Decimate · Combined      |
| 6 | **Limit**      | Output limiter with soft knee and subtle saturation   | Soft · Medium · Hard                 |

Each module has a **bypass toggle**, a **primary amount knob**, and a **mode selector**. Space adds dedicated **Size** and **Tone** controls.

### Global controls

- **Drift** — shared instability signal (from `DriftGenerator`) that fans out to all modules
- **Output Level** — final gain stage (−24 dB to +6 dB)
- **Preset selector** — save/load parameter snapshots as XML files

---

## Architecture

```
source/
├── PluginProcessor.h/.cpp      Main processor — owns module chain and DriftGenerator
├── PluginEditor.h/.cpp         Main editor — lays out module strip and global controls
├── parameters/
│   └── ParameterIDs.h          Single source of truth for all APVTS parameter ID strings
├── modules/
│   ├── EffectModule.h          Base class (+ ProcessContext, ProcessingMode interfaces)
│   ├── NoiseModule.h/.cpp
│   ├── WobbleModule.h/.cpp
│   ├── DistortionModule.h/.cpp
│   ├── SpaceModule.h/.cpp
│   ├── MagicModule.h/.cpp
│   └── LimitModule.h/.cpp
├── dsp/
│   ├── DriftGenerator.h/.cpp   Multi-LFO + noise drift signal generator
│   └── ModulatedDelayLine.h/.cpp  Fractional delay with cubic Hermite interpolation
└── ui/
    ├── ModuleComponent.h/.cpp  Per-module UI (bypass + knob + mode combo)
    └── PresetSelector.h/.cpp   Preset save/load bar
```

### Key design decisions

- **APVTS for everything.** All 23 parameters live in `AudioProcessorValueTreeState`. Modules cache `std::atomic<float>*` pointers at startup for wait-free reads on the audio thread.
- **Strategy pattern for modes.** Each mode within a module is (or will be) its own `ProcessingMode` subclass. Adding a new distortion character means adding a class, not editing existing logic.
- **No allocations on the audio thread.** `processBlock` reads cached atomic pointers, calls `SmoothedValue` ramps, and processes in-place through `juce::dsp::AudioBlock`.
- **Shared `DriftGenerator`.** A single instance in the processor produces a drift signal each block. The value is passed to every module via `ProcessContext` — modules don't own their own generators.
- **Parameter smoothing.** Every amount knob uses a `SmoothedValue` (20 ms ramp). Bypass crossfades over ~10 ms to prevent clicks.
- **Cubic Hermite interpolation** in `ModulatedDelayLine` (Catmull-Rom variant) for clean modulated-delay artifacts in Wobble and Space.

### Parameter reference

All IDs use `snake_case`. Defined in `source/parameters/ParameterIDs.h`.

| Parameter             | Type   | Range          | Default |
|-----------------------|--------|----------------|---------|
| `drift`               | float  | 0.0 – 1.0     | 0.0     |
| `output_level`        | float  | −24.0 – 6.0 dB | 0.0    |
| `{module}_bypass`     | bool   | —              | false   |
| `{module}_amount`     | float  | 0.0 – 1.0     | 0.0     |
| `{module}_type/mode`  | choice | (see table)    | 0       |
| `space_size`          | float  | 0.0 – 1.0     | 0.5     |
| `space_tone`          | float  | 0.0 – 1.0     | 0.5     |

Where `{module}` is one of: `noise`, `wobble`, `distortion`, `space`, `magic`, `limit`.

---

## Prerequisites

| Tool    | Minimum version | Notes                               |
|---------|----------------|--------------------------------------|
| CMake   | 3.22           |                                      |
| Clang   | 12+            | GCC also works; Clang is the default |
| Ninja   | 1.10+          | Can substitute `make`                |
| Git     | —              | For FetchContent to pull JUCE        |

On Debian/Ubuntu:

```bash
sudo apt update && sudo apt install -y clang cmake ninja-build pkg-config \
    libasound2-dev libfreetype-dev libx11-dev libxrandr-dev libxcursor-dev \
    libxinerama-dev libwebkit2gtk-4.0-dev
```

The `libasound2-dev` and X11 packages are JUCE Linux dependencies for audio I/O and the plugin editor window.

On macOS with Homebrew:

```bash
brew install cmake ninja
```

Xcode command line tools provide Clang.

---

## Build

First-time configure fetches JUCE 8.0.4 via Git (~1–2 minutes):

```bash
# Configure (Debug)
cmake -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -G Ninja

# Build
cmake --build build -j$(nproc)
```

### Build artefacts

```
build/RC20Clone_artefacts/
├── Debug/
│   └── VST3/
│       └── RC-20 Clone.vst3    ← load this in your DAW
└── ...
```

On macOS, both `.vst3` and `.component` (AU) are produced.

### Release build

```bash
cmake -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -G Ninja

cmake --build build-release -j$(nproc)
```

### VSCode

The repo includes `.vscode/tasks.json` with pre-configured build tasks. Open the project folder in VSCode and use **Terminal → Run Build Task** (Ctrl+Shift+B) to build. The `.vscode/settings.json` configures clangd for code navigation — install the **clangd** extension and disable the Microsoft C++ extension for the best experience.

---

## Testing the plugin

### In a DAW

Copy or symlink the `.vst3` bundle to your DAW's VST3 scan path:

| Platform | Default VST3 path                          |
|----------|--------------------------------------------|
| Linux    | `~/.vst3/`                                 |
| macOS    | `~/Library/Audio/Plug-Ins/VST3/`           |
| Windows  | `C:\Program Files\Common Files\VST3\`      |

```bash
# Linux example
mkdir -p ~/.vst3
ln -sf "$(pwd)/build/RC20Clone_artefacts/Debug/VST3/RC-20 Clone.vst3" ~/.vst3/
```

Then rescan plugins in your DAW.

### With pluginval

[pluginval](https://github.com/Tracktion/pluginval) validates that the plugin meets the VST3/AU specification:

```bash
pluginval --validate "build/RC20Clone_artefacts/Debug/VST3/RC-20 Clone.vst3" --strictness-level 5
```

---

## Presets

Presets are stored as XML files in the user's application data directory:

| Platform | Path                                                  |
|----------|-------------------------------------------------------|
| Linux    | `~/.local/share/RC20Clone/Presets/`                   |
| macOS    | `~/Library/Application Support/RC20Clone/Presets/`    |
| Windows  | `%APPDATA%\RC20Clone\Presets\`                        |

Each preset is a full APVTS state snapshot. Use the **Save** / **Load** buttons in the plugin's top bar, or drop `.xml` files directly into the presets directory.

---

## Project roadmap

- [x] **Phase 1** — Plugin shell: APVTS parameter tree, module architecture, placeholder UI, DriftGenerator and ModulatedDelayLine stubs
- [ ] **Phase 2** — DSP implementation per module (Noise → Wobble → Distortion → Space → Magic → Limit)
- [ ] **Phase 3** — Custom UI (bespoke painted controls, LookAndFeel, visual feedback)
- [ ] **Phase 4** — Polish (factory presets, Randomize function, pluginval level 10, performance profiling)

---

## License

This is a personal/educational project. JUCE is used under the [AGPLv3 license](https://juce.com/legal/juce-8-licence/).
