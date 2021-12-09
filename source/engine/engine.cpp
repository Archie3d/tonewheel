// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "engine.h"
#include "sample.h"

TW_NAMESPACE_BEGIN

Engine::Engine(int numBuses)
    : GlobalEngine::Client()
    , audioBusPool(*this, numBuses)
    , sampleRate{ DEFAULT_SAMPLE_RATE_F }
    , nonRealTime{ false }
    , transportInfo{}
    , midiKeyboardState{}
    , voiceIdCounter{ 0 }
{
}

Engine::~Engine() = default;

void Engine::reset()
{
    audioBusPool.clearFxChain();
    midiKeyboardState.reset();
}

void Engine::prepareToPlay(float requestedSampleRate, int requestedFrameSize)
{
    sampleRate = requestedSampleRate;
    frameSize = requestedFrameSize;

    audioBusPool.prepareToPlay();
}

void Engine::prepareToPlay()
{
    audioBusPool.prepareToPlay();
}

int Engine::triggerVoice(Trigger trigger)
{
    const int id{ voiceIdCounter++ };
    trigger.voiceId = id;

    if (trigger.fxChain != nullptr) {
        trigger.fxChain->setEngine(this);
        trigger.fxChain->prepareToPlay();
    }

    triggers.send(trigger);

    return id;
}

void Engine::releaseVoice(int voiceId, float releaseTime)
{
    Release rel{};
    rel.voiceId = voiceId;
    rel.releaseTime = releaseTime;

    releases.send(rel);
}

void Engine::triggerActuator(const Actuator::Func& f)
{
    assert(f);

    auto actuator{ std::make_shared<Actuator>(f) };
    actuators.send(std::move(actuator));
}

void Engine::processAudioEvents()
{
    processReleases();
    processTriggers();
    processActuators();
}

void Engine::addSample(int id, const std::string& filePath, int startPos, int stopPos)
{
    auto& samplePool{ GlobalEngine::getInstance()->getSamplePool() };
    auto sample{ samplePool.addSample(filePath, startPos, stopPos) };

    if (sample != nullptr) {
        std::lock_guard<decltype(mutex)> lock(mutex);
        idToSampleMap[id] = sample;
    } else {
        // @todo Report unable to load sample from filePath
    }
}

Sample::Ptr Engine::getSampleById(int id)
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    auto it{ idToSampleMap.find(id) };

    if (it == idToSampleMap.end())
        return nullptr;

    return it->second;
}

void Engine::processTriggers()
{
    auto* g{ GlobalEngine::getInstance() };
    auto& samplePool{ g->getSamplePool() };
    auto& streamPool{ g->getAudioStreamPool() };

    Trigger trig;

    while (triggers.receive(trig)) {
        if (trig.busNumber < 0 || trig.busNumber >= audioBusPool.getNumBuses())
            continue;

        if (auto sample { getSampleById(trig.sampleId) })
        {
            if (sample->isPreloaded()) {
                if (auto* stream{ streamPool.getStream() }) {
                    stream->trigger(sample, &g->getStreamWorker());

                    Voice::Trigger voiceTrigger;
                    voiceTrigger.voiceId = trig.voiceId;
                    voiceTrigger.stream = stream;
                    voiceTrigger.gain = trig.gain;
                    voiceTrigger.tune = trig.tune;
                    voiceTrigger.envelope = trig.envelope;
                    voiceTrigger.fxChain = trig.fxChain;
                    stream->setOffset(trig.offset);
                    stream->setLoop(trig.loopBegin, trig.loopEnd + sample->getStopPosition(), trig.loopXfade);

                    audioBusPool[trig.busNumber].trigger(voiceTrigger);
                } else {
                    // No more streams available
                }
            }
        } else {
            // Unable to find sample trig.sampleId
        }
    }
}

void Engine::processReleases()
{
    auto& voicePool{ GlobalEngine::getInstance()->getVoicePool() };

    Release rel;

    while (releases.receive(rel)) {
        if (auto* voice{ voicePool.findVoiceWithTriggerId(rel.voiceId) }) {
            if (rel.releaseTime < 0.0f)
                voice->release();
            else
                voice->releaseWithReleaseTime(rel.releaseTime);
        }
    }
}

void Engine::processActuators()
{
    auto* g{ GlobalEngine::getInstance() };

    Actuator::Ptr actuator{};

    while (actuators.receive(actuator)) {
        if (actuator != nullptr) {
            actuator->exec();
            g->releaseObject(std::move(actuator));
        }
    }
}

void Engine::clearActuators()
{
    auto* g{ GlobalEngine::getInstance() };

    Actuator::Ptr actuator{};

    while (actuators.receive(actuator)) {
        if (actuator != nullptr)
            g->releaseObject(std::move(actuator));
    }
}

TW_NAMESPACE_END
