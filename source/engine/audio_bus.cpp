// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "audio_bus.h"
#include "engine.h"

TW_NAMESPACE_BEGIN

AudioBus::AudioBus()
    : engine{ nullptr }
    , mixBuffer(MIX_BUFFER_NUM_CHANNELS, MIX_BUFFER_NUM_FRAMES)
    , params(NUM_PARAMS)
    , voiceBuffer(MIX_BUFFER_NUM_CHANNELS, MIX_BUFFER_NUM_FRAMES)
    , busBuffer(MIX_BUFFER_NUM_CHANNELS, MIX_BUFFER_NUM_FRAMES)
{
    params[GAIN].setName("gain");
    params[GAIN].setRange(0.0f, 1.0f);
    params[GAIN].setValue(1.0f, true);

    params[PAN].setName("pan");
    params[PAN].setRange(-1.0f, 1.0f);
    params[PAN].setValue(0.0f, true);
}

AudioBus::~AudioBus() = default;

void AudioBus::prepareToPlay()
{
    assert(engine != nullptr);

    // @todo prepare all the FXs on the  bus
}

void AudioBus::trigger(const Voice::Trigger& voiceTrigger)
{
    auto& voicePool{ GlobalEngine::getInstance()->getVoicePool() };

    if (auto* voice{ voicePool.getVoice() }) {
        voice->trigger(engine, voiceTrigger);
        voices.append(voice);
        assert(!voices.isEmpty());
    }
}

void AudioBus::killAllVoices()
{
    auto* voice{ voices.first() };

    while (voice != nullptr) {
        auto* nextVoice{ voices.removeAndReturnNext(voice) };

        if (auto* stream{ voice->getStream() })
            stream->returnToPool();
    
        voice->resetAndReturnToPool();

        voice = nextVoice;
    }
}

void AudioBus::processAndMix(float* outL, float* outR, int numFrames)
{
    assert(voiceBuffer.getNumFrames() >= numFrames);
    assert(busBuffer.getNumFrames() >= numFrames);

    auto* voice{ voices.first() };

    // @todo Check the FX-chain is empty
    if (voice == nullptr)
        return;

    busBuffer.clear();

    // Process all the active voices
    while (voice != nullptr) {
        voice->process(voiceBuffer.getChannelData(0), voiceBuffer.getChannelData(1), numFrames);
        busBuffer.mix(voiceBuffer);

        if (voice->isOver()) {
            auto* nextVoice{ voices.removeAndReturnNext(voice) };

            if (auto* stream{ voice->getStream() })
                stream->returnToPool();

            voice->resetAndReturnToPool();
            voice = nextVoice;
        } else {
            voice = voice->next();
        }
    }

    // @todo Apply bus effects

    float gain{ params[GAIN].getTargetValue() };
    float pan{ params[PAN].getTargetValue() };
    float panL{ pan <= 0.0f ? 1.0f : 1.0f - pan };
    float panR{ pan >= 0.0f ? 1.0f : 1.0f + pan };

    int i{ 0 };
    const float* bufL{ busBuffer.getChannelData(0) };
    const float* bufR{ busBuffer.getChannelData(0) };

    while ((params[GAIN].isSmoothing() || params[PAN].isSmoothing()) && i < numFrames) {
        gain = params[GAIN].getNextValue();
        pan = params[PAN].getNextValue();

        panL = pan <= 0.0f ? 1.0f : 1.0f - pan;
        panR = pan >= 0.0f ? 1.0f : 1.0f + pan;

        outL[i] += bufL[i] * gain * panL;
        outR[i] += bufR[i] * gain * panR;

        ++i;
    }

    panL *= gain;
    panR *= gain;

    for (; i < numFrames; ++i) {
        outL[i] += bufL[i] * panL;
        outR[i] += bufR[i] * panR;
    }
}

void AudioBus::setEngine(Engine* eng)
{
    assert(eng != nullptr);
    engine = eng;
}

//==============================================================================

AudioBusPool::AudioBusPool(Engine& audioEngine, int size)
    : engine{ audioEngine }
    , buses(size)
{
    for (auto& bus : buses)
        bus.setEngine(&engine);
}

void AudioBusPool::prepareToPlay()
{
    for (auto& bus : buses)
        bus.prepareToPlay();
}

TW_NAMESPACE_END
