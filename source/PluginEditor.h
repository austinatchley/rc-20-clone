#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

#include "PluginProcessor.h"
#include "ui/ModuleComponent.h"
#include "ui/PresetSelector.h"

/**
 * Main plugin editor.
 *
 * Layout (top to bottom):
 *   [Preset selector bar + global Drift knob + Output knob]
 *   [Noise | Wobble | Distortion | Space | Magic | Limit]
 *
 */
class RC20PluginEditor final : public juce::AudioProcessorEditor
{
  public:
    explicit RC20PluginEditor(RC20PluginProcessor& processor);
    ~RC20PluginEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

  private:
    RC20PluginProcessor& processor_;

    // ── Top bar ───────────────────────────────────────────────────────────────
    PresetSelector presetSelector_;

    juce::Label driftLabel_;
    juce::Slider driftSlider_;
    juce::Label outputLabel_;
    juce::Slider outputSlider_;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driftAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment_;

    // ── Module strip ──────────────────────────────────────────────────────────
    ModuleComponent noiseModule_;
    ModuleComponent wobbleModule_;
    ModuleComponent distortionModule_;
    ModuleComponent spaceModule_;
    ModuleComponent magicModule_;
    ModuleComponent limitModule_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RC20PluginEditor)
};
