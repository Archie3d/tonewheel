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
#include "audio_stream.h"
#include "core/list.h"
#include "dsp/envelope.h"
#include <atomic>

TW_NAMESPACE_BEGIN

class Engine;

/**
 * This class represents a single playing voice.
 */
class Voice : public core::ListItem<Voice>
{
public:

    struct Trigger
    {
        int voiceId         { -1 };
        AudioStream* stream { nullptr };
        float gain          { 1.0f };
        float tune          { 1.0f };
        int key             { -1 };
        int rootKey         { -1 };

        dsp::Envelope::Spec envelope;
    };

    enum Params
    {
        GAIN = 0,
        PITCH,

        NUM_PARAMS
    };

    Voice();

    bool isOver() const;
    AudioStream* getStream();

    void resetAndReturnToPool();
    void process(float* outL, float* outR, int numFrames);
    void processOne(float& left, float& right);
    void release();
    void releaseWithReleaseTime(float t);

    void trigger(Engine* eng, const Voice::Trigger& trig);

    const Trigger& getTrigger() const noexcept { return voiceTrigger; }

private:

    void reset();

    Engine* engine;
    Trigger voiceTrigger;

    float srAdjust; ///< Sample rate adjustment.
    float speed;    ///< Playback speed.

    float accL[8];
    float accR[8];
    int accIndex;
    float accFrac;

    dsp::Envelope envelope;

    AudioParameterPool params;

    int samplePos;
};

//==============================================================================

class VoicePool
{
public:
    VoicePool(int size = DEFAULT_VOICE_POOL_SIZE);
    ~VoicePool();

    Voice* getVoice();
    void returnToPool(Voice* voice);

    Voice* findVoiceWithTriggerId(int voiceId);

private:
    std::vector<Voice> voices;
    core::List<Voice> idleVoices;
    std::atomic<int> activeVoicesCount;
};

TW_NAMESPACE_END
