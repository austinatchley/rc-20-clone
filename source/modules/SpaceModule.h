#pragma once
#include "EffectModule.h"
#include "../dsp/ModulatedDelayLine.h"

/**
 * Space module — room ambience from subtle to cavernous.
 *
 * Modes:
 *   Room   — small, tight ambience; short decay
 *   Hall   — large reflective space; longer decay
 *   Plate  — dense, bright metallic reverb character
 *
 * Implementation notes (Phase 2):
 *   - BBD-style delay path with a darkening lowpass in the feedback loop
 *   - Schroeder reverb topology: parallel comb filters into series allpass
 *   - Delay line lengths are mutually prime to prevent metallic resonances
 *   - Modulated delay lines in the reverb prevent static metallic ringing
 *   - Pitch-bend interpolation on delay time changes (same ModulatedDelayLine)
 *   - size  controls decay time / room scale
 *   - tone  controls high-frequency damping (lowpass cutoff in feedback)
 *
 * Phase 1: parameter binding only.  DSP stubbed — audio passes through.
 */
class SpaceModule final : public EffectModule
{
  public:
    enum class Mode
    {
        Room = 0,
        Hall,
        Plate
    };

    SpaceModule();
    ~SpaceModule() override = default;

    void prepareParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(ProcessContext& ctx) override;
    void reset() override;
    const char* getName() const override { return "Space"; }

  private:
    std::atomic<float>* modeParam_ = nullptr;
    std::atomic<float>* sizeParam_ = nullptr;
    std::atomic<float>* toneParam_ = nullptr;

    double sampleRate_ = 44100.0;
};
