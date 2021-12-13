// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "voice.h"
#include "global_engine.h"
#include "engine.h"
#include "core/math.h"
#include <cassert>

TW_NAMESPACE_BEGIN

Voice::Voice()
    : params(NUM_PARAMS)
{
    params[GAIN].setName("gain");
    params[GAIN].setRange(0.0f, 16.0f); // Allow +24dB gain
    params[GAIN].setValue(1.0f, true);

    params[PITCH].setName("pitch");
    params[PITCH].setRange(0.0f, 4.0f);
    params[PITCH].setValue(1.0f, true);
}

bool Voice::isOver() const
{
    if (fxTailCountdown > 0)
        return false;

    bool over{ envelope.getState() == dsp::Envelope::State::Off };

    if (!over && voiceTrigger.stream != nullptr)
        return voiceTrigger.stream->isOver();

    return over;
}

AudioStream* Voice::getStream()
{
    return voiceTrigger.stream;
}

void Voice::resetAndReturnToPool()
{
    reset();

    auto* g{ GlobalEngine::getInstance() };
    g->getVoicePool().returnToPool(this);
}

void Voice::process(float* outL, float* outR, int numFrames)
{
    assert(voiceTrigger.stream != nullptr);

    // Process the FX tail only
    if (envelope.getState() == dsp::Envelope::State::Off && fxTailCountdown > 0) {
        int framesThisTime{ std::min(numFrames, fxTailCountdown) };
        memset(outL, 0, sizeof(float) * numFrames);
        memset(outR, 0, sizeof(float) * numFrames);
        voiceTrigger.fxChain->process(outL, outR, outL, outR, numFrames);
        fxTailCountdown -= framesThisTime;

        return;
    }

    int generatedFrames{ 0 };

    bool streamIsActive{ true };

    while (streamIsActive && generatedFrames < numFrames) {
        const float dFrac{ speed * params[PITCH].getNextValue() };
        accFrac += dFrac;

        while (accFrac >= 1.0f) {
            streamIsActive = voiceTrigger.stream->readOne(accL[accIndex], accR[accIndex]);

            accL[accIndex + 4] = accL[accIndex];
            accR[accIndex + 4] = accR[accIndex];
            accIndex = (accIndex + 1) % 4;
            accFrac -= 1.0f;
        }

        outL[generatedFrames] = core::math::lagr(&accL[accIndex], accFrac);
        outR[generatedFrames] = core::math::lagr(&accR[accIndex], accFrac);

        ++generatedFrames;
    }

    if (generatedFrames < numFrames)
    {
        // Incomplete frame - fill the rest with zeroes
        const auto residual = numFrames - generatedFrames;

        ::memset(&outL[generatedFrames], 0, sizeof(float) * residual);
        ::memset(&outR[generatedFrames], 0, sizeof(float) * residual);

        // If the stream has been depleted we release the voice here.
        if (voiceTrigger.stream->isOver())
            release();
    }

    // Apply voice envelope and gain
    for (size_t i = 0; i < numFrames; ++i)
    {
        const auto env = envelope.getNext() * voiceTrigger.gain * params[GAIN].getNextValue();

        outL[i] *= env;
        outR[i] *= env;
    }

    if (envelope.getState() == dsp::Envelope::State::Off) {
        voiceTrigger.stream->release();

        fxTailCountdown = { voiceTrigger.fxChain == nullptr ? 0 : voiceTrigger.fxChain->getTailLength() };
    }

    if (voiceTrigger.fxChain != nullptr) {
        voiceTrigger.fxChain->process(outL, outR, outL, outR, numFrames);
    }

    samplePos += numFrames;
}

/*
void Voice::processOne(float& left, float& right)
{
    assert(voiceTrigger.stream != nullptr);

    const float dFrac = speed * params[PITCH].getNextValue();
    accFrac += dFrac;

    while (accFrac >= 1.0f) {
        voiceTrigger.stream->readOne(accL[accIndex], accR[accIndex]);

        accL[accIndex + 4] = accL[accIndex];
        accR[accIndex + 4] = accR[accIndex];
        accIndex = (accIndex + 1) % 4;
        accFrac -= 1.0f;
    }

    left  = core::math::lagr(&accL[accIndex], accFrac);
    right = core::math::lagr(&accR[accIndex], accFrac);

    ++samplePos;
}
*/

void Voice::release()
{
    envelope.release();
}

void Voice::releaseWithReleaseTime(float t)
{
    envelope.release(t);
}

void Voice::trigger(Engine* eng, const Voice::Trigger& trig)
{
    assert(eng != nullptr);
    engine = eng;
    voiceTrigger = trig;

    ::memset(accL, 0, sizeof (float) * 8);
    ::memset(accR, 0, sizeof (float) * 8);
    accIndex = 0;
    accFrac = 0.0f;

    // Adjust playback sample rate vs stream sample rate
    srAdjust = (float)voiceTrigger.stream->getSampleRate() / engine->getSampleRate();
    speed = voiceTrigger.tune * srAdjust;

    samplePos = 0;

    voiceTrigger.envelope.sampleRate = engine->getSampleRate();
    envelope.trigger(voiceTrigger.envelope);
}

void Voice::reset()
{
    samplePos = 0;
    fxTailCountdown = 0;
    params[GAIN].setValue(1.0f, true);
    params[PITCH].setValue(1.0f, true);

    if (voiceTrigger.fxChain != nullptr) {
        GlobalEngine::getInstance()->releaseObject(std::move(voiceTrigger.fxChain));
        voiceTrigger.fxChain = nullptr;
    }
}

//==============================================================================

VoicePool::VoicePool(int size)
    : voices(size)
{
    assert(size > 0);

    // Add all voices to the idle list
    for (auto& voice : voices)
        idleVoices.append(&voice);

    activeVoicesCount = 0;
}

VoicePool::~VoicePool() = default;

Voice* VoicePool::getVoice()
{
    if (auto* voice{ idleVoices.first() }) {
        idleVoices.remove(voice);
        ++activeVoicesCount;
        return voice;
    }

    return nullptr;
}

void VoicePool::returnToPool(Voice* voice)
{
    assert(voice != nullptr);

    idleVoices.append(voice);
    --activeVoicesCount;
}

TW_NAMESPACE_END
