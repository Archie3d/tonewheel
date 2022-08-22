// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2022 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "fx/delay_phase_shift.h"
#include "engine.h"
#include <cmath>

TW_NAMESPACE_BEGIN

namespace fx {

const std::string DelayPhaseShift::tag("delay_phase_shift");

DelayPhaseShift::DelayPhaseShift()
    : AudioEffect(NUM_PARAMS)
    , delayLI()
    , delayLQ()
    , delayRI()
    , delayRQ()
    , hilbertSpec{}
    , hilbertL{}
    , hilbertR{}
    , delayToSampleIndex{ 0.0f }
{
    params[DELAY].setName("delay");
    params[DELAY].setRange(0.0f, maxDelayInSeconds);

    params[MAX_DELAY].setName("max_delay");
    params[MAX_DELAY].setRange(0.0f, maxDelayInSeconds);
    params[MAX_DELAY].setValue(maxDelayInSeconds, true);

    params[FEEDBACK_AMPLITUDE].setName("feedback_amplitude");
    params[FEEDBACK_AMPLITUDE].setValue(0.5f, true);

    params[FEEDBACK_PHASE].setName("feedback_phase");
    params[FEEDBACK_PHASE].setValue(0.0f, true);
}

void DelayPhaseShift::prepareToPlay()
{
    const auto delayLength{ (int)std::ceilf(engine->getSampleRate() * params[MAX_DELAY].getTargetValue()) };
    delayLI.resize(delayLength);
    delayLQ.resize(delayLength);
    delayRI.resize(delayLength);
    delayRQ.resize(delayLength);

    delayToSampleIndex = (float)delayLI.getLength() / params[MAX_DELAY].getTargetValue();

    hilbertSpec.sampleRate = engine->getSampleRate();
    dsp::Hilbert::update(hilbertSpec);
    dsp::Hilbert::reset(hilbertSpec, hilbertL);
    dsp::Hilbert::reset(hilbertSpec, hilbertR);

    delayLI.reset();
    delayLQ.reset();
    delayRI.reset();
    delayRQ.reset();

    calculatePhaseIQ();
}

void DelayPhaseShift::process(const float* inL, const float* inR, float* outL, float* outR, int numFrames)
{
    for (int i = 0; i < numFrames; ++i) {
        if (params[FEEDBACK_AMPLITUDE].isSmoothing() || params[FEEDBACK_PHASE].isSmoothing()) {
            calculatePhaseIQ();
        }

        const float delay{ params[DELAY].getNextValue() * delayToSampleIndex };

        std::complex<float> compLfb(delayLI.read(delay), delayLQ.read(delay));
        std::complex<float> compRfb(delayRI.read(delay), delayRQ.read(delay));

        compLfb = compLfb * phaseShift;
        compRfb = compRfb * phaseShift;

        auto compL{ dsp::Hilbert::tick(hilbertSpec, hilbertL, inL[i]) };
        auto compR{ dsp::Hilbert::tick(hilbertSpec, hilbertR, inR[i]) };

        const float l{ compL.real() + compLfb.real() };
        const float r{ compR.real() + compRfb.real() };
        delayLI.write(l);
        delayLQ.write(compL.imag() + compLfb.imag());
        delayRI.write(r);
        delayRQ.write(compR.imag() + compRfb.imag());

        outL[i] = l;
        outR[i] = r;
    }
}

int DelayPhaseShift::getTailLength() const
{
    return -1;
}

void DelayPhaseShift::calculatePhaseIQ()
{
    float amplitude{ params[FEEDBACK_AMPLITUDE].getNextValue() };
    float phase{ params[FEEDBACK_PHASE].getNextValue() };

    phaseShift = std::complex<float>(amplitude * cos(phase), amplitude * sin(phase));
}

} // namespace fx

TW_NAMESPACE_END
