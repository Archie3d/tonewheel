// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include <cstring>
#include <array>

TW_NAMESPACE_BEGIN

namespace dsp {

/**
 * Biquad IIR filter.
 */
struct BiquadFilter
{
    enum class Type
    {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        AllPass,
        PeakingEq,
        LowShelf,
        HighShelf
    };

    struct Spec
    {
        Type type { Type::LowPass };
        float sampleRate { 44100.0f };
        float freq { 1000.0f };
        float q { 0.7071f};
        float dbGain { 1.0f };

        float a[3] { 0.0f };
        float b[4] { 0.0f };
    };

    struct State
    {
        float x[2];
        float y[2];
    };

    static void update(Spec& spec);
    static void reset(const Spec& spec, State& state);
    static float tick(const Spec& spec, State& state, float in);
    static void process(const Spec& spec, State& state, const float* in, float* out, int numFrames);
};

//==============================================================================

/**
 * 4th order Linkwitz-Riley filter.
 */
struct LR4Filter
{
    enum class Type
    {
        LowPass,
        HighPass
    };

    struct Spec
    {
        Type type;
        float sampleRate;
        float freq;

        float a[5];
        float b[4];
    };

    struct State
    {
        float x[4];
        float y[4];
    };

    static void update(Spec& spec);
    static void reset(const Spec& spec, State& state);
    static float tick(const Spec& spec, State& state, float in);
    static void process(const Spec& spec, State& state, const float* in, float* out, int numFrames);
};

//==============================================================================

/**
 * Complimentary pair or LR4 low and high-pass filters.
 */
struct LR4FilterPair
{
    struct Spec
    {
        float sampleRate;
        float freq;

        float a_lp[5];
        float a_hp[5];
        float b[4];
    };

    struct State
    {
        float x[4];
        float y_lp[4];
        float y_hp[4];
    };

    static void update(Spec& spec);
    static void reset(const Spec& spec, State& state);
    static void tick(const Spec& spec, State& state, float in, float& out_lp, float& out_hp);
    static void process(const Spec& spec, State& state, const float* in, float* out_lp, float* out_hp, int numFrames);
};

//==============================================================================

/**
 * DC-blocking filter.
 */
struct DCBlockFilter
{
    struct Spec
    {
        float alpha{ 0.995f };
    };

    struct State
    {
        float x1;
        float y1;
    };

    static void update(Spec& spec);
    static void reset(const Spec& spec, State& state);
    static float tick(const Spec& spec, State& state, float in);
    static void process(const Spec& spec, State& state, const float* in, float* out, int numFrames);
};

//==============================================================================

/**
 * Simple all-pass filter using 1 sample delay.
 */
struct SimpleAllPassFilter
{
    struct Spec
    {
        float alpha{ 0.0f };
    };

    struct State
    {
        float x[2];
        float y[2];
    };

    static void update(Spec& spec);
    static void reset(const Spec& spec, State& state);
    static float tick(const Spec& spec, State& state, float in);
    static void process(const Spec& spec, State& state, const float* in, float* out, int numFrames);
};

//==============================================================================

/**
 * All-pass filter with a specified delay line length.
 */
template <size_t Size>
struct AllPassFilter
{
    using Buffer = std::array<float, Size>;

    struct Spec
    {
        float feedback{ 0.0f };
    };

    struct State
    {
        Buffer buffer;
        size_t index{ 0 };
    };

    static void update(Spec&) {};

    static void reset(const Spec&, State& state)
    {
        state.index = 0;
        ::memset(state.buffer.data(), 0, sizeof(float) *  state.buffer.size());
    }

    static float tick(const Spec& spec, State& state, float in)
    {
        const float bufOut{ state.buffer[state.index] };
        const float out{ bufOut - in };
        state.buffer[state.index] = in + (bufOut * spec.feedback);
        state.index = (state.index + 1) % state.buffer.size();

        return out;
    }

    static void process(const Spec& spec, State& state, const float* in, float* out, int numFrames)
    {
        for (int i = 0; i < numFrames; ++i)
            out[i] = tick(spec, state, in[i]);
    }
};

//==============================================================================

/**
 * Comb filter with specified delay line length.
 */
template<size_t Size>
struct CombFilter
{
    using Buffer = std::array<float, Size>;

    struct Spec
    {
        float feedback{ 0.0f };
        float damp{ 0.0f };
    };

    struct State
    {
        Buffer buffer;
        size_t index{ 0 };
        float y{ 0.0f };
    };

    static void update(Spec&) {};

    static void reset(Spec& spec, State& state)
    {
        state.index = 0;
        state.y = 0.0f;
        ::memset(state.buffer.data(), 0, sizeof(float) * state.buffer.size());
    }

    static float tick(const Spec& spec, State& state, float in)
    {
        const float out{ state.buffer[state.index] };
        state.y = out + (state.y - out) * spec.damp;
        state.buffer[state.index] = in + state.y * spec.feedback;
        state.index = (state.index + 1) % state.buffer.size();

        return out;
    }

    static void process(const Spec& spec, State& state, const float* in, float* out, int numFrames)
    {
        for (int i = 0; i < numFrames; ++i)
            out[i] = tick(spec, state, in[i]);
    }
};

//==============================================================================

/**
 * A cascade of identical filters.
 */
template<class Filter, size_t N>
struct FilterChain
{
    constexpr static size_t length = N;

    struct Spec
    {
        Filter::Spec specs[N];
    };

    struct State
    {
        Filter::State states[N];
    };

    static void update(Spec& spec)
    {
        for (int i = 0; i < N; ++i)
            Filter::update(spec.specs[i]);
    }

    static void reset(const Spec& spec, State& state)
    {
        for (int i = 0; i < N; ++i)
            Filter::reset(spec.specs[i], state.states[i]);
    }

    static float tick(const Spec& spec, State& state, float in)
    {
        float x{ in };

        for (int i = 0; i < N; ++i)
            x = Filter::tick(spec.specs[i], state.states[i], x);

        return x;
    }

    static void process(const Spec& spec, State& state, const float* in, float* out, int numFrames)
    {
        if constexpr (N == 0) {
            ::memcpy(out, in, sizeof(float) * numFrames);
        } else {
            Filter::process(spec.specs[0], state.states[0], in, out, numFrames);

            for (int i = 1; i < N; ++i)
                Filter::process(spec.specs[i], state.states[i], out, out, numFrames);
        }
    }
};

} // namespace dsp

TW_NAMESPACE_END
