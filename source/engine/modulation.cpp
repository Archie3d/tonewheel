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
    std::string errorMessage{};

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

        bool result{ parser.compile(code, expression) };

        errorMessage.clear();

        if (!result && parser.error_count() > 0) {
            // Fetch the very last error
            exprtk::parser_error::type err{ parser.get_error(parser.error_count() - 1) };
            errorMessage = exprtk::parser_error::to_str(err.mode) + " | " + err.diagnostic;
        }

        return result;
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

std::string ModulationExpression::getErrorMessage() const
{
    return d->errorMessage;
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

std::string GenericModulator::getErrorMessage() const
{
    return expr.getErrorMessage();
}

void GenericModulator::eval()
{
    expr.eval();
}

void GenericModulator::addVariable(const std::string& name, size_t index)
{
    expr.addVariable(name, variables[index]);
}

void GenericModulator::addDynamicVariable(const std::string& name, float& value)
{
    expr.addVariable(name, value, false);
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
