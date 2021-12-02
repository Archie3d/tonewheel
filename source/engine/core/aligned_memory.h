// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"

#include <cstdlib>

TW_NAMESPACE_BEGIN

namespace core {

/**
 * Aligned memory allocator.
 *
 * This class implements a custom allocator for aligned memory blocks.
 * Not all compilers provide std::aligned_alloc (e.g. MSVC does not), which
 * is why we need a custom allocator.
 */
template <size_t Align>
struct AlignedMemory
{
    typedef uint16_t offset_t;
    constexpr static size_t PTR_OFFSET_SIZE = sizeof(offset_t);
    constexpr static size_t alignment = Align;

    /**
     * Allocate aligned memory block.
     */
    static void* alloc(size_t size)
    {
        void* ptr = nullptr;

        if (alignment && size) {
            uint32_t hdr_size = PTR_OFFSET_SIZE + (alignment - 1);
            void* p = ::malloc(size + hdr_size);

            if (p) {
                ptr = (void*)alignUp(((uintptr_t)p + PTR_OFFSET_SIZE));

                // Calculate the offset and store it behind our aligned pointer
                *((offset_t *)ptr - 1) = (offset_t) ((uintptr_t)ptr - (uintptr_t)p);
            }
        }

        return ptr;
    }

    /**
     * Release aligned memory block allocated by \ref alloc.
     */
    static void free(void* ptr)
    {
        if (ptr) {
            offset_t offset = *((offset_t *)ptr - 1);

            // Once we have the offset, we can get our original pointer and call free
            void* p = (void*) ((uint8_t*)ptr - offset);
            ::free(p);
        }
    }

private:

    inline static size_t alignUp(size_t num)
    {
        return (num + (alignment - 1)) & ~(alignment - 1);
    }

    AlignedMemory()
    {
        static_assert((alignment & (alignment - 1)) == 0);
    }
};

} // namespace core

TW_NAMESPACE_END
