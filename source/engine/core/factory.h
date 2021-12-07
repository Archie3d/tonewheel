// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include <map>
#include <functional>
#include <type_traits>

TW_NAMESPACE_BEGIN

namespace core {

/**
 * Generic factory.
 */
template<typename KeyType, typename ObjectType>
class Factory final
{
public:
    using CreateFunc = std::function<ObjectType()>;
    using Key = typename std::remove_reference<KeyType>::type;
    using CreateFuncMap = typename std::map<Key, CreateFunc>;

    Factory(std::initializer_list<std::pair<const Key, CreateFunc>> init)
        : createFuncMap(init)
    {}

    ObjectType create(const Key& key) const
    {
        const auto it{ createFuncMap.find(key) };

        if (it != createFuncMap.end())
            return it->second();

        return ObjectType();
    }

    void registerType(const Key& key, const CreateFunc& func)
    {
        createFuncMap[key] = func;
    }

private:
    CreateFuncMap createFuncMap;
};

} // namespace core

TW_NAMESPACE_END
