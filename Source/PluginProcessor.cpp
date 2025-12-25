#include "PluginProcessor.h"
#include "PluginEditor.h"

DawdreyAudioProcessor::DawdreyAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, &undoManager, "Parameters", createParameterLayout())
#endif
{
  presetManager = std::make_unique<PresetManager>(apvts);
  freqParam =
      dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter("freq"));
  fbGainParam =
      dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter("fb_gain"));
  fbDelayParam =
      dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter("fb_delay"));
  fbLpfParam =
      dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter("fb_lpf"));
  fbHpfParam =
      dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter("fb_hpf"));
  verbMixParam =
      dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter("verb_mix"));
  verbDecayParam = dynamic_cast<juce::AudioParameterFloat *>(
      apvts.getParameter("verb_decay"));
  echoSendParam = dynamic_cast<juce::AudioParameterFloat *>(
      apvts.getParameter("echo_send"));
  echoTimeParam = dynamic_cast<juce::AudioParameterFloat *>(
      apvts.getParameter("echo_time"));
  echoFbParam =
      dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter("echo_fb"));
  dryWetParam =
      dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter("dry_wet"));
  widthParam =
      dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter("width"));

  gateEnabledParam = dynamic_cast<juce::AudioParameterBool *>(
      apvts.getParameter("gate_enabled"));
  gateThreshParam = dynamic_cast<juce::AudioParameterFloat *>(
      apvts.getParameter("gate_thresh"));
  gateReleaseParam = dynamic_cast<juce::AudioParameterFloat *>(
      apvts.getParameter("gate_release"));

  driveEnabledParam = dynamic_cast<juce::AudioParameterBool *>(
      apvts.getParameter("drive_enabled"));
  driveAmountParam = dynamic_cast<juce::AudioParameterFloat *>(
      apvts.getParameter("drive_amount"));
  driveGainParam = dynamic_cast<juce::AudioParameterFloat *>(
      apvts.getParameter("drive_gain"));

  pitchEnabledParam = dynamic_cast<juce::AudioParameterBool *>(
      apvts.getParameter("pitch_enabled"));
  pitchShiftParam = dynamic_cast<juce::AudioParameterFloat *>(
      apvts.getParameter("pitch_shift"));
  pitchFineParam = dynamic_cast<juce::AudioParameterFloat *>(
      apvts.getParameter("pitch_fine"));

  instrumentModeParam = dynamic_cast<juce::AudioParameterBool *>(
      apvts.getParameter("instrument_mode"));
}

