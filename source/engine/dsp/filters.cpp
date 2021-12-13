// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "filters.h"
#include "core/math.h"
#include <cstring>
#include <cassert>

TW_NAMESPACE_BEGIN

namespace dsp {

void BiquadFilter::update(Spec& spec)
{
    float A{ 0.0f };

    if (spec.type == Type::PeakingEq || spec.type == Type::LowShelf || spec.type == Type::HighShelf)
        A = sqrt(powf(10.0f, spec.dbGain / 40.0f));
    else
        A = sqrtf(powf(10.0f, spec.dbGain / 20.0f));

    const float w0{ 2.0f * core::math::Constants<float>::pi * spec.freq / spec.sampleRate };

    const float cos_w0{ cos (w0) };
    const float sin_w0{ sin (w0) };
    float alpha{ 0.0f };

    switch (spec.type) {
        case Type::LowPass:
        case Type::HighPass:
        case Type::AllPass:
            alpha = sin_w0 / (2.0f * spec.q);
            break;
        case Type::BandPass:
        case Type::Notch:
        case Type::PeakingEq:
            alpha = sin_w0 * sinh(log(2.0f) / 2.0f * spec.q * w0 / sin_w0);
            break;
        case Type::LowShelf:
        case Type::HighShelf:
            alpha = sin_w0 / 2.0f * sqrt((A + 1.0f / A) * (1.0f / spec.q - 1.0f) + 2.0f);
            break;
        default:
            assert(!"Unsupported filter type");
            break;
    }

    switch (spec.type) {
        case Type::LowPass:
            spec.b[0] = (1.0f - cos_w0) / 2.0f;
            spec.b[1] = 1.0f - cos_w0;
            spec.b[2] = (1.0f - cos_w0) / 2.0f;
            spec.a[0] = 1.0f + alpha;
            spec.a[1] = -2.0f * cos_w0;
            spec.a[2] = 1.0f - alpha;
            break;
        case Type::HighPass:
            spec.b[0] = (1.0f + cos_w0) / 2.0f;
            spec.b[1] = -(1.0f + cos_w0);
            spec.b[2] = (1.0f + cos_w0) / 2.0f;
            spec.a[0] = 1.0f + alpha;
            spec.a[1] = -2.0f * cos_w0;
            spec.a[2] = 1.0f - alpha;
            break;
        case Type::BandPass:
            // Constant 0 dB peak gain
            spec.b[0] = alpha;
            spec.b[1] = 0.0f;
            spec.b[2] = -alpha;
            spec.a[0] = 1.0f + alpha;
            spec.a[1] = -2.0f * cos_w0;
            spec.a[2] = 1.0f - alpha;
            break;
        case Type::Notch:
            spec.b[0] = 1.0f;
            spec.b[1] = -2.0f * cos_w0;
            spec.b[2] = 1.0f;
            spec.a[0] = 1.0f + alpha;
            spec.a[1] = -2.0f * cos_w0;
            spec.a[2] = 1.0f - alpha;
            break;
        case Type::AllPass:
            spec.b[0] = 1.0f - alpha;
            spec.b[1] = -2.0f * cos_w0;
            spec.b[2] = 1.0f + alpha;
            spec.a[0] = 1.0f + alpha;
            spec.a[1] = -2.0f * cos_w0;
            spec.a[2] = 1.0f - alpha;
            break;
        case Type::PeakingEq:
            spec.b[0] = 1.0f + alpha * A;
            spec.b[1] = -2.0f * cos_w0;
            spec.b[2] = 1.0f - alpha * A;
            spec.a[0] = 1.0f + alpha / A;
            spec.a[1] = -2.0f * cos_w0;
            spec.a[2] = 1.0f - alpha / A;
            break;
        case Type::LowShelf:
            spec.b[0] = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrt (A) * alpha);
            spec.b[1] = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_w0);
            spec.b[2] = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrt (A) * alpha);
            spec.a[0] = (A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt (A) * alpha;
            spec.a[1] = -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_w0);
            spec.a[2] = (A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt (A) * alpha;
            break;
        case Type::HighShelf:
            spec.b[0] = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt (A) * alpha);
            spec.b[1] = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_w0);
            spec.b[2] = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt (A) * alpha);
            spec.a[0] = (A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrt (A) * alpha;
            spec.a[1] = 2.0f * ((A - 1.0f) - (A + 1.0f) * cos_w0);
            spec.a[2] = (A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrt(A) * alpha;
            break;
        default:
            assert(!"Should never get here");
            break;
    }

    // Normalize the coefficients.
    spec.a[1] /= spec.a[0];
    spec.a[2] /= spec.a[0];
    spec.b[0] /= spec.a[0];
    spec.b[1] /= spec.a[0];
    spec.b[2] /= spec.a[0];

    spec.a[0] = 1.0f;
}

