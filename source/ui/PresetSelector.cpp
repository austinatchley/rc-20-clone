#include "PresetSelector.h"

PresetSelector::PresetSelector(juce::AudioProcessor& processor) : processor_(processor) {
    titleLabel_.setText("Preset", juce::dontSendNotification);
    titleLabel_.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel_);

    presetCombo_.setTextWhenNothingSelected("-- init --");
    addAndMakeVisible(presetCombo_);

    saveButton_.onClick = [this] { saveCurrentPreset(); };
    addAndMakeVisible(saveButton_);

    loadButton_.onClick = [this] { loadSelectedPreset(); };
    addAndMakeVisible(loadButton_);

    refreshPresetList();
}

void PresetSelector::paint(juce::Graphics& g) {
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(2.0f), 4.0f);
}

void PresetSelector::resized() {
    auto area = getLocalBounds().reduced(4);
    titleLabel_.setBounds(area.removeFromLeft(50));
    saveButton_.setBounds(area.removeFromRight(54));
    area.removeFromRight(4);
    loadButton_.setBounds(area.removeFromRight(54));
    area.removeFromRight(4);
    presetCombo_.setBounds(area);
}

// ── Private helpers ────────────────────────────────────────────────────────────

juce::File PresetSelector::getPresetsDirectory() {
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("RC20Clone")
                   .getChildFile("Presets");
    dir.createDirectory();
    return dir;
}

void PresetSelector::refreshPresetList() {
    presetCombo_.clear(juce::dontSendNotification);
    int id = 1;
    for (auto& f : getPresetsDirectory().findChildFiles(juce::File::findFiles, false, "*.xml"))
        presetCombo_.addItem(f.getFileNameWithoutExtension(), id++);
}

void PresetSelector::saveCurrentPreset() {
    fileChooser_ =
        std::make_unique<juce::FileChooser>("Save Preset", getPresetsDirectory(), "*.xml");

    fileChooser_->launchAsync(
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser) {
            auto file = chooser.getResult();
            if (file == juce::File{})
                return;

            // Ensure .xml extension.
            if (!file.hasFileExtension("xml"))
                file = file.withFileExtension("xml");

            juce::MemoryBlock data;
            processor_.getStateInformation(data);
            file.replaceWithData(data.getData(), data.getSize());
            refreshPresetList();
        });
}

void PresetSelector::loadSelectedPreset() {
    const auto name = presetCombo_.getText();
    if (name.isEmpty())
        return;

    auto file = getPresetsDirectory().getChildFile(name + ".xml");
    if (!file.existsAsFile())
        return;

    juce::MemoryBlock data;
    if (file.loadFileAsData(data))
        processor_.setStateInformation(data.getData(), static_cast<int>(data.getSize()));
}
