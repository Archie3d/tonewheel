// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "hilbert.h"
#include "core/math.h"
#include <cassert>
#include <array>
#include <unordered_map>

TW_NAMESPACE_BEGIN

namespace dsp {

namespace design {

template<class T>
T ipowp(T x, long long n)
{
    assert(n >= 0);

    T z(1);

    while(n != 0) {
        if ((n & 1) != 0)
            z *= x;

        n >>= 1;
        x *= x;
    }

    return z;
}

void compute_transition_param(double &k, double &q, double transition)
{
    assert(transition > 0);
    assert(transition < 0.5);

    k = tan((1 - transition * 2) * core::math::Constants<double>::pi / 4);
    k *= k;
    assert (k < 1);
    assert (k > 0);
    double kksqrt = pow(1 - k * k, 0.25);
    const double e = 0.5 * (1 - kksqrt) / (1 + kksqrt);
    const double e2 = e * e;
    const double e4 = e2 * e2;
    q = e * (1 + e4 * (2 + e4 * (15 + 150 * e4)));
    assert (q > 0);
}

double compute_acc_num(double q, int order, int c)
{
    assert(c >= 1);
    assert(c < order * 2);

    long long i = 0;
    long long j = 1;
    double acc = 0;
    double q_ii1;

    do {
        q_ii1 = ipowp(q, i * (i + 1));
        q_ii1 *= sin((i * 2 + 1) * c * core::math::Constants<double>::pi / order) * j;
        acc += q_ii1;
        j = -j;
        ++i;
    } while (fabs(q_ii1) > 1e-100);

    return acc;
}

double compute_acc_den(double q, int order, int c)
{
    assert(c >= 1);
    assert(c < order * 2);

    long long i = 1;
    long long j = -1;
    double acc = 0;
    double q_i2;

    do {
        q_i2 = ipowp(q, i * i);
        q_i2 *= cos(i * 2 * c * core::math::Constants<double>::pi / order) * j;
        acc += q_i2;
        j = -j;
        ++i;
    } while (fabs(q_i2) > 1e-100);

    return acc;
}

double compute_coef(int index, double k, double q, int order)
{
    assert(index >= 0);
    assert(index * 2 < order);

    const int c = index + 1;
    const double num = compute_acc_num(q, order, c) * pow(q, 0.25);
    const double den = compute_acc_den(q, order, c) + 0.5;
    const double ww = num / den;
    const double wwsq = ww * ww;

    const double x = sqrt((1 - wwsq * k) * (1 - wwsq / k)) / (1 + wwsq);
    const double coef = (1 - x) / (1 + x);

    return coef;
}

int	compute_order(double attenuation, double q)
{
    assert(attenuation > 0);
    assert(q > 0);

    const double attn_p2 = pow(10.0, -attenuation / 10);
    const double a = attn_p2 / (1 - attn_p2);
    int order = int(ceil(log(a * a / 16) / log(q)));

    if ((order & 1) == 0)
        ++order;

    if (order == 1)
        order = 3;

    return order;
}

int	compute_coefs(double coef_arr [], double attenuation, double transition)
{
    assert(attenuation > 0);
    assert(transition > 0);
    assert(transition < 0.5);

    double k;
    double q;
    compute_transition_param(k, q, transition);

    // Computes number of required coefficients
    const int order = compute_order(attenuation, q);
    const int nbr_coefs = (order - 1) / 2;

    // Coefficient calculation
    for (int index = 0; index < nbr_coefs; ++index)
        coef_arr [index] = compute_coef(index, k, q, order);

    return nbr_coefs;
}

void compute_coefs_spec_order_tbw(double coef_arr [], int nbr_coefs, double transition)
{
    assert(nbr_coefs > 0);
    assert(transition > 0);
    assert(transition < 0.5);

    double k;
    double q;
    compute_transition_param(k, q, transition);
    const int order = nbr_coefs * 2 + 1;

    // Coefficient calculation
    for (int index = 0; index < nbr_coefs; ++index)
        coef_arr [index] = compute_coef(index, k, q, order);
}

} // namespace design

//==============================================================================

constexpr int numCoefs = 2 * Hilbert::AllPass::length;
using Coefs = std::array<float, numCoefs>;


void Hilbert::update(Spec& spec)
{
    static std::unordered_map<int, Coefs> sampleRateToCoefs;

    assert(spec.sampleRate > 0.0f);

    const int sr{ int(floor(spec.sampleRate)) };

    auto it{ sampleRateToCoefs.find(sr) };

    if (it != sampleRateToCoefs.end()) {
        int idx{ 0 };

        for (size_t i = 0; i < it->second.size() / 2; ++i) {
            spec.fiSpec.specs[i].alpha = it->second[idx++];
            spec.frSpec.specs[i].alpha = it->second[idx++];
        }

        return;
    }

    double coefs[numCoefs];

    double transition = 2 * 20.0 / spec.sampleRate;
    design::compute_coefs_spec_order_tbw(coefs, numCoefs, transition);

    std::array<float, numCoefs> stashedCoefs;
    size_t idx = 0;

    for (size_t i = 0; i < numCoefs / 2; ++i) {
        spec.fiSpec.specs[i].alpha = stashedCoefs[idx] = (float)sqrt(coefs[idx]);
        ++idx;
        spec.frSpec.specs[i].alpha = stashedCoefs[idx] = (float)sqrt(coefs[idx]);
        ++idx;
    }
}

void Hilbert::reset(const Spec& spec, State& state)
{
    AllPass::reset(spec.frSpec, state.frState);
    AllPass::reset(spec.fiSpec, state.fiState);
    state.frDelayed = 0.0f;
}

void Hilbert::tick(const Spec& spec, State& state, float in, float& outR, float& outI)
{
    outR = state.frDelayed;
    state.frDelayed = Hilbert::AllPass::tick(spec.frSpec, state.frState, in);
    outI = Hilbert::AllPass::tick(spec.fiSpec, state.fiState, in);
}

std::complex<float> Hilbert::tick(const Hilbert::Spec& spec, Hilbert::State& state, float x)
{
    float real{};
    float imag{};
    tick(spec, state, x, real, imag);

    return std::complex(real, imag);
}

void Hilbert::process(const Spec& spec, State& state, const float* in, float* outR, float* outI, int numFrames)
{
    for (int i = 0; i < numFrames; ++i)
        tick(spec, state, in[i], outR[i], outI[i]);
}

} // namespace dsp

TW_NAMESPACE_END