void BiquadFilter::reset(const Spec&, State& state)
{
    ::memset(&state, 0, sizeof(state));
}

float BiquadFilter::tick(const Spec& spec, State& state, float in)
{
    const float x{ in };
    const float y{ spec.b[0] * x + spec.b[1] * state.x[0] + spec.b[2] * state.x[1]
                                 - spec.a[1] * state.y[0] - spec.a[2] * state.y[1] };

    state.x[1] = state.x[0];
    state.x[0] = x;
    state.y[1] = state.y[0];
    state.y[0] = y;

    return y;
}

void BiquadFilter::process(const Spec& spec, State& state, const float* in, float* out, int numFrames)
{
    for (int i = 0; i < numFrames; ++i) {
        const float x{ in[i] };
        const float y{ spec.b[0] * x + spec.b[1] * state.x[0] + spec.b[2] * state.x[1]
                                     - spec.a[1] * state.y[0] - spec.a[2] * state.y[1] };

        state.x[1] = state.x[0];
        state.x[0] = x;
        state.y[1] = state.y[0];
        state.y[0] = y;

        out[i] = y;
    }
}

//==============================================================================

void LR4Filter::update(LR4Filter::Spec& spec)
{
    const float wc{ core::math::Constants<float>::twoPi * spec.freq };
    const float wc2{ wc * wc };
    const float wc3{ wc2 * wc };
    const float wc4{ wc2 * wc2 };

    const float k{ wc / std::tanf (core::math::Constants<float>::pi * spec.freq / spec.sampleRate) };
    const float k2{ k * k };
    const float k3{ k2 * k };
    const float k4{ k2 * k2 };

    const float sq_tmp1{ core::math::Constants<float>::sqrt2 * wc3 * k };
    const float sq_tmp2{ core::math::Constants<float>::sqrt2 * wc * k3 };
    const float a_tmp{ 4.0f * wc2 * k2 + 2.0f * sq_tmp1 + k4 + 2.0f * sq_tmp2 + wc4 };

    spec.b[0] = (4.0f * (wc4 + sq_tmp1 - k4 - sq_tmp2)) / a_tmp;
    spec.b[1] = (6.0f * wc4 - 8.0f * wc2 * k2 + 6.0f * k4) / a_tmp;
    spec.b[2] = (4.0f * (wc4 - sq_tmp1 + sq_tmp2 - k4)) / a_tmp;
    spec.b[3] = (k4 - 2.0f * sq_tmp1 + wc4 - 2.0f * sq_tmp2 + 4.0f * wc2 * k2) / a_tmp;

    switch (spec.type) {
        case Type::LowPass:
            spec.a[0] = wc4 / a_tmp;
            spec.a[1] = 4.0f * wc4 / a_tmp;
            spec.a[2] = 6.0f * wc4 / a_tmp;
            break;
        case Type::HighPass:
            spec.a[0] = k4 / a_tmp;
            spec.a[1] = -4.0f * k4 / a_tmp;
            spec.a[2] = 6.0f * k4 / a_tmp;
            break;
        default:
            assert(!"Should never get here");
            break;
    }

    spec.a[3] = spec.a[1];
    spec.a[4] = spec.a[0];
}

void LR4Filter::reset(const Spec& spec, State& state)
{
    ::memset(&state, 0, sizeof(state));
}

float LR4Filter::tick(const Spec& spec, State& state, float in)
{
    const float x{ in };
    const float y{ spec.a[0] * x + spec.a[1] * state.x[0] + spec.a[2] * state.x[1] + spec.a[3] * state.x[2] + spec.a[4] * state.x[3]
                                 - spec.b[0] * state.y[0] - spec.b[1] * state.y[1] - spec.b[2] * state.y[2] - spec.b[3] * state.y[3] };

    state.x[3] = state.x[2];
    state.x[2] = state.x[1];
    state.x[1] = state.x[0];
    state.x[0] = x;

    state.y[3] = state.y[2];
    state.y[2] = state.y[1];
    state.y[1] = state.y[0];
    state.y[0] = y;

    return y;
}