DawdreyAudioProcessor::~DawdreyAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
DawdreyAudioProcessor::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "freq", "Frequency", juce::NormalisableRange<float>(0.0f, 127.0f, 0.1f),
      40.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "fb_gain", "Feedback Gain",
      juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f), -6.0f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "fb_delay", "Feedback Body",
      juce::NormalisableRange<float>(0.001f, 0.25f, 0.001f, 0.3f), 0.064f,
      juce::String(), juce::AudioProcessorParameter::genericParameter,
      [](float value, int) {
        return juce::String(std::round(value * 1000.0f)) + " ms";
      },
      [](const juce::String &text) { return text.getFloatValue() * 0.001f; }));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "fb_lpf", "Feedback LPF",
      juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 18000.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "fb_hpf", "Feedback HPF",
      juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 60.0f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "verb_mix", "Reverb Mix",
      juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "verb_decay", "Reverb Decay",
      juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.85f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "echo_send", "Echo Send",
      juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "echo_time", "Echo Time",
      juce::NormalisableRange<float>(0.01f, 5.0f, 0.01f, 0.3f), 0.5f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "echo_fb", "Echo Feedback",
      juce::NormalisableRange<float>(0.0f, 1.2f, 0.01f), 0.5f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "dry_wet", "Dry/Wet Mix",
      juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "width", "Stereo Width",
      juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f), 1.0f));
  layout.add(std::make_unique<juce::AudioParameterBool>("gate_enabled",
                                                        "Gate Enabled", false));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "gate_thresh", "Gate Threshold",
      juce::NormalisableRange<float>(-100.0f, 0.0f, 0.1f), -60.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "gate_release", "Gate Release",
      juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f, 0.5f), 200.0f));

  layout.add(std::make_unique<juce::AudioParameterBool>(
      "drive_enabled", "Drive Enabled", false));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "drive_amount", "Drive Amount",
      juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "drive_gain", "Drive Gain",
      juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

  layout.add(std::make_unique<juce::AudioParameterBool>(
      "pitch_enabled", "Pitch Enabled", false));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "pitch_shift", "Pitch Shift",
      juce::NormalisableRange<float>(-12.0f, 12.0f, 1.0f), 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "pitch_fine", "Pitch Fine",
      juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f), 0.0f));

  layout.add(std::make_unique<juce::AudioParameterBool>(
      "instrument_mode", "Instrument Mode", false));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "lfo1_rate", "LFO 1 Rate",
      juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f, 0.5f), 1.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "lfo1_depth", "LFO 1 Depth",
      juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "lfo1_shape", "LFO 1 Shape",
      juce::StringArray{"Sine", "Tri", "Saw", "Ramp", "Square", "Random"}, 0));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "lfo1_target", "LFO 1 Target",
      juce::StringArray{"None", "Freq", "FB Gain", "FB Delay", "FB LPF",
                        "FB HPF", "Verb Mix", "Verb Decay", "Echo Send",
                        "Echo Time", "Echo FB", "Dry/Wet", "Width",
                        "Gate Thresh", "Gate Release", "Drive Amt",
                        "Drive Gain", "Pitch Shift", "Pitch Fine"},
      0));
  layout.add(std::make_unique<juce::AudioParameterBool>("lfo1_sync",
                                                        "LFO 1 Sync", false));
  layout.add(std::make_unique<juce::AudioParameterBool>("lfo1_bipolar",
                                                        "LFO 1 Bipolar", true));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "lfo1_div", "LFO 1 Division",
      juce::StringArray{"64 Bars", "32 Bars", "16 Bars", "8 Bars", "4 Bars",
                        "2 Bars", "1 Bar", "1/2", "1/4", "1/8", "1/16", "1/32",
                        "1/64"},
      8));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "lfo2_rate", "LFO 2 Rate",
      juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f, 0.5f), 0.5f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "lfo2_depth", "LFO 2 Depth",
      juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "lfo2_shape", "LFO 2 Shape",
      juce::StringArray{"Sine", "Tri", "Saw", "Ramp", "Square", "Random"}, 1));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "lfo2_target", "LFO 2 Target",
      juce::StringArray{"None", "Freq", "FB Gain", "FB Delay", "FB LPF",
                        "FB HPF", "Verb Mix", "Verb Decay", "Echo Send",
                        "Echo Time", "Echo FB", "Dry/Wet", "Width",
                        "Gate Thresh", "Gate Release", "Drive Amt",
                        "Drive Gain", "Pitch Shift", "Pitch Fine"},
      0));
  layout.add(std::make_unique<juce::AudioParameterBool>("lfo2_sync",
                                                        "LFO 2 Sync", false));
  layout.add(std::make_unique<juce::AudioParameterBool>("lfo2_bipolar",
                                                        "LFO 2 Bipolar", true));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "lfo2_div", "LFO 2 Division",
      juce::StringArray{"64 Bars", "32 Bars", "16 Bars", "8 Bars", "4 Bars",
                        "2 Bars", "1 Bar", "1/2", "1/4", "1/8", "1/16", "1/32",
                        "1/64"},
      8));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "lfo3_rate", "LFO 3 Rate",
      juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f, 0.5f), 0.2f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "lfo3_depth", "LFO 3 Depth",
      juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "lfo3_shape", "LFO 3 Shape",
      juce::StringArray{"Sine", "Tri", "Saw", "Ramp", "Square", "Random"}, 0));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "lfo3_target", "LFO 3 Target",
      juce::StringArray{"None", "Freq", "FB Gain", "FB Delay", "FB LPF",
                        "FB HPF", "Verb Mix", "Verb Decay", "Echo Send",
                        "Echo Time", "Echo FB", "Dry/Wet", "Width",
                        "Gate Thresh", "Gate Release", "Drive Amt",
                        "Drive Gain", "Pitch Shift", "Pitch Fine"},
      0));
  layout.add(std::make_unique<juce::AudioParameterBool>("lfo3_sync",
                                                        "LFO 3 Sync", false));
  layout.add(std::make_unique<juce::AudioParameterBool>("lfo3_bipolar",
                                                        "LFO 3 Bipolar", true));
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      "lfo3_div", "LFO 3 Division",
      juce::StringArray{"64 Bars", "32 Bars", "16 Bars", "8 Bars", "4 Bars",
                        "2 Bars", "1 Bar", "1/2", "1/4", "1/8", "1/16", "1/32",
                        "1/64"},
      8));

  return layout;
}

