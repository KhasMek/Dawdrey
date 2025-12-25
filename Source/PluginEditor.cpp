#include "PluginEditor.h"
#include "PluginProcessor.h"

DawdreyAudioProcessorEditor::DawdreyAudioProcessorEditor(
    DawdreyAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p) {
  setSize(1300, 800);
  startTimerHz(60);

  auto &apvts = audioProcessor.apvts;

  setLookAndFeel(&customLookAndFeel);

  addAndMakeVisible(presetBox);
  presetBox.setJustificationType(juce::Justification::centred);
  presetBox.setTextWhenNothingSelected("Select Preset...");

  auto presets = audioProcessor.presetManager->getAllPresets();
  presetBox.addItemList(presets, 1);

  presetBox.onChange = [this] {
    if (presetBox.getSelectedId() > 0) {
      audioProcessor.presetManager->loadPreset(presetBox.getText());
    }
  };

  addAndMakeVisible(savePresetButton);
  addAndMakeVisible(importPresetButton);
  addAndMakeVisible(exportPresetButton);

  savePresetButton.onClick = [this] {
    auto *w = new juce::AlertWindow(
        "Save Preset", "Enter preset name:", juce::AlertWindow::NoIcon);
    w->addTextEditor("name", audioProcessor.presetManager->getCurrentPreset(),
                     "Preset Name");
    w->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey, 0, 0));
    w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey, 0, 0));

    w->setLookAndFeel(&customLookAndFeel);
    w->setOpaque(false);
    w->setColour(juce::AlertWindow::backgroundColourId,
                 juce::Colours::transparentBlack);
    w->setColour(juce::AlertWindow::textColourId, juce::Colour(0xfff0f0f0));
    w->setColour(juce::AlertWindow::outlineColourId, juce::Colour(0xffd4af37));

    w->enterModalState(
        true, juce::ModalCallbackFunction::create([this, w](int result) {
          if (result == 1) {
            auto name = w->getTextEditorContents("name");
            audioProcessor.presetManager->savePreset(name);

            presetBox.clear();
            presetBox.addItemList(audioProcessor.presetManager->getAllPresets(),
                                  1);
            presetBox.setText(name);
          }
          delete w;
        }));
  };

  addAndMakeVisible(initPresetButton);
  initPresetButton.onClick = [this] {
    audioProcessor.presetManager->loadInitPreset();
    presetBox.setText("Init");
    presetBox.setSelectedId(0, juce::dontSendNotification);
  };

  addAndMakeVisible(prevPresetButton);
  prevPresetButton.onClick = [this] {
    audioProcessor.presetManager->loadPreviousPreset();
    presetBox.setText(audioProcessor.presetManager->getCurrentPreset());
  };

  addAndMakeVisible(nextPresetButton);
  nextPresetButton.onClick = [this] {
    audioProcessor.presetManager->loadNextPreset();
    presetBox.setText(audioProcessor.presetManager->getCurrentPreset());
  };

  importPresetButton.onClick = [this] {
    fileChooser = std::make_unique<juce::FileChooser>(
        "Import Preset",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*.xml");
    auto folderFlags = juce::FileBrowserComponent::openMode |
                       juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(folderFlags, [this](const juce::FileChooser &fc) {
      auto file = fc.getResult();
      if (file != juce::File{}) {
        audioProcessor.presetManager->loadPresetFromFile(file);
        updatePresetList();
      }
    });
  };

  exportPresetButton.onClick = [this] {
    fileChooser = std::make_unique<juce::FileChooser>(
        "Export Preset",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*.xml");
    auto folderFlags = juce::FileBrowserComponent::saveMode |
                       juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(folderFlags, [this](const juce::FileChooser &fc) {
      auto file = fc.getResult();
      if (file != juce::File{}) {
        if (!file.hasFileExtension("xml"))
          file = file.withFileExtension("xml");

        audioProcessor.presetManager->savePresetToFile(
            audioProcessor.presetManager->getCurrentPreset(), file);
      }
    });
  };

  addAndMakeVisible(undoButton);
  undoButton.onClick = [this] { audioProcessor.undoManager.undo(); };
  undoButton.setTooltip("Undo last parameter change");

  addAndMakeVisible(redoButton);
  redoButton.onClick = [this] { audioProcessor.undoManager.redo(); };
  redoButton.setTooltip("Redo last undone parameter change");

  addAndMakeVisible(instrumentModeButton);
  instrumentModeButton.setButtonText("Instrument Mode");
  instrumentModeButton.setTooltip(
      "Enable to control Resonator Pitch with MIDI Notes");
  instrumentModeAttachment.reset(
      new juce::AudioProcessorValueTreeState::ButtonAttachment(
          apvts, "instrument_mode", instrumentModeButton));

  presetBox.setTooltip("Load a preset");
  savePresetButton.setTooltip("Save current settings as a new preset");
  initPresetButton.setTooltip("Reset all parameters to default");
  prevPresetButton.setTooltip("Load previous preset");
  nextPresetButton.setTooltip("Load next preset");

  inputLevelMeter.setTooltip("Input Level (Stereo Peak)");
  outputLevelMeter.setTooltip("Output Level (Stereo Peak)");

  gateEnabledButton.setTooltip("Enable Noise Gate");
  gateThreshSlider.setTooltip(
      "Gate Threshold: Signal level below this will be silenced");
  gateReleaseSlider.setTooltip(
      "Gate Release: Time to close the gate after signal drops");

  driveEnabledButton.setTooltip("Enable Input Drive");
  driveAmountSlider.setTooltip("Drive Amount: Saturation intensity");
  driveGainSlider.setTooltip("Drive Gain: Makeup gain after saturation");

  freqSlider.setTooltip("Resonator Frequency: Base pitch of the feedback loop");
  fbGainSlider.setTooltip(
      "Feedback Gain: Amount of signal fed back into the loop");
  fbDelaySlider.setTooltip("Feedback Delay: Length of the delay line (tuned)");
  fbLpfSlider.setTooltip(
      "Feedback Lowpass: Damps high frequencies in the loop");
  fbHpfSlider.setTooltip(
      "Feedback Highpass: Removes low frequencies from the loop");

  pitchEnabledButton.setTooltip("Enable Pitch Shifter in Feedback Loop");
  pitchShiftSlider.setTooltip("Pitch Shift: Semitones (-12 to +12)");
  pitchFineSlider.setTooltip("Pitch Fine: Cents (-100 to +100)");

  echoSendSlider.setTooltip(
      "Echo Send: Amount of signal sent to the echo delay");
  echoTimeSlider.setTooltip("Echo Time: Delay time in seconds");
  echoFbSlider.setTooltip("Echo Feedback: Number of repeats");

  verbMixSlider.setTooltip("Reverb Mix: Blend between dry and wet signal");
  verbDecaySlider.setTooltip("Reverb Decay: Length of the reverb tail");

  widthSlider.setTooltip("Stereo Width: 0=Mono, 1=Stereo, >1=Wide");
  dryWetSlider.setTooltip(
      "Dry/Wet Mix: Blend between original and processed signal");

  auto setupLfoTooltips = [](juce::Slider &rate, juce::Slider &depth,
                             juce::ComboBox &shape, juce::ComboBox &target,
                             juce::ToggleButton &sync,
                             juce::ToggleButton &bipolar, juce::Slider &div) {
    rate.setTooltip("LFO Rate: Speed of modulation (Hz)");
    depth.setTooltip("LFO Depth: Amount of modulation (-1 to +1)");
    shape.setTooltip("LFO Shape: Waveform type");
    target.setTooltip("LFO Target: Parameter to modulate");
    sync.setTooltip("Tempo Sync: Lock LFO to host BPM");
    bipolar.setTooltip("Bipolar: Modulation swings positive and negative");
    div.setTooltip("LFO Division: Rhythmic subdivision (Bars/Beats)");
  };

  setupLfoTooltips(lfo1RateSlider, lfo1DepthSlider, lfo1ShapeBox, lfo1TargetBox,
                   lfo1SyncButton, lfo1BipolarButton, lfo1DivSlider);
  setupLfoTooltips(lfo2RateSlider, lfo2DepthSlider, lfo2ShapeBox, lfo2TargetBox,
                   lfo2SyncButton, lfo2BipolarButton, lfo2DivSlider);
  setupLfoTooltips(lfo3RateSlider, lfo3DepthSlider, lfo3ShapeBox, lfo3TargetBox,
                   lfo3SyncButton, lfo3BipolarButton, lfo3DivSlider);

  auto setupSlider = [this,
                      &apvts](juce::Slider &slider, juce::Label &label,
                              std::string paramID, std::string name,
                              std::unique_ptr<SliderAttachment> &attachment) {
    addAndMakeVisible(slider);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    attachment.reset(new SliderAttachment(apvts, paramID, slider));

    addAndMakeVisible(label);
    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    // label.attachToComponent(&slider, false); // Removed for manual layout
  };

  setupSlider(freqSlider, freqLabel, "freq", "Freq", freqAttachment);
  setupSlider(fbGainSlider, fbGainLabel, "fb_gain", "FB Gain",
              fbGainAttachment);
  setupSlider(fbDelaySlider, fbDelayLabel, "fb_delay", "FB Delay",
              fbDelayAttachment);
  setupSlider(fbLpfSlider, fbLpfLabel, "fb_lpf", "FB LPF", fbLpfAttachment);
  setupSlider(fbHpfSlider, fbHpfLabel, "fb_hpf", "FB HPF", fbHpfAttachment);

  addAndMakeVisible(pitchEnabledButton);
  pitchEnabledButton.setButtonText("Enable");
  pitchEnabledAttachment.reset(
      new juce::AudioProcessorValueTreeState::ButtonAttachment(
          apvts, "pitch_enabled", pitchEnabledButton));

  setupSlider(pitchShiftSlider, pitchShiftLabel, "pitch_shift", "Shift",
              pitchShiftAttachment);
  setupSlider(pitchFineSlider, pitchFineLabel, "pitch_fine", "Fine",
              pitchFineAttachment);

  setupSlider(verbMixSlider, verbMixLabel, "verb_mix", "Verb Mix",
              verbMixAttachment);
  setupSlider(verbDecaySlider, verbDecayLabel, "verb_decay", "Verb Decay",
              verbDecayAttachment);

  setupSlider(echoSendSlider, echoSendLabel, "echo_send", "Echo Send",
              echoSendAttachment);
  setupSlider(echoTimeSlider, echoTimeLabel, "echo_time", "Echo Time",
              echoTimeAttachment);
  setupSlider(echoFbSlider, echoFbLabel, "echo_fb", "Echo FB",
              echoFbAttachment);

  auto setupCombo =
      [this, &apvts](juce::ComboBox &box, juce::Label &label,
                     std::string paramID, std::string name,
                     std::unique_ptr<
                         juce::AudioProcessorValueTreeState::ComboBoxAttachment>
                         &attachment) {
        addAndMakeVisible(box);
        box.addItemList(apvts.getParameter(paramID)->getAllValueStrings(), 1);
        box.setJustificationType(juce::Justification::centred);
        attachment.reset(
            new juce::AudioProcessorValueTreeState::ComboBoxAttachment(
                apvts, paramID, box));

        addAndMakeVisible(label);
        label.setText(name, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&box, false);
      };

  addAndMakeVisible(lfo1RateSlider);
  lfo1RateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lfo1RateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  lfo1RateAttachment = std::make_unique<SliderAttachment>(
      audioProcessor.apvts, "lfo1_rate", lfo1RateSlider);

  addAndMakeVisible(lfo1RateLabel);
  lfo1RateLabel.setText("Rate", juce::dontSendNotification);
  lfo1RateLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(lfo1DepthSlider);
  lfo1DepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lfo1DepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  lfo1DepthAttachment = std::make_unique<SliderAttachment>(
      audioProcessor.apvts, "lfo1_depth", lfo1DepthSlider);

  addAndMakeVisible(lfo1DepthLabel);
  lfo1DepthLabel.setText("Depth", juce::dontSendNotification);
  lfo1DepthLabel.setJustificationType(juce::Justification::centred);

  setupCombo(lfo1ShapeBox, lfo1ShapeLabel, "lfo1_shape", "Shape",
             lfo1ShapeAttachment);
  setupCombo(lfo1TargetBox, lfo1TargetLabel, "lfo1_target", "Target",
             lfo1TargetAttachment);

  addAndMakeVisible(lfo1SyncButton);
  lfo1SyncButton.setButtonText("Sync");
  lfo1SyncAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "lfo1_sync", lfo1SyncButton);

  addAndMakeVisible(lfo1BipolarButton);
  lfo1BipolarButton.setButtonText("Bipolar");
  lfo1BipolarAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "lfo1_bipolar", lfo1BipolarButton);

  addAndMakeVisible(lfo1DivSlider);
  lfo1DivSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lfo1DivSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  lfo1DivAttachment = std::make_unique<SliderAttachment>(
      audioProcessor.apvts, "lfo1_div", lfo1DivSlider);

  lfo1DivSlider.textFromValueFunction = [](double value) {
    int index = (int)value;
    switch ((int)value) {
    case 0:
      return "64 Bars";
    case 1:
      return "32 Bars";
    case 2:
      return "16 Bars";
    case 3:
      return "8 Bars";
    case 4:
      return "4 Bars";
    case 5:
      return "2 Bars";
    case 6:
      return "1 Bar";
    case 7:
      return "1/2";
    case 8:
      return "1/4";
    case 9:
      return "1/8";
    case 10:
      return "1/16";
    case 11:
      return "1/32";
    case 12:
      return "1/64";
    default:
      return "1 Bar";
    }
  };

  lfo1SyncButton.onClick = [this] {
    bool synced = lfo1SyncButton.getToggleState();
    lfo1RateSlider.setVisible(!synced);
    lfo1DivSlider.setVisible(synced);
    lfo1RateLabel.setText(synced ? "Div" : "Rate", juce::dontSendNotification);
  };
  lfo1SyncButton.onClick();

  addAndMakeVisible(dryWetSlider);
  dryWetSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  dryWetAttachment =
      std::make_unique<SliderAttachment>(apvts, "dry_wet", dryWetSlider);

  addAndMakeVisible(widthSlider);
  widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  widthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  widthAttachment =
      std::make_unique<SliderAttachment>(apvts, "width", widthSlider);

  addAndMakeVisible(gateEnabledButton);
  gateEnabledButton.setButtonText("Gate");
  gateEnabledAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          apvts, "gate_enabled", gateEnabledButton);

  addAndMakeVisible(gateThreshSlider);
  gateThreshSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  gateThreshSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  gateThreshAttachment = std::make_unique<SliderAttachment>(
      apvts, "gate_thresh", gateThreshSlider);

  addAndMakeVisible(gateThreshLabel);
  gateThreshLabel.setText("Thresh", juce::dontSendNotification);
  gateThreshLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(gateReleaseSlider);
  gateReleaseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  gateReleaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  gateReleaseAttachment = std::make_unique<SliderAttachment>(
      apvts, "gate_release", gateReleaseSlider);

  addAndMakeVisible(gateReleaseLabel);
  gateReleaseLabel.setText("Release", juce::dontSendNotification);
  gateReleaseLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(driveEnabledButton);
  driveEnabledButton.setButtonText("Drive");
  driveEnabledAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          apvts, "drive_enabled", driveEnabledButton);

  addAndMakeVisible(driveAmountSlider);
  driveAmountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  driveAmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  driveAmountAttachment = std::make_unique<SliderAttachment>(
      apvts, "drive_amount", driveAmountSlider);

  addAndMakeVisible(driveAmountLabel);
  driveAmountLabel.setText("Amount", juce::dontSendNotification);
  driveAmountLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(driveGainSlider);
  driveGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  driveGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  driveGainAttachment =
      std::make_unique<SliderAttachment>(apvts, "drive_gain", driveGainSlider);

  addAndMakeVisible(driveGainLabel);
  driveGainLabel.setText("Gain", juce::dontSendNotification);
  driveGainLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(inputLevelMeter);
  addAndMakeVisible(outputLevelMeter);

  addAndMakeVisible(lfo2SyncButton);
  lfo2SyncButton.setButtonText("Sync");
  lfo2SyncAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "lfo2_sync", lfo2SyncButton);

  addAndMakeVisible(lfo2BipolarButton);
  lfo2BipolarButton.setButtonText("Bipolar");
  lfo2BipolarAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "lfo2_bipolar", lfo2BipolarButton);

  addAndMakeVisible(lfo2RateSlider);
  lfo2RateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lfo2RateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  lfo2RateAttachment = std::make_unique<SliderAttachment>(
      audioProcessor.apvts, "lfo2_rate", lfo2RateSlider);

  addAndMakeVisible(lfo2RateLabel);
  lfo2RateLabel.setText("Rate", juce::dontSendNotification);
  lfo2RateLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(lfo2DepthSlider);
  lfo2DepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lfo2DepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  lfo2DepthAttachment = std::make_unique<SliderAttachment>(
      audioProcessor.apvts, "lfo2_depth", lfo2DepthSlider);

  addAndMakeVisible(lfo2DepthLabel);
  lfo2DepthLabel.setText("Depth", juce::dontSendNotification);
  lfo2DepthLabel.setJustificationType(juce::Justification::centred);

  setupCombo(lfo2ShapeBox, lfo2ShapeLabel, "lfo2_shape", "Shape",
             lfo2ShapeAttachment);
  setupCombo(lfo2TargetBox, lfo2TargetLabel, "lfo2_target", "Target",
             lfo2TargetAttachment);

  addAndMakeVisible(lfo2SyncButton);
  lfo2SyncButton.setButtonText("Sync");
  lfo2SyncAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "lfo2_sync", lfo2SyncButton);

  addAndMakeVisible(lfo2DivSlider);
  lfo2DivSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lfo2DivSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  lfo2DivAttachment = std::make_unique<SliderAttachment>(
      audioProcessor.apvts, "lfo2_div", lfo2DivSlider);

  lfo2DivSlider.textFromValueFunction = lfo1DivSlider.textFromValueFunction;

  lfo2SyncButton.onClick = [this] {
    bool synced = lfo2SyncButton.getToggleState();
    lfo2RateSlider.setVisible(!synced);
    lfo2DivSlider.setVisible(synced);
    lfo2RateLabel.setText(synced ? "Div" : "Rate", juce::dontSendNotification);
  };
  lfo2SyncButton.onClick();

  addAndMakeVisible(lfo3RateSlider);
  lfo3RateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lfo3RateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  lfo3RateAttachment = std::make_unique<SliderAttachment>(
      audioProcessor.apvts, "lfo3_rate", lfo3RateSlider);

  addAndMakeVisible(lfo3RateLabel);
  lfo3RateLabel.setText("Rate", juce::dontSendNotification);
  lfo3RateLabel.setJustificationType(juce::Justification::centred);

  addAndMakeVisible(lfo3DepthSlider);
  lfo3DepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lfo3DepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  lfo3DepthAttachment = std::make_unique<SliderAttachment>(
      audioProcessor.apvts, "lfo3_depth", lfo3DepthSlider);

  addAndMakeVisible(lfo3DepthLabel);
  lfo3DepthLabel.setText("Depth", juce::dontSendNotification);
  lfo3DepthLabel.setJustificationType(juce::Justification::centred);

  setupCombo(lfo3ShapeBox, lfo3ShapeLabel, "lfo3_shape", "Shape",
             lfo3ShapeAttachment);
  setupCombo(lfo3TargetBox, lfo3TargetLabel, "lfo3_target", "Target",
             lfo3TargetAttachment);

  addAndMakeVisible(lfo3SyncButton);
  lfo3SyncButton.setButtonText("Sync");
  lfo3SyncAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "lfo3_sync", lfo3SyncButton);

  addAndMakeVisible(lfo3BipolarButton);
  lfo3BipolarButton.setButtonText("Bipolar");
  lfo3BipolarAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          audioProcessor.apvts, "lfo3_bipolar", lfo3BipolarButton);

  addAndMakeVisible(lfo3DivSlider);
  lfo3DivSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  lfo3DivSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  lfo3DivAttachment = std::make_unique<SliderAttachment>(
      audioProcessor.apvts, "lfo3_div", lfo3DivSlider);

  lfo3DivSlider.textFromValueFunction = lfo1DivSlider.textFromValueFunction;

  lfo3SyncButton.onClick = [this] {
    bool synced = lfo3SyncButton.getToggleState();
    lfo3RateSlider.setVisible(!synced);
    lfo3DivSlider.setVisible(synced);
    lfo3RateLabel.setText(synced ? "Div" : "Rate", juce::dontSendNotification);
  };
  lfo3SyncButton.onClick();

  setupSlider(dryWetSlider, dryWetLabel, "dry_wet", "Dry/Wet",
              dryWetAttachment);
  setupSlider(widthSlider, widthLabel, "width", "Width", widthAttachment);
}

