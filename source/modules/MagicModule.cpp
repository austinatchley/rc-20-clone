#include "MagicModule.h"
#include "../parameters/ParameterIDs.h"

MagicModule::MagicModule() = default;

void MagicModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::magic_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::magic_amount);
    modeParam_ = apvts.getRawParameterValue(ParameterIDs::magic_mode);
}

void MagicModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);
    reset();
}

void MagicModule::process(ProcessContext& ctx)
{
    juce::ignoreUnused(ctx);
    // Phase 1: pass through.
}

void MagicModule::reset()
{
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
}
