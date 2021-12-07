// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "fx/delay.h"
#include "engine.h"
#include <cmath>

TW_NAMESPACE_BEGIN

namespace fx {

const std::string Delay::tag("delay");

Delay::Delay()
    : AudioEffect(NUM_PARAMS)
    , delayL()
    , delayR()
    , delayToSampleIndex{ 0.0f }
{
    params[DRY].setName("dry");
    params[DRY].setValue(1.0f, true);

    params[WET].setName("wet");
    params[WET].setValue(0.5f, true);

    params[DELAY].setName("delay");
    params[DELAY].setRange(0.0f, maxDelayInSeconds);

    params[MAX_DELAY].setName("max_delay");
    params[MAX_DELAY].setRange(0.0f, maxDelayInSeconds);
    params[MAX_DELAY].setValue(maxDelayInSeconds, true);

    params[FEEDBACK].setName("feedback");
    params[FEEDBACK].setValue(0.5f, true);
}

void Delay::prepareToPlay()
{
    const auto delayLength{ (int)std::ceilf(engine->getSampleRate() * params[MAX_DELAY].getTargetValue()) };
    delayL.resize(delayLength);
    delayR.resize(delayLength);

    delayToSampleIndex = (float)delayL.getLength() / params[MAX_DELAY].getTargetValue();
}

void Delay::process(const float* inL, const float* inR, float* outL, float* outR, int numFrames)
{
    for (int i = 0; i < numFrames; ++i) {
        const float dry{ params[DRY].getNextValue() };
        const float wet{ params[WET].getNextValue() };
        const float delay{ params[DELAY].getNextValue() * delayToSampleIndex };
        const float fb{ params[FEEDBACK].getNextValue() };
        const float l{ delayL.read(delay) };
        const float r{ delayR.read(delay) };

        delayL.write(l * fb + inL[i]);
        delayR.write(r * fb + inR[i]);
        outL[i] = l * wet + inL[i] * dry;
        outR[i] = r * wet + inR[i] * dry;
    }
}

int Delay::getTailLength() const
{
    // This actually depends on feedback parameter. With feedback == 1
    // we'll have an infinite delay
    //return (int)(params[MAX_DELAY].getTargetValue() * delayToSampleIndex);
    return -1;
}

} // namespace fx

TW_NAMESPACE_END
