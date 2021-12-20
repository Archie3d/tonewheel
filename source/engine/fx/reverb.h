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
#include "core/audio_buffer.h"
#include "dsp/reverb.h"
#include "fx/pitch_shift.h"

TW_NAMESPACE_BEGIN

namespace fx {

class Reverb : public AudioEffect
{
public:

    enum Params
    {
        DRY = 0,
        WET,
        ROOM_SIZE,
        DAMP,
        WIDTH,
        PITCH,
        FEEDBACK,

        NUM_PARAMS
    };

    const static std::string tag;

    Reverb();

    const std::string& getTag() const override { return Reverb::tag; }

    void prepareToPlay() override;
    void process(const float* inL, const float* inR, float* outL, float* outR, int numFrames) override;
    int getTailLength() const override;

private:

    void update();

    static constexpr int stereoSpread = 23;

    using ReverbL = dsp::Reverb<0>;
    using ReverbR = dsp::Reverb<stereoSpread>;

    ReverbL::Spec reverbLSpec;
    ReverbR::Spec reverbRSpec;

    ReverbL::State reverbLState;
    ReverbR::State reverbRState;

    core::AudioBuffer<float> intermediateBuffer;
    fx::PitchShift pitchShift;
};

} // namespace fx

TW_NAMESPACE_END
