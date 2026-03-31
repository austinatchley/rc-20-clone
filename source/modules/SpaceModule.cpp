#include "SpaceModule.h"

#include "../parameters/ParameterIDs.h"

SpaceModule::SpaceModule() = default;

void SpaceModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts) {
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::space_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::space_amount);
    modeParam_ = apvts.getRawParameterValue(ParameterIDs::space_mode);
    sizeParam_ = apvts.getRawParameterValue(ParameterIDs::space_size);
    toneParam_ = apvts.getRawParameterValue(ParameterIDs::space_tone);
}

void SpaceModule::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);

    reverb_.setSampleRate(spec.sampleRate);

    reset();
}

void SpaceModule::process(ProcessContext& ctx) {
    if (bypassParam_->load() > 0.5f)
        return;

    auto& block = ctx.block;
    const int numChannels = static_cast<int>(block.getNumChannels());
    const int numSamples = static_cast<int>(block.getNumSamples());

    const auto mode = static_cast<Mode>(static_cast<int>(modeParam_->load()));
    const float size = sizeParam_->load();
    const float tone = toneParam_->load();
    const float amount = amountParam_->load();

    smoothedAmount_.setTargetValue(amount);
    // Advance smoother — we use block-level amount for reverb parameters.
    for (int i = 0; i < numSamples; ++i)
        smoothedAmount_.getNextValue();
    const float smoothAmount = smoothedAmount_.getCurrentValue();

    // Build reverb parameters per mode.
    // size  [0,1] → room size scalar
    // tone  [0,1] → damping (0=bright, 1=dark)
    juce::Reverb::Parameters params;
    params.width = 1.0f;
    params.freezeMode = 0.0f;
    params.dryLevel = 1.0f;  // always pass dry through
    params.wetLevel = smoothAmount;

    switch (mode) {
        case Mode::Room:
            params.roomSize = 0.2f + size * 0.4f;  // 0.20 .. 0.60
            params.damping = (1.0f - tone) * 0.8f;
            break;

        case Mode::Hall:
            params.roomSize = 0.5f + size * 0.45f;  // 0.50 .. 0.95
            params.damping = (1.0f - tone) * 0.6f;
            break;

        case Mode::Plate:
            params.roomSize = 0.7f + size * 0.28f;  // 0.70 .. 0.98
            params.damping = (1.0f - tone) * 0.3f;  // plates are bright
            break;

        default:
            params.roomSize = 0.5f;
            params.damping = 0.5f;
            break;
    }

    reverb_.setParameters(params);

    float* left = block.getChannelPointer(0);
    if (numChannels >= 2) {
        float* right = block.getChannelPointer(1);
        reverb_.processStereo(left, right, numSamples);
    } else {
        reverb_.processMono(left, numSamples);
    }
}

void SpaceModule::reset() {
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
    reverb_.reset();
}