void LR4Filter::process(const Spec& spec, State& state, const float* in, float* out, int numFrames)
{
    for (int i = 0; i < numFrames; ++i) {
        const float x{ in[i] };
        const float y{ spec.a[0] * x + spec.a[1] * state.x[0] + spec.a[2] * state.x[1] + spec.a[3] * state.x[2] + spec.a[4] * state.x[3]
                                     - spec.b[0] * state.y[0] - spec.b[1] * state.y[1] - spec.b[2] * state.y[2] - spec.b[3] * state.y[3] };

        state.x[3] = state.x[2];
        state.x[2] = state.x[1];
        state.x[1] = state.x[0];
        state.x[0] = x;

        state.y[3] = state.y[2];
        state.y[2] = state.y[1];
        state.y[1] = state.y[0];
        state.y[0] = y;

        out[i] = y;
    }
}

//==============================================================================

void LR4FilterPair::update(Spec& spec)
{
    const float wc{ core::math::Constants<float>::twoPi * spec.freq };
    const float wc2{ wc * wc };
    const float wc3{ wc2 * wc };
    const float wc4{ wc2 * wc2 };

    const float k{ wc / std::tanf(core::math::Constants<float>::pi * spec.freq / spec.sampleRate) };
    const float k2{ k * k };
    const float k3{ k2 * k };
    const float k4{ k2 * k2 };

    const float sq_tmp1{ core::math::Constants<float>::sqrt2 * wc3 * k };
    const float sq_tmp2{ core::math::Constants<float>::sqrt2 * wc * k3 };
    const float a_tmp{ 4.0f * wc2 * k2 + 2.0f * sq_tmp1 + k4 + 2.0f * sq_tmp2 + wc4 };

    spec.b[0] = (4.0f * (wc4 + sq_tmp1 - k4 - sq_tmp2)) / a_tmp;
    spec.b[1] = (6.0f * wc4 - 8.0f * wc2 * k2 + 6.0f * k4) / a_tmp;
    spec.b[2] = (4.0f * (wc4 - sq_tmp1 + sq_tmp2 - k4)) / a_tmp;
    spec.b[3] = (k4 - 2.0f * sq_tmp1 + wc4 - 2.0f * sq_tmp2 + 4.0f * wc2 * k2) / a_tmp;

    spec.a_lp[0] = wc4 / a_tmp;
    spec.a_lp[1] = 4.0f * wc4 / a_tmp;
    spec.a_lp[2] = 6.0f * wc4 / a_tmp;
    spec.a_lp[3] = spec.a_lp[1];
    spec.a_lp[4] = spec.a_lp[0];

    spec.a_hp[0] = k4 / a_tmp;
    spec.a_hp[1] = -4.0f * k4 / a_tmp;
    spec.a_hp[2] = 6.0f * k4 / a_tmp;
    spec.a_hp[3] = spec.a_hp[1];
    spec.a_hp[4] = spec.a_hp[0];
}

void LR4FilterPair::reset(const Spec&, State& state)
{
    ::memset(&state, 0, sizeof(state));
}

void LR4FilterPair::tick(const Spec& spec, State& state, float in, float& out_lp, float& out_hp)
{
    const float x{ in };

    const float y_lp{ spec.a_lp[0] * x + spec.a_lp[1] * state.x[0] + spec.a_lp[2] * state.x[1] + spec.a_lp[3] * state.x[2] + spec.a_lp[4] * state.x[3]
                        - spec.b[0] * state.y_lp[0] - spec.b[1] * state.y_lp[1] - spec.b[2] * state.y_lp[2] - spec.b[3] * state.y_lp[3] };

    state.y_lp[3] = state.y_lp[2];
    state.y_lp[2] = state.y_lp[1];
    state.y_lp[1] = state.y_lp[0];
    state.y_lp[0] = y_lp;

    const float y_hp{ spec.a_hp[0] * x + spec.a_hp[1] * state.x[0] + spec.a_hp[2] * state.x[1] + spec.a_hp[3] * state.x[2] + spec.a_hp[4] * state.x[3]
                        - spec.b[0] * state.y_hp[0] - spec.b[1] * state.y_hp[1] - spec.b[2] * state.y_hp[2] - spec.b[3] * state.y_hp[3] };

    state.y_hp[3] = state.y_hp[2];
    state.y_hp[2] = state.y_hp[1];
    state.y_hp[1] = state.y_hp[0];
    state.y_hp[0] = y_hp;

    state.x[3] = state.x[2];
    state.x[2] = state.x[1];
    state.x[1] = state.x[0];
    state.x[0] = x;

    out_lp = y_lp;
    out_hp = y_hp;
}

