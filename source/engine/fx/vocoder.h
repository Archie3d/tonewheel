// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include "core/audio_buffer.h"
#include "audio_effect.h"
#include "dsp/filters.h"
#include "dsp/hilbert.h"
#include <array>

TW_NAMESPACE_BEGIN

namespace fx {

namespace vocoder {

constexpr size_t NUM_BANDS = 32;

class FilterBank final
{
public:
    void update(float sampleRate);
    void reset();
    float tick(int band, float in);

protected:
    std::array<dsp::BiquadFilter::Spec, NUM_BANDS> specs;
    std::array<dsp::BiquadFilter::State, NUM_BANDS> states;
};

} // namespace vocoder

//==============================================================================

class VocoderAnalyzer final : public AudioEffect
{
public:
    const static std::string tag;

    VocoderAnalyzer();

    const std::string& getTag() const override { return VocoderAnalyzer::tag; }

    void prepareToPlay() override;
    void process(const float* inL, const float* inR, float* outL, float* outR, int numFrames) override;
    int getTailLength() const override { return 16; }

    const core::AudioBuffer<float>& getEnvelope() const noexcept { return envelope; }

private:
    vocoder::FilterBank filterBank;
    dsp::Hilbert::Spec hilbertSpec;
    std::array<dsp::Hilbert::State, vocoder::NUM_BANDS> hilbertStates;
    core::AudioBuffer<float> envelope;
};

//==============================================================================

class VocoderSynthesizer final : public AudioEffect
{
public:

    enum Params
    {
        ANALYZER_BUS = 0,

        NUM_PARAMS
    };

    const static std::string tag;

    VocoderSynthesizer();

    const std::string& getTag() const override { return VocoderAnalyzer::tag; }

    void setAnalyzer(VocoderAnalyzer* ptr);

    void prepareToPlay() override;
    void process(const float* inL, const float* inR, float* outL, float* outR, int numFrames) override;

private:

    VocoderAnalyzer* analyzer{ nullptr };

    vocoder::FilterBank filterBankL;
    vocoder::FilterBank filterBankR;
};


} // namespace fx

TW_NAMESPACE_END
