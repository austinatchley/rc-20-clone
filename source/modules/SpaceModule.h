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

    // Freeverb-based reverb engine (juce::Reverb wraps the Schroeder topology).
    juce::Reverb reverb_;
};
