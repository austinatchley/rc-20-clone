#include "ModulatedDelayLine.h"

void ModulatedDelayLine::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    bufferSize_ = static_cast<int>(kMaxDelaySeconds * sampleRate_) + 4; // +4 for interpolation headroom
    buffer_.assign(static_cast<size_t>(bufferSize_), 0.0f);
    reset();
}

float ModulatedDelayLine::processSample(float /*input*/, float /*delayInSamples*/) noexcept
{
    // Phase 1 stub — returns silence.
    // Phase 2 will: write input at writeHead_, compute read position,
    // gather four samples, call cubicHermite(), advance writeHead_.
    return 0.0f;
}

void ModulatedDelayLine::reset()
{
    std::fill(buffer_.begin(), buffer_.end(), 0.0f);
    writeHead_ = 0;
}

float ModulatedDelayLine::cubicHermite(float y0, float y1, float y2, float y3, float t) noexcept
{
    // Catmull-Rom variant of cubic Hermite:
    //   c0 = y1
    //   c1 = 0.5 * (y2 - y0)
    //   c2 = y0 - 2.5*y1 + 2*y2 - 0.5*y3
    //   c3 = 0.5 * (y3 - y0) + 1.5 * (y1 - y2)
    const float c0 = y1;
    const float c1 = 0.5f * (y2 - y0);
    const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * t + c2) * t + c1) * t + c0;
}
