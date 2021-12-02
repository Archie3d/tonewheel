// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include <string>

TW_NAMESPACE_BEGIN

namespace core {

/**
 * Class to hold error flag and error message.
 */
class Error
{
public:
    Error()
    {
    }

    Error(const std::string& errorMessage)
        : error{ true }
        , message{ errorMessage }
    {
    }

    bool ok() const noexcept { return !error; }
    bool failed() const noexcept { return error; }

private:
    bool error{ false };
    std::string message{};
};

} // namespace core

TW_NAMESPACE_END