void LR4FilterPair::process(const Spec& spec, State& state, const float* in, float* out_lp, float* out_hp, int numFrames)
{
    for (int i = 0; i < numFrames; ++i) {
        const float x{ in[i] };

        const float y_lp{ spec.a_lp[0] * x + spec.a_lp[1] * state.x[0] + spec.a_lp[2] * state.x[1] + spec.a_lp[3] * state.x[2] + spec.a_lp[4] * state.x[3]
                            - spec.b[0] * state.y_lp[0] - spec.b[1] * state.y_lp[1] - spec.b[2] * state.y_lp[2] - spec.b[3] * state.y_lp[3] };

        state.y_lp[3] = state.y_lp[2];
        state.y_lp[2] = state.y_lp[1];
        state.y_lp[1] = state.y_lp[0];
        state.y_lp[0] = y_lp;

        const float y_hp{ spec.a_hp[0] * x + spec.a_hp[1] * state.x[0] + spec.a_hp[2] * state.x[1] + spec.a_hp[3] * state.x[2] + spec.a_hp[4] * state.x[3]
                            - spec.b[0] * state.y_hp[0] - spec.b[1] * state.y_hp[1] - spec.b[2] * state.y_hp[2] - spec.b[3] * state.y_hp[3] };

        state.y_hp[3] = state.y_hp[2];
        state.y_hp[2] = state.y_hp[1];
        state.y_hp[1] = state.y_hp[0];
        state.y_hp[0] = y_hp;

        state.x[3] = state.x[2];
        state.x[2] = state.x[1];
        state.x[1] = state.x[0];
        state.x[0] = x;

        out_lp[i] = y_lp;
        out_hp[i] = y_hp;
    }
}

//==============================================================================

void DCBlockFilter::update(Spec&)
{
}

void DCBlockFilter::reset(const Spec&, State& state)
{
    state.x1 = 0.0f;
    state.y1 = 0.0f;
}

float DCBlockFilter::tick(const Spec& spec, State& state, float in)
{
    state.y1 = in - state.x1 + spec.alpha * state.y1;
    state.x1 = in;
    return state.y1;
}

void DCBlockFilter::process(const Spec& spec, State& state, const float* in, float* out, int numFrames)
{
    float x = state.x1;
    float y = state.y1;

    for (size_t i = 0; i < numFrames; ++i) {
        y = in[i] - x + spec.alpha * y;
        out[i] = y;
        x = in[i];
    }

    state.x1 = x;
    state.y1 = y;
}

//==============================================================================

void SimpleAllPassFilter::update(Spec&)
{
}

void SimpleAllPassFilter::reset(const Spec&, State& state)
{
    ::memset(&state, 0, sizeof(state));
}

float SimpleAllPassFilter::tick(const Spec& spec, State& state, float in)
{
    float out = (in + state.y[1]) * spec.alpha - state.x[1];
    state.y[1] = state.y[0];
    state.y[0] = out;
    state.x[1] = state.x[0];
    state.x[0] = in;

    return out;
}

void SimpleAllPassFilter::process(const Spec& spec, State& state, const float* in, float* out, int numFrames)
{
    for (int i = 0; i < numFrames; ++i) {
        const float x{ in[i] };
        const float out{ (x + state.y[1]) * spec.alpha - state.x[1] };
        state.y[1] = state.y[0];
        state.y[0] = out;
        state.x[1] = state.x[0];
        state.x[0] = x;
    }
}

} // namespace dsp

TW_NAMESPACE_END
