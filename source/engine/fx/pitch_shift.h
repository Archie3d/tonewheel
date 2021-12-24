// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include "audio_effect.h"
#include "dsp/delay_line.h"
#include "dsp/filters.h"
#include "dsp/hilbert.h"

TW_NAMESPACE_BEGIN

namespace fx {

/**
 * Pitch-shift effect.
 */
class PitchShift : public AudioEffect
{
public:

    enum Params
    {
        PITCH = 0,

        NUM_PARAMS
    };

    const static std::string tag;

    PitchShift();

    const std::string& getTag() const override { return PitchShift::tag; }

    void prepareToPlay() override;
    void process(const float* inL, const float* inR, float *outL, float* outR, int numFrames) override;

private:

    void updateLowPassFilter();

    void updatePhaseA();
    void updatePhaseB();

    dsp::BiquadFilter::Spec lowPassSpec;
    dsp::BiquadFilter::State lowPassStateL;
    dsp::BiquadFilter::State lowPassStateR;

    dsp::DelayLine delayL;
    dsp::DelayLine delayR;

    dsp::DCBlockFilter::Spec dcBlockSpec;
    dsp::DCBlockFilter::State dcBlockStateL;
    dsp::DCBlockFilter::State dcBlockStateR;

    dsp::Hilbert::Spec hilbertSpec;
    dsp::Hilbert::State hilbertStateLA;
    dsp::Hilbert::State hilbertStateRA;
    dsp::Hilbert::State hilbertStateLB;
    dsp::Hilbert::State hilbertStateRB;

    float phaseA;
    float phaseB;
    float sinA, cosA;
    float sinB, cosB;

    float dA;
    float dB;
    float w;
};

} // namespace fx

TW_NAMESPACE_END
