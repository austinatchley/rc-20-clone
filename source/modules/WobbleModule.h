#pragma once
#include "EffectModule.h"
#include "../dsp/ModulatedDelayLine.h"

/**
 * Wobble module — wow and flutter pitch/time instability.
 *
 * Modes:
 *   Slow    — gentle, low-frequency drift (tape wow)
 *   Medium  — moderate speed, moderate depth (VHS)
 *   Fast    — high-speed flutter (worn cassette)
 *
 * Uses a ModulatedDelayLine with cubic Hermite interpolation per channel.
 * Drift input from ProcessContext biases the LFO phase/depth.
 *
 * Phase 1: parameter binding and delay line allocation only.  No DSP.
 */
class WobbleModule final : public EffectModule
{
  public:
    enum class Mode
    {
        Slow = 0,
        Medium,
        Fast
    };

    WobbleModule();
    ~WobbleModule() override = default;

    void prepareParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(ProcessContext& ctx) override;
    void reset() override;
    const char* getName() const override { return "Wobble"; }

  private:
    std::atomic<float>* modeParam_ = nullptr;

    // One delay line per channel (stereo).
    ModulatedDelayLine delayL_;
    ModulatedDelayLine delayR_;

    // LFO phase accumulators; R is offset by 90° for stereo width.
    float lfoPhaseL_ = 0.0f;
    float lfoPhaseR_ = 0.25f; // quarter-cycle offset

    double sampleRate_ = 44100.0;
};
