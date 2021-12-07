// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"

#include <string>
#include <vector>

TW_NAMESPACE_BEGIN

/**
 * Audio parameters with exponential smoothing.
 */
class AudioParameter
{
public:
    AudioParameter(float value = 0.0f,
                   float min = 0.0f,
                   float max = 1.0f,
                   float smooth = 0.5f);

    void setName(const std::string& n) { name = n; }
    const std::string& getName() const noexcept { return name; }
    void setValueAndSmoothing(float value, float smooth, bool force = false);
    void setValue(float value, bool force = false);
    void setSmoothing(float smooth) noexcept;
    void setRange(float min, float max);

    AudioParameter& operator=(float value);

    float getNextValue();
    float getCurrentValue() const noexcept { return currentValue; }
    float getTargetValue() const noexcept { return targetValue; }
    void getValues(float* data, int size);

    bool isSmoothing() const noexcept { return smoothing; }

private:

    void updateThreshold();
    void updateSmoothing();

    std::string name;   ///< Parameter name.

    float currentValue;
    float minValue;
    float maxValue;
    float targetValue;
    float frac;
    float threshold;
    bool smoothing;
};

//==============================================================================

/**
 * A collection of audio parameters.
 */
class AudioParameterPool
{
public:
    AudioParameterPool(int size);
    AudioParameter& operator[](int index);
    const AudioParameter& operator[](int index) const;
    AudioParameter& getParameterByName(const std::string& name);

    int getNumParameters() const noexcept { return (int)parameters.size(); }
private:
    std::vector<AudioParameter> parameters;
};

TW_NAMESPACE_END
