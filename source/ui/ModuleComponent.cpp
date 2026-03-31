#include "ModuleComponent.h"

ModuleComponent::ModuleComponent(const juce::String& moduleName,
                                 juce::AudioProcessorValueTreeState& apvts,
                                 const juce::String& bypassParamID,
                                 const juce::String& amountParamID,
                                 const juce::String& modeParamID) {
    // ── Name label ────────────────────────────────────────────────────────────
    nameLabel_.setText(moduleName, juce::dontSendNotification);
    nameLabel_.setJustificationType(juce::Justification::centred);
    nameLabel_.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(nameLabel_);

    // ── Bypass button ─────────────────────────────────────────────────────────
    bypassButton_.setButtonText("On");
    bypassButton_.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(bypassButton_);

    // ── Amount knob ───────────────────────────────────────────────────────────
    amountKnob_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    amountKnob_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    addAndMakeVisible(amountKnob_);

    // ── Mode combo box ────────────────────────────────────────────────────────
    // Populate items from the AudioParameterChoice so labels always match DSP.
    if (auto* choiceParam =
            dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(modeParamID))) {
        modeCombo_.addItemList(choiceParam->choices, 1);
    }
    addAndMakeVisible(modeCombo_);

    // ── APVTS attachments ─────────────────────────────────────────────────────
    bypassAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, bypassParamID, bypassButton_);
    amountAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, amountParamID, amountKnob_);
    modeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, modeParamID, modeCombo_);
}

void ModuleComponent::paint(juce::Graphics& g) {
    // Placeholder: draw a rounded border so modules are visually distinct.
    g.setColour(juce::Colours::white.withAlpha(0.05f));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(2.0f), 6.0f);

    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(2.0f), 6.0f, 1.0f);
}

void ModuleComponent::resized() {
    auto area = getLocalBounds().reduced(6);

    nameLabel_.setBounds(area.removeFromTop(22));
    bypassButton_.setBounds(area.removeFromTop(24));
    area.removeFromTop(4);
    modeCombo_.setBounds(area.removeFromBottom(24));
    area.removeFromBottom(4);
    amountKnob_.setBounds(area);  // remaining space for the rotary
}
