#pragma once
#ifndef INFS_DSPUTILS_H
#define INFS_DSPUTILS_H

#include <cmath>
#include <algorithm>

// Minimal DaisySP replacements
namespace daisysp {
    inline float fmin(float a, float b) { return (a < b) ? a : b; }
    inline float fmax(float a, float b) { return (a > b) ? a : b; }
    inline float fclamp(float in, float min, float max) {
        return (in < min) ? min : (in > max) ? max : in;
    }
    inline float pow10f(float x) { return powf(10.0f, x); }
    inline float fastlog10f(float x) { return log10f(x); }
    
    static constexpr float PI_F = 3.1415927410125732421875f;
    static constexpr float TWOPI_F = 2.0f * PI_F;
}

namespace infrasonic {

static constexpr float PI_F = daisysp::PI_F;
static constexpr float TWOPI_F = daisysp::TWOPI_F;

inline float dbfs2lin(float dbfs) {
    return daisysp::pow10f(dbfs * 0.05f);
}

inline float lin2dbfs(float lin) {
    return daisysp::fastlog10f(lin) * 20.0f;
}

// Coefficient for one pole smoothing filter based on Tau time constant for `time_s`
inline float onepole_coef(float time_s, float sample_rate) {
    if (time_s <= 0.0f || sample_rate <= 0.0f) { return 1.0f; }
    return daisysp::fmin(1.0f / (time_s * sample_rate), 1.0f);
}

inline float onepole_coef_t60(float time_s, float sample_rate)
{
	return onepole_coef(time_s * 0.1447597f, sample_rate);
}

inline float ftension(const float in, const float factor)
{
    if (factor == 0.0f) return in;
    const float denom = expm1f(factor);
    return expm1f(in * factor) / denom;
}

inline float tanf(const float x)
{
    return std::tan(x);
}

}

#endif