const juce::String DawdreyAudioProcessor::getName() const { return "Dawdrey"; }

bool DawdreyAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool DawdreyAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool DawdreyAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double DawdreyAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int DawdreyAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int DawdreyAudioProcessor::getCurrentProgram() { return 0; }

void DawdreyAudioProcessor::setCurrentProgram(int index) {}

const juce::String DawdreyAudioProcessor::getProgramName(int index) {
  return {};
}

void DawdreyAudioProcessor::changeProgramName(int index,
                                              const juce::String &newName) {}

void DawdreyAudioProcessor::prepareToPlay(double sampleRate,
                                          int samplesPerBlock) {
  engine.Init(static_cast<float>(sampleRate));
  lfo1.Init(static_cast<float>(sampleRate));
  lfo2.Init(static_cast<float>(sampleRate));
  lfo3.Init(static_cast<float>(sampleRate));
}

void DawdreyAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DawdreyAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void DawdreyAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                         juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  // Helper to get BPM
  float bpm = 120.0f;
  if (auto *ph = getPlayHead()) {
    if (auto pos = ph->getPosition())
      if (pos->getBpm().hasValue())
        bpm = *pos->getBpm();
  }

  // Helper to calculate freq from division
  auto getSyncedFreq = [&](int divIndex) -> float {
    float oneBarHz = bpm / 240.0f; // 1 Bar = 4 Beats
    switch (divIndex) {
    case 0:
      return oneBarHz / 64.0f;
    case 1:
      return oneBarHz / 32.0f;
    case 2:
      return oneBarHz / 16.0f;
    case 3:
      return oneBarHz / 8.0f;
    case 4:
      return oneBarHz / 4.0f;
    case 5:
      return oneBarHz / 2.0f;
    case 6:
      return oneBarHz;
    case 7:
      return oneBarHz * 2.0f;
    case 8:
      return oneBarHz * 4.0f;
    case 9:
      return oneBarHz * 8.0f;
    case 10:
      return oneBarHz * 16.0f;
    case 11:
      return oneBarHz * 32.0f;
    case 12:
      return oneBarHz * 64.0f;
    default:
      return oneBarHz;
    }
  };

  bool lfo1Sync = (bool)apvts.getRawParameterValue("lfo1_sync")->load();
  float lfo1Rate =
      lfo1Sync
          ? getSyncedFreq((int)apvts.getRawParameterValue("lfo1_div")->load())
          : apvts.getRawParameterValue("lfo1_rate")->load();
  lfo1.SetRate(lfo1Rate);
  lfo1.SetAmp(apvts.getRawParameterValue("lfo1_depth")->load());
  lfo1.SetWaveform((int)apvts.getRawParameterValue("lfo1_shape")->load());
  int lfo1Target = (int)apvts.getRawParameterValue("lfo1_target")->load();

  bool lfo2Sync = (bool)apvts.getRawParameterValue("lfo2_sync")->load();
  float lfo2Rate =
      lfo2Sync
          ? getSyncedFreq((int)apvts.getRawParameterValue("lfo2_div")->load())
          : apvts.getRawParameterValue("lfo2_rate")->load();
  lfo2.SetRate(lfo2Rate);
  lfo2.SetAmp(apvts.getRawParameterValue("lfo2_depth")->load());
  lfo2.SetWaveform((int)apvts.getRawParameterValue("lfo2_shape")->load());
  int lfo2Target = (int)apvts.getRawParameterValue("lfo2_target")->load();

  bool lfo3Sync = (bool)apvts.getRawParameterValue("lfo3_sync")->load();
  float lfo3Rate =
      lfo3Sync
          ? getSyncedFreq((int)apvts.getRawParameterValue("lfo3_div")->load())
          : apvts.getRawParameterValue("lfo3_rate")->load();
  lfo3.SetRate(lfo3Rate);
  lfo3.SetAmp(apvts.getRawParameterValue("lfo3_depth")->load());
  lfo3.SetWaveform((int)apvts.getRawParameterValue("lfo3_shape")->load());
  int lfo3Target = (int)apvts.getRawParameterValue("lfo3_target")->load();

  float lfo1Val = 0.0f;
  float lfo2Val = 0.0f;
  float lfo3Val = 0.0f;

  for (int s = 0; s < buffer.getNumSamples(); ++s) {
    lfo1Val = lfo1.Process();
    lfo2Val = lfo2.Process();
    lfo3Val = lfo3.Process();
  }

  bool lfo1Bipolar = *apvts.getRawParameterValue("lfo1_bipolar") > 0.5f;
  bool lfo2Bipolar = *apvts.getRawParameterValue("lfo2_bipolar") > 0.5f;
  bool lfo3Bipolar = *apvts.getRawParameterValue("lfo3_bipolar") > 0.5f;

  if (!lfo1Bipolar)
    lfo1Val = (lfo1Val + 1.0f) * 0.5f;
  if (!lfo2Bipolar)
    lfo2Val = (lfo2Val + 1.0f) * 0.5f;
  if (!lfo3Bipolar)
    lfo3Val = (lfo3Val + 1.0f) * 0.5f;

  lfo1Value.store(lfo1Val);
  lfo2Value.store(lfo2Val);
  lfo3Value.store(lfo3Val);

  // --- Apply Modulation (Normalized "Knob Sweep") ---

  // Helper to calculate modulated value for a parameter
  auto getModulatedValue = [&](juce::AudioParameterFloat *param,
                               int targetEnum) -> float {
    if (!param)
      return 0.0f;

    float currentNorm =
        param->getNormalisableRange().convertTo0to1(param->get()); // 0.0 to 1.0
    float totalMod = 0.0f;

    // Sum modulation from all LFOs targeting this parameter
    // We assume 100% Depth = +/- 50% of the knob range
    if (lfo1Target == targetEnum)
      totalMod +=
          lfo1Val * apvts.getRawParameterValue("lfo1_depth")->load() * 0.5f;
    if (lfo2Target == targetEnum)
      totalMod +=
          lfo2Val * apvts.getRawParameterValue("lfo2_depth")->load() * 0.5f;
    if (lfo3Target == targetEnum)
      totalMod +=
          lfo3Val * apvts.getRawParameterValue("lfo3_depth")->load() * 0.5f;

    float newNorm = juce::jlimit(0.0f, 1.0f, currentNorm + totalMod);
    return param->getNormalisableRange().convertFrom0to1(newNorm);
  };

  float freq = getModulatedValue(freqParam, TARGET_FREQ);
  float fbGain = getModulatedValue(fbGainParam, TARGET_FB_GAIN);
  float fbDelay = getModulatedValue(fbDelayParam, TARGET_FB_DELAY);
  float fbLpf = getModulatedValue(fbLpfParam, TARGET_FB_LPF);
  float fbHpf = getModulatedValue(fbHpfParam, TARGET_FB_HPF);
  float verbMix = getModulatedValue(verbMixParam, TARGET_VERB_MIX);
  float verbDecay = getModulatedValue(verbDecayParam, TARGET_VERB_DECAY);
  float echoSend = getModulatedValue(echoSendParam, TARGET_ECHO_SEND);
  float echoTime = getModulatedValue(echoTimeParam, TARGET_ECHO_TIME);
  float echoFb = getModulatedValue(echoFbParam, TARGET_ECHO_FB);
  float dryWet = getModulatedValue(dryWetParam, TARGET_DRY_WET);
  float width = getModulatedValue(widthParam, TARGET_WIDTH);

  float gateThresh = getModulatedValue(gateThreshParam, TARGET_GATE_THRESH);
  float driveAmt = getModulatedValue(driveAmountParam, TARGET_DRIVE_AMT);
  float gateRelease = getModulatedValue(gateReleaseParam, TARGET_GATE_RELEASE);
  float driveGain = getModulatedValue(driveGainParam, TARGET_DRIVE_GAIN);

  float pitchShift = getModulatedValue(pitchShiftParam, TARGET_PITCH_SHIFT);
  float pitchFine = getModulatedValue(pitchFineParam, TARGET_PITCH_FINE);

  inputLevelL = buffer.getMagnitude(0, 0, buffer.getNumSamples());
  if (totalNumInputChannels > 1)
    inputLevelR = buffer.getMagnitude(1, 0, buffer.getNumSamples());
  else
    inputLevelR = inputLevelL.load();

  for (const auto metadata : midiMessages) {
    auto message = metadata.getMessage();
    if (message.isNoteOn() && message.getVelocity() > 0) {
      int note = juce::jlimit(0, 127, message.getNoteNumber());
      lastMidiNote.store(note);
      DBG("MIDI Note On: " << note);
    }
  }

  // --- Synth Processing ---
  engine.SetStringPitch(freq);
  engine.SetFeedbackGain(fbGain);

  engine.instrumentMode = instrumentModeParam->get();

  // Apply Frequency Modulation to MIDI Pitch in Instrument Mode
  // Calculate the modulation amount (delta) from the freq parameter
  float freqModDelta = freq - freqParam->get();
  engine.SetMidiPitch((float)lastMidiNote.load() + freqModDelta);

  engine.SetFeedbackDelay(fbDelay);
  engine.SetFeedbackLPFCutoff(fbLpf);
  engine.SetFeedbackHPFCutoff(fbHpf);
  engine.SetReverbMix(verbMix);
  engine.SetReverbFeedback(verbDecay);
  engine.SetEchoDelaySendAmount(echoSend);
  engine.SetEchoDelayTime(echoTime);
  engine.SetEchoDelayFeedback(echoFb);
  engine.SetOutputLevel(1.0f); // Engine output is full wet level

  engine.pitchEnabled = *pitchEnabledParam;
  engine.pitchShift = pitchShift;
  engine.pitchFine = pitchFine;

  // Process Audio
  auto *leftIn = buffer.getReadPointer(0);
  auto *rightIn =
      (totalNumInputChannels > 1) ? buffer.getReadPointer(1) : leftIn;
  auto *leftOut = buffer.getWritePointer(0);
  auto *rightOut =
      (totalNumOutputChannels > 1) ? buffer.getWritePointer(1) : leftOut;

  // Store dry signal for later dry/wet mix
  juce::AudioBuffer<float> dryBuffer(totalNumOutputChannels,
                                     buffer.getNumSamples());
  for (int channel = 0; channel < totalNumOutputChannels; ++channel)
    dryBuffer.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());

  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    float dryL = leftIn[i];
    float dryR = (totalNumInputChannels > 1) ? rightIn[i] : dryL;

    float in = 0.5f * (dryL + dryR); // Sum to mono for input

    // --- Input Processing (Gate & Drive) ---

    // 1. Noise Gate
    if (gateEnabledParam->get()) {
      float inputLevel = std::abs(in);
      float threshLinear = juce::Decibels::decibelsToGain(gateThresh);
      float targetGain = (inputLevel >= threshLinear) ? 1.0f : 0.0f;

      float releaseMs = gateRelease;
      float releaseCoeff =
          std::exp(-1.0f / (releaseMs * 0.001f * getSampleRate()));

      if (targetGain > gateCurrentGain)
        gateCurrentGain = targetGain;
      else
        gateCurrentGain =
            targetGain + releaseCoeff * (gateCurrentGain - targetGain);

      in *= gateCurrentGain;
    }

    // 2. Drive (Tanh Saturation)
    if (driveEnabledParam->get()) {
      float driveFactor = 1.0f + (driveAmt * 19.0f);
      in = std::tanh(in * driveFactor);

      float outGain = juce::Decibels::decibelsToGain(driveGain);
      in *= outGain;
    }

    float wetL, wetR;
    engine.Process(in, wetL, wetR);

    leftOut[i] = wetL;
    if (totalNumOutputChannels > 1)
      rightOut[i] = wetR;
  }

  float wetMix = dryWet;
  for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
    auto *outData = buffer.getWritePointer(channel);
    auto *dryData = dryBuffer.getReadPointer(channel);
    float currentWet = wetMix;

    for (int i = 0; i < buffer.getNumSamples(); ++i) {
      outData[i] =
          (dryData[i] * (1.0f - currentWet)) + (outData[i] * currentWet);
    }
  }

  outputLevelL = buffer.getMagnitude(0, 0, buffer.getNumSamples());
  if (totalNumOutputChannels > 1)
    outputLevelR = buffer.getMagnitude(1, 0, buffer.getNumSamples());
  else
    outputLevelR = outputLevelL.load();

  // --- Stereo Widening (Post-Process) ---
  if (totalNumOutputChannels > 1 && width != 1.0f) {
    auto *leftChannel = buffer.getWritePointer(0);
    auto *rightChannel = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); ++i) {
      float l = leftChannel[i];
      float r = rightChannel[i];

      float mid = (l + r) * 0.5f;
      float side = (l - r) * 0.5f;

      side *= width;

      leftChannel[i] = mid + side;
      rightChannel[i] = mid - side;
    }
  }
}

bool DawdreyAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *DawdreyAudioProcessor::createEditor() {
  return new DawdreyAudioProcessorEditor(*this);
}

void DawdreyAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void DawdreyAudioProcessor::setStateInformation(const void *data,
                                                int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr) {
    if (xmlState->hasTagName(apvts.state.getType())) {
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
  }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new DawdreyAudioProcessor();
}
