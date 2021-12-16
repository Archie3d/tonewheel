// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "modulation.h"
#include "exprtk.hpp"

TW_NAMESPACE_BEGIN

struct ModulationExpression::Impl
{
    using SymbolTable = exprtk::symbol_table<float>;
    using Expression  = exprtk::expression<float>;
    using Parser      = exprtk::parser<float>;

    SymbolTable symbolTable;
    Expression expression;

    Impl()
        : symbolTable{}
        , expression{}
    {
        symbolTable.add_constants();
        expression.register_symbol_table(symbolTable);
    }

    void addVariable(const std::string& name, float& v, bool constant)
    {
        symbolTable.add_variable(name, v, constant);
    }

    void addVector(const std::string& name, std::vector<float>& v)
    {
        symbolTable.add_vector(name, v);
    }

    void addConstant(const std::string& name, float v)
    {
        symbolTable.add_constant(name, v);
    }

    bool compile(const std::string& code)
    {
        static Parser parser;

        return parser.compile(code, expression);
    }

    float eval()
    {
        return expression.value();
    }
};

//==============================================================================

ModulationExpression::ModulationExpression()
    : d{ std::make_unique<Impl>() }
{
}

ModulationExpression::~ModulationExpression() = default;

void ModulationExpression::addVariable(const std::string& name, float& v, bool constant)
{
    d->addVariable(name, v, constant);
}

void ModulationExpression::addVector(const std::string& name, std::vector<float>& v)
{
    d->addVector(name, v);
}

void ModulationExpression::addConstant(const std::string& name, float v)
{
    d->addConstant(name, v);
}

bool ModulationExpression::compile(const std::string& code)
{
    return d->compile(code);
}

float ModulationExpression::eval()
{
    return d->eval();
}

//==============================================================================

GenericModulator::GenericModulator(size_t numVariables)
    : variables(numVariables, 0.0f)
    , dynamicVariables()
    , expr()
{
}

void GenericModulator::addConstant(const std::string& name, float v)
{
    expr.addConstant(name, v);
}

bool GenericModulator::compile(const std::string& code)
{
    return expr.compile(code);
}

void GenericModulator::eval()
{
    expr.eval();
}

void GenericModulator::addVariable(const std::string& name, size_t index)
{
    expr.addVariable(name, variables[index]);
}

void GenericModulator::addDynamicVariables(const std::map<std::string, float>& vars)
{
    // The vector must be preallocated otherwise the references
    // to the elements will become invalid.
    dynamicVariables.resize(vars.size());

    size_t i{ 0 };

    for (const auto& [name, value] : vars) {
        dynamicVariables[i] = value;
        expr.addVariable(name, dynamicVariables[i]);
        ++i;
    }
}

void GenericModulator::addVector(const std::string& name, std::vector<float>& v)
{
    expr.addVector(name, v);
}

TW_NAMESPACE_END
