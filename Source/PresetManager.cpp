#include "PresetManager.h"

PresetManager::PresetManager(juce::AudioProcessorValueTreeState &apvts)
    : valueTreeState(apvts) {
  const auto defaultDirectory = getPresetsDirectory();
  if (!defaultDirectory.exists()) {
    defaultDirectory.createDirectory();
  }

  createFactoryPresets();
}

void PresetManager::savePreset(const juce::String &presetName) {
  if (presetName.isEmpty())
    return;

  const auto xml = valueTreeState.copyState().createXml();
  const auto presetFile =
      getPresetsDirectory().getChildFile(presetName + ".xml");

  if (xml->writeTo(presetFile)) {
    currentPresetName = presetName;
    DBG("Preset saved: " + presetName);
  }
}

void PresetManager::deletePreset(const juce::String &presetName) {
  if (presetName.isEmpty())
    return;

  const auto presetFile =
      getPresetsDirectory().getChildFile(presetName + ".xml");
  if (presetFile.existsAsFile()) {
    if (!presetFile.deleteFile()) {
      DBG("Failed to delete preset: " + presetName);
    }
  }
}

void PresetManager::loadPreset(const juce::String &presetName) {
  if (presetName.isEmpty())
    return;

  const auto presetFile =
      getPresetsDirectory().getChildFile(presetName + ".xml");
  if (!presetFile.existsAsFile())
    return;

  auto xmlElement = juce::XmlDocument::parse(presetFile);
  if (xmlElement) {
    valueTreeState.replaceState(juce::ValueTree::fromXml(*xmlElement));
    currentPresetName = presetName;
    DBG("Preset loaded: " + presetName);
  }
}

void PresetManager::loadInitPreset() {
  auto &state = valueTreeState.state;

  auto &processor = valueTreeState.processor;
  for (auto *param : processor.getParameters()) {
    if (auto *p = dynamic_cast<juce::AudioProcessorParameterWithID *>(param)) {
      p->setValueNotifyingHost(p->getDefaultValue());
    }
  }

  currentPresetName = "Init";
}

void PresetManager::loadPresetFromFile(const juce::File &file) {
  if (!file.existsAsFile())
    return;

  auto xmlElement = juce::XmlDocument::parse(file);
  if (xmlElement) {
    valueTreeState.replaceState(juce::ValueTree::fromXml(*xmlElement));

    auto presetName = file.getFileNameWithoutExtension();
    savePreset(presetName);

    currentPresetName = presetName;
    DBG("Preset imported: " + presetName);
  }
}

void PresetManager::savePresetToFile(const juce::String &presetName,
                                     const juce::File &file) {
  const auto xml = valueTreeState.copyState().createXml();
  if (xml->writeTo(file)) {
    DBG("Preset exported to: " + file.getFullPathName());
  }
}

void PresetManager::loadNextPreset() {
  const auto allPresets = getAllPresets();
  if (allPresets.isEmpty())
    return;

  const auto currentIndex = allPresets.indexOf(currentPresetName);
  const auto nextIndex = (currentIndex + 1) % allPresets.size();
  loadPreset(allPresets[nextIndex]);
}

void PresetManager::loadPreviousPreset() {
  const auto allPresets = getAllPresets();
  if (allPresets.isEmpty())
    return;

  const auto currentIndex = allPresets.indexOf(currentPresetName);
  const auto previousIndex =
      (currentIndex - 1 + allPresets.size()) % allPresets.size();
  loadPreset(allPresets[previousIndex]);
}

juce::StringArray PresetManager::getAllPresets() const {
  juce::StringArray presets;
  const auto directory = getPresetsDirectory();
  const auto files =
      directory.findChildFiles(juce::File::findFiles, false, "*.xml");

  for (const auto &file : files) {
    presets.add(file.getFileNameWithoutExtension());
  }
  return presets;
}

juce::String PresetManager::getCurrentPreset() const {
  return currentPresetName;
}

int PresetManager::getCurrentPresetIndex() const {
  return getAllPresets().indexOf(currentPresetName);
}

