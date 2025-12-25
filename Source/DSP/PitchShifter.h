#pragma once
#include <vector>
#include <cmath>
#include <JuceHeader.h>

class PitchShifter
{
public:
    PitchShifter() = default;
    ~PitchShifter() = default;

    void Init(float sampleRate)
    {
        sr = sampleRate;
        // Buffer size: 200ms should be plenty for the window
        bufferSize = static_cast<int>(sr * 0.2f); 
        buffer.resize(bufferSize, 0.0f);
        writePos = 0;
        phase = 0.0f;
        
        // Default window size ~50ms
        windowSize = sr * 0.05f;
        increment = 0.0f;
    }

    void SetShift(float semitones)
    {
        // Ratio = 2^(semitones/12)
        float ratio = std::pow(2.0f, semitones / 12.0f);
        
        // d(Delay)/dt = 1 - ratio
        // phase goes 0->1. delay = phase * windowSize.
        // d(delay)/dt = increment * windowSize (per sample, implicitly)
        // So increment = (1 - ratio) / windowSize
        
        if (windowSize > 0.0f)
            increment = (1.0f - ratio) / windowSize;
        else
            increment = 0.0f;
    }
    
    // Optional: Fine tune in cents
    void SetFine(float cents)
    {
        // Total semitones = semitones + cents/100
        // We'll handle this by combining them in the caller or adding a member.
        // For now, let's assume SetShift receives the total combined value.
    }

    float Process(float input)
    {
        if (bufferSize == 0) return input;

        // Write to buffer
        buffer[writePos] = input;

        // Calculate delays for two heads
        float delay1 = phase * windowSize;
        float delay2 = std::fmod(phase + 0.5f, 1.0f) * windowSize;
        
        // Get samples
        float r1 = GetSample(delay1);
        float r2 = GetSample(delay2);
        
        // Triangle Window
        // 0.0 -> 0.5 -> 1.0 (phase) => 0 -> 1 -> 0 (weight)
        // We want weight to be 1 when phase is 0.5 (center of window)
        // And 0 when phase is 0 or 1 (edges)
        
        // Head 1 phase: 'phase' (0..1)
        // Center is 0.5. Dist from center is abs(phase - 0.5).
        // Weight = 1 - 2 * dist = 1 - 2 * abs(phase - 0.5)
        float w1 = 1.0f - 2.0f * std::abs(phase - 0.5f);
        
        // Head 2 phase: 'phase + 0.5' (wrapped)
        float p2 = std::fmod(phase + 0.5f, 1.0f);
        float w2 = 1.0f - 2.0f * std::abs(p2 - 0.5f);
        
        // Sum
        float output = (r1 * w1 + r2 * w2);
        // Note: w1 + w2 is always 1 for triangle waves separated by 0.5 phase.
        
        // Update phase
        phase += increment;
        if (phase >= 1.0f) phase -= 1.0f;
        if (phase < 0.0f) phase += 1.0f;
        
        // Update write pos
        writePos++;
        if (writePos >= bufferSize) writePos = 0;
        
        return output;
    }

private:
    float GetSample(float delaySamples)
    {
        float pos = (float)writePos - delaySamples;
        while (pos < 0) pos += bufferSize;
        while (pos >= bufferSize) pos -= bufferSize;
        
        int i = (int)pos;
        float f = pos - i;
        int i2 = (i + 1);
        if (i2 >= bufferSize) i2 = 0;
        
        return buffer[i] * (1.0f - f) + buffer[i2] * f;
    }

    std::vector<float> buffer;
    int bufferSize = 0;
    int writePos = 0;
    float sr = 44100.0f;
    
    float phase = 0.0f;
    float increment = 0.0f;
    float windowSize = 0.0f;
};
