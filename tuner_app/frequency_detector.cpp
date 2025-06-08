#include "frequency_detector.h"
#define PI 3.14159265358979323846

// Constructor: initialize sample rate
FrequencyDetector::FrequencyDetector(float sampleRate)
    : sampleRate(sampleRate) {}

// Set the frequency we want to match
void FrequencyDetector::setTarget(float freq)
{
    targetFreq = freq;
}

// Detect the dominant frequency in the given audio buffer
float FrequencyDetector::detect(const float *input, int size)
{
    
    //Hann windowing to reduce spectral leakage
    std::vector<float> windowed(size);
    for (int i = 0; i < size; ++i)
    {
        float window = 0.5f * (1 - std::cos(2 * PI * i / (size - 1)));
        windowed[i] = input[i] * window;
    }

    int minLag = sampleRate / (targetFreq * 1.5f);
    int maxLag = sampleRate / (targetFreq / 1.5f);

    // Autocorrelation to find periodicity
    float maxCorr = 0;
    int bestLag = -1;
    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        float sum = 0;
        for (int i = 0; i < size - lag; ++i)
            sum += windowed[i] * windowed[i + lag];

        if (sum > maxCorr)
        {
            maxCorr = sum;
            bestLag = lag;
        }
    }

    // No valid lag found
    if (bestLag <= 0)
        return 0.0f;

    // Estimate frequency from lag
    float freq = sampleRate / bestLag;

    // Fold down harmonics
    float bestFreq = freq;
    float bestDiff = std::abs(freq - targetFreq);
    for (int d = 2; d <= 6; ++d)
    {
        float f = freq / d;
        float diff = std::abs(f - targetFreq);
        if (diff < bestDiff && f > 20.0f)
        {
            bestFreq = f;
            bestDiff = diff;
        }
    }

    return bestFreq;
}
