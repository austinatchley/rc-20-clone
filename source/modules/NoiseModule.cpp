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
    if (bypassParam_->load() > 0.5f)
        return;

    auto&      block       = ctx.block;
    const int  numChannels = static_cast<int>(block.getNumChannels());
    const int  numSamples  = static_cast<int>(block.getNumSamples());
    const auto noiseType   = static_cast<Type>(static_cast<int>(typeParam_->load()));

    smoothedAmount_.setTargetValue(amountParam_->load());

    const float twoPi      = juce::MathConstants<float>::twoPi;
    const float sampleRateF = static_cast<float>(sampleRate_);

    for (int i = 0; i < numSamples; ++i)
    {
        const float amount = smoothedAmount_.getNextValue();
        float noise = 0.0f;

        switch (noiseType)
        {
            case Type::TapeHiss:
            {
                // Paul Kellet 3-coefficient pink noise approximation.
                const float white = rng_.nextFloat() * 2.0f - 1.0f;
                b0_ = 0.99886f * b0_ + white * 0.0555179f;
                b1_ = 0.99332f * b1_ + white * 0.0750759f;
                b2_ = 0.96900f * b2_ + white * 0.1538520f;
                noise = (b0_ + b1_ + b2_ + white * 0.5362f) * 0.11f;
                break;
            }

            case Type::VinylCrackle:
            {
                if (--crackleCountdown_ <= 0)
                {
                    // Poisson inter-arrival: rate proportional to amount.
                    const float rateHz = 0.5f + amount * 9.5f; // 0.5..10 crackles/sec
                    const float meanInterval = sampleRateF / rateHz;
                    // Exponential distribution: -ln(U) * mean
                    crackleCountdown_ = static_cast<int>(
                        -std::log(juce::jmax(1e-6f, rng_.nextFloat())) * meanInterval) + 1;
                    // Random amplitude, random polarity.
                    const float amp = rng_.nextFloat() * 0.6f + 0.4f;
                    crackleEnv_ = amp * (rng_.nextFloat() > 0.5f ? 1.0f : -1.0f);
                }
                noise = crackleEnv_;
                crackleEnv_ *= 0.97f; // exponential decay ~7 ms at 44.1 kHz
                break;
            }

            case Type::ElectricalHum:
            {
                // 60 Hz fundamental + 2nd and 3rd harmonics.
                noise = std::sin(twoPi * humPhase_[0]) * 0.70f
                      + std::sin(twoPi * humPhase_[1]) * 0.20f
                      + std::sin(twoPi * humPhase_[2]) * 0.10f;

                humPhase_[0] += 60.0f  / sampleRateF;
                humPhase_[1] += 120.0f / sampleRateF;
                humPhase_[2] += 180.0f / sampleRateF;

                if (humPhase_[0] >= 1.0f) humPhase_[0] -= 1.0f;
                if (humPhase_[1] >= 1.0f) humPhase_[1] -= 1.0f;
                if (humPhase_[2] >= 1.0f) humPhase_[2] -= 1.0f;
                break;
            }
        }

        // Noise is additive — inject into every channel.
        for (int ch = 0; ch < numChannels; ++ch)
            block.getChannelPointer(static_cast<size_t>(ch))[i] += amount * noise;
    }
}

void NoiseModule::reset()
{
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
    b0_ = b1_ = b2_ = 0.0f;
    crackleCountdown_ = 1;
    crackleEnv_       = 0.0f;
    humPhase_[0] = humPhase_[1] = humPhase_[2] = 0.0f;
}