juce::File PresetManager::getPresetsDirectory() const {
  juce::File rootDir;

#if JUCE_MAC
  // ~/Library/Audio/Presets/Dawdrey/
  rootDir =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getChildFile("Audio")
          .getChildFile("Presets")
          .getChildFile("Dawdrey");
#elif JUCE_WINDOWS
  // AppData/Roaming/Dawdrey/Presets/
  rootDir =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getChildFile("Dawdrey")
          .getChildFile("Presets");
#elif JUCE_LINUX
  // ~/.config/Dawdrey/Presets/
  rootDir =
      juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
          .getChildFile("Dawdrey")
          .getChildFile("Presets");
#else
  // Fallback
  rootDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("Dawdrey")
                .getChildFile("Presets");
#endif

  return rootDir;
}

void PresetManager::createFactoryPresets() {
  const auto dir = getPresetsDirectory();
  if (dir.findChildFiles(juce::File::findFiles, false, "*.xml").size() > 0)
    return;

  auto createPreset = [&](juce::String name,
                          std::map<juce::String, float> params) {
    loadInitPreset();

    for (auto &p : params) {
      if (auto *param = valueTreeState.getParameter(p.first)) {
        if (auto *rangedParam =
                dynamic_cast<juce::RangedAudioParameter *>(param)) {
          float normalized = rangedParam->convertTo0to1(p.second);
          rangedParam->setValueNotifyingHost(normalized);
        }
      }
    }

    savePreset(name);
  };

  // 1. Factory - Lush Pad
  // Slow attack, reverb, chorus-y modulation
  std::map<juce::String, float> lushPad;
  lushPad["freq"] = 0.2f; // Low pitch
  lushPad["verb_mix"] = 0.6f;
  lushPad["verb_decay"] = 0.8f;
  lushPad["width"] = 1.5f;
  lushPad["lfo1_rate"] = 0.2f;
  lushPad["lfo1_depth"] = 0.3f;
  lushPad["lfo1_target"] = 1.0f; // Freq
  createPreset("Factory - Lush Pad", lushPad);

  // 2. Factory - Rhythmic Gater
  // Gate enabled, LFO on Gate Thresh
  std::map<juce::String, float> rhythmicGater;
  rhythmicGater["gate_enabled"] = 1.0f;
  rhythmicGater["gate_thresh"] = -20.0f;
  rhythmicGater["lfo1_sync"] = 1.0f;
  rhythmicGater["lfo1_div"] = 10.0f;    // 1/16
  rhythmicGater["lfo1_shape"] = 4.0f;   // Square
  rhythmicGater["lfo1_target"] = 13.0f; // Gate Thresh
  rhythmicGater["lfo1_depth"] = 0.8f;
  createPreset("Factory - Rhythmic Gater", rhythmicGater);

  // 3. Factory - Deep Space
  // High feedback, long delay, echo
  std::map<juce::String, float> deepSpace;
  deepSpace["fb_gain"] = 0.95f;
  deepSpace["fb_delay"] = 0.5f;
  deepSpace["echo_send"] = 0.7f;
  deepSpace["echo_time"] = 1.5f;
  deepSpace["echo_fb"] = 0.8f;
  deepSpace["verb_mix"] = 0.8f;
  createPreset("Factory - Deep Space", deepSpace);

  // 4. Factory - Acid Lead
  // Drive, Saw LFO on Filter
  std::map<juce::String, float> acidLead;
  acidLead["drive_enabled"] = 1.0f;
  acidLead["drive_amount"] = 0.6f;
  acidLead["drive_gain"] = -3.0f;
  acidLead["fb_lpf"] = 2000.0f;
  acidLead["lfo1_rate"] = 8.0f;
  acidLead["lfo1_shape"] = 2.0f;  // Saw
  acidLead["lfo1_target"] = 4.0f; // FB LPF
  acidLead["lfo1_depth"] = 0.7f;
  createPreset("Factory - Acid Lead", acidLead);

  // Reset to Init after creation
  loadInitPreset();
}
