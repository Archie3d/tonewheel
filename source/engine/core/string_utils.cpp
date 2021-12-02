// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "string_utils.h"
#include <locale>
#include <iterator>
#include <sstream>

TW_NAMESPACE_BEGIN

namespace core {
namespace str {

std::string toLower(const std::string& s)
{
    std::locale loc;
    std::ostringstream ss;

    for (std::string::size_type i = 0; i < s.length(); ++i)
        ss << std::tolower(s[i], loc);

    return ss.str();
}

std::string toUpper(const std::string& s)
{
    std::locale loc;
    std::ostringstream ss;

    for (std::string::size_type i = 0; i < s.length(); ++i)
        ss << std::toupper(s[i], loc);

    return ss.str();
}

bool beginsWith(const std::string& s, const std::string& prefix)
{
    return (s.length() >= prefix.length()) && (s.substr(0, prefix.length()) == prefix);
}

bool endsWith(const std::string& s, const std::string& suffix)
{
    return (s.length() >= suffix.length()) && (s.substr(s.length() - suffix.length(), suffix.length()) == suffix);
}

} // namespace str
} // namespace core

TW_NAMESPACE_END
