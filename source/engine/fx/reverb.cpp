// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "fx/reverb.h"
#include <cassert>

TW_NAMESPACE_BEGIN

namespace fx {

const std::string Reverb::tag("reverb");

namespace defaults {
    constexpr static float dry      = 1.0f;
    constexpr static float wet      = 0.4f;
    constexpr static float roomSize = 0.8f;
    constexpr static float damp     = 0.2f;
    constexpr static float width    = 0.5f;
    constexpr static float pitch    = 1.0f;
    constexpr static float feedback = 0.0f;
} // defaults

Reverb::Reverb()
    : AudioEffect(NUM_PARAMS)
    , intermediateBuffer(MIX_BUFFER_NUM_CHANNELS, MIX_BUFFER_NUM_FRAMES)
    , pitchShift()
{
    params[DRY].setName("dry");
    params[DRY].setValue(defaults::dry, true);

    params[WET].setName("wet");
    params[WET].setValue(defaults::wet, true);

    params[ROOM_SIZE].setName("roomSize");
    params[ROOM_SIZE].setValue(defaults::roomSize, true);

    params[DAMP].setName("damp");
    params[DAMP].setValue(defaults::damp, true);

    params[WIDTH].setName("width");
    params[WIDTH].setValue(defaults::width, true);

    params[PITCH].setName("pitch");
    params[PITCH].setRange(0.0f, 2.0f);
    params[PITCH].setValue(defaults::pitch, true);

    params[FEEDBACK].setName("feedback");
    params[FEEDBACK].setRange(0.0f, 1.0f);
    params[FEEDBACK].setValue(defaults::feedback, true);

    reverbLSpec.roomSize = defaults::roomSize;
    reverbLSpec.damp = defaults::damp;

    reverbRSpec.roomSize = defaults::roomSize;
    reverbRSpec.damp = defaults::damp;
}

void Reverb::prepareToPlay()
{
    ReverbL::update(reverbLSpec);
    ReverbR::update(reverbRSpec);

    ReverbL::reset(reverbLSpec, reverbLState);
    ReverbR::reset(reverbRSpec, reverbRState);

    intermediateBuffer.clear();

    pitchShift.setEngine(engine);
    pitchShift.prepareToPlay();
    auto& psParams{ pitchShift.getParameters() };
    psParams[PitchShift::PITCH].setValue(params[PITCH].getTargetValue(), true);
}

void Reverb::process(const float* inL, const float* inR, float* outL, float* outR, int numFrames)
{
    assert(numFrames <= MIX_BUFFER_NUM_FRAMES);

    update();

    float* tmpL{ intermediateBuffer.getChannelData(0) };
    float* tmpR{ intermediateBuffer.getChannelData(1) };

    const auto pitch{ params[PITCH].getCurrentValue() };
    const auto feedback{ params[FEEDBACK].getCurrentValue() };

    if (feedback > 0.0f && pitch != 1.0f) {
        // Shimmer reverb
        pitchShift.getParameters()[PitchShift::PITCH].setValue(pitch);
        pitchShift.process(tmpL, tmpR, tmpL, tmpR, numFrames);

        for (size_t i = 0; i < numFrames; ++i) {
            tmpL[i] = inL[i] + feedback * tmpL[i];
            tmpR[i] = inR[i] + feedback * tmpR[i];
        }

        ReverbL::process(reverbLSpec, reverbLState, tmpL, tmpL, numFrames);
        ReverbR::process(reverbRSpec, reverbRState, tmpR, tmpR, numFrames);
    } else {
        // Normal reverb
        ReverbL::process(reverbLSpec, reverbLState, inL, tmpL, numFrames);
        ReverbR::process(reverbRSpec, reverbRState, inR, tmpR, numFrames);
    }

    // Dry/wet mixing
    for (size_t i = 0; i < numFrames; ++i)
    {
        // Advance smoothed parameters
        params[PITCH].getNextValue();
        params[FEEDBACK].getNextValue();
        params[ROOM_SIZE].getNextValue();

        const auto width = params[WIDTH].getNextValue();
        const auto dry = params[DRY].getNextValue();
        const auto wet = params[WET].getNextValue();
        const auto wet1 = wet * (width * 0.5f + 0.5f);
        const auto wet2 = wet * (0.5f * (1.0f - width));

        outL[i] = tmpL[i] * wet1 + tmpR[i] * wet2 + inL[i] * dry;
        outR[i] = tmpR[i] * wet1 + tmpL[i] * wet2 + inR[i] * dry;
    }
}

int Reverb::getTailLength() const
{
    return -1;
}

void Reverb::update()
{
    reverbLSpec.roomSize = params[ROOM_SIZE].getTargetValue();
    reverbLSpec.damp = params[DAMP].getTargetValue();

    reverbRSpec.roomSize = params[ROOM_SIZE].getTargetValue();
    reverbRSpec.damp = params[DAMP].getTargetValue();

    ReverbL::update(reverbLSpec);
    ReverbR::update(reverbRSpec);
}

} // namespace fx

TW_NAMESPACE_END
