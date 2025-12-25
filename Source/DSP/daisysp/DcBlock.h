#pragma once
#ifndef DSY_DCBLOCK_H
#define DSY_DCBLOCK_H

namespace daisysp
{

class DcBlock
{
  public:
    DcBlock() {}
    ~DcBlock() {}

    void Init(float sample_rate)
    {
        // R = 1 - (pi * 2 * cutoff / samplerate)
        // cutoff ~ 10Hz
        R_ = 1.0f - (3.14159f * 2.0f * 10.0f / sample_rate);
        x_ = 0.0f;
        y_ = 0.0f;
    }

    float Process(float in)
    {
        y_ = in - x_ + R_ * y_;
        x_ = in;
        return y_;
    }

  private:
    float R_;
    float x_, y_;
};

}
#endif
