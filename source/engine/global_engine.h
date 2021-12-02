// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"
#include "core/list.h"
#include "core/release_pool.h"
#include "core/worker.h"
#include <memory>
#include <array>

TW_NAMESPACE_BEGIN

class VoicePool;
class SamplePool;
class AudioStreamPool;

/**
 * Engine global singleton.
 */
class GlobalEngine final : public core::Worker::Job
{
public:

    /**
     * Global engine registered clients.
     * These are normally the instances of the engine that get registered
     * with the global singleton so that they could share common resources.
     * 
     * @note A client must be created on the main thread.
     */
    class Client : public core::ListItem<Client>
    {
    public:
        Client();
        virtual ~Client();

        GlobalEngine* getGlobalEngine() noexcept { return globalEngine; }

    private:
        GlobalEngine* globalEngine{ nullptr };

        friend class GlobalEngine;
    };

    //------------------------------------------------------

    static GlobalEngine* getInstance();
    static void destroy();

    GlobalEngine(const GlobalEngine&) = delete;
    GlobalEngine operator=(const GlobalEngine&) = delete;

    /**
     * Register a new client.
     * 
     * @note This method must be called on the main thread, it is not thread-safe.
     */
    void addClient(Client* const client);

    /**
     * Remove a client.
     * 
     * @note This method must be called on the main thread, it is not thread-safe.
     */
    void removeClient(Client* const client);

    ~GlobalEngine();

    VoicePool& getVoicePool();
    SamplePool& getSamplePool();
    AudioStreamPool& getAudioStreamPool();

    core::Worker& getStreamWorker() noexcept;

    void releaseObject(core::Releasable::Ptr&& ptr);

    // Worker::Job
    void run() override;

private:

    GlobalEngine();

    static std::unique_ptr<GlobalEngine> instance;
    core::List<Client> clients;

    core::ReleasePool<DEFAULT_RELEASE_POOL_SIZE> releasePool;

    std::unique_ptr<VoicePool> voicePool;
    std::unique_ptr<SamplePool> samplePool;
    std::unique_ptr<AudioStreamPool> audioStreamPool;

    std::array<core::Worker, NUM_STREAM_WORKERS> streamWorkers;
    std::atomic<int> nextWorkerIndex{ 0 };

    core::Worker backgroundWorker;
};

TW_NAMESPACE_END
