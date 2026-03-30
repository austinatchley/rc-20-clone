# CLAUDE.md — RC-20 Clone

Working notes for LLM assistants. Read this before touching any code.
The README covers architecture and build steps; this file covers the things
that aren't obvious from reading the code.

---

## Build environment

- **OS:** Debian 11 (Bullseye) in WSL2 on Windows
- **glibc:** 2.31 — prebuilt binaries requiring 2.32+ will not run
- **Compiler:** Clang 11 (`/usr/bin/clang++`)
- **CMake:** 4.3 (installed via pip — the system apt version was 3.18, too old for JUCE 8)
- **Build system:** Ninja
- **JUCE:** 8.0.4 via FetchContent (cached in `build/_deps/` after first configure)

To build:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja
cmake --build build -j$(nproc)
```

VST3 lands at `build/RC20Clone_artefacts/Debug/VST3/RC-20 Clone.vst3`.

---

## JUCE 8 API gotchas (already bitten us once each)

- **`std::atomic<float>*` dereferencing.** `*atomicPtr` returns an `atomic<float>`,
  not a `float`. Passing that directly into a template function (e.g.
  `Decibels::decibelsToGain`) instantiates the template on `atomic<float>`, which
  tries to copy-construct an atomic and fails. Always call `.load()` first:
  ```cpp
  const float db = outputLevelParam_->load();
  ```

- **`juce::Font` constructor deprecated.** `Font(float size, int style)` is gone.
  Use `FontOptions`:
  ```cpp
  label.setFont(juce::FontOptions(14.0f, juce::Font::bold));
  ```

- **`AlertWindow::showInputBoxAsync` removed.** Use `FileChooser::launchAsync`
  instead. The `FileChooser` instance must be kept alive as a member — if it goes
  out of scope the callback is never called.

- **Initializer-list `addAndMakeVisible` loops don't compile** when the list mixes
  component subclasses (type deduction fails). Call `addAndMakeVisible` individually.

- **Constructor parameter `processor` shadows `AudioProcessorEditor::processor`.**
  Clang's `-Wshadow-field` treats this as an error under
  `juce_recommended_warning_flags`. Rename the constructor param (e.g. `p`).

---

## Platform notes

- This is a **Linux VST3**. It cannot be loaded in a Windows DAW directly.
- For Windows testing: clone the repo on the Windows filesystem and build with
  Visual Studio 2022 or LLVM/Clang for Windows. The CMakeLists.txt is already
  cross-platform.
- **pluginval:** The prebuilt Linux binary requires glibc 2.35+. On this machine
  it must be built from source (`/tmp/pluginval`). Once built, the binary is at
  `/tmp/pluginval/build/pluginval_artefacts/Release/pluginval`.

---

## Architecture decisions (the "why" behind the code)

These were settled before writing Phase 1 and should not be revisited without
good reason.

- **Fixed signal chain, not dynamic.** Modules are stored in a
  `std::array<std::unique_ptr<EffectModule>, 6>` in processing order. Reorderable
  chains are a future option, not a current goal.

- **Single `DriftGenerator` in the processor.** It runs once per block and passes
  a single `float driftValue` to all modules via `ProcessContext`. Modules do not
  own generators.

- **Modes are mutually exclusive** (`AudioParameterChoice`, not bool flags).
  Each mode will be a `ProcessingMode` subclass (strategy pattern). Crossfading
  between modes on change lives in the module, not the mode.

- **`juce::dsp::AudioBlock` in `ProcessContext`.** In-place, zero-copy. Do not
  wrap in `ProcessContextReplacing` — the custom `ProcessContext` struct is
  intentional to carry the drift value alongside the audio.

- **Parameter IDs are all `snake_case`**, centralised in `ParameterIDs.h`. Never
  hard-code an ID string anywhere else.

- **`AudioParameterFloat` uses `juce::ParameterID{id, 1}`** (with version hint
  `1`), not the deprecated string-only constructor.

- **`SmoothedValue` ramps:** amount knobs use 20 ms, bypass crossfade uses 10 ms.
  These are initialised in each module's `prepare()` call.

---

## Phase status

| Phase | Description                                         | Status      |
|-------|-----------------------------------------------------|-------------|
| 1     | Plugin shell — APVTS tree, stubs, placeholder UI    | Complete    |
| 2     | DSP implementation per module                       | Not started |
| 3     | Custom painted UI + LookAndFeel                     | Not started |
| 4     | Polish — factory presets, Randomize, perf profiling | Not started |

**Phase 2 start point:** implement `NoiseModule` first — no time-domain state,
no oversampling, self-contained. Good warm-up before Wobble (delay line) and
Distortion (oversampling pipeline).

---

## Audio thread rules

Strictly enforced — do not add exceptions:

- No heap allocation in `processBlock` or any function it calls
- No mutex locks on the audio thread
- Parameter values are read via cached `std::atomic<float>*` pointers only
- `prepareParameters()` is message-thread only (called at construction)
- `prepare()` and `process()` are audio-thread only
