#pragma once
#include "effects.h"
#include <string>
#include <vector>
#include <iostream>
#include <memory>

class EffectWrapper
{
public:
    EffectWrapper(std::shared_ptr<AudioEffect> effect, const std::string& name, bool enabled = true);
    float process(float inputSample);
    void setEnabled(bool state);
    bool isEnabled() const;
    std::shared_ptr<AudioEffect> getEffect() const;
    std::string getName() const;

private:
    std::shared_ptr<AudioEffect> effect_;
    std::string name_;
    bool enabled_;
};
// A composite effect that contains and manages a chain of multiple effects
class EffectChain : public AudioEffect
{
public:
    EffectChain(float inputGain = 1.0f);
    float process(float inputSample) override;
    void addEffect(std::shared_ptr<AudioEffect> effect, const std::string& name, bool enabled = true);
    void setInputGain(float gain);
    float getInputGain() const;
    void enableEffect(size_t index, bool enabled);
    void toggleEffect(size_t index);
    // Gets the underlying AudioEffect by index
    std::shared_ptr<AudioEffect> getEffect(size_t index) const;
    void listEffects() const;

private:
    std::vector<EffectWrapper> effects_;
    float inputGain_;
};
