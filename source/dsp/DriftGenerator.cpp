#include "DriftGenerator.h"

void DriftGenerator::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_      = spec.sampleRate;
    samplesPerBlock_ = static_cast<int>(spec.maximumBlockSize);
    reset();
}

float DriftGenerator::process(float /*depth*/) noexcept
{
    // Phase 1 stub — no instability.
    return 0.0f;
}

void DriftGenerator::reset()
{
    currentValue_ = 0.0f;
}
