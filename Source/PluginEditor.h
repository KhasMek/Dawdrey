#pragma once

#include "PluginProcessor.h"
#include "StyleSheet.h"
#include <JuceHeader.h>

class SimpleMeter : public juce::Component, public juce::SettableTooltipClient {
public:
  void setLevel(float newLevel) {
    level = newLevel;
    repaint();
  }

  void paint(juce::Graphics &g) override {
    drawBar(g, getLocalBounds().toFloat(), level);
  }

private:
  float level = 0.0f;

  void drawBar(juce::Graphics &g, juce::Rectangle<float> bounds, float level) {
    float cornerRadius = 5.0f;

    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(bounds, cornerRadius);

    float height = bounds.getHeight() * juce::jlimit(0.0f, 1.0f, level);

    auto fillRect = bounds;
    auto fillBounds = fillRect.removeFromBottom(height);

    juce::Colour c = juce::Colour(0xffd4af37);

    if (level > 0.9f)
      c = juce::Colours::white.withAlpha(0.9f);
    else if (level > 0.7f)
      c = juce::Colour(0xffffd700);
    else
      c = juce::Colour(0xffd4af37).withAlpha(0.8f);

    g.setColour(c);

    g.saveState();

    juce::Path backgroundPath;
    backgroundPath.addRoundedRectangle(bounds, cornerRadius);

    g.reduceClipRegion(backgroundPath);
    g.fillRect(fillBounds);
    g.restoreState();

    g.setColour(juce::Colours::grey.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, cornerRadius, 1.0f);
  }
};

class DawdreyAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer {
public:
  DawdreyAudioProcessorEditor(DawdreyAudioProcessor &);
  ~DawdreyAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;
  void timerCallback() override;

  void updatePresetList();

private:
  DawdreyAudioProcessor &audioProcessor;
  daisysp_gui::CustomLookAndFeel customLookAndFeel;

  using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

  juce::Slider freqSlider, fbGainSlider, fbDelaySlider, fbLpfSlider,
      fbHpfSlider;
  juce::Slider verbMixSlider, verbDecaySlider;
  juce::Slider echoSendSlider, echoTimeSlider, echoFbSlider;

  juce::Slider lfo1RateSlider, lfo1DepthSlider;
  juce::ComboBox lfo1ShapeBox, lfo1TargetBox;
  juce::ToggleButton lfo1SyncButton;
  juce::Slider lfo1DivSlider;

  juce::Slider lfo2RateSlider, lfo2DepthSlider;
  juce::ComboBox lfo2ShapeBox, lfo2TargetBox;
  juce::ToggleButton lfo2SyncButton;
  juce::Slider lfo2DivSlider;

  juce::Slider lfo3RateSlider, lfo3DepthSlider;
  juce::ComboBox lfo3ShapeBox, lfo3TargetBox;
  juce::ToggleButton lfo3SyncButton;
  juce::Slider lfo3DivSlider;

  std::unique_ptr<SliderAttachment> freqAttachment, fbGainAttachment,
      fbDelayAttachment, fbLpfAttachment, fbHpfAttachment;

  juce::ToggleButton pitchEnabledButton;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      pitchEnabledAttachment;
  juce::Slider pitchShiftSlider, pitchFineSlider;
  std::unique_ptr<SliderAttachment> pitchShiftAttachment, pitchFineAttachment;
  juce::Label pitchShiftLabel, pitchFineLabel;

  std::unique_ptr<SliderAttachment> verbMixAttachment, verbDecayAttachment;
  std::unique_ptr<SliderAttachment> echoSendAttachment, echoTimeAttachment,
      echoFbAttachment;

  juce::Slider dryWetSlider;
  std::unique_ptr<SliderAttachment> dryWetAttachment;

  juce::Slider widthSlider;
  std::unique_ptr<SliderAttachment> widthAttachment;

  // Input Gate
  juce::ToggleButton gateEnabledButton;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      gateEnabledAttachment;
  juce::Slider gateThreshSlider, gateReleaseSlider;
  std::unique_ptr<SliderAttachment> gateThreshAttachment, gateReleaseAttachment;
  juce::Label gateThreshLabel, gateReleaseLabel;

  juce::ToggleButton driveEnabledButton;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      driveEnabledAttachment;
  juce::Slider driveAmountSlider, driveGainSlider;
  std::unique_ptr<SliderAttachment> driveAmountAttachment, driveGainAttachment;
  juce::Label driveAmountLabel, driveGainLabel;

  SimpleMeter inputLevelMeter;

  juce::ComboBox presetBox;
  juce::TextButton savePresetButton{"Save"};
  juce::TextButton initPresetButton{"Init"};
  juce::TextButton prevPresetButton{"<"};
  juce::TextButton nextPresetButton{">"};
  juce::TextButton importPresetButton{"Import"};
  juce::TextButton exportPresetButton{"Export"};

  std::unique_ptr<juce::FileChooser> fileChooser;

  juce::TextButton undoButton{"Undo"};
  juce::TextButton redoButton{"Redo"};

  juce::ToggleButton instrumentModeButton;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      instrumentModeAttachment;
  bool wasInstrumentMode = false;

  juce::TooltipWindow tooltipWindow{this, 700};

  std::unique_ptr<SliderAttachment> lfo1RateAttachment, lfo1DepthAttachment,
      lfo1DivAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
      lfo1ShapeAttachment, lfo1TargetAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      lfo1SyncAttachment;
  juce::ToggleButton lfo1BipolarButton;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      lfo1BipolarAttachment;

  std::unique_ptr<SliderAttachment> lfo2RateAttachment, lfo2DepthAttachment,
      lfo2DivAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
      lfo2ShapeAttachment, lfo2TargetAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      lfo2SyncAttachment;
  juce::ToggleButton lfo2BipolarButton;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      lfo2BipolarAttachment;

  std::unique_ptr<SliderAttachment> lfo3RateAttachment, lfo3DepthAttachment,
      lfo3DivAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
      lfo3ShapeAttachment, lfo3TargetAttachment;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      lfo3SyncAttachment;
  juce::ToggleButton lfo3BipolarButton;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      lfo3BipolarAttachment;

  juce::Label freqLabel, fbGainLabel, fbDelayLabel, fbLpfLabel, fbHpfLabel;
  juce::Label verbMixLabel, verbDecayLabel;
  juce::Label echoSendLabel, echoTimeLabel, echoFbLabel;
  juce::Label dryWetLabel, widthLabel;

  SimpleMeter outputLevelMeter;

  juce::Label lfo1RateLabel, lfo1DepthLabel, lfo1ShapeLabel, lfo1TargetLabel;
  juce::Label lfo2RateLabel, lfo2DepthLabel, lfo2ShapeLabel, lfo2TargetLabel;
  juce::Label lfo3RateLabel, lfo3DepthLabel, lfo3ShapeLabel, lfo3TargetLabel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DawdreyAudioProcessorEditor)
};
