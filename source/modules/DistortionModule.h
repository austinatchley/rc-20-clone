#pragma once
#include "EffectModule.h"
#include <memory>

/**
 * Distortion module — nonlinear saturation with vintage character.
 *
 * Modes (mutually exclusive, each with a distinct harmonic signature):
 *   Tape        — soft even-order harmonics, frequency-dependent drive
 *   Tube        — warm even-order distortion with slight asymmetry (DC offset)
 *   Transistor  — harsher odd-order clipping, more aggressive tone
 *
 * Implementation notes (Phase 2):
 *   - 4x oversampling on all nonlinear stages to suppress aliasing
 *   - tanh() waveshaper as baseline; each mode shapes the curve differently
 *   - Output gain compensation so perceived loudness stays constant vs. amount
 *   - Mode crossfades over ~10 ms to prevent clicks on mode changes
 *
 * Phase 1: parameter binding only.  DSP stubbed — audio passes through.
 */
class DistortionModule final : public EffectModule
{
  public:
    enum class Mode
    {
        Tape = 0,
        Tube,
        Transistor
    };

    DistortionModule();
    ~DistortionModule() override = default;

    void prepareParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(ProcessContext& ctx) override;
    void reset() override;
    const char* getName() const override { return "Distortion"; }

  private:
    std::atomic<float>* modeParam_ = nullptr;
    double sampleRate_ = 44100.0;

    // 4x oversampling to suppress aliasing on nonlinear stages.
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling_;

    // Waveshaper helpers.
    static float shapeTape       (float x, float drive) noexcept;
    static float shapeTube       (float x, float drive) noexcept;
    static float shapeTransistor (float x, float drive) noexcept;
};
