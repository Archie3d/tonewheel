// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "event_loop.h"
#include "list.h"
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cassert>

TW_NAMESPACE_BEGIN

namespace core {

struct Event : public ListItem<Event>
{
    EventLoop::Handler   handler;   // Event handler.
    EventLoop::TimePoint expire;    // Event due time.
};

//----------------------------------------------------------

struct EventLoop::Impl
{
    int turnaroundMs;
    std::vector<Event> events;
    List<Event> pendingEvents;
    List<Event> freeEvents;
    std::atomic_bool running;
    int exitCode;
    std::recursive_mutex mutex;
    std::condition_variable wait;

    Impl (int turnaround, size_t capacity)
        : turnaroundMs{ turnaround }
        , events(capacity)
        , pendingEvents()
        , freeEvents()
        , running{ false }
        , exitCode{ 0 }
        , mutex()
        , wait()
    {
        for (auto& evt : events)
            freeEvents.append (&evt);
    }

    bool enqueue(const EventLoop::Handler& handler, const EventLoop::TimePoint& expire)
    {
        std::unique_lock<decltype(mutex)> lock(mutex);

        if (auto* event = freeEvents.first()) {
            freeEvents.remove (event);
            event->handler = handler;
            event->expire = expire;
            pendingEvents.append (event);
            wait.notify_all();

            return true;
        }

        // Events queue is saturated - need to increase capacity or
        // make sure the events are promptly processed.
        assert(!"Event queue is saturated");

        return false;
    }

    int process() {
        EventLoop::TimePoint now = std::chrono::steady_clock::now();
        auto sleep_ms = turnaroundMs;

        std::unique_lock<decltype(mutex)> lock(mutex);

        auto* event = pendingEvents.first();

        while (event != nullptr) {
            if (now >= event->expire) {
                event->handler();
                sleep_ms = 0;
                auto* freed = event;
                event = pendingEvents.removeAndReturnNext(event);
                freeEvents.append(freed);
            } else {
                if (sleep_ms > 0) {
                    const auto eventSleep{ std::chrono::duration_cast<std::chrono::milliseconds>(event->expire - now).count() };

                    if (eventSleep < sleep_ms)
                        sleep_ms = (int)eventSleep;
                }

                event = event->next();
            }
        }

        return sleep_ms;
    }

    int exec()
    {
        running = true;

        while (running) {
            const auto sleep_ms{ process() };

            if (sleep_ms > 0) {
                std::mutex waitMutex;
                std::unique_lock<decltype(waitMutex)> lock (waitMutex);
                wait.wait_for (lock, std::chrono::milliseconds (sleep_ms));
            }
        }

        return exitCode;
    }

    void quit (int code)
    {
        std::unique_lock<decltype(mutex)> lock(mutex);

        exitCode = code;
        running = false;
        wait.notify_all();
    }

    bool hasPendingEvents()
    {
        std::unique_lock<decltype(mutex)> lock (mutex);

        return pendingEvents.first() != nullptr;
    }
};

//----------------------------------------------------------

EventLoop::EventLoop(int turnaroundMs, size_t capacity)
    : d (std::make_unique<Impl>(turnaroundMs, capacity))
{
}

EventLoop::~EventLoop() = default;

int EventLoop::exec()
{
    return d->exec();
}

void EventLoop::quit(int code)
{
    d->quit(code);
}

bool EventLoop::processAllPendingEventsAndQuit(int code)
{
    return d->enqueue([this, code] {
            this->quit(code);
        },
        std::chrono::steady_clock::now()
    );
}

bool EventLoop::emit(const EventLoop::Handler& handler)
{
    return d->enqueue(handler, std::chrono::steady_clock::now());
}

bool EventLoop::emitDelayed(const EventLoop::Handler& handler, int delay_ms)
{
    auto now = std::chrono::steady_clock::now();
    auto expiration = now + std::chrono::milliseconds(delay_ms);
    return d->enqueue(handler, expiration);
}

bool EventLoop::hasPendingEvents()
{
    return d->hasPendingEvents();
}

} // namespace core

TW_NAMESPACE_END
