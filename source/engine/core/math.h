// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"

TW_NAMESPACE_BEGIN

namespace core {
namespace math {

/// Clamp a value to the range.
template <typename T>
T clamp(T minValue, T maxValue, T x)
{
    return (x < minValue) ? minValue
                          : (x > maxValue) ? maxValue
                                           : x;
}

/// Linear interpolation.
template <typename T>
T lerp(T a, T b, T frac) { return a + (b - a) * frac; }

/// Lagrange polynomial interpolation.
template <typename T>
T lagr(T x_1, T x0, T x1, T x2, T frac)
{
    const T c1{ x1 - (1.0f / 3.0f) * x_1 - 0.5f * x0 - (1.0f / 6.0f) * x2 };
    const T c2{ 0.5f * (x_1 + x1) - x0 };
    const T c3{ (1.0f / 6.0f) * (x2 - x_1) + 0.5f * (x0 - x1) };
    return ((c3 * frac + c2) * frac + c1) * frac + x0;
}

/// Lagrange inperpolation working on an array.
template <typename T>
T lagr(T* x, T frac)
{
    const T c1{ x[2] - (1.0f / 3.0f) * x[0] - 0.5f * x[1] - (1.0f / 6.0f) * x[3] };
    const T c2{ 0.5f * (x[0] + x[2]) - x[1] };
    const T c3{ (1.0f / 6.0f) * (x[3] - x[0]) + 0.5f * (x[1] - x[2]) };
    return ((c3 * frac + c2) * frac + c1) * frac + x[1];
}

} // namespace math
} // namespace core

TW_NAMESPACE_END
