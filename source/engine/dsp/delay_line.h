// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include <vector>

TW_NAMESPACE_BEGIN

namespace dsp {

/**
 * Delay line with linear interpolation.
 */
class DelayLine final
{
public:
    DelayLine(int totalLength = 1024);
    void resize(int newLength);
    void reset();
    void write(float x);
    float read(float delay) const;

    int getLength() const noexcept;

private:
    std::vector<float> buffer;
    size_t writeIndex;
};

} // namespace dsp

TW_NAMESPACE_END
