// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "fx/frequency_shift.h"
#include "core/math.h"
#include "engine.h"

TW_NAMESPACE_BEGIN

namespace fx {

const std::string FrequencyShift::tag("frequency_shift");

FrequencyShift::FrequencyShift()
    : AudioEffect(NUM_PARAMS)
{
    params[FREQUENCY].setName("frequency");
    params[FREQUENCY].setRange(-22000.0f, 22000.0f);
    params[FREQUENCY].setValue(0.0f, true);
}

void FrequencyShift::prepareToPlay()
{
    hilbertSpec.sampleRate = engine->getSampleRate();
    dsp::Hilbert::update(hilbertSpec);
    dsp::Hilbert::reset(hilbertSpec, hilbertStateL);
    dsp::Hilbert::reset(hilbertSpec, hilbertStateR);

    phase = 0.0f;
}

void FrequencyShift::process(const float* inL, const float* inR, float *outL, float* outR, int numFrames)
{
    const float k{ core::math::Constants<float>::twoPi / engine->getSampleRate() };
    float f{ params[FREQUENCY].getCurrentValue() };
    float df{ f * k };

    while (params[FREQUENCY].isSmoothing() && numFrames > 0) {
        f = params[FREQUENCY].getNextValue();
        df = f * k;

        const auto l{ dsp::Hilbert::tick(hilbertSpec, hilbertStateL, *(inL++)) };
        const auto r{ dsp::Hilbert::tick(hilbertSpec, hilbertStateR, *(inR++)) };

        const auto c { std::cos(phase) };
        const auto s { std::sin(phase) };
        phase += df;

        *(outL++) = l.real() * c + l.imag() * s;
        *(outR++) = r.real() * c + r.imag() * s;

        --numFrames;
    }

    while (numFrames > 0)
    {
        const auto l{ dsp::Hilbert::tick(hilbertSpec, hilbertStateL, *(inL++)) };
        const auto r{ dsp::Hilbert::tick(hilbertSpec, hilbertStateR, *(inR++)) };

        const auto c { std::cos(phase) };
        const auto s { std::sin(phase) };
        phase += df;

        *(outL++) = l.real() * c + l.imag() * s;
        *(outR++) = r.real() * c + r.imag() * s;

        --numFrames;
    }
}

} // namespace fx

TW_NAMESPACE_END
