#include "MagicModule.h"

#include "../parameters/ParameterIDs.h"

MagicModule::MagicModule() = default;

void MagicModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts) {
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::magic_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::magic_amount);
    modeParam_ = apvts.getRawParameterValue(ParameterIDs::magic_mode);
}

void MagicModule::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);
    reset();
}

void MagicModule::process(ProcessContext& ctx) {
    if (bypassParam_->load() > 0.5f)
        return;

    auto& block = ctx.block;
    const int numChannels = static_cast<int>(block.getNumChannels());
    const int numSamples = static_cast<int>(block.getNumSamples());
    const auto mode = static_cast<Mode>(static_cast<int>(modeParam_->load()));
    const float drift = ctx.driftValue;

    smoothedAmount_.setTargetValue(amountParam_->load());

    for (int i = 0; i < numSamples; ++i) {
        const float amount = smoothedAmount_.getNextValue();

        switch (mode) {
            case Mode::BitCrush: {
                // Map amount [0,1] to bit depth [16, 4]; drift narrows it further.
                const float bits = juce::jmax(1.0f, 16.0f - amount * 12.0f - drift * 2.0f);
                const float steps = std::exp2f(bits);
                for (int ch = 0; ch < numChannels; ++ch) {
                    float& s = block.getChannelPointer(static_cast<size_t>(ch))[i];
                    s = std::floor(s * steps + 0.5f) / steps;
                }
                break;
            }

            case Mode::Decimate: {
                // Amount [0,1] → hold rate [1, 32]; drift adds a few extra samples.
                const int holdRate =
                    juce::jmax(1, static_cast<int>(1.0f + amount * 31.0f + drift * 8.0f));

                if (holdCounter_ <= 0) {
                    for (int ch = 0; ch < numChannels; ++ch)
                        holdSamples_[ch] = block.getChannelPointer(static_cast<size_t>(ch))[i];
                    holdCounter_ = holdRate;
                }
                --holdCounter_;

                for (int ch = 0; ch < numChannels; ++ch)
                    block.getChannelPointer(static_cast<size_t>(ch))[i] = holdSamples_[ch];
                break;
            }

            case Mode::Combined: {
                // Apply bit crush first, then hold the crushed value.
                const float bits = juce::jmax(1.0f, 16.0f - amount * 10.0f - drift * 2.0f);
                const float steps = std::exp2f(bits);
                float crushed[2];
                for (int ch = 0; ch < numChannels; ++ch) {
                    const float s = block.getChannelPointer(static_cast<size_t>(ch))[i];
                    crushed[ch] = std::floor(s * steps + 0.5f) / steps;
                }

                const int holdRate =
                    juce::jmax(1, static_cast<int>(1.0f + amount * 15.0f + drift * 4.0f));

                if (holdCounter_ <= 0) {
                    for (int ch = 0; ch < numChannels; ++ch)
                        holdSamples_[ch] = crushed[ch];
                    holdCounter_ = holdRate;
                }
                --holdCounter_;

                for (int ch = 0; ch < numChannels; ++ch)
                    block.getChannelPointer(static_cast<size_t>(ch))[i] = holdSamples_[ch];
                break;
            }
        }
    }
}

void MagicModule::reset() {
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
    holdSamples_[0] = holdSamples_[1] = 0.0f;
    holdCounter_ = 0;
}
