// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "engine.h"
#include "fx/filters.h"

TW_NAMESPACE_BEGIN

namespace fx {

constexpr float defaultFrequency = 10000.0f;
constexpr float defaultQ = 0.7071f;

static void updateFilter(dsp::BiquadFilter::Spec& spec, float f, float q)
{
    spec.freq = f;
    spec.q = q;
    spec.dbGain = 1.0f;

    dsp::BiquadFilter::update(spec);
}

BiquadFilter::BiquadFilter(dsp::BiquadFilter::Type filterType)
    : AudioEffect(NUM_PARAMS)
{
    spec.type = filterType;

    params[FREQUENCY].setName("frequency");
    params[FREQUENCY].setValue(defaultFrequency, true);
    params[FREQUENCY].setRange(1.0f, 20000.0f);

    params[Q_FACTOR].setName("q");
    params[Q_FACTOR].setValue(defaultQ, true);
    params[Q_FACTOR].setRange(0.0f, 100.0f);
}

void BiquadFilter::prepareToPlay()
{
    spec.sampleRate = engine->getSampleRate();

    updateFilter(spec, params[FREQUENCY].getCurrentValue(), params[Q_FACTOR].getCurrentValue());

    dsp::BiquadFilter::reset(spec, filterL);
    dsp::BiquadFilter::reset(spec, filterR);
}

void BiquadFilter::process(const float* inL, const float* inR, float* outL, float* outR, int numFrames)
{
    float f = params[FREQUENCY].getNextValue();
    float q = params[Q_FACTOR].getNextValue();

    while ((params[FREQUENCY].isSmoothing() ||
            params[Q_FACTOR].isSmoothing()) && numFrames > 0) {

        f = params[FREQUENCY].getNextValue();
        q = params[Q_FACTOR].getNextValue();

        updateFilter(spec, f, q);

        *(outL++) = dsp::BiquadFilter::tick(spec, filterL, *(inL++));
        *(outR++) = dsp::BiquadFilter::tick(spec, filterR, *(inR++));

        --numFrames;
    }

    if (numFrames > 0) {
        dsp::BiquadFilter::process(spec, filterL, inL, outL, numFrames);
        dsp::BiquadFilter::process(spec, filterR, inR, outR, numFrames);
    }
}

int BiquadFilter::getTailLength() const
{
    return 0;
}

template<> const std::string LowPassFilter::tag("low_pass_filter");
template<> const std::string HighPassFilter::tag("high_pass_filter");
template<> const std::string LowShelfFilter::tag("low_shelf_filter");
template<> const std::string HighShelfFilter::tag("high_shelf_filter");
template<> const std::string BandPassFilter::tag("band_pass_filter");
template<> const std::string NotchFilter::tag("notch_filter");
template<> const std::string AllPassFilter::tag("all_pass_filter");

} // namespace fx

TW_NAMESPACE_END
