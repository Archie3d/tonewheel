// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "../globals.h"
#include "audio_effect.h"
#include "dsp/delay_line.h"
#include <string>

TW_NAMESPACE_BEGIN

namespace fx {

class Delay : public AudioEffect
{
public:

    enum Params
    {
        DRY = 0,
        WET,
        DELAY,
        MAX_DELAY,
        FEEDBACK,

        NUM_PARAMS
    };

    const static std::string tag;

    Delay();

    const std::string& getTag() const override { return Delay::tag; }

    void prepareToPlay() override;
    void process(const float* inL, const float* inR, float* outL, float* outR, int numFrames) override;
    int getTailLength() const override;

private:

    constexpr static float maxDelayInSeconds{ 10.0f };

    dsp::DelayLine delayL;
    dsp::DelayLine delayR;

    float delayToSampleIndex;
};

} // namespace fx

TW_NAMESPACE_END
