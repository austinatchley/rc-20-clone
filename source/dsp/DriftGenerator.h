#pragma once
#include <juce_dsp/juce_dsp.h>

/**
 * Produces a slowly evolving modulation signal in [0, 1].
 *
 * Several sine LFOs at slightly different rates (0.3, 0.7, 1.1 Hz) are
 * summed with a small amount of band-limited noise, then normalised.  This
 * avoids the mechanical periodicity of a single-LFO drift source and mimics
 * the organic instability of real tape/vinyl playback.
 *
 * The output is scaled by the global "drift" parameter and injected into every
 * module's ProcessContext so they all respond to the same instability signal.
 */
class DriftGenerator
{
  public:
    DriftGenerator() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);

    /**
     * Advance by one block and return the current drift value in [0, 1].
     * Call once per processBlock, before iterating over modules.
     *
     * @param depth  Master drift depth in [0, 1] (the global "drift" parameter).
     */
    float process(float depth) noexcept;

    void reset();

  private:
    double sampleRate_ = 44100.0;
    int samplesPerBlock_ = 512;
    float currentValue_ = 0.0f;

    // Three sine LFOs at slightly different rates for organic instability.
    float phase0_ = 0.0f; // 0.3 Hz
    float phase1_ = 0.0f; // 0.7 Hz
    float phase2_ = 0.0f; // 1.1 Hz

    // One-pole lowpass filter for band-limited noise (~5 Hz cutoff).
    float noiseSmooth_ = 0.0f;
    juce::Random rng_;
};
