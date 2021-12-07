// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "fx/send.h"
#include "engine.h"
#include "audio_bus.h"

TW_NAMESPACE_BEGIN

namespace fx {

const std::string Send::tag("send");

Send::Send()
    : AudioEffect(NUM_PARAMS)
{
    params[BUS].setName("bus");
    params[BUS].setRange(0.0f, 255.0f);
    params[BUS].setValue(0.0f, true);

    params[GAIN].setName("gain");
    params[GAIN].setRange(0.0f, 1.0f);
    params[GAIN].setValue(1.0f, true);
}

void Send::prepareToPlay()
{
}

void Send::process(const float* inL, const float* inR, float* outL, float* outR, int numFrames)
{
    if (inL != outL)
        ::memcpy(outL, inL, sizeof(float) * numFrames);

    if (inR != outR)
        ::memcpy(outR, inR, sizeof(float) * numFrames);

    auto& audioBusPool{ engine->getAudioBusPool() };
    int bus{ (int)params[BUS].getTargetValue() };

    if (bus >= 0 && bus < audioBusPool.getNumBuses()) {
        auto& sendBuffer{ audioBusPool[bus].getSendBuffer() };

        float* bufL{ sendBuffer.getChannelData(0) };
        float* bufR{ sendBuffer.getChannelData(1) };

        for (int i = 0; i < numFrames; ++i) {
            float gain{ params[GAIN].getNextValue() };
            bufL[i] += inL[i] * gain;
            bufR[i] += inR[i] * gain;
        }
    }
}

} // namespace fx

TW_NAMESPACE_END
