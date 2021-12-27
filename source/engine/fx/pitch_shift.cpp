// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "fx/pitch_shift.h"
#include "core/math.h"
#include "engine.h"

TW_NAMESPACE_BEGIN

namespace fx {

namespace design {

constexpr float lowPassOpenFreq{ 22000.0f };
constexpr float bufferLengthMs{ 50.0f };

} // namespace design

const std::string PitchShift::tag("pitch_shift");


PitchShift::PitchShift()
    : AudioEffect(NUM_PARAMS)
{
    params[PITCH].setName("pitch");
    params[PITCH].setRange(0.0f, 4.0f);
    params[PITCH].setValue(1.0f, true);
}

void PitchShift::prepareToPlay()
{
    const float sampleRate{ engine->getSampleRate() };

    lowPassSpec.sampleRate = sampleRate;
    lowPassSpec.type = dsp::BiquadFilter::Type::LowPass;
    lowPassSpec.q = 0.7071f;
    lowPassSpec.freq = design::lowPassOpenFreq;

    updateLowPassFilter();

    dsp::BiquadFilter::reset(lowPassSpec, lowPassStateL);
    dsp::BiquadFilter::reset(lowPassSpec, lowPassStateR);

    dcBlockSpec.alpha = 0.995f;
    dsp::DCBlockFilter::reset(dcBlockSpec, dcBlockStateL);
    dsp::DCBlockFilter::reset(dcBlockSpec, dcBlockStateR);

    float nq{ 0.5f * sampleRate};

    int numSamples{ static_cast<int>(sampleRate * design::bufferLengthMs / 1000.0f) };
    delayL.resize(numSamples);
    delayR.resize(numSamples);

    hilbertSpec.sampleRate = sampleRate;
    dsp::Hilbert::update(hilbertSpec);
    dsp::Hilbert::reset(hilbertSpec, hilbertStateLA);
    dsp::Hilbert::reset(hilbertSpec, hilbertStateRA);
    dsp::Hilbert::reset(hilbertSpec, hilbertStateLB);
    dsp::Hilbert::reset(hilbertSpec, hilbertStateRB);

    phaseA = 0.0f;
    phaseB = 0.0f;
    sinA = 0.0f;
    cosA = 1.0f;
    sinB = 0.0f;
    cosB = 1.0f;

    dA = 0.0f;
    dB = 0.5f * numSamples;
    w = core::math::Constants<float>::pi / (float)numSamples;

    delayL.reset();
    delayR.reset();
}

void PitchShift::process(const float* inL, const float* inR, float *outL, float* outR, int numFrames)
{
    // Update the low-pass filter once per processing block only.
    if (params[PITCH].isSmoothing())
        updateLowPassFilter();

    const float delayLength{ (float)delayL.getLength() };

    for (int i = 0; i < numFrames; ++i) {
        const float p{ 1.0f - params[PITCH].getNextValue() };

        float l{ dsp::BiquadFilter::tick(lowPassSpec, lowPassStateL, dsp::DCBlockFilter::tick(dcBlockSpec, dcBlockStateL, inL[i])) };
        float r{ dsp::BiquadFilter::tick(lowPassSpec, lowPassStateR, dsp::DCBlockFilter::tick(dcBlockSpec, dcBlockStateR, inR[i])) };

        delayL.write(l);
        delayR.write(r);

        const float wa{ std::sin(w * dA) };
        const float wb{ std::sin(w * dB) };

        auto hla = dsp::Hilbert::tick(hilbertSpec, hilbertStateLA, delayL.read(dA));
        auto hra = dsp::Hilbert::tick(hilbertSpec, hilbertStateRA, delayR.read(dA));
        auto hlb = dsp::Hilbert::tick(hilbertSpec, hilbertStateLB, delayL.read(dB));
        auto hrb = dsp::Hilbert::tick(hilbertSpec, hilbertStateRB, delayR.read(dB));

        float la = hla.real() * cosA - hla.imag() * sinA;
        float ra = hra.real() * cosA - hra.imag() * sinA;
        float lb = hlb.real() * cosB - hlb.imag() * sinB;
        float rb = hrb.real() * cosB - hrb.imag() * sinB;

        outL[i] = wa * la + wb * lb;
        outR[i] = wa * ra + wb * rb;

        dA += p;
        dB += p;

        if (dA < 0.0f) {
            dA += delayLength;
            updatePhaseA();
        } else if (dA >= delayLength) {
            dA -= delayLength;
            updatePhaseA();
        }

        if (dB < 0.0f) {
            dB += delayLength;
            updatePhaseB();
        } else if (dB >= delayLength) {
            dB -= delayLength;
            updatePhaseB();
        }
    }
}

void PitchShift::updateLowPassFilter()
{
    const auto p = params[PITCH].getCurrentValue();

    if (p > 0.0f) {
        lowPassSpec.freq = p > 1.0f ? (0.5f * lowPassSpec.sampleRate / p) : design::lowPassOpenFreq;
        dsp::BiquadFilter::update(lowPassSpec);
    }
}

void PitchShift::updatePhaseA()
{
    phaseA = fmod(phaseB + 0.3169f, 1.0f);
    const float a{ core::math::Constants<float>::twoPi * phaseA };
    cosA = cos(a);
    sinA = sin(a);
}

void PitchShift::updatePhaseB()
{
    phaseB = fmod(phaseA + 0.1931f, 1.0f);
    const float a{ core::math::Constants<float>::twoPi * phaseB };
    cosB = cos(a);
    sinB = sin(a);
}

} // namespace fx

TW_NAMESPACE_END
