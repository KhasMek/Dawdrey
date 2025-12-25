#pragma once
#ifndef DSY_TONE_H
#define DSY_TONE_H

#include <cmath>
#include "../DSPUtils.h" // for PI_F

namespace daisysp
{

class Tone
{
  public:
    Tone() {}
    ~Tone() {}

    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        out_ = 0.0f;
        SetFreq(1000.0f);
    }

    void SetFreq(float freq)
    {
        float wc = 2.0f * PI_F * freq / sample_rate_;
        float c = 2.0f - cosf(wc);
        coef_ = c - sqrtf(c * c - 1.0f);
    }

    float Process(float in)
    {
        out_ = out_ * coef_ + in * (1.0f - coef_);
        return out_;
    }

  private:
    float sample_rate_;
    float coef_;
    float out_;
};

}
#endif
