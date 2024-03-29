// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include <atomic>
#include <utility>

TW_NAMESPACE_BEGIN

namespace core {

/**
 * Lock-free queue using a ring buffer.
 */
template <typename T, size_t Size>
class RingBuffer final
{
public:

    RingBuffer() noexcept
        : readIdx{ 0 },
          writeIdx{ 0 },
          data{ {} }
    {
    }

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator =(const RingBuffer&) = delete;

    ~RingBuffer() = default;

    bool send(const T& obj) noexcept
    {
        size_t nextIdx = writeIdx + 1 < Size ? writeIdx + 1 : 0;

        if (nextIdx != readIdx) {
            data[writeIdx] = obj;
            writeIdx = nextIdx;
            return true;
        }

        return false;
    }

    bool receive(T& obj) noexcept
    {
        size_t nextIdx = readIdx + 1 < Size ? readIdx + 1 : 0;

        if (readIdx != writeIdx) {
            obj = std::move(data[readIdx]);
            readIdx = nextIdx;
            return true;
        }

        return false;
    }

    size_t count() const noexcept
    {
        return (writeIdx - readIdx) % Size;
    }

private:

    std::atomic<size_t> readIdx;
    std::atomic<size_t> writeIdx;
    T data[Size];
};

} // namespace core

TW_NAMESPACE_END
