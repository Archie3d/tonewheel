// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "fx/phase_shift.h"
#include "core/math.h"
#include "engine.h"

TW_NAMESPACE_BEGIN

namespace fx {

const std::string PhaseShift::tag("phase_shift");

PhaseShift::PhaseShift()
    : AudioEffect(NUM_PARAMS)
{
    params[PHASE].setName("phase");
    params[PHASE].setRange(-1.0f, 1.0f);
    params[PHASE].setValue(0.0f, true);
}

void PhaseShift::prepareToPlay()
{
    hilbertSpec.sampleRate = engine->getSampleRate();
    dsp::Hilbert::update(hilbertSpec);
    dsp::Hilbert::reset(hilbertSpec, hilbertStateL);
    dsp::Hilbert::reset(hilbertSpec, hilbertStateR);
}

void PhaseShift::process(const float* inL, const float* inR, float *outL, float* outR, int numFrames)
{
    auto phase{ params[PHASE].getNextValue() * core::math::Constants<float>::pi };
    float cos{ std::cos(phase) };
    float sin{ std::sin(phase) };

    while (params[PHASE].isSmoothing() && numFrames > 0) {
        const auto l{ dsp::Hilbert::tick(hilbertSpec, hilbertStateL, *(inL++)) };
        const auto r{ dsp::Hilbert::tick(hilbertSpec, hilbertStateR, *(inR++)) };

        phase = params[PHASE].getNextValue() * core::math::Constants<float>::pi;
        cos = std::cos(phase);
        sin = std::sin(phase);

        *(outL++) = l.real() * cos - l.imag() * sin;
        *(outR++) = r.real() * cos - r.imag() * sin;

        --numFrames;
    }

    while (numFrames > 0) {
        const auto l{ dsp::Hilbert::tick(hilbertSpec, hilbertStateL, *(inL++)) };
        const auto r{ dsp::Hilbert::tick(hilbertSpec, hilbertStateR, *(inR++)) };

        *(outL++) = l.real() * cos - l.imag() * sin;
        *(outR++) = r.real() * cos - r.imag() * sin;

        --numFrames;
    }
}

} // namespace fx

TW_NAMESPACE_END
