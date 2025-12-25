#pragma once
#ifndef DSY_CROSSFADE_H
#define DSY_CROSSFADE_H

#include <cmath>

namespace daisysp
{

class CrossFade
{
  public:
    CrossFade() {}
    ~CrossFade() {}

    void Init(int curve = 0) // 0 = linear, 1 = constant power
    {
        curve_ = curve;
        pos_ = 0.0f;
    }

    void SetPos(float pos)
    {
        pos_ = pos;
    }

    float Process(float in1, float in2)
    {
        // Simple linear crossfade for now
        return in1 * (1.0f - pos_) + in2 * pos_;
    }

  private:
    int curve_;
    float pos_;
};

}
#endif
