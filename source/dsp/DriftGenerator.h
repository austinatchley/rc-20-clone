#pragma once
#include <juce_dsp/juce_dsp.h>

/**
 * Produces a slowly evolving modulation signal in [0, 1].
 *
 * Design intent (Phase 2)
 * -----------------------
 * Several sine LFOs at slightly different rates (e.g. 0.3, 0.7, 1.1 Hz) are
 * summed with a small amount of band-limited noise, then normalised.  This
 * avoids the mechanical periodicity of a single-LFO drift source and mimics
 * the organic instability of real tape/vinyl playback.
 *
 * The output is scaled by the global "drift" parameter and injected into every
 * module's ProcessContext so they all respond to the same instability signal.
 *
 * Phase 1: prepare/reset wired up; process() returns 0 (no instability).
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

    // Phase 2: LFO phase accumulators and noise state will live here.
};
