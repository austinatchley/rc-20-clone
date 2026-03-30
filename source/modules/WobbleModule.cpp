#include "WobbleModule.h"
#include "../parameters/ParameterIDs.h"

WobbleModule::WobbleModule() = default;

void WobbleModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::wobble_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::wobble_amount);
    modeParam_ = apvts.getRawParameterValue(ParameterIDs::wobble_mode);
}

void WobbleModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);

    // Prepare one delay line per channel using a single-channel spec.
    juce::dsp::ProcessSpec monoSpec{spec.sampleRate, spec.maximumBlockSize, 1};
    delayL_.prepare(monoSpec);
    delayR_.prepare(monoSpec);

    reset();
}

void WobbleModule::process(ProcessContext& ctx)
{
    juce::ignoreUnused(ctx);
    // Phase 1: pass through.
}

void WobbleModule::reset()
{
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
    delayL_.reset();
    delayR_.reset();
}
