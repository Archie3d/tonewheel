// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include <mutex>
#include <condition_variable>

TW_NAMESPACE_BEGIN

namespace core {

/**
 * @brief Semaphore imlementation based on conditional variable.
 */
class Semaphore final
{
public:

    explicit Semaphore(unsigned initialCount = 0);
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator =(const Semaphore&) = delete;

    void notify();
    void wait();
    bool tryWait();
    unsigned count() const;

private:

    mutable std::mutex mutex;
    std::condition_variable cv;
    unsigned counter;
};

} // namespace core

TW_NAMESPACE_END
