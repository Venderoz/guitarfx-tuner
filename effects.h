#pragma once
#include <vector>
#include <memory>
#include <cmath>

class AudioEffect
{
public:
    virtual ~AudioEffect() = default;
    virtual float process(float inputSample) = 0;
};

class DistortionEffect : public AudioEffect
{
public:
    DistortionEffect(float gain = 10.0f, float mix = 1.0f);
    void setGain(float gain);
    void setMix(float mix);
    float getGain() const;
    float getMix() const;
    float process(float inputSample) override;

private:
    float gain_;
    float mix_;
};

class ChorusEffect : public AudioEffect
{
public:
    ChorusEffect(unsigned int sampleRate, float rate = 0.25f, float depth = 0.002f);
    void setRate(float rate);
    void setDepth(float depth);
    float getRate() const;
    float getDepth() const;
    float process(float inputSample) override;

private:
    float rate_, depth_, delayBase_, phase_;
    unsigned int sampleRate_;
    std::vector<float> buffer_;
    int writeIndex_;
};

class DelayEffect : public AudioEffect
{
public:
    DelayEffect(unsigned int sampleRate, float delayTime = 0.15f, float feedback = 0.6f);
    void setDelayTime(float dt);
    float getDelayTime() const;
    float process(float inputSample) override;

private:
    unsigned int sampleRate_;
    float delayTime_, feedback_;
    std::vector<float> buffer_;
    int writeIndex_;
};
