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
#include "sema.h"
#include <memory>
#include <thread>

TW_NAMESPACE_BEGIN

namespace core {

/**
 * This class runs jobs on a side thread.
 */
class Worker final
{
public:

    class Job
    {
    public:
        virtual void run() = 0;
        virtual ~Job() = default;
    };

    //------------------------------------------------------

    Worker();
    ~Worker();
    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;

    void start();
    void stop();

    bool addJob(Job* job);
    bool hasPendingJobs() const noexcept;
    bool isRunning() const noexcept;

    void purge();

    void run();

private:

    void wait();
    void wakeUp();

    static constexpr size_t defaultQueueCapacity{ 1024 };

    core::RingBuffer<Job*, defaultQueueCapacity> jobsQueue;
    Semaphore sema;
    std::atomic_bool running;
    std::unique_ptr<std::thread> thread;

};

} // namespace core

TW_NAMESPACE_END