DawdreyAudioProcessorEditor::~DawdreyAudioProcessorEditor() {
  setSize(1100, 800);
  setLookAndFeel(nullptr);
}

void DawdreyAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(0xff1a1a1a));

  g.setColour(juce::Colour(0xffd4af37));
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(5), 10.0f, 2.0f);

  g.setColour(juce::Colour::fromString("FFF0F0F0"));
  g.setFont(customLookAndFeel.getCustomFont(30.0f));
  g.drawFittedText("Dawdrey",
                   getLocalBounds().removeFromTop(60).removeFromLeft(300),
                   juce::Justification::centred, 1);

  auto drawGroup = [&](juce::Rectangle<int> bounds, juce::String title) {
    auto groupBounds = bounds.reduced(10);

    juce::ColourGradient gradient(
        juce::Colour::fromString("FFD4AF37").withAlpha(0.2f),
        groupBounds.getX(), groupBounds.getY(),
        juce::Colour::fromString("FFD4AF37").withAlpha(0.05f),
        groupBounds.getBottomRight().getX(),
        groupBounds.getBottomRight().getY(), true);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(groupBounds.toFloat(), 10.0f);

    g.setColour(juce::Colour::fromString("FFD4AF37"));
    g.drawRoundedRectangle(groupBounds.toFloat(), 10.0f, 2.0f);

    g.setColour(juce::Colour::fromString("FFF0F0F0"));
    g.setFont(customLookAndFeel.getCustomFont(18.0f));
    g.drawText(title, groupBounds.removeFromTop(25),
               juce::Justification::centredTop);
  };

  auto area = getLocalBounds().reduced(10);
  area.removeFromTop(40);

  // --- MAIN COLUMNS ---
  // 1. Input Meter (Far Left)
  area.removeFromLeft(30);

  // 2. Output Meter (Far Right)
  area.removeFromRight(30);

  // 3. LFO Column (Right of Effects)
  int lfoColW = 300;
  auto lfoArea = area.removeFromRight(lfoColW);

  // Remaining area for Controls (Input, Resonator, Effects)
  int remainingWidth = area.getWidth();
  int colInputW = remainingWidth * 0.22; // Input
  int col1W = remainingWidth * 0.39;     // Resonator
  int col2W = remainingWidth * 0.39;     // Effects

  auto inputArea = area.removeFromLeft(colInputW);
  auto leftArea = area.removeFromLeft(col1W);
  auto midArea = area; // Remaining is Effects/Master

  // --- INPUT COLUMN ---
  auto gateArea = inputArea.removeFromTop(inputArea.getHeight() / 2);
  drawGroup(gateArea, "INPUT GATE");

  drawGroup(inputArea, "INPUT DRIVE");
  auto driveArea = inputArea; // Remaining is drive

  // --- LEFT COLUMN: RESONATOR & PITCH ---
  auto pitchGroup = leftArea.removeFromBottom(leftArea.getHeight() / 4);
  auto resGroup = leftArea;

  drawGroup(resGroup, "RESONATOR");
  drawGroup(pitchGroup, "PITCH SHIFT");

  // --- MIDDLE COLUMN: EFFECTS + MASTER ---
  auto echoArea = midArea.removeFromTop(midArea.getHeight() / 3);
  drawGroup(echoArea, "ECHO");

  auto reverbArea = midArea.removeFromTop(midArea.getHeight() / 2);
  drawGroup(reverbArea, "REVERB");

  auto masterGroup = midArea; // Remaining is Master
  drawGroup(masterGroup, "MASTER");

  // --- RIGHT COLUMN: LFOs ---
  auto lfo1Area = lfoArea.removeFromTop(lfoArea.getHeight() / 3);
  drawGroup(lfo1Area, "LFO 1");

  auto lfo2Area = lfoArea.removeFromTop(lfoArea.getHeight() / 2);
  drawGroup(lfo2Area, "LFO 2");

  drawGroup(lfoArea, "LFO 3");
}

