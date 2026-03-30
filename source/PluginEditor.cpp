#include "PluginEditor.h"
#include "parameters/ParameterIDs.h"

RC20PluginEditor::RC20PluginEditor(RC20PluginProcessor& processor)
    : AudioProcessorEditor(&processor),
      processor_(processor),
      presetSelector_(processor),
      noiseModule_     ("Noise",      processor.apvts,
                        ParameterIDs::noise_bypass,
                        ParameterIDs::noise_amount,
                        ParameterIDs::noise_type),
      wobbleModule_    ("Wobble",     processor.apvts,
                        ParameterIDs::wobble_bypass,
                        ParameterIDs::wobble_amount,
                        ParameterIDs::wobble_mode),
      distortionModule_("Distortion", processor.apvts,
                        ParameterIDs::distortion_bypass,
                        ParameterIDs::distortion_amount,
                        ParameterIDs::distortion_mode),
      spaceModule_     ("Space",      processor.apvts,
                        ParameterIDs::space_bypass,
                        ParameterIDs::space_amount,
                        ParameterIDs::space_mode),
      magicModule_     ("Magic",      processor.apvts,
                        ParameterIDs::magic_bypass,
                        ParameterIDs::magic_amount,
                        ParameterIDs::magic_mode),
      limitModule_     ("Limit",      processor.apvts,
                        ParameterIDs::limit_bypass,
                        ParameterIDs::limit_amount,
                        ParameterIDs::limit_mode)
{
    // ── Global controls ───────────────────────────────────────────────────────
    driftLabel_.setText("Drift", juce::dontSendNotification);
    driftLabel_.setJustificationType(juce::Justification::centred);
    driftSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driftSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

    outputLabel_.setText("Output", juce::dontSendNotification);
    outputLabel_.setJustificationType(juce::Justification::centred);
    outputSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);

    driftAttachment_  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, ParameterIDs::drift,        driftSlider_);
    outputAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, ParameterIDs::output_level, outputSlider_);

    // ── Add all children ──────────────────────────────────────────────────────
    for (auto* child : { static_cast<juce::Component*>(&presetSelector_),
                         &driftLabel_, &driftSlider_,
                         &outputLabel_, &outputSlider_,
                         &noiseModule_, &wobbleModule_, &distortionModule_,
                         &spaceModule_, &magicModule_, &limitModule_ })
        addAndMakeVisible(child);

    setSize(920, 340);
    setResizable(false, false);
}

void RC20PluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a2e));  // placeholder dark background
}

void RC20PluginEditor::resized()
{
    auto area = getLocalBounds().reduced(8);

    // ── Top bar ───────────────────────────────────────────────────────────────
    auto topBar = area.removeFromTop(50);

    // Global knobs on the right
    auto outputArea = topBar.removeFromRight(65);
    outputLabel_.setBounds(outputArea.removeFromTop(18));
    outputSlider_.setBounds(outputArea);

    auto driftArea = topBar.removeFromRight(65);
    driftLabel_.setBounds(driftArea.removeFromTop(18));
    driftSlider_.setBounds(driftArea);

    topBar.removeFromRight(8);
    presetSelector_.setBounds(topBar);

    area.removeFromTop(8);

    // ── Module strip ──────────────────────────────────────────────────────────
    const int moduleWidth = area.getWidth() / 6;
    noiseModule_     .setBounds(area.removeFromLeft(moduleWidth));
    wobbleModule_    .setBounds(area.removeFromLeft(moduleWidth));
    distortionModule_.setBounds(area.removeFromLeft(moduleWidth));
    spaceModule_     .setBounds(area.removeFromLeft(moduleWidth));
    magicModule_     .setBounds(area.removeFromLeft(moduleWidth));
    limitModule_     .setBounds(area);  // last module gets any remaining pixels
}
