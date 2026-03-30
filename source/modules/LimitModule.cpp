#include "LimitModule.h"
#include "../parameters/ParameterIDs.h"

LimitModule::LimitModule() = default;

void LimitModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::limit_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::limit_amount);
    modeParam_   = apvts.getRawParameterValue(ParameterIDs::limit_mode);
}

void LimitModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);
    reset();
}

void LimitModule::process(ProcessContext& ctx)
{
    juce::ignoreUnused(ctx);
    // Phase 1: pass through.
}

void LimitModule::reset()
{
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
}
