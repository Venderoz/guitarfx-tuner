#include "RtAudio.h"
#include "frequency_detector.h"
#include <iostream>
#include <cmath>
#include <thread>
#include <map>
#include <string>
#include <limits>
#pragma once

// Class that manages user interaction and audio processing for tuning
class GuitarTuner
{
public:
    GuitarTuner();
    void run();

private:
    RtAudio::StreamParameters inputParams;
    FrequencyDetector detector;
    std::string currentString;
    float targetFreq;
    int frameCount;

    // RtAudio callback wrapper (static)
    static int audioCallbackWrapper(void *outputBuffer, void *inputBuffer, unsigned int nFrames,
                                    double streamTime, RtAudioStreamStatus status, void *userData);

    // Actual audio callback that processes the input audio
    int audioCallback(float *input, unsigned int nFrames, RtAudioStreamStatus status);

    // Function to select the string to tune
    bool selectString();
};
