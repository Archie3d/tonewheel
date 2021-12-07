// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "dsp/delay_line.h"
#include "core/math.h"
#include <cassert>
#include <cmath>

TW_NAMESPACE_BEGIN

namespace dsp {

DelayLine::DelayLine(int totalLength)
    : buffer((size_t)totalLength, 0.0f)
    , writeIndex{ 0 }
{
    assert(totalLength > 0);
}

void DelayLine::resize(int newLength)
{
    assert(newLength > 0);
    buffer.resize((size_t)newLength);
    reset();
}

void DelayLine::reset()
{
    writeIndex = 0;
    ::memset(buffer.data(), 0, sizeof(float) * buffer.size());
}

void DelayLine::write(float x)
{
    if (writeIndex == 0)
        writeIndex = buffer.size() - 1;
    else
        --writeIndex;

    buffer[writeIndex] = x;
}

float DelayLine::read(float delay) const
{
    int index{ (int)std::floor(delay) };
    float frac{ delay - (float)index };

    index = (index + writeIndex) % (int)buffer.size();
    const auto a{ buffer[index] };
    const auto b{ index < buffer.size() - 1 ? buffer[index + 1] : buffer[0] };

    return core::math::lerp(a, b, frac);
}

int DelayLine::getLength() const noexcept
{
    return (int)buffer.size();
}

} // namespace dsp

TW_NAMESPACE_END