void DawdreyAudioProcessorEditor::resized() {
  int width = getWidth();
  int height = getHeight();

  auto area = getLocalBounds().reduced(10);

  // Header (Presets)
  auto headerArea = area.removeFromTop(40);

  // Title (Left)
  auto titleArea = headerArea.removeFromLeft(200);

  // Instrument Mode Toggle (Right)
  auto instrumentArea = headerArea.removeFromRight(150);
  instrumentModeButton.setBounds(instrumentArea.reduced(5));

  int buttonW = 50;
  int wideButtonW = 60; // Wider for Import/Export/Save
  int comboW = 200;
  int spacer = 20;

  int totalPresetW = comboW + (buttonW * 5) + (wideButtonW * 3) + spacer;

  // Center the preset group in the remaining header area
  int xOffset = headerArea.getX() + (headerArea.getWidth() - totalPresetW) / 2;
  auto presetGroupArea = headerArea.withX(xOffset).withWidth(totalPresetW);

  undoButton.setBounds(presetGroupArea.removeFromLeft(buttonW).reduced(2));
  redoButton.setBounds(presetGroupArea.removeFromLeft(buttonW).reduced(2));

  presetGroupArea.removeFromLeft(spacer);

  prevPresetButton.setBounds(
      presetGroupArea.removeFromLeft(buttonW).reduced(2));
  presetBox.setBounds(presetGroupArea.removeFromLeft(comboW).reduced(2));
  nextPresetButton.setBounds(
      presetGroupArea.removeFromLeft(buttonW).reduced(2));
  initPresetButton.setBounds(
      presetGroupArea.removeFromLeft(buttonW).reduced(2));
  savePresetButton.setBounds(
      presetGroupArea.removeFromLeft(wideButtonW).reduced(2));
  importPresetButton.setBounds(
      presetGroupArea.removeFromLeft(wideButtonW).reduced(2));
  exportPresetButton.setBounds(
      presetGroupArea.removeFromLeft(wideButtonW).reduced(2));

  // --- MAIN COLUMNS ---
  // Layout: [Input Meter] [Input Controls] [Resonator] [Effects] [LFOs] [Output
  // Meter]

  // 1. Input Meter (Far Left)
  auto inputMeterArea = area.removeFromLeft(30);
  inputLevelMeter.setBounds(inputMeterArea.reduced(5, 20));

  // 2. Output Meter (Far Right)
  auto outputMeterArea = area.removeFromRight(30);
  outputLevelMeter.setBounds(outputMeterArea.reduced(5, 20));

  // 3. LFO Column (Right of Effects)
  int lfoColW = 300;
  auto lfoArea = area.removeFromRight(lfoColW);

  // Remaining area for Controls (Input, Resonator, Effects)
  int remainingWidth = area.getWidth();
  int colInputW = remainingWidth * 0.22; // Input
  int col1W = remainingWidth * 0.39;     // Resonator
  int col2W = remainingWidth * 0.39;     // Effects

  auto inputArea = area.removeFromLeft(colInputW);
  auto leftArea = area.removeFromLeft(col1W);
  auto midArea = area; // Remaining is Effects/Master

  // --- INPUT COLUMN ---
  // Split first to match paint()
  auto gateArea = inputArea.removeFromTop(inputArea.getHeight() / 2);
  auto driveArea = inputArea;

  // Gate Group
  auto gateGroup = gateArea.reduced(14); // Increased padding
  gateGroup.removeFromTop(40);           // Skip Title

  // Gate Toggle
  auto gateToggleArea = gateGroup.removeFromTop(30);
  gateEnabledButton.setBounds(gateToggleArea.getCentreX() - 30,
                              gateToggleArea.getY(), 60, 20);

  // Gate Controls
  // Use flexible height based on remaining space
  int rowHeight = gateGroup.getHeight() / 2;

  auto gateRow1 = gateGroup.removeFromTop(rowHeight);
  gateThreshLabel.setBounds(gateRow1.removeFromTop(18));
  gateThreshSlider.setBounds(
      gateRow1.reduced(0, 5)); // Add vertical padding to slider

  auto gateRow2 = gateGroup;
  gateReleaseLabel.setBounds(gateRow2.removeFromTop(18));
  gateReleaseSlider.setBounds(
      gateRow2.reduced(0, 5)); // Add vertical padding to slider

  // Drive Group
  auto driveGroup = driveArea.reduced(14); // Increased padding
  driveGroup.removeFromTop(40);            // Skip Title

  // Drive Toggle
  auto driveToggleArea = driveGroup.removeFromTop(30);
  driveEnabledButton.setBounds(driveToggleArea.getCentreX() - 30,
                               driveToggleArea.getY(), 60, 20);

  // Drive Controls
  rowHeight = driveGroup.getHeight() / 2;

  auto driveRow1 = driveGroup.removeFromTop(rowHeight);
  driveAmountLabel.setBounds(driveRow1.removeFromTop(18));
  driveAmountSlider.setBounds(driveRow1.reduced(0, 5));

  auto driveRow2 = driveGroup;
  driveGainLabel.setBounds(driveRow2.removeFromTop(18));
  driveGainSlider.setBounds(driveRow2.reduced(0, 5));

  // --- LEFT COLUMN: RESONATOR & PITCH ---
  // Split Left Area: Top 3/4 for Resonator, Bottom 1/4 for Pitch
  auto pitchGroupArea = leftArea.removeFromBottom(leftArea.getHeight() / 4);
  auto resGroupArea = leftArea;

  // --- Resonator Group ---
  auto resGroup = resGroupArea.reduced(10);
  resGroup.removeFromTop(40); // Skip Title

  // Row 1: Freq, Gain
  auto resRow1 = resGroup.removeFromTop(resGroup.getHeight() / 3);

  auto freqArea = resRow1.removeFromLeft(resRow1.getWidth() / 2);
  freqLabel.setBounds(freqArea.removeFromTop(18));
  freqSlider.setBounds(freqArea.reduced(5));

  auto fbGainArea = resRow1;
  fbGainLabel.setBounds(fbGainArea.removeFromTop(18));
  fbGainSlider.setBounds(fbGainArea.reduced(5));

  // Row 2: Delay
  auto resRow2 = resGroup.removeFromTop(resGroup.getHeight() / 2);
  fbDelayLabel.setBounds(resRow2.removeFromTop(18));
  fbDelaySlider.setBounds(resRow2.reduced(10, 5));

  // Row 3: LPF, HPF
  auto resRow3 = resGroup;

  auto fbLpfArea = resRow3.removeFromLeft(resRow3.getWidth() / 2);
  fbLpfLabel.setBounds(fbLpfArea.removeFromTop(18));
  fbLpfSlider.setBounds(fbLpfArea.reduced(5));

  auto fbHpfArea = resRow3;
  fbHpfLabel.setBounds(fbHpfArea.removeFromTop(18));
  fbHpfSlider.setBounds(fbHpfArea.reduced(5));

  // --- Pitch Shift Group ---
  auto pitchGroup = pitchGroupArea.reduced(10);
  pitchGroup.removeFromTop(40); // Skip Title

  // Toggle
  auto pitchToggleArea = pitchGroup.removeFromTop(30);
  pitchEnabledButton.setBounds(pitchToggleArea.getCentreX() - 30,
                               pitchToggleArea.getY(), 60, 20);

  // Controls
  auto pitchControlsArea = pitchGroup;
  auto pitchShiftArea =
      pitchControlsArea.removeFromLeft(pitchControlsArea.getWidth() / 2);
  pitchShiftLabel.setBounds(pitchShiftArea.removeFromTop(18));
  pitchShiftSlider.setBounds(pitchShiftArea.reduced(5));

  auto pitchFineArea = pitchControlsArea;
  pitchFineLabel.setBounds(pitchFineArea.removeFromTop(18));
  pitchFineSlider.setBounds(pitchFineArea.reduced(5));

  // --- ECHO GROUP ---
  auto echoArea = midArea.removeFromTop(midArea.getHeight() / 3);
  auto echoGroup = echoArea.reduced(10);
  echoGroup.removeFromTop(40); // Skip Title

  int echoW = echoGroup.getWidth() / 3;

  auto echoSendArea = echoGroup.removeFromLeft(echoW);
  echoSendLabel.setBounds(echoSendArea.removeFromTop(18));
  echoSendSlider.setBounds(echoSendArea.reduced(5));

  auto echoTimeArea = echoGroup.removeFromLeft(echoW);
  echoTimeLabel.setBounds(echoTimeArea.removeFromTop(18));
  echoTimeSlider.setBounds(echoTimeArea.reduced(5));

  auto echoFbArea = echoGroup;
  echoFbLabel.setBounds(echoFbArea.removeFromTop(18));
  echoFbSlider.setBounds(echoFbArea.reduced(5));

  // --- REVERB GROUP ---
  auto reverbArea = midArea.removeFromTop(midArea.getHeight() / 2);
  auto reverbGroup = reverbArea.reduced(10);
  reverbGroup.removeFromTop(40); // Skip Title

  int verbW = reverbGroup.getWidth() / 2;

  auto verbMixArea = reverbGroup.removeFromLeft(verbW);
  verbMixLabel.setBounds(verbMixArea.removeFromTop(18));
  verbMixSlider.setBounds(verbMixArea.reduced(5));

  auto verbDecayArea = reverbGroup;
  verbDecayLabel.setBounds(verbDecayArea.removeFromTop(18));
  verbDecaySlider.setBounds(verbDecayArea.reduced(5));

  // Master Group
  auto masterGroup = midArea.reduced(10);
  masterGroup.removeFromTop(40); // Skip Title

  int masterW = masterGroup.getWidth() / 2;
  auto widthArea = masterGroup.removeFromLeft(masterW).reduced(5);
  widthLabel.setBounds(widthArea.removeFromTop(18));
  widthSlider.setBounds(widthArea);

  auto dryWetArea = masterGroup.reduced(5);
  dryWetLabel.setBounds(dryWetArea.removeFromTop(18));
  dryWetSlider.setBounds(dryWetArea);

  // --- LFO 1 GROUP ---
  auto lfo1Area = lfoArea.removeFromTop(lfoArea.getHeight() / 3);
  auto lfo1Group = lfo1Area.reduced(14);

  auto lfo1Header = lfo1Group.removeFromTop(70);

  auto lfo1SyncArea = lfo1Header.removeFromBottom(20);
  lfo1BipolarButton.setBounds(lfo1SyncArea.getCentreX() - 30,
                              lfo1SyncArea.getY(), 60, 20);

  auto lfo1BipolarArea = lfo1Header.removeFromBottom(20);
  lfo1SyncButton.setBounds(lfo1BipolarArea.getCentreX() - 25,
                           lfo1BipolarArea.getY(), 50, 20);

  auto lfo1Row1 = lfo1Group.removeFromTop(lfo1Group.getHeight() * 0.65f);

  // Rate / Sync / Div Area
  auto lfo1RateArea = lfo1Row1.removeFromLeft(lfo1Row1.getWidth() / 2);
  lfo1RateLabel.setBounds(lfo1RateArea.removeFromTop(18));
  lfo1RateSlider.setBounds(lfo1RateArea);
  lfo1DivSlider.setBounds(lfo1RateArea);

  // Depth Area (Match Rate)
  auto lfo1DepthArea = lfo1Row1;
  lfo1DepthLabel.setBounds(lfo1DepthArea.removeFromTop(18));
  lfo1DepthSlider.setBounds(lfo1DepthArea);

  auto lfo1Row2 = lfo1Group; // Remaining 35% for combos

  // Combos
  auto lfo1ShapeArea =
      lfo1Row2.removeFromLeft(lfo1Row2.getWidth() / 2).reduced(2);
  lfo1ShapeBox.setBounds(lfo1ShapeArea.removeFromBottom(24));

  auto lfo1TargetArea = lfo1Row2.reduced(2);
  lfo1TargetBox.setBounds(lfo1TargetArea.removeFromBottom(24));

  // --- LFO 2 GROUP ---
  auto lfo2Area = lfoArea.removeFromTop(lfoArea.getHeight() / 2);
  auto lfo2Group = lfo2Area.reduced(14);

  auto lfo2Header = lfo2Group.removeFromTop(70);

  auto lfo2SyncArea = lfo2Header.removeFromBottom(20);
  lfo2BipolarButton.setBounds(lfo2SyncArea.getCentreX() - 30,
                              lfo2SyncArea.getY(), 60, 20);

  auto lfo2BipolarArea = lfo2Header.removeFromBottom(20);
  lfo2SyncButton.setBounds(lfo2BipolarArea.getCentreX() - 25,
                           lfo2BipolarArea.getY(), 50, 20);

  auto lfo2Row1 = lfo2Group.removeFromTop(lfo2Group.getHeight() * 0.65f);

  auto lfo2RateArea = lfo2Row1.removeFromLeft(lfo2Row1.getWidth() / 2);
  lfo2RateLabel.setBounds(lfo2RateArea.removeFromTop(18));
  lfo2RateSlider.setBounds(lfo2RateArea);
  lfo2DivSlider.setBounds(lfo2RateArea);

  auto lfo2DepthArea = lfo2Row1;
  lfo2DepthLabel.setBounds(lfo2DepthArea.removeFromTop(18));
  lfo2DepthSlider.setBounds(lfo2DepthArea);

  auto lfo2Row2 = lfo2Group;
  auto lfo2ShapeArea =
      lfo2Row2.removeFromLeft(lfo2Row2.getWidth() / 2).reduced(2);
  lfo2ShapeBox.setBounds(lfo2ShapeArea.removeFromBottom(24));

  auto lfo2TargetArea = lfo2Row2.reduced(2);
  lfo2TargetBox.setBounds(lfo2TargetArea.removeFromBottom(24));

  // --- LFO 3 GROUP ---
  auto lfo3Group = lfoArea.reduced(14);

  auto lfo3Header = lfo3Group.removeFromTop(70);

  auto lfo3SyncArea = lfo3Header.removeFromBottom(20);
  lfo3BipolarButton.setBounds(lfo3SyncArea.getCentreX() - 30,
                              lfo3SyncArea.getY(), 60, 20);

  auto lfo3BipolarArea = lfo3Header.removeFromBottom(20);
  lfo3SyncButton.setBounds(lfo3BipolarArea.getCentreX() - 25,
                           lfo3BipolarArea.getY(), 50, 20);

  auto lfo3Row1 = lfo3Group.removeFromTop(lfo3Group.getHeight() * 0.65f);

  auto lfo3RateArea = lfo3Row1.removeFromLeft(lfo3Row1.getWidth() / 2);
  lfo3RateLabel.setBounds(lfo3RateArea.removeFromTop(18));
  lfo3RateSlider.setBounds(lfo3RateArea);
  lfo3DivSlider.setBounds(lfo3RateArea);

  auto lfo3DepthArea = lfo3Row1;
  lfo3DepthLabel.setBounds(lfo3DepthArea.removeFromTop(18));
  lfo3DepthSlider.setBounds(lfo3DepthArea);

  auto lfo3Row2 = lfo3Group;
  auto lfo3ShapeArea =
      lfo3Row2.removeFromLeft(lfo3Row2.getWidth() / 2).reduced(2);
  lfo3ShapeBox.setBounds(lfo3ShapeArea.removeFromBottom(24));

  auto lfo3TargetArea = lfo3Row2.reduced(2);
  lfo3TargetBox.setBounds(lfo3TargetArea.removeFromBottom(24));
}

