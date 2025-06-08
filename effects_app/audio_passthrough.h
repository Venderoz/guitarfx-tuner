#pragma once
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <atomic>
#include "RtAudio.h"
#include "effects.h"

extern std::atomic<bool> running;

class AudioPassthrough
{
public:
    explicit AudioPassthrough(AudioEffect* effect = nullptr);

    void start(unsigned int sampleRate = 48000, unsigned int bufferFrames = 64);
    void stop();

private:
    static int callback(void* outputBuffer, void* inputBuffer,
                        unsigned int nFrames, double streamTime,
                        RtAudioStreamStatus status, void* userData);

    RtAudio audio_;
    RtAudio::StreamParameters inputParams_, outputParams_;
    AudioEffect* effect_;
};

