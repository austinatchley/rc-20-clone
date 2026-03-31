#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "RC20LookAndFeel.h"

/**
 * UI component for a single effect module.
 *
 * Contains:
 *   - Module name label (coloured with the module's accent)
 *   - Bypass LED toggle (lit = module active, dim = bypassed)
 *   - Amount rotary knob
 *   - Mode combo box (choices populated from AudioParameterChoice)
 *
 * All controls are wired to APVTS via attachment objects so automation and
 * preset recall work immediately with no extra plumbing.
 *
 * Each instance owns its own RC20LookAndFeel so the accent colour propagates
 * to all child controls without walking the parent hierarchy.
 */
class ModuleComponent final : public juce::Component
{
  public:
    ModuleComponent(const juce::String& moduleName,
                    juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& bypassParamID,
                    const juce::String& amountParamID,
                    const juce::String& modeParamID,
                    juce::Colour accentColour);

    ~ModuleComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

  private:
    // Declared first — must outlive all child components.
    RC20LookAndFeel lookAndFeel_;
    juce::Colour    accentColour_;

    juce::Label        nameLabel_;
    juce::ToggleButton bypassButton_;
    juce::Slider       amountKnob_;
    juce::ComboBox     modeCombo_;

    // Attachments keep controls in sync with APVTS on both threads.
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment_;
};
