// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include <functional>
#include <chrono>

TW_NAMESPACE_BEGIN

namespace core {

/**
 * @brief Thread-safe fixed-size event loop.
 * 
 * Events can be emitted from different threads, however
 * all the events will be processed on the same thread
 * (the one that executes the loop).
 * 
 * @note This class implements an events queue based on
 *       preallocated linked list. This means that it can be saturated
 *       if no events get processed and removed from the queue.
 */
class EventLoop final
{
public:
    constexpr static size_t defaultCapacity = 1024;

    using Handler = std::function<void()>;
    using TimePoint = std::chrono::steady_clock::time_point;

    EventLoop(int turnaround_ms = 200, size_t capacity = defaultCapacity);
    EventLoop(EventLoop&) = delete;
    EventLoop& operator =(EventLoop&) = delete;
    ~EventLoop();

    int exec();
    void quit (int exitCode = 0);
    bool processAllPendingEventsAndQuit (int exitCode = 0);
    bool emit (const Handler& handler);
    bool emitDelayed (const Handler& handler, int delay_ms);
    bool hasPendingEvents();

private:

    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace core

TW_NAMESPACE_END
