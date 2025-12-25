#include "Overdrive.h"
#include <cmath>
#include <algorithm>

using namespace daisysp;

void Overdrive::Init()
{
    drive_ = 0.0f;
    pre_gain_ = 1.0f;
    post_gain_ = 1.0f;
}

void Overdrive::SetDrive(float drive)
{
    drive_ = drive;
    // Mapping from Plaits
    float pre = 1.0f + drive * drive * 48.0f;
    pre_gain_ = pre;
    // Compensation
    post_gain_ = 1.0f / std::sqrt(pre);
}

float Overdrive::Process(float in)
{
    float x = in * pre_gain_;
    // Soft clip (tanh approximation or similar)
    // Using a simple soft clipper for now: x / (1 + |x|)
    // Or just tanh
    // Plaits uses a specific smooth saturation
    
    // Simple tanh-like saturation:
    float x_abs = std::abs(x);
    float y;
    if (x_abs > 1.0f) {
        y = (x > 0.0f) ? 1.0f : -1.0f;
    } else {
        y = x * (1.5f - 0.5f * x * x); // cubic approximation
    }
    
    return y * post_gain_;
}
