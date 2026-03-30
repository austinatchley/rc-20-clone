#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>

/**
 * Single-channel delay line with cubic Hermite interpolation.
 *
 * Supports continuously varying delay times, making it suitable for:
 *   - Wobble (wow/flutter pitch modulation)
 *   - Space (BBD-style delay with pitch-bend on time changes)
 *
 * Interpolation detail
 * --------------------
 * Linear interpolation introduces audible artefacts when the delay time
 * changes smoothly (the "zipper" effect in wobble).  Cubic Hermite uses four
 * surrounding samples to fit a smooth curve, trading slightly more compute for
 * clean modulated-delay artefacts.
 *
 * Phase 1: class interface and buffer management only.  DSP is stubbed.
 */
class ModulatedDelayLine
{
  public:
    /** Maximum supported delay in seconds.  Sets internal buffer capacity. */
    static constexpr float kMaxDelaySeconds = 0.5f;

    ModulatedDelayLine() = default;

    /** Allocate buffer and reset state.  Must be called before processing. */
    void prepare(const juce::dsp::ProcessSpec& spec);

    /**
     * Write one input sample and return the interpolated output at
     * \p delayInSamples behind the write head.
     *
     * @param input          Sample to push into the delay line.
     * @param delayInSamples Fractional delay in samples, clamped to [1, maxDelay].
     * @return               Interpolated output sample.
     */
    float processSample(float input, float delayInSamples) noexcept;

    /** Silence the internal buffer and reset the write head. */
    void reset();

  private:
    /**
     * Cubic Hermite spline interpolation between y1 and y2.
     *
     * @param y0, y1, y2, y3  Four consecutive samples (y1..y2 is the interpolated span).
     * @param t                Fractional position in [0, 1).
     */
    static float cubicHermite(float y0, float y1, float y2, float y3, float t) noexcept;

    std::vector<float> buffer_;
    int writeHead_ = 0;
    int bufferSize_ = 0;
    double sampleRate_ = 44100.0;
};
