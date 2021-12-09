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
#include "sample.h"
#include "midi.h"
#include <map>
#include <mutex>

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

        AudioEffectChain::Ptr fxChain{};
    };

    /**
     * Voice release.
     */
    struct Release
    {
        int voiceId { -1 };
        float releaseTime { -1.0f };
    };

    struct TransportInfo
    {
        double bpm{ 120.0 };
        double time{ 0.0 };
        double ppqPosition{ 0.0 };
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

    /**
     * Construct the engine with a given number of buses.
     */
    Engine(int numBuses = NUM_BUSES);
    virtual ~Engine();

    /**
     * Reset the engine.
     *
     * This will remove all the effects on engine's buses.
     *
     * @note This must not be called on the audio thread.
     */
    void reset();

    /**
     * Prepare the engine for the playback.
     */
    void prepareToPlay(float requestedSampleRate, int requestedFrameSize);

    /**
     * Prepare to play using previously set sample rate and frame size parameters.
     */
    void prepareToPlay();

    /**
     * Trigger a new voice.
     * This should be called on a non-audio thread.
     */
    int triggerVoice(Trigger trigger);

    /**
     * Release a voice.
     * Negative release time will cause the voice to release
     * with it's envelope release time, positive value will override the
     * envelope's release.
     */
    void releaseVoice(int voiceId, float releaseTime = -1.0f);

    /**
     * Trigger an actuator to be executed on an audio thread.
     *
     * @note This call is asynchronous and it does not guarantee the
     *       actuator's function to be executed, for example when the
     *       audio thread is not running or the queue is full.
     */
    void triggerActuator(const Actuator::Func& f);

    void setNonRealtime(bool nonRT) noexcept { nonRealTime = nonRT; }
    bool isNonRealtime() const noexcept { return nonRealTime; }

    void setTransportInfo(const TransportInfo& info) { transportInfo = info; }
    const TransportInfo& getTransportInfo() const noexcept { return transportInfo; }

    AudioBusPool& getAudioBusPool() noexcept { return audioBusPool; }
    const AudioBusPool& getAudioBusPool() const noexcept { return audioBusPool; }
    float getSampleRate() const noexcept { return sampleRate; }

    MidiKeyboardState& getMidiKeyboardState() noexcept { return midiKeyboardState; }
    const MidiKeyboardState& getMidiKeyboardState() const noexcept { return midiKeyboardState; }

    /**
     * Process all pending events on the audio thread.
     *
     * This method will process all the triggers, releases,
     * and the actuators. It does not process any audio though.
     */
    void processAudioEvents();

    /**
     * Add a sample to the engine.
     *
     * This method will add a sample to the global pool if it's not already
     * there and the register the sample with the current engine using the
     * ID number provided.
     *
     * @note This method is thread-safe however it will lock while the
     *       sample is being added.
     */
    void addSample(int id, const std::string& filePath, int startPos = 0, int stopPos = 0);

    /**
     * Returns a sample by its ID.
     *
     * @note This method is thread-safe however it will lock.
     */
    Sample::Ptr getSampleById(int id);

private:
    void processTriggers();
    void processReleases();
    void processActuators();
    void clearActuators();

    AudioBusPool audioBusPool;

    std::mutex mutex;

    /// Mapping IDs to samples
    std::map<int, Sample::Ptr> idToSampleMap;

    float sampleRate;
    int frameSize;

    std::atomic<bool> nonRealTime;

    TransportInfo transportInfo;

    MidiKeyboardState midiKeyboardState;

    int voiceIdCounter;
    core::RingBuffer<Trigger, DEFAULT_TRIGGER_BUFFER_SIZE> triggers;
    core::RingBuffer<Release, DEFAULT_RELEASE_BUFFER_SIZE> releases;
    core::RingBuffer<Actuator::Ptr, DEFAULT_ACTUATOR_BUFFER_SUZE> actuators;
};


TW_NAMESPACE_END
