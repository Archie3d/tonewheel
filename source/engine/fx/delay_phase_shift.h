// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2022 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "../globals.h"
#include "audio_effect.h"
#include "dsp/delay_line.h"
#include "dsp/hilbert.h"
#include <string>

TW_NAMESPACE_BEGIN

namespace fx {

class DelayPhaseShift final : public AudioEffect
{
public:

    enum Params
    {
        DELAY,
        MAX_DELAY,
        FEEDBACK_AMPLITUDE,
        FEEDBACK_PHASE,

        NUM_PARAMS
    };

    const static std::string tag;

    DelayPhaseShift();

    const std::string& getTag() const override { return DelayPhaseShift::tag; }

    void prepareToPlay() override;
    void process(const float* inL, const float* inR, float* outL, float* outR, int numFrames) override;
    int getTailLength() const override;

private:
    constexpr static float maxDelayInSeconds{ 10.0f };

    void calculatePhaseIQ();

    dsp::DelayLine delayLI;
    dsp::DelayLine delayLQ;
    dsp::DelayLine delayRI;
    dsp::DelayLine delayRQ;

    dsp::Hilbert::Spec hilbertSpec;
    dsp::Hilbert::State hilbertL;
    dsp::Hilbert::State hilbertR;

    float delayToSampleIndex;
    std::complex<float> phaseShift{};
};

} // namespace fx

TW_NAMESPACE_END
