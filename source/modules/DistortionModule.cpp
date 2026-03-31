#include "DistortionModule.h"

#include "../parameters/ParameterIDs.h"

DistortionModule::DistortionModule() = default;

void DistortionModule::prepareParameters(juce::AudioProcessorValueTreeState& apvts) {
    bypassParam_ = apvts.getRawParameterValue(ParameterIDs::distortion_bypass);
    amountParam_ = apvts.getRawParameterValue(ParameterIDs::distortion_amount);
    modeParam_ = apvts.getRawParameterValue(ParameterIDs::distortion_mode);
}

void DistortionModule::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate_ = spec.sampleRate;
    smoothedAmount_.reset(spec.sampleRate, 0.02);
    bypassGain_.reset(spec.sampleRate, 0.01);

    // Build oversampler for the configured channel count.
    oversampling_ = std::make_unique<juce::dsp::Oversampling<float>>(
        spec.numChannels,
        2,  // 2^2 = 4x
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true /*maximumQuality*/);
    oversampling_->initProcessing(spec.maximumBlockSize);

    reset();
}

void DistortionModule::process(ProcessContext& ctx) {
    if (bypassParam_->load() > 0.5f)
        return;

    auto& block = ctx.block;
    const int numSamples = static_cast<int>(block.getNumSamples());
    const auto mode = static_cast<Mode>(static_cast<int>(modeParam_->load()));

    smoothedAmount_.setTargetValue(amountParam_->load());

    // Upsample to 4x.
    auto oversampledBlock = oversampling_->processSamplesUp(block);
    const int osNumSamples = static_cast<int>(oversampledBlock.getNumSamples());
    const int osNumChannels = static_cast<int>(oversampledBlock.getNumChannels());

    // Process the oversampled block sample-by-sample.
    // SmoothedValue must advance at the *original* sample rate, so we read one
    // smoothed value per original sample and apply it to the 4 upsampled samples.
    const int factor = osNumSamples / numSamples;

    for (int i = 0; i < numSamples; ++i) {
        const float amount = smoothedAmount_.getNextValue();
        // Drive ranges from just-past-unity (1.0) up to 10 at amount=1.
        const float drive = 1.0f + amount * 9.0f;

        for (int ch = 0; ch < osNumChannels; ++ch) {
            float* data = oversampledBlock.getChannelPointer(static_cast<size_t>(ch));
            for (int k = 0; k < factor; ++k) {
                const int idx = i * factor + k;
                switch (mode) {
                    case Mode::Tape:
                        data[idx] = shapeTape(data[idx], drive);
                        break;
                    case Mode::Tube:
                        data[idx] = shapeTube(data[idx], drive);
                        break;
                    case Mode::Transistor:
                        data[idx] = shapeTransistor(data[idx], drive);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    // Downsample back to original rate.
    oversampling_->processSamplesDown(block);
}

void DistortionModule::reset() {
    smoothedAmount_.setCurrentAndTargetValue(0.0f);
    bypassGain_.setCurrentAndTargetValue(1.0f);
    if (oversampling_)
        oversampling_->reset();
}

// ── Waveshapers ───────────────────────────────────────────────────────────────

// Tape: symmetric soft clip via tanh, gain-compensated so unity at drive=1.
float DistortionModule::shapeTape(float x, float drive) noexcept {
    const float tanhDrive = std::tanh(drive);
    if (tanhDrive < 1e-6f)
        return x;
    return std::tanh(x * drive) / tanhDrive;
}

// Tube: sigmoid (logistic) shape — soft even-order harmonics, slight warmth.
// Biased to produce gentle asymmetry without audible DC.
float DistortionModule::shapeTube(float x, float drive) noexcept {
    // Sigmoid centred on 0 and normalised to [-1, 1].
    // f(x) = 2 / (1 + exp(-drive * x)) - 1
    // At drive=1 this is the standard tanh approximation; higher drive clips harder.
    const float y = 2.0f / (1.0f + std::exp(-drive * x)) - 1.0f;
    // Compensate for lower gain at drive=1 (sigmoid gain ≈ drive/2 at x=0).
    const float compensation = 2.0f / drive;
    return y * juce::jmin(compensation, 1.0f);
}

// Transistor: hard clip — cubic soft shoulder near the threshold.
float DistortionModule::shapeTransistor(float x, float drive) noexcept {
    const float threshold = 1.0f / drive;
    if (x > threshold)
        return threshold;
    if (x < -threshold)
        return -threshold;
    // Cubic soft shoulder: y = x - (drive^2 * x^3) / 3  within ±threshold
    return x - (drive * drive * x * x * x) / 3.0f;
}
