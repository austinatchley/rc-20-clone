#include "SpaceModule.h"
#include "../parameters/ParameterIDs.h"

SpaceModule::SpaceModule() = default;

void SpaceModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::space_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::space_amount);
    modeParam_   = apvts.getRawParameterValue(ParameterIDs::space_mode);
    sizeParam_   = apvts.getRawParameterValue(ParameterIDs::space_size);
    toneParam_   = apvts.getRawParameterValue(ParameterIDs::space_tone);
}

void SpaceModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);
    reset();
}

void SpaceModule::process(ProcessContext& ctx)
{
    juce::ignoreUnused(ctx);
    // Phase 1: pass through.
}

void SpaceModule::reset()
{
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
}
