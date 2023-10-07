// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"
#include "core/release_pool.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cassert>

TW_NAMESPACE_BEGIN

/**
 * Modulation expression.
 *
 * This class evaluates a mathimatical expression that reads from and writes
 * to variables mapped to the expression. It is used to modulate parameters
 * on the voices during playback.
 */
class ModulationExpression final
{
public:
    ModulationExpression();
    ModulationExpression(const ModulationExpression&) = delete;
    ModulationExpression& operator =(const ModulationExpression&) = delete;
    ~ModulationExpression();

    void addVariable(const std::string& name, float& v, bool constant = false);
    void addVector(const std::string& name, std::vector<float>& v);
    void addConstant(const std::string& name, float v);
    bool compile(const std::string& code);

    float eval();

private:

    struct Impl;
    std::unique_ptr<Impl> d;
};

//==============================================================================

/**
 * Generic modulator.
 *
 * This class provides a storage for the variables exposed to
 * the modulation expression. All variables are floating point.
 */
class GenericModulator : public core::Releasable
{
public:

    using Ptr = std::shared_ptr<GenericModulator>;

    GenericModulator(size_t numVariables = 0);

    GenericModulator(const GenericModulator&) = delete;
    GenericModulator& operator =(const GenericModulator&) = delete;

    void addConstant(const std::string& name, float v);
    bool compile(const std::string& code);
    void eval();

    float& operator[](size_t i) { return variables[i]; }

    void addVariable(const std::string& name, size_t index);
    void addDynamicVariable(const std::string& name, float& value);
    void addDynamicVariables(const std::map<std::string, float>& vars);
    void addVector(const std::string& name, std::vector<float>& v);

private:

    std::vector<float> variables{};
    std::vector<float> dynamicVariables{};
    ModulationExpression expr{};
};

TW_NAMESPACE_END
