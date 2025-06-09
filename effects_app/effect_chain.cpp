#include "effect_chain.h"

// Wrapper
EffectWrapper::EffectWrapper(std::shared_ptr<AudioEffect> effect, const std::string& name, bool enabled)
    : effect_(std::move(effect)), name_(name), enabled_(enabled) {}

float EffectWrapper::process(float inputSample)
{
    return (enabled_ && effect_) ? effect_->process(inputSample) : inputSample;
}

void EffectWrapper::setEnabled(bool state) { enabled_ = state; }
bool EffectWrapper::isEnabled() const { return enabled_; }
std::shared_ptr<AudioEffect> EffectWrapper::getEffect() const { return effect_; }
std::string EffectWrapper::getName() const { return name_; }

// Chain
EffectChain::EffectChain(float inputGain) : inputGain_(inputGain) {}

float EffectChain::process(float inputSample)
{
    float sample = inputSample * inputGain_;
    for (auto& wrapper : effects_)
        sample = wrapper.process(sample);
    return sample;
}

void EffectChain::addEffect(std::shared_ptr<AudioEffect> effect, const std::string& name, bool enabled)
{
    effects_.emplace_back(effect, name, enabled);
}

void EffectChain::setInputGain(float gain) { inputGain_ = gain; }
float EffectChain::getInputGain() const { return inputGain_; }

void EffectChain::enableEffect(size_t index, bool enabled)
{
    if (index < effects_.size())
        effects_[index].setEnabled(enabled);
}

void EffectChain::toggleEffect(size_t index)
{
    if (index < effects_.size())
        effects_[index].setEnabled(!effects_[index].isEnabled());
}

std::shared_ptr<AudioEffect> EffectChain::getEffect(size_t index) const
{
    if (index < effects_.size())
        return effects_[index].getEffect();
    return nullptr;
}

void EffectChain::listEffects() const
{
    std::cout << "Input Gain: " << inputGain_ << "\n";
    std::cout << "EffectChain [" << effects_.size() << " effects]:\n";
    for (size_t i = 0; i < effects_.size(); ++i)
    {
        std::cout << " - [" << i + 1 << "] " << effects_[i].getName()
                  << " - " << (effects_[i].isEnabled() ? "\033[32mEnabled\033[0m" : "\033[31mDisabled\033[0m") << "\n";
    }
}
