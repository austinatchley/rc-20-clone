#include "WobbleModule.h"
#include "../parameters/ParameterIDs.h"

WobbleModule::WobbleModule() = default;

void WobbleModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts)
{
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::wobble_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::wobble_amount);
    modeParam_   = apvts.getRawParameterValue(ParameterIDs::wobble_mode);
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
    if (bypassParam_->load() > 0.5f)
        return;

    auto&     block       = ctx.block;
    const int numChannels = static_cast<int>(block.getNumChannels());
    const int numSamples  = static_cast<int>(block.getNumSamples());
    const auto mode       = static_cast<Mode>(static_cast<int>(modeParam_->load()));
    const float drift     = ctx.driftValue;

    smoothedAmount_.setTargetValue(amountParam_->load());

    // LFO base rate per mode (Hz).
    float lfoRateHz;
    switch (mode)
    {
        case Mode::Slow:   lfoRateHz = 0.4f; break;
        case Mode::Medium: lfoRateHz = 1.5f; break;
        case Mode::Fast:   lfoRateHz = 4.0f; break;
        default:           lfoRateHz = 0.4f; break;
    }

    // Drift modulates rate slightly for organic feel.
    lfoRateHz *= (1.0f + drift * 0.3f);

    const float twoPi      = juce::MathConstants<float>::twoPi;
    const float sampleRateF = static_cast<float>(sampleRate_);

    // Centre delay keeps the read head in positive territory at all modulation depths.
    // 5 ms gives enough headroom for the deepest wobble setting.
    const float centreDelaySamples = 5.0f * 0.001f * sampleRateF;

    for (int i = 0; i < numSamples; ++i)
    {
        const float amount = smoothedAmount_.getNextValue();

        // Depth in samples: up to 20 samples at amount=1, plus drift adds a touch more.
        const float depthSamples = amount * 20.0f + drift * 5.0f;

        const float delayL = centreDelaySamples + depthSamples * std::sin(twoPi * lfoPhaseL_);
        const float delayR = centreDelaySamples + depthSamples * std::sin(twoPi * lfoPhaseR_);

        // Advance LFO phases.
        lfoPhaseL_ += lfoRateHz / sampleRateF;
        lfoPhaseR_ += lfoRateHz / sampleRateF;
        if (lfoPhaseL_ >= 1.0f) lfoPhaseL_ -= 1.0f;
        if (lfoPhaseR_ >= 1.0f) lfoPhaseR_ -= 1.0f;

        // Process left channel (index 0) and right channel (index 1 if present).
        if (numChannels >= 1)
        {
            float* dataL = block.getChannelPointer(0);
            dataL[i] = delayL_.processSample(dataL[i], delayL);
        }
        if (numChannels >= 2)
        {
            float* dataR = block.getChannelPointer(1);
            dataR[i] = delayR_.processSample(dataR[i], delayR);
        }
    }
}

void WobbleModule::reset()
{
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
    delayL_.reset();
    delayR_.reset();
    lfoPhaseL_ = 0.0f;
    lfoPhaseR_ = 0.25f;
}
