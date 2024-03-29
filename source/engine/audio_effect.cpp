// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "audio_effect.h"
#include "fx/filters.h"
#include "fx/delay.h"
#include "fx/send.h"
#include "fx/vocoder.h"
#include "fx/pitch_shift.h"
#include "fx/frequency_shift.h"
#include "fx/phase_shift.h"
#include "fx/reverb.h"
#include <cassert>

TW_NAMESPACE_BEGIN

AudioEffect::AudioEffect(int numParameters)
    : engine{ nullptr }
    , params(numParameters)
{
}

AudioEffect::~AudioEffect() = default;

void AudioEffect::setEngine(Engine* eng)
{
    assert(eng != nullptr);
    engine = eng;
}

void AudioEffect::updateParametersSmoothing()
{
    params.updateSmoothing();
}

AudioEffect::UniquePtr AudioEffect::createByTag(const std::string& tag)
{
    static const AudioEffectFactory factory {
        { fx::LowPassFilter::tag,      []() { return std::make_unique<fx::LowPassFilter>(); } },
        { fx::HighPassFilter::tag,     []() { return std::make_unique<fx::HighPassFilter>(); } },
        { fx::LowShelfFilter::tag,     []() { return std::make_unique<fx::LowShelfFilter>(); } },
        { fx::HighShelfFilter::tag,    []() { return std::make_unique<fx::HighShelfFilter>(); } },
        { fx::BandPassFilter::tag,     []() { return std::make_unique<fx::BandPassFilter>(); } },
        { fx::NotchFilter::tag,        []() { return std::make_unique<fx::NotchFilter>(); } },
        { fx::AllPassFilter::tag,      []() { return std::make_unique<fx::AllPassFilter>(); } },
        { fx::Delay::tag,              []() { return std::make_unique<fx::Delay>(); } },
        { fx::Send::tag,               []() { return std::make_unique<fx::Send>(); } },
        { fx::VocoderAnalyzer::tag,    []() { return std::make_unique<fx::VocoderAnalyzer>(); } },
        { fx::VocoderSynthesizer::tag, []() { return std::make_unique<fx::VocoderSynthesizer>(); } },
        { fx::PitchShift::tag,         []() { return std::make_unique<fx::PitchShift>(); } },
        { fx::FrequencyShift::tag,     []() { return std::make_unique<fx::FrequencyShift>(); } },
        { fx::PhaseShift::tag,         []() { return std::make_unique<fx::PhaseShift>(); } },
        { fx::Reverb::tag,             []() { return std::make_unique<fx::Reverb>(); } }
    };

    // This will return nullptr if effect tag cannot be found in the factory
    return factory.create(tag);
}

//==============================================================================

AudioEffectChain::AudioEffectChain()
    : engine{ nullptr }
    , effects()
    , mixBuffer(MIX_BUFFER_NUM_CHANNELS, MIX_BUFFER_NUM_FRAMES)
{
}

AudioEffectChain::~AudioEffectChain() = default;

void AudioEffectChain::prepareToPlay()
{
    assert(engine != nullptr);

    for (auto& fx : effects)
        fx->prepareToPlay();

    mixBuffer.clear();
}

void AudioEffectChain::setEngine(Engine* eng)
{
    assert(eng != nullptr);
    engine = eng;

    for (auto& fx : effects)
        fx->setEngine(engine);
}

AudioEffect* AudioEffectChain::addEffectByTag(const std::string& tag)
{
    if (auto fx{ AudioEffect::createByTag(tag)}) {
        fx->setEngine(engine);
        auto* ptr{ fx.get() };
        effects.push_back(std::move(fx));
        return ptr;
    }

    return nullptr;
}

bool AudioEffectChain::isEmpty() const noexcept
{
    return effects.empty();
}

void AudioEffectChain::clear()
{
    effects.clear();
}

int AudioEffectChain::getNumEffects() const noexcept
{
    return (int)effects.size();
}

AudioEffect* AudioEffectChain::operator [](int index)
{
    return getEffectByIndex(index);
}

AudioEffect* AudioEffectChain::getEffectByIndex(int index)
{
    if (index >= 0 && index < (int)effects.size())
        return effects.at((size_t)index).get();

    return nullptr;
}

int AudioEffectChain::getTailLength() const
{
    int length{};

    for (auto& fx : effects)
        length += fx->getTailLength();

    return length;
}

void AudioEffectChain::process(const float* inL, const float* inR,
                               float* outL, float* outR,
                               int numFrames)
{
    assert(numFrames <= mixBuffer.getNumFrames());

    if (isEmpty()) {
        ::memcpy(outL, inL, sizeof(float) * numFrames);
        ::memcpy(outR, inR, sizeof(float) * numFrames);
    } else if (effects.size() == 1) {
        effects.front()->updateParametersSmoothing();
        effects.front()->process(inL, inR, outL, outR, numFrames);
    } else {
        const float* inBufL{ inL };
        const float* inBufR{ inR };

        const bool evenNumberOfEffects{ effects.size() % 2 == 0 };

        float* outBufL{ outL };
        float* outBufR{ outR };
        float* outNextBufL{ mixBuffer.getChannelData(0) };
        float* outNextBufR{ mixBuffer.getChannelData(1) };

        if (evenNumberOfEffects) {
            std::swap(outBufL, outNextBufL);
            std::swap(outBufR, outNextBufR);
        }

        auto it{ effects.begin() };
        (*it)->updateParametersSmoothing();
        (*it)->process(inBufL, inBufR, outBufL, outBufR, numFrames);
        ++it;

        while (it != effects.end()) {
            (*it)->updateParametersSmoothing();
            (*it)->process(outBufL, outBufR, outNextBufL, outNextBufR, numFrames);

            std::swap(outBufL, outNextBufL);
            std::swap(outBufR, outNextBufR);

            ++it;
        }

        assert(outBufL == outL);
        assert(outBufR == outR);
    }
}


TW_NAMESPACE_END
