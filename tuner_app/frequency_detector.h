#include <cmath>
#include <vector>
#pragma once

// Class to detect the fundamental frequency of an audio signal
class FrequencyDetector
{
public:
    FrequencyDetector(float sampleRate);
    void setTarget(float freq);
    float detect(const float *input, int size);

private:
    float sampleRate;
    float targetFreq = 0.0f;
};