// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"
#include "core/ring_buffer.h"
#include "dsp/envelope.h"
#include "global_engine.h"
#include "audio_bus.h"

TW_NAMESPACE_BEGIN

/**
 * Audio engine.
 *
 * This class represents an audio engine that manages a set of buses
 * and schedules voices playback on those buses. It shares resources
 * like samples and streams with other engine instances via the
 * @ref GlobalEngine singleton.
 * 
 * @see GlobalEngine
 */
class Engine : public GlobalEngine::Client
{
public:

    /**
     * Voice trigger.
     */
    struct Trigger
    {
        int voiceId     { -1 }; ///< Unique voice identifier.
        int sampleId    { -1 }; ///< Sample ID as defined in the sample pool.
        int busNumber   { -1 }; ///< Bus number the voice till be placed to.
        int key         { -1 }; ///< MIDI key (note).
        int rootKey     { -1 }; ///< Root key (note).
        int offset      { -1 }; ///< Sample offset.
        int loopBegin   { -1 }; ///< Loop begin position.
        int loopEnd     { -1 }; ///< Loop end position.
        int loopXfade   { DEFAULT_XFADE_BUFFER_SIZE };  ///< Loop cross-fade length (in samples).
        float gain      { 1.0f };   ///< Voice gain factor.
        float tune      { 1.0f };   ///< Voice tune (aka playback speed).

        dsp::Envelope::Spec envelope{}; ///< Voice envelope.
    };

    /**
     * Voice release.
     */
    struct Release
    {
        int voiceId { -1 };
        float releaseTime { -1.0f };
    };

    /**
     * A functor that can be called on the audio thread
     * but will be allocated and released on a non-audio thread.
     */
    struct Actuator final : public core::Releasable
    {
        using Ptr = std::shared_ptr<Actuator>;
        using Func = std::function<void()>;

        Actuator(const Func& f)
            : func{ f }
        {}

        void exec()
        {
            func();
        }

        Func func;
    };

    //------------------------------------------------------

    Engine();
    virtual ~Engine();

    /**
     * Prepare the engine for the playback.
     */
    void prepareToPlay(float requestedSampleRate, int requestedFrameSize);

    /**
     * Trigger a new voice.
     * This should be called on a non-audio thread.
     */
    int triggerVoice(Trigger trigger);
    void releaseVoice(int voiceId, float releaseTime = -1.0f);

    void triggerActuator(const Actuator::Func& f);

    AudioBusPool& getAudioBusPool() noexcept { return audioBusPool; }
    const AudioBusPool& getAudioBusPool() const noexcept { return audioBusPool; }
    float getSampleRate() const noexcept { return sampleRate; }

    void process()
    {
        processReleases();
        processTriggers();
        processActuators();
    }

private:
    void processTriggers();
    void processReleases();
    void processActuators();
    void clearActuators();


    AudioBusPool audioBusPool;

    float sampleRate;
    int frameSize;

    int voiceIdCounter;
    core::RingBuffer<Trigger, DEFAULT_TRIGGER_BUFFER_SIZE> triggers;
    core::RingBuffer<Release, DEFAULT_RELEASE_BUFFER_SIZE> releases;
    core::RingBuffer<Actuator::Ptr, DEFAULT_ACTUATOR_BUFFER_SUZE> actuators;
};


TW_NAMESPACE_END
