#pragma once
#include <cmath>
#include <numbers>


namespace daisysp
{

class SimpleLFO
{
public:
    enum Waveform
    {
        WAVE_SINE,
        WAVE_TRI,
        WAVE_SAW,
        WAVE_RAMP,
        WAVE_SQUARE,
        WAVE_S_AND_H,
        WAVE_LAST
    };

    SimpleLFO() : sample_rate_(48000.0f), phase_(0.0f), rate_(1.0f), amp_(1.0f), waveform_(WAVE_SINE), last_out_(0.0f) {}
    ~SimpleLFO() {}

    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        phase_ = 0.0f;
        rate_ = 1.0f;
        amp_ = 1.0f;
        waveform_ = WAVE_SINE;
        last_out_ = 0.0f;
    }

    void SetRate(float rate) { rate_ = rate; }
    void SetAmp(float amp) { amp_ = amp; }
    void SetWaveform(int waveform) { waveform_ = static_cast<Waveform>(waveform); }

    float Process()
    {
        float out = 0.0f;
        if (sample_rate_ <= 0.0f) return 0.0f;
        float inc = rate_ / sample_rate_;
        phase_ += inc;
        if (phase_ > 1.0f) phase_ -= 1.0f;

        switch (waveform_)
        {
            case WAVE_SINE:
                out = std::sin(phase_ * 2.0f * std::numbers::pi_v<float>);
                break;
            case WAVE_TRI:
                out = phase_ < 0.5f ? 4.0f * phase_ - 1.0f : 1.0f - 4.0f * (phase_ - 0.5f);
                break;
            case WAVE_SAW:
                out = 1.0f - 2.0f * phase_;
                break;
            case WAVE_RAMP:
                out = 2.0f * phase_ - 1.0f;
                break;
            case WAVE_SQUARE:
                out = phase_ < 0.5f ? 1.0f : -1.0f;
                break;
            case WAVE_S_AND_H:
                // Sample and Hold updates only when phase wraps
                if (phase_ < inc)
                {
                    last_out_ = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;
                }
                out = last_out_;
                break;
            default:
                out = 0.0f;
                break;
        }

        return out * amp_;
    }

private:
    float sample_rate_;
    float phase_;
    float rate_;
    float amp_;
    Waveform waveform_;
    float last_out_;
};

} // namespace daisysp
