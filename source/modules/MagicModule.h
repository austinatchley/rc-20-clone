#pragma once
#include "EffectModule.h"

/**
 * Magic module — lo-fi digital degradation.
 *
 * Modes:
 *   BitCrush   — quantisation noise; reduces effective bit depth
 *   Decimate   — sample-rate reduction; aliasing artefacts
 *   Combined   — both simultaneously, blended with dry/wet
 *
 * The amount knob sweeps from clean through progressively heavier degradation.
 * Drift modulates the effective bit depth / sample rate for organic instability.
 *
 * Phase 1: parameter binding only.  DSP stubbed — audio passes through.
 */
class MagicModule final : public EffectModule
{
public:
    enum class Mode { BitCrush = 0, Decimate, Combined };

    MagicModule();
    ~MagicModule() override = default;

    void prepareParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(ProcessContext& ctx) override;
    void reset() override;
    const char* getName() const override { return "Magic"; }

private:
    std::atomic<float>* modeParam_ = nullptr;
    double              sampleRate_ = 44100.0;
};
