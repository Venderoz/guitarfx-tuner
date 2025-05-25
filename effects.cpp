#include <iostream>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <memory>
#include "RtAudio.h"

class AudioEffect
{
public:
    virtual ~AudioEffect() = default;
    virtual float process(float inputSample) = 0;
};

// Wraps an effect with enable/disable support
class EffectWrapper
{
public:
    EffectWrapper(std::shared_ptr<AudioEffect> effect, bool enabled = true)
        : effect(std::move(effect)), enabled(enabled) {}

    float process(float inputSample)
    {
        return (enabled && effect) ? effect->process(inputSample) : inputSample;
    }

    void setEnabled(bool state) { enabled = state; }
    bool isEnabled() const { return enabled; }

    std::shared_ptr<AudioEffect> getEffect() const { return effect; }

private:
    std::shared_ptr<AudioEffect> effect;
    bool enabled;
};

// Chain of effects with input gain control
class EffectChain : public AudioEffect
{
public:
    EffectChain(float inputGain = 1.0f)
        : inputGain(inputGain) {}

    float process(float inputSample) override
    {
        float sample = inputSample * inputGain;
        for (auto &wrapper : effects)
        {
            sample = wrapper.process(sample);
        }
        return sample;
    }

    void addEffect(std::shared_ptr<AudioEffect> effect, bool enabled = true)
    {
        effects.emplace_back(effect, enabled);
    }

    void setInputGain(float gain)
    {
        inputGain = gain;
    }

    float getInputGain() const
    {
        return inputGain;
    }

    void enableEffect(size_t index, bool enabled)
    {
        if (index < effects.size())
        {
            effects[index].setEnabled(enabled);
        }
    }

    void toggleEffect(size_t index)
    {
        if (index < effects.size())
        {
            bool current = effects[index].isEnabled();
            effects[index].setEnabled(!current);
        }
    }

    void listEffects() const
    {
        std::cout << "EffectChain [" << effects.size() << " effects]:\n";
        for (size_t i = 0; i < effects.size(); ++i)
        {
            std::cout << " - Effect #" << i << ": "
                      << (effects[i].isEnabled() ? "Enabled" : "Disabled") << "\n";
        }
    }

private:
    std::vector<EffectWrapper> effects;
    float inputGain;
};

class DistortionEffect : public AudioEffect
{
public:
    DistortionEffect(float gain = 10.0f, float mix = 1.0f, bool useTanh = false)
        : gain(gain), mix(mix), useTanh(useTanh) {}

    float process(float inputSample) override
    {
        float x = gain * inputSample;
        float distorted;

        if (useTanh)
        {
            // Soft but aggressive: non-linear waveshaping
            distorted = std::tanh(x);
        }
        else
        {
            const float threshold = 0.4f; // more aggressive
            if (x > threshold)
                distorted = threshold;
            else if (x < -threshold)
                distorted = -threshold;
            else
                distorted = x;
        }

        // Blend dry and wet
        return mix * distorted + (1.0f - mix) * inputSample;
    }

private:
    float gain;
    float mix;
    bool useTanh;
};
class ChorusEffect : public AudioEffect
{
public:
    ChorusEffect(unsigned int sampleRate)
        : rate(0.25f), depth(0.002f), delayBase(0.01f), sampleRate(sampleRate), phase(0.0f), writeIndex(0)
    {
        buffer.resize(sampleRate); // 1 second buffer
    }

    float process(float input) override
    {
        float delayTime = delayBase + depth * std::sin(2.0f * 3.141592f * phase);
        int delaySamples = static_cast<int>(delayTime * sampleRate);

        phase += rate / sampleRate;
        if (phase >= 1.0f)
            phase -= 1.0f;

        buffer[writeIndex] = input;
        int readIndex = (writeIndex - delaySamples + buffer.size()) % buffer.size();
        float delayed = buffer[readIndex];

        writeIndex = (writeIndex + 1) % buffer.size();
        return 0.5f * (input + delayed);
    }

private:
    float rate, depth, delayBase, phase;
    unsigned int sampleRate;
    std::vector<float> buffer;
    int writeIndex;
};
class DelayEffect : public AudioEffect
{
public:
    DelayEffect(unsigned int sampleRate, float delayTime = 0.15f, float feedback = 0.4f)
        : sampleRate(sampleRate), feedback(feedback), writeIndex(0)
    {
        int delaySamples = static_cast<int>(delayTime * sampleRate);
        buffer.resize(delaySamples);
    }