void DawdreyAudioProcessorEditor::timerCallback() {
  auto getTargetSlider = [&](int targetIndex) -> juce::Slider * {
    switch (targetIndex) {
    case 1:
      return &freqSlider;
    case 2:
      return &fbGainSlider;
    case 3:
      return &fbDelaySlider;
    case 4:
      return &fbLpfSlider;
    case 5:
      return &fbHpfSlider;
    case 6:
      return &verbMixSlider;
    case 7:
      return &verbDecaySlider;
    case 8:
      return &echoSendSlider;
    case 9:
      return &echoTimeSlider;
    case 10:
      return &echoFbSlider;
    case 11:
      return &dryWetSlider;
    case 12:
      return &widthSlider;
    case 13:
      return &gateThreshSlider;
    case 14:
      return &gateReleaseSlider;
    case 15:
      return &driveAmountSlider;
    case 16:
      return &driveGainSlider;
    case 17:
      return &pitchShiftSlider;
    case 18:
      return &pitchFineSlider;
    default:
      return nullptr;
    }
  };

  // Helper to get modulation multiplier (Normalized)
  // Since we standardized DSP to use +/- 0.5 normalized range for full depth,
  // this is now constant for all parameters!
  const float MOD_SCALE = 0.5f;

  std::map<juce::Slider *, float> modAmounts;

  auto accumulateMod = [&](int lfoIdx) {
    juce::String prefix = "lfo" + juce::String(lfoIdx) + "_";
    int targetIdx =
        (int)audioProcessor.apvts.getRawParameterValue(prefix + "target")
            ->load();

    if (auto *s = getTargetSlider(targetIdx)) {
      float depth =
          audioProcessor.apvts.getRawParameterValue(prefix + "depth")->load();
      float lfoVal = 0.0f;
      if (lfoIdx == 1)
        lfoVal = audioProcessor.lfo1Value.load();
      else if (lfoIdx == 2)
        lfoVal = audioProcessor.lfo2Value.load();
      else if (lfoIdx == 3)
        lfoVal = audioProcessor.lfo3Value.load();

      modAmounts[s] += lfoVal * depth * MOD_SCALE;
    }
  };

  accumulateMod(1);
  accumulateMod(2);
  accumulateMod(3);
  for (auto const &[slider, modAmount] : modAmounts) {
    float currentNorm = slider->valueToProportionOfLength(slider->getValue());

    float targetNorm = juce::jlimit(0.0f, 1.0f, currentNorm + modAmount);

    slider->getProperties().set("modValue", targetNorm);
    slider->repaint();
  }

  std::vector<juce::Slider *> allTargets = {
      &freqSlider,       &fbGainSlider,      &fbDelaySlider,
      &fbLpfSlider,      &fbHpfSlider,       &verbMixSlider,
      &verbDecaySlider,  &echoSendSlider,    &echoTimeSlider,
      &echoFbSlider,     &dryWetSlider,      &widthSlider,
      &gateThreshSlider, &driveAmountSlider, &gateReleaseSlider,
      &driveGainSlider,  &pitchShiftSlider,  &pitchFineSlider};

  for (auto *s : allTargets) {
    if (modAmounts.find(s) == modAmounts.end()) {
      if (s->getProperties().contains("modValue")) {
        s->getProperties().remove("modValue");
        s->repaint();
      }
    }
  }

  // --- Update Meters ---
  // Simple decay for smooth visualization
  static float inputLevel = 0.0f;
  static float outputLevel = 0.0f;

  bool isInstrumentMode = instrumentModeButton.getToggleState();

  freqSlider.setEnabled(!isInstrumentMode);
  freqSlider.setAlpha(isInstrumentMode ? 0.6f : 1.0f);

  if (isInstrumentMode) {
    // If we JUST switched to Instrument Mode:
    // 1. Detach the slider from the parameter
    // 2. Initialize lastMidiNote from current Slider Value (which is now MIDI
    // Note)
    if (!wasInstrumentMode) {
      freqAttachment.reset();

      int midiNote = std::round(freqSlider.getValue());
      midiNote = juce::jlimit(0, 127, midiNote);
      audioProcessor.lastMidiNote.store(midiNote);
    }

    int midiNote = audioProcessor.lastMidiNote.load();

    if (std::abs(freqSlider.getValue() - (float)midiNote) > 0.01f) {
      freqSlider.setValue((float)midiNote, juce::dontSendNotification);
    }
  } else if (wasInstrumentMode) {
    // Just disabled Instrument Mode - Re-attach to Parameter
    freqAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts,
                                                        "freq", freqSlider);
  }

  wasInstrumentMode = isInstrumentMode;

  float inL = audioProcessor.inputLevelL.load();
  float inR = audioProcessor.inputLevelR.load();
  float outL = audioProcessor.outputLevelL.load();
  float outR = audioProcessor.outputLevelR.load();

  float maxIn = std::max(inL, inR);
  float maxOut = std::max(outL, outR);

  // Smooth decay
  inputLevel = std::max(maxIn, inputLevel - 0.05f);
  outputLevel = std::max(maxOut, outputLevel - 0.05f);

  inputLevelMeter.setLevel(inputLevel);
  outputLevelMeter.setLevel(outputLevel);
}

void DawdreyAudioProcessorEditor::updatePresetList() {
  presetBox.clear();
  presetBox.addItemList(audioProcessor.presetManager->getAllPresets(), 1);
  presetBox.setText(audioProcessor.presetManager->getCurrentPreset(),
                    juce::dontSendNotification);
}
