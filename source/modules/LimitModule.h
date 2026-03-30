#pragma once
#include "EffectModule.h"

/**
 * Limit module — output stage with soft-knee limiting and subtle saturation.
 *
 * Modes:
 *   Soft    — transparent limiting; minimal colouration
 *   Medium  — gentle glue compression character
 *   Hard    — aggressive peak limiting with audible saturation on transients
 *
 * Implementation notes (Phase 2):
 *   - Attack/release envelope follower with smoothed gain reduction
 *   - Soft knee: gain reduction eases in over a configurable knee width
 *   - Output saturation (tanh) at higher amount settings adds glue rather
 *     than a hard ceiling — pushing the limiter should sound musical
 *   - Amount knob controls threshold (high amount = lower threshold = more limiting)
 *
 * Phase 1: parameter binding only.  DSP stubbed — audio passes through.
 */
class LimitModule final : public EffectModule
{
public:
    enum class Mode { Soft = 0, Medium, Hard };

    LimitModule();
    ~LimitModule() override = default;

    void prepareParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(ProcessContext& ctx) override;
    void reset() override;
    const char* getName() const override { return "Limit"; }

private:
    std::atomic<float>* modeParam_ = nullptr;
    double              sampleRate_ = 44100.0;
};
