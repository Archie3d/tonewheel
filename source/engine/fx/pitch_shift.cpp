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

        outL[i] = 0.5f * (wa * delayL.read(dA) + wb * delayL.read(dB));
        outR[i] = 0.5f * (wa * delayR.read(dA) + wb * delayR.read(dB));

        dA += p;
        dB += p;

        if (dA < 0.0f)
            dA += delayLength;
        else if (dA >= delayLength)
            dA -= delayLength;

        if (dB < 0.0f)
            dB += delayLength;
        else if (dB >= delayLength)
            dB -= delayLength;
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

} // namespace fx

TW_NAMESPACE_END
