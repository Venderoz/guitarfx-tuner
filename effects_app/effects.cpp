#include "effects.h"
#include <cmath>

// --- Distortion ---
DistortionEffect::DistortionEffect(float gain, float mix) : gain_(gain), mix_(mix) {}

void DistortionEffect::setGain(float gain) { gain_ = gain; }
void DistortionEffect::setMix(float mix) { mix_ = mix; }

float DistortionEffect::getGain() const { return gain_; }
float DistortionEffect::getMix() const { return mix_; }

float DistortionEffect::process(float inputSample)
{
    float x = gain_ * inputSample;
    float threshold = 0.5f;
    float distorted = (x > threshold) ? threshold : (x < -threshold ? -threshold : x);
    return mix_ * distorted + (1.0f - mix_) * inputSample;
}

// --- Chorus ---
ChorusEffect::ChorusEffect(unsigned int sampleRate, float rate, float depth)
    : rate_(rate), depth_(depth), delayBase_(0.01f), sampleRate_(sampleRate), phase_(0.0f), writeIndex_(0)
{
    buffer_.resize(sampleRate);
}

void ChorusEffect::setRate(float rate) { rate_ = rate; }
void ChorusEffect::setDepth(float depth) { depth_ = (depth / 100); }

float ChorusEffect::getRate() const { return rate_; }
float ChorusEffect::getDepth() const { return depth_; }

float ChorusEffect::process(float inputSample)
{
    float delayTime = delayBase_ + depth_ * std::sin(2.0f * 3.141592f * phase_);
    int delaySamples = static_cast<int>(delayTime * sampleRate_);

    phase_ += rate_ / sampleRate_;
    if (phase_ >= 1.0f) phase_ -= 1.0f;

    buffer_[writeIndex_] = inputSample;
    int readIndex = (writeIndex_ - delaySamples + buffer_.size()) % buffer_.size();
    float delayed = buffer_[readIndex];

    writeIndex_ = (writeIndex_ + 1) % buffer_.size();
    return 0.5f * (inputSample + delayed);
}

// --- Delay ---
DelayEffect::DelayEffect(unsigned int sampleRate, float delayTime, float feedback)
    : sampleRate_(sampleRate), delayTime_(delayTime), feedback_(feedback), writeIndex_(0)
{
    int delaySamples = static_cast<int>(delayTime * sampleRate_);
    buffer_.resize(delaySamples);
}

void DelayEffect::setDelayTime(float dt)
{
    delayTime_ = dt;
    int delaySamples = static_cast<int>(delayTime_ * sampleRate_);
    if (delaySamples < 1) delaySamples = 1;
    buffer_.assign(delaySamples, 0.0f);
    writeIndex_ = 0;
}

float DelayEffect::getDelayTime() const { return delayTime_; }

float DelayEffect::process(float inputSample)
{
    float delayed = buffer_[writeIndex_];
    buffer_[writeIndex_] = inputSample + delayed * feedback_;

    writeIndex_ = (writeIndex_ + 1) % buffer_.size();
    return inputSample + delayed * 0.5f;
}
