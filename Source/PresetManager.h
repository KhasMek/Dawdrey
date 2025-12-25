#pragma once

#include <JuceHeader.h>

class PresetManager
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts);

    void savePreset(const juce::String& presetName);
    void deletePreset(const juce::String& presetName);
    void loadPreset(const juce::String& presetName);
    void loadInitPreset();
    void loadNextPreset();
    void loadPreviousPreset();
    juce::StringArray getAllPresets() const;
    juce::String getCurrentPreset() const;
    int getCurrentPresetIndex() const;
    
    void loadPresetFromFile(const juce::File& file);
    void savePresetToFile(const juce::String& presetName, const juce::File& file);

private:
    void valueTreeRedirected(juce::ValueTree& treeWhichHasBeenChanged);
    void createFactoryPresets();

    juce::File getPresetsDirectory() const;

    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::String currentPresetName;
};
