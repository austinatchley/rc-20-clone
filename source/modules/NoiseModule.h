#pragma once
#include "EffectModule.h"

/**
 * Noise module — adds analog-style background texture.
 *
 * Modes (mutually exclusive):
 *   TapeHiss      — spectrally-shaped pink noise
 *   VinylCrackle  — Poisson-distributed random impulses
 *   ElectricalHum — 50/60 Hz sine with harmonics
 *
 * Phase 1: parameter binding only.  DSP stubbed — audio passes through.
 */
class NoiseModule final : public EffectModule
{
  public:
    enum class Type
    {
        TapeHiss = 0,
        VinylCrackle,
        ElectricalHum
    };

    NoiseModule();
    ~NoiseModule() override = default;

    void prepareParameters(juce::AudioProcessorValueTreeState& apvts) override;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(ProcessContext& ctx) override;
    void reset() override;
    const char* getName() const override { return "Noise"; }

  private:
    std::atomic<float>* typeParam_ = nullptr;
    double sampleRate_ = 44100.0;
};
