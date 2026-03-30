#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// ── ProcessContext ─────────────────────────────────────────────────────────────

/**
 * Passed by reference to every module's process() call each block.
 *
 * Holds the audio data (as an AudioBlock for in-place processing) and any
 * shared modulation state that modules may read.  Constructed on the stack in
 * processBlock() — never heap-allocated.
 */
struct ProcessContext
{
    juce::dsp::AudioBlock<float>& block;
    float driftValue = 0.0f; ///< Global drift in [0, 1]
};

// ── ProcessingMode ─────────────────────────────────────────────────────────────

/**
 * Abstract base for one character/algorithm within a module.
 *
 * Each concrete subclass implements a single processing "style" — e.g.
 * TapeDistortion, TubeDistortion, TransistorDistortion.  Adding a new style
 * means adding a new subclass only; existing code is untouched.
 *
 * Modules own their mode instances and crossfade between them on mode changes.
 * The crossfade logic lives in EffectModule, not in the mode itself.
 */
class ProcessingMode
{
  public:
    virtual ~ProcessingMode() = default;

    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;

    /**
     * Process the audio block in-place.
     *
     * @param block   Audio data — modify in place.
     * @param amount  Normalised depth [0, 1], already smoothed by the module.
     * @param drift   Drift value from DriftGenerator [0, 1].
     */
    virtual void process(juce::dsp::AudioBlock<float>& block, float amount, float drift) = 0;

    virtual void reset() = 0;

    virtual const char* getName() const = 0;
};

// ── EffectModule ───────────────────────────────────────────────────────────────

/**
 * Abstract base for all six processing modules in the signal chain.
 *
 * Responsibilities of this base class:
 *   - Own the raw APVTS parameter pointers (safe atomic reads on audio thread)
 *   - Provide a SmoothedValue for the amount knob and bypass crossfade
 *
 * Responsibilities of each concrete subclass:
 *   - Create and own its ProcessingMode instances
 *   - Cache its own parameter pointers in prepareParameters()
 *   - Drive mode crossfading and DSP in process()
 *
 * Thread safety
 * -------------
 * prepareParameters() is called from the message thread (once at startup).
 * prepare() and process() are called from the audio thread.
 * No allocations or locks are permitted in prepare() or process().
 */
class EffectModule
{
  public:
    virtual ~EffectModule() = default;

    /**
     * Cache raw APVTS parameter pointers for lock-free audio-thread access.
     * Call once from the message thread before audio processing begins.
     */
    virtual void prepareParameters(juce::AudioProcessorValueTreeState& apvts) = 0;

    /** Called when sample rate or maximum block size changes. */
    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;

    /** Process one block in-place.  Audio thread only. */
    virtual void process(ProcessContext& ctx) = 0;

    /** Reset all internal state (smoothers, delay lines, filter histories). */
    virtual void reset() = 0;

    virtual const char* getName() const = 0;

  protected:
    // Raw parameter pointers — dereferencing these is wait-free on the audio thread.
    std::atomic<float>* bypassParam_ = nullptr;
    std::atomic<float>* amountParam_ = nullptr;

    // One-pole smoother for the primary amount knob (20 ms ramp).
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedAmount_;

    // Bypass crossfade gain: 1.0 = fully active, 0.0 = fully bypassed (~10 ms).
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassGain_{1.0f};
};
