// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"
#include "audio_parameter.h"
#include "core/release_pool.h"
#include "core/audio_buffer.h"
#include <memory>
#include <vector>

TW_NAMESPACE_BEGIN

class Engine;

class AudioEffect
{
public:

    using UniquePtr = std::unique_ptr<AudioEffect>;

    AudioEffect(int numParameters = 0);
    AudioEffect(const AudioEffect&) = delete;
    AudioEffect& operator =(const AudioEffect&) = delete;

    virtual ~AudioEffect();

    void setEngine(Engine* eng);

    virtual void prepareToPlay() = 0;

    virtual void process(const float* inL, const float* inR,
                         float* outL, float* outR,
                         int numFrames) = 0;

    /**
     * Returns the effect's processing tail length in samples.
     */
    virtual int getTailLength() const { return 0; }

protected:

    Engine* engine;

    AudioParameterPool params;
};

//==========================================================

class AudioEffectChain : public core::Releasable
{
public:

    using Ptr = std::shared_ptr<AudioEffectChain>;

    AudioEffectChain();
    AudioEffectChain(const AudioEffectChain&) = delete;
    AudioEffectChain& operator =(const AudioEffectChain&) = delete;

    ~AudioEffectChain();

    void prepareToPlay();

    void setEngine(Engine* eng);

    template<class Effect, typename... Args>
    Effect* addEffect (Args&&... args)
    {
        auto effect{ std::make_unique<Effect>(std::forward<Args>(args)...) };
        effect->setEngine(engine);
        auto* ptr{ effect.get() };
        effects.push_back(std::move(effect));
    }

    bool isEmpty() const noexcept;

    void clear();

    int getNumEffects() const noexcept;

    AudioEffect* operator [](int index);

    /**
     * Returns the effects chain processing tail length in samples.
     */
    int getTailLength() const;

    void process(const float* inL, const float* inR,
                 float* outL, float* outR,
                 int numFrames);

private:
    Engine* engine;
    std::vector<AudioEffect::UniquePtr> effects;

    core::AudioBuffer<float> mixBuffer;
};

TW_NAMESPACE_END
