#include "ModulatedDelayLine.h"

void ModulatedDelayLine::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    bufferSize_ =
        static_cast<int>(kMaxDelaySeconds * sampleRate_) + 4; // +4 for interpolation headroom
    buffer_.assign(static_cast<size_t>(bufferSize_), 0.0f);
    reset();
}

float ModulatedDelayLine::processSample(float input, float delayInSamples) noexcept
{
    // Write input sample at the current write head.
    buffer_[static_cast<size_t>(writeHead_)] = input;

    // Clamp delay to valid range (need at least 1, leave 3 samples headroom for interpolation).
    const float clamped  = juce::jlimit(1.0f, static_cast<float>(bufferSize_ - 4), delayInSamples);
    const int   delayInt = static_cast<int>(clamped);
    const float frac     = clamped - static_cast<float>(delayInt);

    // Compute the four read positions needed for cubic Hermite interpolation.
    // r1 is the sample just before the fractional position, r2 just after.
    auto wrap = [&](int idx) -> int
    {
        idx %= bufferSize_;
        if (idx < 0) idx += bufferSize_;
        return idx;
    };

    const int r0 = wrap(writeHead_ - delayInt - 1);
    const int r1 = wrap(writeHead_ - delayInt);
    const int r2 = wrap(writeHead_ - delayInt + 1);
    const int r3 = wrap(writeHead_ - delayInt + 2);

    // Advance write head for next call.
    writeHead_ = (writeHead_ + 1) % bufferSize_;

    return cubicHermite(buffer_[static_cast<size_t>(r0)],
                        buffer_[static_cast<size_t>(r1)],
                        buffer_[static_cast<size_t>(r2)],
                        buffer_[static_cast<size_t>(r3)],
                        frac);
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
