#include "LimitModule.h"

#include "../parameters/ParameterIDs.h"

LimitModule::LimitModule() = default;

void LimitModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts) {
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::limit_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::limit_amount);
    modeParam_ = apvts.getRawParameterValue(ParameterIDs::limit_mode);
}

void LimitModule::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);
    reset();
}

void LimitModule::process(ProcessContext& ctx) {
    if (bypassParam_->load() > 0.5f)
        return;

    auto& block = ctx.block;
    const int numChannels = static_cast<int>(block.getNumChannels());
    const int numSamples = static_cast<int>(block.getNumSamples());
    const auto mode = static_cast<Mode>(static_cast<int>(modeParam_->load()));

    smoothedAmount_.setTargetValue(amountParam_->load());

    // Attack / release times and knee width per mode.
    float attackMs, releaseMs, kneeDb;
    switch (mode) {
        case Mode::Soft:
            attackMs = 5.0f;
            releaseMs = 150.0f;
            kneeDb = 6.0f;
            break;
        case Mode::Medium:
            attackMs = 1.0f;
            releaseMs = 60.0f;
            kneeDb = 3.0f;
            break;
        case Mode::Hard:
            attackMs = 0.1f;
            releaseMs = 20.0f;
            kneeDb = 0.0f;
            break;
        default:
            attackMs = 5.0f;
            releaseMs = 150.0f;
            kneeDb = 6.0f;
            break;
    }

    const float sr = static_cast<float>(sampleRate_);
    const float attackCoeff = std::exp(-1.0f / (attackMs * 0.001f * sr));
    const float releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * sr));

    for (int i = 0; i < numSamples; ++i) {
        const float amount = smoothedAmount_.getNextValue();

        // Threshold: amount 0 → 0 dBFS (no limiting), amount 1 → −24 dBFS (heavy).
        const float thresholdDb = -amount * 24.0f;
        const float thresholdLinear = juce::Decibels::decibelsToGain(thresholdDb);

        for (int ch = 0; ch < numChannels; ++ch) {
            float& s = block.getChannelPointer(static_cast<size_t>(ch))[i];
            const float absIn = std::abs(s);

            // Peak envelope follower.
            if (absIn > env_[ch])
                env_[ch] = attackCoeff * env_[ch] + (1.0f - attackCoeff) * absIn;
            else
                env_[ch] = releaseCoeff * env_[ch] + (1.0f - releaseCoeff) * absIn;

            // Compute gain reduction.
            float gain = 1.0f;
            if (env_[ch] > thresholdLinear && thresholdLinear > 0.0f) {
                if (kneeDb > 0.0f) {
                    // Soft knee: linear interpolation in dB around threshold.
                    const float envDb = juce::Decibels::gainToDecibels(env_[ch]);
                    const float halfKnee = kneeDb * 0.5f;
                    const float overThresh = envDb - thresholdDb;
                    if (overThresh < halfKnee) {
                        // Inside the knee: blend between no-reduction and full-reduction.
                        const float t = (overThresh + halfKnee) / kneeDb;
                        const float kneeGainDb = -t * t * halfKnee;
                        gain = juce::Decibels::decibelsToGain(kneeGainDb);
                    } else {
                        gain = thresholdLinear / env_[ch];
                    }
                } else {
                    gain = thresholdLinear / env_[ch];
                }
            }

            s *= gain;

            // Hard mode: tanh saturation at high amounts for musical clipping.
            if (mode == Mode::Hard && amount > 0.3f) {
                const float drive = 1.0f + (amount - 0.3f) * 2.0f;
                s = std::tanh(s * drive) / drive;
            }
        }
    }
}

void LimitModule::reset() {
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
    env_[0] = env_[1] = 0.0f;
}
