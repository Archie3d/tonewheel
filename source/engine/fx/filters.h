// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "../globals.h"
#include "audio_effect.h"
#include "dsp/filters.h"

#include <string>

TW_NAMESPACE_BEGIN

namespace fx {

class BiquadFilter : public AudioEffect
{
public:

    enum Params
    {
        FREQUENCY = 0,
        Q_FACTOR,

        NUM_PARAMS
    };

    BiquadFilter(dsp::BiquadFilter::Type filterType);

    void prepareToPlay() override;
    void process(const float* inL, const float* inR, float* outL, float* outR, int numFrames) override;
    int getTailLength() const override;

private:

    dsp::BiquadFilter::Spec spec;
    dsp::BiquadFilter::State filterL;
    dsp::BiquadFilter::State filterR;
};

template <dsp::BiquadFilter::Type T>
class Filter : public BiquadFilter
{
public:
    const static std::string tag;
    Filter()
        : BiquadFilter(T)
    {}

    const std::string& getTag() const override { return tag; }
};

using LowPassFilter = Filter<dsp::BiquadFilter::Type::LowPass>;
using HighPassFilter = Filter<dsp::BiquadFilter::Type::HighPass>;
using LowShelfFilter = Filter<dsp::BiquadFilter::Type::LowShelf>;
using HighShelfFilter = Filter<dsp::BiquadFilter::Type::HighShelf>;
using BandPassFilter = Filter<dsp::BiquadFilter::Type::BandPass>;
using NotchFilter = Filter<dsp::BiquadFilter::Type::Notch>;
using AllPassFilter = Filter<dsp::BiquadFilter::Type::AllPass>;

} // namespace fx

TW_NAMESPACE_END
