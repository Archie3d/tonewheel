// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "global_engine.h"
#include "voice.h"
#include "sample.h"
#include "audio_stream.h"

#include <cassert>

TW_NAMESPACE_BEGIN

GlobalEngine::Client::Client()
{
    GlobalEngine::getInstance()->addClient(this);
}

GlobalEngine::Client::~Client()
{
    if (globalEngine != nullptr)
        globalEngine->removeClient(this);
}

//==============================================================================

std::unique_ptr<GlobalEngine> GlobalEngine::instance{ nullptr };

GlobalEngine::GlobalEngine()
    : voicePool{ std::make_unique<VoicePool>() }
    , samplePool{ std::make_unique<SamplePool>() }
    , audioStreamPool{ std::make_unique<AudioStreamPool>() }
{
    backgroundWorker.start();

    for (auto& worker : streamWorkers)
        worker.start();
}

GlobalEngine::~GlobalEngine() = default;

GlobalEngine* GlobalEngine::getInstance()
{
    if (instance == nullptr) {
        // GlobalEngine constructor is private, so we have to allocate it directly.
        instance = std::unique_ptr<GlobalEngine>(new GlobalEngine);
    }

    return instance.get();
}

void GlobalEngine::destroy()
{
    instance.reset();
}

void GlobalEngine::addClient(Client* const client)
{
    assert(client != nullptr);
    assert(!clients.contains(client));

    clients.append(client);
    client->globalEngine = this;
}

void GlobalEngine::removeClient(Client* const client)
{
    assert(client != nullptr);
    assert(clients.contains(client));

    clients.remove(client);
    client->globalEngine = nullptr;
}

VoicePool& GlobalEngine::getVoicePool()
{
    return *voicePool;
}

SamplePool& GlobalEngine::getSamplePool()
{
    return *samplePool;
}

AudioStreamPool& GlobalEngine::getAudioStreamPool()
{
    return *audioStreamPool;
}

core::Worker& GlobalEngine::getStreamWorker() noexcept
{
    auto& worker{ streamWorkers[(size_t)nextWorkerIndex] };
    nextWorkerIndex = (nextWorkerIndex + 1) % (int)streamWorkers.size();
    return worker;
}

void GlobalEngine::releaseObject(core::Releasable::Ptr&& ptr)
{
    releasePool.push(std::move(ptr));

    if (releasePool.isHalfFull())
        backgroundWorker.addJob(this);
}

void GlobalEngine::run()
{
    releasePool.release();
}

TW_NAMESPACE_END
