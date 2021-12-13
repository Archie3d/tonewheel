// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "../globals.h"
#include "dsp/filters.h"

TW_NAMESPACE_BEGIN

namespace dsp {

/**
 * Reverberator.
 *
 * Alogrithmic reverb composed of a set of comb filters
 * running in parallel, follodef by a series of all-pass filters.
 */
template<size_t Offset = 0>
struct Reverb
{
    static constexpr size_t combTuning1 = 1116 + Offset;
    static constexpr size_t combTuning2 = 1188 + Offset;;
    static constexpr size_t combTuning3 = 1277 + Offset;;
    static constexpr size_t combTuning4 = 1356 + Offset;;
    static constexpr size_t combTuning5 = 1422 + Offset;;
    static constexpr size_t combTuning6 = 1491 + Offset;;
    static constexpr size_t combTuning7 = 1557 + Offset;;
    static constexpr size_t combTuning8 = 1617 + Offset;;
    static constexpr size_t allPassTuning1 = 556 + Offset;;
    static constexpr size_t allPassTuning2 = 441 + Offset;;
    static constexpr size_t allPassTuning3 = 341 + Offset;;
    static constexpr size_t allPassTuning4 = 225 + Offset;;

    struct Spec
    {
        float roomSize;
        float damp;

        typename CombFilter<combTuning1>::Spec comb1;
        typename CombFilter<combTuning2>::Spec comb2;
        typename CombFilter<combTuning3>::Spec comb3;
        typename CombFilter<combTuning4>::Spec comb4;
        typename CombFilter<combTuning5>::Spec comb5;
        typename CombFilter<combTuning6>::Spec comb6;
        typename CombFilter<combTuning7>::Spec comb7;
        typename CombFilter<combTuning8>::Spec comb8;

        typename AllPassFilter<allPassTuning1>::Spec allPass1;
        typename AllPassFilter<allPassTuning2>::Spec allPass2;
        typename AllPassFilter<allPassTuning3>::Spec allPass3;
        typename AllPassFilter<allPassTuning4>::Spec allPass4;
    };

    struct State
    {
        typename CombFilter<combTuning1>::State comb1;
        typename CombFilter<combTuning2>::State comb2;
        typename CombFilter<combTuning3>::State comb3;
        typename CombFilter<combTuning4>::State comb4;
        typename CombFilter<combTuning5>::State comb5;
        typename CombFilter<combTuning6>::State comb6;
        typename CombFilter<combTuning7>::State comb7;
        typename CombFilter<combTuning8>::State comb8;

        typename AllPassFilter<allPassTuning1>::State allPass1;
        typename AllPassFilter<allPassTuning2>::State allPass2;
        typename AllPassFilter<allPassTuning3>::State allPass3;
        typename AllPassFilter<allPassTuning4>::State allPass4;
    };

    static void update(Spec& spec)
    {
        spec.comb1.feedback = spec.roomsize;
        spec.comb2.feedback = spec.roomsize;
        spec.comb3.feedback = spec.roomsize;
        spec.comb4.feedback = spec.roomsize;
        spec.comb5.feedback = spec.roomsize;
        spec.comb6.feedback = spec.roomsize;
        spec.comb7.feedback = spec.roomsize;
        spec.comb8.feedback = spec.roomsize;

        spec.comb1.damp = spec.damp;
        spec.comb2.damp = spec.damp;
        spec.comb3.damp = spec.damp;
        spec.comb4.damp = spec.damp;
        spec.comb5.damp = spec.damp;
        spec.comb6.damp = spec.damp;
        spec.comb7.damp = spec.damp;
        spec.comb8.damp = spec.damp;

        spec.allPass1.feedback = 0.5f;
        spec.allPass2.feedback = 0.5f;
        spec.allPass3.feedback = 0.5f;
        spec.allPass4.feedback = 0.5f;
    }

    static void reset(const Spec& spec, State& state)
    {
        CombFilter<combTuning1>::reset(spec.comb1, state.comb1);
        CombFilter<combTuning2>::reset(spec.comb2, state.comb2);
        CombFilter<combTuning3>::reset(spec.comb3, state.comb3);
        CombFilter<combTuning4>::reset(spec.comb4, state.comb4);
        CombFilter<combTuning5>::reset(spec.comb5, state.comb5);
        CombFilter<combTuning6>::reset(spec.comb6, state.comb6);
        CombFilter<combTuning7>::reset(spec.comb7, state.comb7);
        CombFilter<combTuning8>::reset(spec.comb8, state.comb8);

        AllPassFilter<allPassTuning1>::reset(spec.allPass1, state.allPass1);
        AllPassFilter<allPassTuning2>::reset(spec.allPass2, state.allPass2);
        AllPassFilter<allPassTuning3>::reset(spec.allPass3, state.allPass3);
        AllPassFilter<allPassTuning4>::reset(spec.allPass4, state.allPass4);
    }

    static float tick(const Spec& spec, State& state, float in)
    {
        float y{ CombFilter<combTuning1>::tick(spec.comb1, state.comb1, in) };
        y += CombFilter<combTuning2>::tick(spec.comb2, state.comb2, in);
        y += CombFilter<combTuning3>::tick(spec.comb3, state.comb3, in);
        y += CombFilter<combTuning4>::tick(spec.comb4, state.comb4, in);
        y += CombFilter<combTuning5>::tick(spec.comb5, state.comb5, in);
        y += CombFilter<combTuning6>::tick(spec.comb6, state.comb6, in);
        y += CombFilter<combTuning7>::tick(spec.comb7, state.comb7, in);
        y += CombFilter<combTuning8>::tick(spec.comb8, state.comb8, in);

        y *= 0.125f; // normalize due to x8 combs added together 1/8

        y = AllPassFilter<allPassTuning1>::tick(spec.allPass1, state.allPass1, in);
        y = AllPassFilter<allPassTuning2>::tick(spec.allPass2, state.allPass2, in);
        y = AllPassFilter<allPassTuning3>::tick(spec.allPass3, state.allPass3, in);
        y = AllPassFilter<allPassTuning4>::tick(spec.allPass4, state.allPass4, in);

        out[i] = y;
    }

    static void process(const Spec& spec, State& state, const float* in, float* out, int numFrames)
    {
        for (size_t i = 0; i < numFrames; ++i)
            out[i] = tick(spec, state, in[i]);
    }

};

} // namespace dsp

TW_NAMESPACE_END
