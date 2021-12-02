// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "sample.h"
#include "global_engine.h"
#include "audio_stream.h"

TW_NAMESPACE_BEGIN

Sample::Sample(AudioFile* audioFile, int start, int stop)
    : file(audioFile)
    , preloadBuffer(MIX_BUFFER_NUM_CHANNELS, MIX_BUFFER_NUM_FRAMES)
    , nPreloadedFrames{ 0 }
    , startPos{ std::max(0, start) }
    , stopPos{ stop }
{
}

Sample::~Sample() = default;

std::size_t Sample::getHash() const
{
    std::size_t hash{ std::hash<std::string>{}(file->getPath()) };
    hash = hash ^ (std::hash<int>{}(startPos) << 1);
    hash = hash ^ (std::hash<int>{}(stopPos) << 1);

    return hash;
}

core::Error Sample::preload(int numFrames)
{
    auto res{ file->open() };

    if (res.failed())
        return res;

    res = file->seek(startPos);

    if (res.failed())
        return res;

    int numFramesToPreload{ std::min(numFrames, MAX_PRELOAD_BUFFER_SIZE) };
    if (stopPos > startPos)
        numFramesToPreload = std::min(numFramesToPreload, stopPos - startPos);

    preloadBuffer.allocate(MIX_BUFFER_NUM_CHANNELS, (size_t)numFramesToPreload);
    nPreloadedFrames = file->read(numFramesToPreload, preloadBuffer.getChannelData(0), preloadBuffer.getChannelData(1));

    file->close();

    if (nPreloadedFrames <= 0)
        return core::Error("Sample preload failed");

    return {};
}

//==============================================================================

SamplePool::SamplePool()
{
}

SamplePool::~SamplePool()
{
    preloadWorker.stop();
}

void SamplePool::addSample(int id, const std::string& filePath, int startPos, int stopPos)
{
    auto format{ AudioFile::guessFormatFromFileName(filePath) };

    if (format == AudioFile::Format::Unknown)
        return;
    
    auto sample{ std::make_shared<Sample>(new AudioFile(filePath, format), startPos, stopPos) };
    
    std::lock_guard<decltype(mutex)> lock(mutex);

    const auto hash{ sample->getHash() };

    auto it{ hashToSampleMap.find(hash) };

    if (it != hashToSampleMap.end()) {
        // Sample already exists
        sample = it->second;

        // @todo Check for conflicting IDs
        idToSampleMap[id] = sample;
    } else {
        idToSampleMap[id] = sample;
        samples.push_back(std::move(sample));
        ++numSamples;
    }
}

void SamplePool::clear()
{
    std::lock_guard<decltype(mutex)> lock(mutex);
    idToSampleMap.clear();
    hashToSampleMap.clear();
    samples.clear();
    numPreloadedSamples = 0;
    numSamples = 0;
}

Sample::Ptr SamplePool::getSampleById(int id)
{
    std::lock_guard<decltype(mutex)> lock(mutex);

    if (auto it{ idToSampleMap.find(id) }; it != idToSampleMap.end())
        return it->second;

    return nullptr;
}

Sample::Ptr SamplePool::getSampleByHash(std::size_t hash)
{
    std::lock_guard<decltype(mutex)> lock(mutex);

    if (auto it{ hashToSampleMap.find(hash) }; it != hashToSampleMap.end())
        return it->second;

    return nullptr;
}

void SamplePool::preload(int numFrames)
{
    std::lock_guard<decltype(mutex)> lock(mutex);

    numPreloadFrames = numFrames;

    if (!preloadWorker.isRunning())
        preloadWorker.start();

    preloadWorker.addJob(this);
}

void SamplePool::run()
{
    size_t idx = 0;

    while (preloadWorker.isRunning()) {
        std::unique_lock<decltype(mutex)> lock(mutex);
        size_t total{ samples.size() };
        int frames{ numPreloadFrames };

        if (idx < total) {
            auto sample{ samples[idx] };
            lock.unlock();

            if (!sample->isPreloaded()) {
                if (sample->preload(frames).ok())
                    ++numPreloadedSamples;
            }

            ++idx;
        } else {
            break;
        }
    }
}

TW_NAMESPACE_END
