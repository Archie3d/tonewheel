// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"

TW_NAMESPACE_BEGIN

namespace dsp {

/**
 * ADSR-style envelope with exponential attack and release.
 */
class Envelope final
{
public:

    enum class State
    {
        Off,
        Attack,
        Decay,
        Sustain,
        Release,
        NumStates
    };

    constexpr static float AttackTargetRatio{ 0.3f };
    constexpr static float DecayReleaseTargetRatio{ 0.0001f };

    struct Spec
    {
        float attack  { 0.0f };
        float decay   { 0.0f };
        float sustain { 1.0f }; 
        float release { 1.0f };
        float sampleRate { DEFAULT_SAMPLE_RATE_F };
    };

    Envelope();

    State getState() const noexcept { return currentState; }

    void trigger(const Spec& spec);
    void release();
    void release(float t);

    float getNext();

    float getLevel() const noexcept { return currentLevel; }

private:

    static float calculate (float rate, float targetRatio);

    State currentState { State::Off };
    float currentLevel { 0.0f };

    float attackRate   { 0.0f };
    float attackCoef   { 0.0f };
    float attackBase   { 0.0f };

    float decayRate    { 0.0f };
    float decayCoef    { 0.0f };
    float decayBase    { 0.0f };

    float releaseRate  { 0.0f };
    float releaseCoef  { 0.0f };
    float releaseBase  { 0.0f };

    float sustainLevel { 0.0f };

    float sampleRate { DEFAULT_SAMPLE_RATE_F };
};

} // namespace dsp

TW_NAMESPACE_END
