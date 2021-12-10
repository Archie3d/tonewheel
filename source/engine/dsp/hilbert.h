// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include "dsp/filters.h"
#include <complex>

TW_NAMESPACE_BEGIN

namespace dsp {

/**
 * Hilbert transform.
 *
 * This filter takes a real-number signal and outputs
 * a quadrature pair: R(eal) and I(maginary) offset by 90
 * degrees in phase to each other.
 *
 * @note The output is not quadrature to the original input.
 */
struct Hilbert
{
    using AllPass = AllPassFilterChain<4>;

    struct Spec
    {
        float sampleRate;

        AllPass::Spec frSpec;
        AllPass::Spec fiSpec;
    };

    struct State
    {
        AllPass::State frState;
        AllPass::State fiState;
        float frDelayed;
    };

    static void update(Spec& spec);
    static void reset(const Spec& spec, State& state);
    static void tick(const Spec& spec, State& state, float in, float& ourR, float& outI);
    static std::complex<float> tick (const Spec& spec, State& state, float in);
    static void process(const Spec& spec, State& state, const float* in, float* outR, float* outI, int numFrames);
};

} // namespace dsp

TW_NAMESPACE_END
