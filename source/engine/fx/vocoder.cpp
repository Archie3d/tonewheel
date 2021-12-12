// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "fx/vocoder.h"
#include "core/math.h"
#include "engine.h"
#include <cassert>

TW_NAMESPACE_BEGIN

namespace fx {

namespace vocoder {

const static float bandFreq[vocoder::NUM_BANDS] = {
    76.0f, 89.0f, 104.0f, 122.0f, 142.0f, 166.0f, 196.0f, 230.0f,
    270.0f, 318.0f, 371.0f, 436.0f, 510.0f, 597.0f, 701.0f, 823.0f,
    962.0f, 1130.0f, 1325.0f, 1553.0f, 1822.0f, 2134.0f, 2508.0f, 2935.0f,
    3439.0f, 4037.0f, 4740.0f, 5551.0f, 6509.0f, 7632.0f, 8949.0f, 10500.0f
};

void FilterBank::update(float sampleRate)
{
    const float k{ core::math::Constants<float>::pi / sampleRate };

    for (size_t i = 0; i < specs.size(); ++i) {
        float lf{ i == 0 ? 60.0f : 0.5f * (bandFreq[i - 1] + bandFreq[i]) };
        float uf{ i == specs.size() - 1 ? 12000.0f : 0.5f * (bandFreq[i] + bandFreq[i + 1]) };
        float q{ (uf - lf) / bandFreq[i] };
        specs[i].type = dsp::BiquadFilter::Type::BandPass;
        specs[i].sampleRate = sampleRate;
        specs[i].freq = bandFreq[i];
        specs[i].q = 0.05f;
        dsp::BiquadFilter::update(specs[i]);
    }
}

void FilterBank::reset()
{
    for (size_t i = 0; i < states.size(); ++i)
        dsp::BiquadFilter::reset(specs[i], states[i]);
}

float FilterBank::tick(int band, float in)
{
    return dsp::BiquadFilter::tick(specs[band], states[band], in);
}

} // namespace vocoder

//==============================================================================

const std::string VocoderAnalyzer::tag("vocoder_analyzer");

VocoderAnalyzer::VocoderAnalyzer()
    : AudioEffect()
    , envelope(vocoder::NUM_BANDS, MIX_BUFFER_NUM_FRAMES)
{
}

void VocoderAnalyzer::prepareToPlay()
{
    assert(engine != nullptr);

    filterBank.update(engine->getSampleRate());

    hilbertSpec.sampleRate = engine->getSampleRate();
    dsp::Hilbert::update(hilbertSpec);

    filterBank.reset();

    for (auto& state : hilbertStates)
        dsp::Hilbert::reset(hilbertSpec, state);

    envelope.clear();
}

void VocoderAnalyzer::process(const float* inL, const float* inR, float* outL, float* outR, int numFrames)
{
    assert(numFrames <= MIX_BUFFER_NUM_FRAMES);

    std::array<float, MIX_BUFFER_NUM_FRAMES> tmp;
    for (int i = 0; i < numFrames; ++i)
        tmp[i] = 0.5f * (inL[i] + inR[i]);

    for (int band = 0; band < vocoder::NUM_BANDS; ++band) {
        float* env{ envelope.getChannelData(band) };

        for (int i = 0; i < numFrames; ++i) {
            const float x{ filterBank.tick(band, tmp[i]) };
            const auto y{ dsp::Hilbert::tick(hilbertSpec, hilbertStates[band], x) };

            env[i] = std::abs(y) * 100.0f;
            //const float prev{ i == 0 ? env[MIX_BUFFER_NUM_FRAMES - 1] : env[i - 1] };
            //env[i] = 0.95f * prev + 0.05f * std::abs(y);
        }
    }
}

//==============================================================================

const std::string VocoderSynthesizer::tag("vocoder_synthesizer");

VocoderSynthesizer::VocoderSynthesizer()
    : AudioEffect(NUM_PARAMS)
    , analyzer{ nullptr }
{
    params[ANALYZER_BUS].setName("analyzer_bus");
    params[ANALYZER_BUS].setValue(0.0f, true);
    params[ANALYZER_BUS].setRange(0.0f, 255.0f);
}

void VocoderSynthesizer::setAnalyzer(VocoderAnalyzer* ptr)
{
    assert(ptr != nullptr);
    analyzer = ptr;
}

void VocoderSynthesizer::prepareToPlay()
{
    assert(engine != nullptr);
    filterBankL.update(engine->getSampleRate());
    filterBankR.update(engine->getSampleRate());

    filterBankL.reset();
    filterBankR.reset();

    // Looking for the analyzer
    int bus{ (int) params[ANALYZER_BUS].getTargetValue() };

    auto& audioBusPool{ engine->getAudioBusPool() };

    analyzer = nullptr;

    if (bus >= 0 && bus < audioBusPool.getNumBuses()) {
        auto& fxChain{ audioBusPool.getBuses()[bus].getFxChain() };

        for (int i = 0; i < fxChain.getNumEffects(); ++i) {
            auto* fx{ fxChain[i] };

            if (fx->getTag() == VocoderAnalyzer::tag)
                analyzer = dynamic_cast<VocoderAnalyzer*>(fx);
        }
    }
}

void VocoderSynthesizer::process(const float* inL, const float* inR, float* outL, float* outR, int numFrames)
{
    assert(numFrames <= MIX_BUFFER_NUM_FRAMES);

    if (analyzer == nullptr) {
        if (inL != outL)
            ::memcpy(outL, inL, sizeof(float) * numFrames);
        if (inR != outR)
            ::memcpy(outR, inR, sizeof(float) * numFrames);

        return;
    }

    const auto& envelope{ analyzer->getEnvelope() };

    ::memcpy(tmpL.data(), inL, sizeof(float) * numFrames);
    ::memcpy(tmpR.data(), inR, sizeof(float) * numFrames);

    ::memset(outL, 0, sizeof(float) * numFrames);
    ::memset(outR, 0, sizeof(float) * numFrames);

    for (int i = 0; i < numFrames; ++i) {
        for (int band = 0; band < vocoder::NUM_BANDS; ++band) {
            const float l{ filterBankL.tick(band, tmpL[i]) };
            const float r{ filterBankR.tick(band, tmpR[i]) };

            const float env{ envelope.getChannelData(band)[i] };

            outL[i] += l * env;
            outR[i] += r * env;
        }
    }
}

} // namespace fx

TW_NAMESPACE_END
