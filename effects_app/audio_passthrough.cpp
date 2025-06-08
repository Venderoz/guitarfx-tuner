#include "audio_passthrough.h"

AudioPassthrough::AudioPassthrough(AudioEffect *effect)
    : audio_(RtAudio::WINDOWS_ASIO), effect_(effect)
{
    if (audio_.getDeviceCount() < 1)
        throw std::runtime_error("No audio devices found.");

    inputParams_.deviceId = audio_.getDefaultInputDevice();
    outputParams_.deviceId = audio_.getDefaultOutputDevice();

    inputParams_.nChannels = 1;
    outputParams_.nChannels = 2;
    inputParams_.firstChannel = 1;
    outputParams_.firstChannel = 0;
}

void AudioPassthrough::start(unsigned int sampleRate, unsigned int bufferFrames)
{
    try
    {
        audio_.openStream(&outputParams_, &inputParams_, RTAUDIO_FLOAT32,
                          sampleRate, &bufferFrames, &AudioPassthrough::callback, this);

        audio_.startStream();
        while (running)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        stop();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void AudioPassthrough::stop()
{
    if (audio_.isStreamRunning())
        audio_.stopStream();
    if (audio_.isStreamOpen())
        audio_.closeStream();
    std::cout << "Audio passthrough stopped.\n";
}

int AudioPassthrough::callback(void *outputBuffer, void *inputBuffer,
                               unsigned int nFrames, double /*streamTime*/,
                               RtAudioStreamStatus status, void *userData)
{
    if (status)
        std::cerr << "Stream underflow/overflow detected.\n";

    auto *self = static_cast<AudioPassthrough *>(userData);
    float *in = static_cast<float *>(inputBuffer);
    float *out = static_cast<float *>(outputBuffer);

    for (unsigned int i = 0; i < nFrames; ++i)
    {
        float inputSample = in ? in[i] : 0.0f;
        float processed = self->effect_ ? self->effect_->process(inputSample) : inputSample;
        out[2 * i] = processed;
        out[2 * i + 1] = processed;
    }

    return 0;
}