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
#include "dsp/hilbert.h"

TW_NAMESPACE_BEGIN

namespace fx {

class FrequencyShift final : public AudioEffect
{
public:

    enum Params
    {
        FREQUENCY = 0,

        NUM_PARAMS
    };

    const static std::string tag;

    FrequencyShift();

    const std::string& getTag() const override { return tag; }

    void prepareToPlay() override;
    void process(const float* inL, const float* inR, float* outL, float* outR, int numFrames) override;

private:

    dsp::Hilbert::Spec hilbertSpec;
    dsp::Hilbert::State hilbertStateL;
    dsp::Hilbert::State hilbertStateR;

    float phase{};
};

} // namespace fx

TW_NAMESPACE_END
