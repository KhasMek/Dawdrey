#pragma once

#include "DSP/FeedbackSynthEngine.h"
#include "DSP/PitchShifter.h"
#include "DSP/SimpleLFO.h"
#include "PresetManager.h"
#include <JuceHeader.h>

class DawdreyAudioProcessor : public juce::AudioProcessor {
public:
  enum ModulationTarget {
    TARGET_NONE,
    TARGET_FREQ,
    TARGET_FB_GAIN,
    TARGET_FB_DELAY,
    TARGET_FB_LPF,
    TARGET_FB_HPF,
    TARGET_VERB_MIX,
    TARGET_VERB_DECAY,
    TARGET_ECHO_SEND,
    TARGET_ECHO_TIME,
    TARGET_ECHO_FB,
    TARGET_DRY_WET,
    TARGET_WIDTH,
    TARGET_GATE_THRESH,
    TARGET_GATE_RELEASE,
    TARGET_DRIVE_AMT,
    TARGET_DRIVE_GAIN,
    TARGET_PITCH_SHIFT,
    TARGET_PITCH_FINE,
    TARGET_LAST
  };
  DawdreyAudioProcessor();
  ~DawdreyAudioProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  juce::UndoManager undoManager;

  juce::AudioProcessorValueTreeState apvts;
  std::unique_ptr<PresetManager> presetManager;

  std::atomic<float> lfo1Value{0.0f};
  std::atomic<float> lfo2Value{0.0f};
  std::atomic<float> lfo3Value{0.0f};

  std::atomic<float> inputLevelL{0.0f};
  std::atomic<float> inputLevelR{0.0f};
  std::atomic<float> outputLevelL{0.0f};
  std::atomic<float> outputLevelR{0.0f};

  float gateCurrentGain = 0.0f;

  juce::AudioParameterFloat *freqParam = nullptr;
  juce::AudioParameterFloat *fbGainParam = nullptr;
  juce::AudioParameterFloat *fbDelayParam = nullptr;
  juce::AudioParameterFloat *fbLpfParam = nullptr;
  juce::AudioParameterFloat *fbHpfParam = nullptr;
  juce::AudioParameterFloat *verbMixParam = nullptr;
  juce::AudioParameterFloat *verbDecayParam = nullptr;
  juce::AudioParameterFloat *echoSendParam = nullptr;
  juce::AudioParameterFloat *echoTimeParam = nullptr;
  juce::AudioParameterFloat *echoFbParam = nullptr;
  juce::AudioParameterFloat *dryWetParam = nullptr;
  juce::AudioParameterFloat *widthParam = nullptr;

  juce::AudioParameterBool *gateEnabledParam = nullptr;
  juce::AudioParameterFloat *gateThreshParam = nullptr;
  juce::AudioParameterFloat *gateReleaseParam = nullptr;

  juce::AudioParameterBool *driveEnabledParam = nullptr;
  juce::AudioParameterFloat *driveAmountParam = nullptr;
  juce::AudioParameterFloat *driveGainParam = nullptr;

  juce::AudioParameterBool *pitchEnabledParam = nullptr;
  juce::AudioParameterFloat *pitchShiftParam = nullptr;
  juce::AudioParameterFloat *pitchFineParam = nullptr;

  juce::AudioParameterBool *instrumentModeParam = nullptr;
  std::atomic<int> lastMidiNote{69};

private:
  infrasonic::FeedbackSynth::Engine engine;
  daisysp::SimpleLFO lfo1;
  daisysp::SimpleLFO lfo2;
  daisysp::SimpleLFO lfo3;

  juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DawdreyAudioProcessor)
};
