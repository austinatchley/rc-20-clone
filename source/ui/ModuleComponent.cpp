#include "ModuleComponent.h"

static constexpr int kHeaderH = 30;

ModuleComponent::ModuleComponent(const juce::String& moduleName,
                                 juce::AudioProcessorValueTreeState& apvts,
                                 const juce::String& bypassParamID,
                                 const juce::String& amountParamID,
                                 const juce::String& modeParamID,
                                 juce::Colour accentColour)
    : lookAndFeel_(accentColour),
      accentColour_(accentColour) {
    setLookAndFeel(&lookAndFeel_);

    // ── Name label ────────────────────────────────────────────────────────────
    nameLabel_.setText(moduleName, juce::dontSendNotification);
    nameLabel_.setJustificationType(juce::Justification::centred);
    nameLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    nameLabel_.setColour(juce::Label::textColourId, accentColour_);
    addAndMakeVisible(nameLabel_);

    // ── Bypass LED button ─────────────────────────────────────────────────────
    bypassButton_.setButtonText("");
    addAndMakeVisible(bypassButton_);

    // ── Amount knob ───────────────────────────────────────────────────────────
    amountKnob_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    amountKnob_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(amountKnob_);

    // ── Mode combo box ────────────────────────────────────────────────────────
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

ModuleComponent::~ModuleComponent() {
    setLookAndFeel(nullptr);
}

void ModuleComponent::paint(juce::Graphics& g) {
    const auto b = getLocalBounds().reduced(2).toFloat();

    // Panel body
    g.setColour(RC20LookAndFeel::panel);
    g.fillRoundedRectangle(b, 6.0f);

    // Accent-tinted header band (rounded top, squared bottom)
    g.setColour(accentColour_.withAlpha(0.15f));
    g.fillRoundedRectangle(b.withHeight(kHeaderH), 6.0f);
    g.fillRect(b.withTrimmedTop(kHeaderH - 6.0f).withHeight(6.0f));

    // Subtle border in accent colour
    g.setColour(accentColour_.withAlpha(0.25f));
    g.drawRoundedRectangle(b, 6.0f, 1.0f);
}

void ModuleComponent::resized() {
    auto area = getLocalBounds().reduced(4);

    // ── Header: name label + bypass LED ──────────────────────────────────────
    auto header = area.removeFromTop(kHeaderH - 2);
    bypassButton_.setBounds(header.removeFromRight(28));
    nameLabel_.setBounds(header);

    area.removeFromTop(6);

    // ── Mode combo at bottom ──────────────────────────────────────────────────
    modeCombo_.setBounds(area.removeFromBottom(24));
    area.removeFromBottom(6);

    // ── Knob: centred in remaining space ──────────────────────────────────────
    const int side = juce::jmin(area.getWidth(), area.getHeight());
    amountKnob_.setBounds(area.withSizeKeepingCentre(side, side));
}
