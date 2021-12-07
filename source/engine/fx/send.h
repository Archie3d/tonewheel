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
#include "audio_bus.h"
#include <string>

TW_NAMESPACE_BEGIN

namespace fx {

class Send : public AudioEffect
{
public:

    enum Params
    {
        BUS = 0,
        GAIN,

        NUM_PARAMS
    };

    const static std::string tag;

    Send();

    void prepareToPlay() override;
    void process(const float* inL, const float* inR, float* outL, float* outR, int numFrames) override;
    int getTailLength() const override { return 0; }

private:

};

} // namespace fx

TW_NAMESPACE_END
