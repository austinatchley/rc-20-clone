#include "DriftGenerator.h"

void DriftGenerator::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_      = spec.sampleRate;
    samplesPerBlock_ = static_cast<int>(spec.maximumBlockSize);
    reset();
}

float DriftGenerator::process(float depth) noexcept
{
    if (depth <= 0.0f)
        return 0.0f;

    // Advance LFO phases by one block's worth of time.
    const float blockTime = static_cast<float>(samplesPerBlock_)
                            / static_cast<float>(sampleRate_);

    phase0_ += 0.3f * blockTime;
    phase1_ += 0.7f * blockTime;
    phase2_ += 1.1f * blockTime;

    if (phase0_ >= 1.0f) phase0_ -= 1.0f;
    if (phase1_ >= 1.0f) phase1_ -= 1.0f;
    if (phase2_ >= 1.0f) phase2_ -= 1.0f;

    const float twoPi = juce::MathConstants<float>::twoPi;

    // Sum three LFOs with diminishing amplitude.
    const float lfoSum = std::sin(twoPi * phase0_)
                       + 0.5f * std::sin(twoPi * phase1_)
                       + 0.25f * std::sin(twoPi * phase2_);

    // Band-limited noise: one-pole LPF at ~5 Hz.
    const float noiseIn = rng_.nextFloat() * 2.0f - 1.0f;
    const float alpha   = 1.0f - (twoPi * 5.0f * blockTime);
    noiseSmooth_ = alpha * noiseSmooth_ + (1.0f - alpha) * noiseIn;

    // Blend LFOs (dominant) with noise (texture).
    const float raw = (lfoSum / 1.75f) + 0.2f * noiseSmooth_;

    // Map raw (roughly -1..1) to [0, 1] and clamp.
    currentValue_ = juce::jlimit(0.0f, 1.0f, (raw + 1.0f) * 0.5f);
    return currentValue_ * depth;
}

void DriftGenerator::reset()
{
    currentValue_ = 0.0f;
    noiseSmooth_  = 0.0f;
}
