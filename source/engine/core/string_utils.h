// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "../globals.h"
#include <string>

TW_NAMESPACE_BEGIN

namespace core {
namespace str {

std::string toLower(const std::string& s);
std::string toUpper(const std::string& s);
bool beginsWith(const std::string& s, const std::string& prefix);
bool endsWith(const std::string& s, const std::string& suffix);


} // namespace str
} // namespace core

TW_NAMESPACE_END
