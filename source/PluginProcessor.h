#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <memory>

#include "dsp/DriftGenerator.h"
#include "modules/EffectModule.h"
#include "modules/NoiseModule.h"
#include "modules/WobbleModule.h"
#include "modules/DistortionModule.h"
#include "modules/SpaceModule.h"
#include "modules/MagicModule.h"
#include "modules/LimitModule.h"

/**
 * Main plugin processor.
 *
 * Owns the fixed six-module signal chain and the shared DriftGenerator.
 * All parameter state lives in `apvts` and is serialised via XML.
 *
 * Signal chain order:  Noise → Wobble → Distortion → Space → Magic → Limit
 *
 * Audio thread contract
 * ---------------------
 * - No allocations in processBlock.
 * - No locks in processBlock.
 * - Parameters are read via cached std::atomic<float>* pointers only.
 */
class RC20PluginProcessor final : public juce::AudioProcessor
{
  public:
    RC20PluginProcessor();
    ~RC20PluginProcessor() override = default;

    // ── AudioProcessor ────────────────────────────────────────────────────────
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    // Tail time accounts for reverb decay in the Space module.
    double getTailLengthSeconds() const override { return 4.0; }

    // ── Programs (unused — presets handled via APVTS) ─────────────────────────
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    // ── State serialisation ───────────────────────────────────────────────────
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // ── Public state ──────────────────────────────────────────────────────────
    juce::AudioProcessorValueTreeState apvts;

  private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    DriftGenerator driftGenerator_;

    // Fixed signal chain — index order matches processing order.
    std::array<std::unique_ptr<EffectModule>, 6> modules_;

    std::atomic<float>* driftParam_ = nullptr;
    std::atomic<float>* outputLevelParam_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RC20PluginProcessor)
};
