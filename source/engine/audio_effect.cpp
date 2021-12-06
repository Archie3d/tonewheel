// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "audio_effect.h"
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
        ::memcpy(outR, inL, sizeof(float) * numFrames);
    } else if (effects.size() == 1) {
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
        (*it)->process(inBufL, inBufR, outBufL, outBufR, numFrames);
        ++it;

        while (it != effects.end()) {
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