#pragma once
#ifndef IFS_FEEDBACK_SYNTH_ENGINE_H
#define IFS_FEEDBACK_SYNTH_ENGINE_H

#include "BiquadFilters.h"
#include "DSPUtils.h"
#include "EchoDelay.h"
#include "KarplusString.h"
#include "PitchShifter.h"
#include "daisysp/DelayLine.h"
#include "daisysp/Overdrive.h"
#include "daisysp/ReverbSc.h"
#include "daisysp/WhiteNoise.h"
#include <memory>

namespace infrasonic {
namespace FeedbackSynth {

class Engine {

public:
  Engine() = default;
  ~Engine() = default;

  void Init(const float sample_rate);

  void SetStringPitch(const float nn);

  void SetFeedbackGain(const float gain_dbfs);

  void SetFeedbackDelay(const float delay_s);
  void SetFeedbackLPFCutoff(const float cutoff_hz);
  void SetFeedbackHPFCutoff(const float cutoff_hz);

  void SetEchoDelayTime(const float echo_time);
  void SetEchoDelayFeedback(const float echo_fb);
  void SetEchoDelaySendAmount(const float echo_send);

  // Both range 0-1
  void SetReverbMix(const float mix);
  void SetReverbFeedback(const float time);

  void SetOutputLevel(const float level);

  void Process(float in, float &outL, float &outR);

  // Pitch Shifter parameters
  bool pitchEnabled = false;
  float pitchShift = 0.0f; // Semitones
  float pitchFine = 0.0f;  // Cents

  // Instrument Mode
  void SetMidiPitch(float pitch) { midi_pitch_ = pitch; }
  bool instrumentMode = false;

private:
  float midi_pitch_ = 60.0f; // Default Middle C
  // long enough for 250ms at 48kHz
  static constexpr size_t kMaxFeedbackDelaySamp = 12000;
  // long enough for 5s at 48kHz
  static constexpr size_t kMaxEchoDelaySamp = 48000 * 5;

  float sample_rate_;
  float fb_gain_ = 0.0f;
  float echo_send_ = 0.0f;
  float verb_mix_ = 0.0f;
  float output_level_ = 0.5f;

  float freq_param_ = 440.0f;
  float freq_ = 440.0f;

  float fb_delay_smooth_coef_;
  float fb_delay_samp_ = 1000.f;
  float fb_delay_samp_target_ = 64.f;

  infrasonic::KarplusString strings_[2];
  daisysp::WhiteNoise noise_;
  daisysp::DelayLine<float, kMaxFeedbackDelaySamp> fb_delayline_[2];
  daisysp::Overdrive overdrive_[2];
  PitchShifter pitchShifter[2]; // Stereo Pitch Shifter

  LPF12 fb_lpf_;
  HPF12 fb_hpf_;

  using VerbPtr = std::unique_ptr<daisysp::ReverbSc>;
  VerbPtr verb_;

  using EchoDelayPtr = std::unique_ptr<EchoDelay<kMaxEchoDelaySamp>>;
  EchoDelayPtr echo_delay_[2];

  Engine(const Engine &other) = delete;
  Engine(Engine &&other) = delete;
  Engine &operator=(const Engine &other) = delete;
  Engine &operator=(Engine &&other) = delete;
};

} // namespace FeedbackSynth
} // namespace infrasonic

#endif
