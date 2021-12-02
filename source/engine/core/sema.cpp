// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "sema.h"

TW_NAMESPACE_BEGIN

namespace core {

Semaphore::Semaphore(unsigned initialCount)
    : mutex(),
      cv(),
      counter (initialCount)
{
}

void Semaphore::notify()
{
    std::unique_lock<decltype(mutex)> lock(mutex);
    ++counter;
    cv.notify_one();
}

void Semaphore::wait()
{
    std::unique_lock<decltype(mutex)> lock(mutex);

    while (counter == 0)
        cv.wait (lock);

    --counter;
}

bool Semaphore::tryWait()
{
    std::unique_lock<decltype(mutex)> lock(mutex);

    if (counter != 0) {
        --counter;
        return true;
    }

    return false;
}

unsigned Semaphore::count() const
{
    std::unique_lock<decltype(mutex)> lock(mutex);
    return counter;
}

} // namespace core

TW_NAMESPACE_END
