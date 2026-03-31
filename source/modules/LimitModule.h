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
 */
class LimitModule final : public EffectModule
{
  public:
    enum class Mode
    {
        Soft = 0,
        Medium,
        Hard
    };

    LimitModule();
    ~LimitModule() override = default;

    void prepareParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(ProcessContext& ctx) override;
    void reset() override;
    const char* getName() const override { return "Limit"; }

  private:
    std::atomic<float>* modeParam_ = nullptr;
    double sampleRate_ = 44100.0;

    // Envelope follower state per channel (peak detector, max 2 ch).
    float env_[2] = {};
};
