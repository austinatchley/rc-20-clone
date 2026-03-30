#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Minimal preset selector bar.
 *
 * Displays a combo box of saved presets with Save and Load buttons.
 * Serialisation uses APVTS XML via getStateInformation / setStateInformation,
 * stored as individual .xml files in the user's app data directory.
 *
 * Phase 1: UI laid out and wired up.  File I/O is functional but basic —
 * no preset browser, no factory presets, no undo for overwrites.
 */
class PresetSelector final : public juce::Component
{
public:
    explicit PresetSelector(juce::AudioProcessor& processor);
    ~PresetSelector() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void refreshPresetList();
    void saveCurrentPreset();
    void loadSelectedPreset();

    static juce::File getPresetsDirectory();

    juce::AudioProcessor& processor_;

    juce::ComboBox   presetCombo_;
    juce::TextButton saveButton_  { "Save"  };
    juce::TextButton loadButton_  { "Load"  };
    juce::Label      titleLabel_;
};
