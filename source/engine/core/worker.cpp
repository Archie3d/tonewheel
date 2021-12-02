// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "worker.h"
#include <cassert>

TW_NAMESPACE_BEGIN

namespace core {

Worker::Worker()
{
}

Worker::~Worker()
{
    stop();
}

void Worker::start()
{
    purge();

    if (thread == nullptr) {
        running = true;
        thread = std::make_unique<std::thread>(&Worker::run, this);
    }
}

void Worker::stop()
{
    if (thread != nullptr) {
        running = false;
        wakeUp();

        if (thread->joinable())
            thread->join();
    }
}

bool Worker::addJob(Job* job)
{
    assert(job != nullptr);

    const bool ok{ jobsQueue.send(job) };
    wakeUp();

    return ok;
}

bool Worker::hasPendingJobs() const noexcept
{
    return jobsQueue.count() > 0;
}

bool Worker::isRunning() const noexcept
{
    return running;
}

void Worker::purge()
{
    Job* job{ nullptr };

    while (jobsQueue.receive(job)) {}
}

void Worker::run()
{
    while (running) {
        Job* job{ nullptr };

        wait();

        if (running && jobsQueue.receive(job)) {
            assert(job != nullptr);
            job->run();
        }
    }
}

void Worker::wait()
{
    sema.wait();
}

void Worker::wakeUp()
{
    sema.notify();
}

} // namespace core

TW_NAMESPACE_END
