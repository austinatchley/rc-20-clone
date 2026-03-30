#include "NoiseModule.h"
#include "../parameters/ParameterIDs.h"

NoiseModule::NoiseModule() = default;

void NoiseModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::noise_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::noise_amount);
    typeParam_   = apvts.getRawParameterValue(ParameterIDs::noise_type);
}

void NoiseModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);
    reset();
}

void NoiseModule::process(ProcessContext& ctx)
{
    juce::ignoreUnused(ctx);
    // Phase 1: pass through.
}

void NoiseModule::reset()
{
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
}
