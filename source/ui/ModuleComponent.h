#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Placeholder UI component for a single effect module.
 *
 * Contains:
 *   - Module name label
 *   - Bypass toggle button
 *   - Amount rotary knob
 *   - Mode combo box (choices populated from the AudioParameterChoice)
 *
 * All controls are wired to APVTS via attachment objects so automation and
 * preset recall work immediately with no extra plumbing.
 *
 * Phase 2: replace placeholder Slider/ComboBox with bespoke painted controls.
 */
class ModuleComponent final : public juce::Component
{
public:
    ModuleComponent(const juce::String&                    moduleName,
                    juce::AudioProcessorValueTreeState&    apvts,
                    const juce::String&                    bypassParamID,
                    const juce::String&                    amountParamID,
                    const juce::String&                    modeParamID);

    ~ModuleComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label        nameLabel_;
    juce::ToggleButton bypassButton_;
    juce::Slider       amountKnob_;
    juce::ComboBox     modeCombo_;

    // Attachments keep controls in sync with APVTS on both threads.
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   bypassAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   amountAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment_;
};
