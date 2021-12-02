// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"
#include "global_engine.h"
#include "audio_parameter.h"
#include "voice.h"
#include "core/audio_buffer.h"
#include "core/list.h"
#include <vector>

TW_NAMESPACE_BEGIN

class Engine;

/**
 * This class represents a single audio bus.
 */
class AudioBus
{
public:

    enum Params
    {
        GAIN = 0,
        PAN,

        NUM_PARAMS
    };

    AudioBus();
    AudioBus(const AudioBus&) = delete;
    AudioBus& operator=(const AudioBus&) = delete;
    ~AudioBus();

    AudioParameterPool& getParameters() noexcept { return params; }
    const AudioParameterPool& getParameters() const noexcept { return params; }

    void prepareToPlay();

    void trigger(const Voice::Trigger& voiceTrigger);
    void killAllVoices();

    void processAndMix(float* outL, float* outR, int numFrames);

private:

    friend class AudioBusPool;

    void setEngine(Engine* eng);

    Engine* engine;
    core::AudioBuffer<float> mixBuffer;

    AudioParameterPool params;

    core::List<Voice> voices;   ///< Active voices.
    core::AudioBuffer<float> voiceBuffer;
    core::AudioBuffer<float> busBuffer;
};

//==============================================================================

class AudioBusPool
{
public:
    AudioBusPool() = delete;
    AudioBusPool(Engine& audioEngine, int size = NUM_BUSES);
    AudioBusPool(const AudioBusPool&) = delete;
    AudioBusPool& operator =(const AudioBusPool&) = delete;

    int getNumBuses() const noexcept { return (int)buses.size(); }
    std::vector<AudioBus>& getBuses() noexcept { return buses; }
    const std::vector<AudioBus>& getBuses() const noexcept { return buses; }

    AudioBus& operator[](int index) { return buses.at((size_t)index); }
    const AudioBus& operator[](int index) const { return buses.at((size_t)index); }

    void prepareToPlay();

private:
    Engine& engine;
    std::vector<AudioBus> buses;
};

TW_NAMESPACE_END
