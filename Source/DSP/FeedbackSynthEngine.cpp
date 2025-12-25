#include "FeedbackSynthEngine.h"
#include "DSPUtils.h"

using namespace infrasonic;
using namespace infrasonic::FeedbackSynth;
using namespace daisysp;

static float mtof(float m) { return 440.0f * powf(2.0f, (m - 69.0f) / 12.0f); }

static void fonepole(float &out, float in, float coeff) {
  out += coeff * (in - out);
}

void Engine::Init(const float sample_rate) {
  using ED = EchoDelay<kMaxEchoDelaySamp>;

  // Use standard allocation instead of SDRAM
  echo_delay_[0] = std::make_unique<ED>();
  echo_delay_[1] = std::make_unique<ED>();
  verb_ = std::make_unique<ReverbSc>();

  sample_rate_ = sample_rate;
  fb_delay_smooth_coef_ = onepole_coef(0.2f, sample_rate);

  noise_.Init();
  noise_.SetAmp(dbfs2lin(-90.0f));

  for (unsigned int i = 0; i < 2; i++) {

    strings_[i].Init(sample_rate);
    strings_[i].SetBrightness(0.98f);
    strings_[i].SetFreq(440.0f); // Default freq
    strings_[i].SetDamping(0.4f);

    fb_delayline_[i].Init();

    echo_delay_[i]->Init(sample_rate);
    echo_delay_[i]->SetDelayTime(0.5f, true); // Default 500ms
    echo_delay_[i]->SetFeedback(0.5f);
    echo_delay_[i]->SetLagTime(0.5f);

    overdrive_[i].Init();
    overdrive_[i].SetDrive(0.4f);

    pitchShifter[i].Init(sample_rate);
  }

  verb_->Init(sample_rate);
  verb_->SetFeedback(0.85f);
  verb_->SetLpFreq(18000.0f);

  fb_lpf_.Init(sample_rate);
  fb_lpf_.SetQ(0.9f);
  fb_lpf_.SetCutoff(18000.0f);

  fb_hpf_.Init(sample_rate);
  fb_hpf_.SetQ(0.9f);
  fb_hpf_.SetCutoff(60.f);
}

void Engine::SetStringPitch(const float nn) { freq_param_ = nn; }

void Engine::SetFeedbackGain(const float gain_db) {
  fb_gain_ = dbfs2lin(gain_db);
}

void Engine::SetFeedbackDelay(const float delay_s) {
  fb_delay_samp_target_ = fclamp(delay_s * sample_rate_, 1.0f,
                                 static_cast<float>(kMaxFeedbackDelaySamp - 1));
}

void Engine::SetFeedbackLPFCutoff(const float cutoff_hz) {
  fb_lpf_.SetCutoff(cutoff_hz);
}

void Engine::SetFeedbackHPFCutoff(const float cutoff_hz) {
  fb_hpf_.SetCutoff(cutoff_hz);
}

void Engine::SetEchoDelayTime(const float echo_time) {
  echo_delay_[0]->SetDelayTime(echo_time);
  echo_delay_[1]->SetDelayTime(echo_time);
}

void Engine::SetEchoDelayFeedback(const float echo_fb) {
  echo_delay_[0]->SetFeedback(echo_fb);
  echo_delay_[1]->SetFeedback(echo_fb);
}

void Engine::SetEchoDelaySendAmount(const float echo_send) {
  echo_send_ = echo_send;
}

void Engine::SetReverbMix(const float mix) {
  verb_mix_ = fclamp(mix, 0.0f, 1.0f);
}

void Engine::SetReverbFeedback(const float time) { verb_->SetFeedback(time); }

void Engine::SetOutputLevel(const float level) { output_level_ = level; }

void Engine::Process(float in, float &outL, float &outR) {
  // --- Update audio-rate-smoothed control params ---

  // fonepole(fb_delay_samp_, fb_delay_samp_target_, fb_delay_smooth_coef_);
  // Manual one pole
  fb_delay_samp_ +=
      fb_delay_smooth_coef_ * (fb_delay_samp_target_ - fb_delay_samp_);

  // --- Process Samples ---

  float inL, inR, sampL, sampR, echoL, echoR, verbL, verbR;
  const float noise_samp = noise_.Process();

  // ---> Feedback Loop

  // Get noise + feedback output
  // Read from delay line
  // Note: DaisySP DelayLine Read takes float delay
  inL = fb_delayline_[0].Read(fb_delay_samp_) + noise_samp + in;
  inR = fb_delayline_[1].Read(daisysp::fmax(1.0f, fb_delay_samp_ - 4.f)) +
        noise_samp + in;

  // Process through KS resonator
  sampL = strings_[0].Process(inL);
  sampR = strings_[1].Process(inR);

  // Distort + Clip
  sampL = overdrive_[0].Process(sampL);
  sampR = overdrive_[1].Process(sampR);

  // Filter in feedback loop
  fb_lpf_.ProcessStereo(sampL, sampR);
  fb_hpf_.ProcessStereo(sampL, sampR);

  // ---> Reverb

  verb_->Process(sampL, sampR, &verbL, &verbR);

  //       (sampL * (1.0f - verb_mix_)) + verbL * verb_mix_;
  //       sampL - sampL * verb_mix + verbL * verb_mix_;
  sampL -= (sampL - verbL) * verb_mix_;
  sampR -= (sampR - verbR) * verb_mix_;

  // Calculate Target Frequency
  float target_freq = mtof(freq_param_);
  if (instrumentMode) {
    target_freq = mtof(midi_pitch_);
  }

  // Smooth Frequency
  fonepole(freq_, target_freq, 0.01f); // Smooth transition
  strings_[0].SetFreq(freq_);
  strings_[1].SetFreq(freq_);

  // ---> Resonator feedback

  float fbL = sampL;
  float fbR = sampR;

  // Pitch Shifter (Applied only to feedback signal)
  if (pitchEnabled) {
    float shift = pitchShift + (pitchFine / 100.0f);

    pitchShifter[0].SetShift(shift);
    fbL = pitchShifter[0].Process(fbL);

    pitchShifter[1].SetShift(shift);
    fbR = pitchShifter[1].Process(fbR);
  }

  // Write back into delay with attenuation
  fb_delayline_[0].Write(fbL * fb_gain_);
  fb_delayline_[1].Write(fbR * fb_gain_);

  // ---> Echo Delay

  echoL = echo_delay_[0]->Process(sampL * echo_send_);
  echoR = echo_delay_[1]->Process(sampR * echo_send_);

  sampL = 0.5f * (sampL + echoL);
  sampR = 0.5f * (sampR + echoR);

  // ---> Output
  outL = sampL * output_level_;
  outR = sampR * output_level_;
}
