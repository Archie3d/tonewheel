// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "audio_parameter.h"
#include "core/math.h"
#include <cmath>
#include <cassert>
#include <algorithm>

TW_NAMESPACE_BEGIN

AudioParameter::AudioParameter(float value, float min, float max, float smooth)
    : currentValue{ value }
    , minValue{ min }
    , maxValue{ max }
    , targetValue{ value }
    , frac{ smooth }
    , smoothing{ false }
{
    updateThreshold();
}

void AudioParameter::setValueAndSmoothing(float value, float smooth, bool force)
{
    targetValue = core::math::clamp(minValue, maxValue, value);
    setSmoothing(smooth);

    if (force) {
        currentValue = targetValue;
        smoothing = false;
    } else {
        updateSmoothing();
    }
}

void AudioParameter::setValue(float value, bool force)
{
    targetValue = core::math::clamp(minValue, maxValue, value);

    if (force) {
        currentValue = targetValue;
        smoothing = false;
    } else {
        updateSmoothing();
    }
}

void AudioParameter::setSmoothing(float smooth) noexcept
{
    // @todo The frac coefficient must be adjusted to the sampling rate of the engine
    frac = core::math::clamp(0.0f, 1.0f, smooth);
}

void AudioParameter::setRange(float min, float max)
{
    minValue = std::min(min, max);
    maxValue = std::max(min, max);

    updateThreshold();
}

AudioParameter& AudioParameter::operator=(float value)
{
    setValue(value);
    return *this;
}

float AudioParameter::getNextValue()
{
    if (smoothing) {
        currentValue += frac * (targetValue - currentValue);
        updateSmoothing();
    }

    return currentValue;
}

void AudioParameter::getValues(float* data, int size)
{
    assert(data != nullptr);
    assert(size >= 0);

    if (smoothing) {
        for (int i = 0; i < size; ++i) {
            currentValue += frac * (targetValue - currentValue);
            data[i] = currentValue;
        }
        updateSmoothing();
    } else {
        for (int i = 0; i < size; ++i)
            data[i] = targetValue;
    }
}

void AudioParameter::updateThreshold()
{
    constexpr float epsilon = 1e-6f;
    threshold = epsilon * std::fabsf(maxValue - minValue);
}

void AudioParameter::updateSmoothing()
{
    smoothing = std::fabsf(currentValue - targetValue) > threshold;

    if (!smoothing)
        currentValue = targetValue;
}

//==============================================================================

static AudioParameter dummyParameter{};

AudioParameterPool::AudioParameterPool(int size)
    : parameters(size)
{
    assert(size >= 0);
}

AudioParameter& AudioParameterPool::operator[](int index)
{
    assert(index >= 0 && index < (int)parameters.size());

    if (index >= 0 && index < (int)parameters.size())
        return parameters.at(index);

    return dummyParameter;
}

const AudioParameter& AudioParameterPool::operator[](int index) const
{
    assert(index >= 0 && index < (int)parameters.size());

    if (index >= 0 && index < (int)parameters.size())
        return parameters.at(index);

    return dummyParameter;
}

AudioParameter& AudioParameterPool::getParameterByName(const std::string& name)
{
    for (auto& param : parameters) {
        if (param.getName() == name)
            return param;
    }

    return dummyParameter;
}

TW_NAMESPACE_END
