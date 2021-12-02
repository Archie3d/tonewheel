// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include "ring_buffer.h"
#include <memory>

TW_NAMESPACE_BEGIN

namespace core {

struct Releasable
{
    using Ptr = std::shared_ptr<Releasable>;
    virtual ~Releasable() = default;
};

template<size_t Size>
class ReleasePool
{
public:
    ReleasePool()
        : objects()
    {}

    ~ReleasePool()
    {
        release();
    }

    void push(Releasable::Ptr&& ptr)
    {
        objects.send(std::move(ptr));
    }

    bool isHalfFull() const noexcept
    {
        return objects.count() >= Size / 2;
    }

    void release()
    {
        Releasable::Ptr ptr;
        while (objects.receive(ptr)) {};
    }

private:
    RingBuffer<Releasable::Ptr, Size> objects;
};

} // namespace core

TW_NAMESPACE_END
