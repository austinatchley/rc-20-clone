#include "DistortionModule.h"
#include "../parameters/ParameterIDs.h"

DistortionModule::DistortionModule() = default;

void DistortionModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::distortion_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::distortion_amount);
    modeParam_   = apvts.getRawParameterValue(ParameterIDs::distortion_mode);
}

void DistortionModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);
    reset();
}

void DistortionModule::process(ProcessContext& ctx)
{
    juce::ignoreUnused(ctx);
    // Phase 1: pass through.
}

void DistortionModule::reset()
{
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
}