    float process(float input) override
    {
        float delayed = buffer[writeIndex];
        buffer[writeIndex] = input + delayed * feedback;

        writeIndex = (writeIndex + 1) % buffer.size();
        return input + delayed * 0.5f; // Blend original + delayed
    }

private:
    unsigned int sampleRate;
    float feedback;
    std::vector<float> buffer;
    int writeIndex;
};

class AudioPassthrough
{
public:
    AudioPassthrough(AudioEffect *effect = nullptr)
        : audio(RtAudio::WINDOWS_ASIO), effect(effect)
    {
        if (audio.getDeviceCount() < 1)
        {
            throw std::runtime_error("No audio devices found.");
        }

        inputParams.deviceId = audio.getDefaultInputDevice();
        outputParams.deviceId = audio.getDefaultOutputDevice();

        inputParams.nChannels = 1;    // Mono input
        outputParams.nChannels = 2;   // Stereo output
        inputParams.firstChannel = 1; // Try channel 1 (try 0 if no signal)
        outputParams.firstChannel = 0;

        std::cout << "Input:  " << audio.getDeviceInfo(inputParams.deviceId).name << "\n";
        std::cout << "Output:  " << audio.getDeviceInfo(outputParams.deviceId).name << "\n";
    }

    void start(unsigned int sampleRate = 48000, unsigned int bufferFrames = 128)
    {
        try
        {
            audio.openStream(&outputParams, &inputParams, RTAUDIO_FLOAT32,
                             sampleRate, &bufferFrames, &AudioPassthrough::callback, this);

            audio.startStream();
            std::cout << "🔊 Audio passthrough with distortion running. Press Enter to quit.\n";
            std::cin.get();
            stop();
        }
        catch (std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void stop()
    {
        if (audio.isStreamRunning())
            audio.stopStream();
        if (audio.isStreamOpen())
            audio.closeStream();
        std::cout << "Audio passthrough stopped.\n";
    }

private:
    static int callback(void *outputBuffer, void *inputBuffer,
                        unsigned int nFrames, double /*streamTime*/,
                        RtAudioStreamStatus status, void *userData)
    {
        if (status)
        {
            std::cerr << "Stream underflow/overflow detected.\n";
        }

        auto *self = static_cast<AudioPassthrough *>(userData);
        float *in = static_cast<float *>(inputBuffer);
        float *out = static_cast<float *>(outputBuffer);

        for (unsigned int i = 0; i < nFrames; ++i)
        {
            float inputSample = in ? in[i] : 0.0f;
            float processed = self->effect ? self->effect->process(inputSample) : inputSample;

            // Output stereo
            out[2 * i] = processed;     // Left
            out[2 * i + 1] = processed; // Right
        }

        return 0;
    }

    RtAudio audio;
    RtAudio::StreamParameters inputParams, outputParams;
    AudioEffect *effect;
};

int main()
{
    try
    {
        unsigned int sampleRate = 48000;

        auto chain = std::make_shared<EffectChain>();
        chain->setInputGain(5.0f); // boost input signal

        chain->addEffect(std::make_shared<DistortionEffect>(10.0f, 1.0f, true), false);
        chain->addEffect(std::make_shared<ChorusEffect>(sampleRate), true); // disabled by default
        chain->addEffect(std::make_shared<DelayEffect>(sampleRate), true);

        chain->listEffects();

        // Pass to passthrough
        AudioPassthrough passthrough(chain.get()); // pass raw pointer for callback use
        passthrough.start(sampleRate);             // make sure sampleRate is passed in
    }
    catch (const std::exception &e)
    {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
