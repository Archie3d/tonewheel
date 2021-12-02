// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"
#include "audio_file.h"
#include "core/audio_buffer.h"
#include "core/error.h"
#include "core/worker.h"
#include "core/release_pool.h"
#include <memory>
#include <atomic>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <queue>
#include <functional>

TW_NAMESPACE_BEGIN

class AudioStream;

/**
 * This class represents a single sample.
 * 
 * The sample's preload buffer is handled by this class as well.
 * 
 * @note Identical samples with different start and stop positions
 *       should be considered as different ones as the preloading
 *       and playback will be different for such samples even
 *       when streamed from the same file.
 */
class Sample final : public core::Releasable
{
public:

    using Ptr = std::shared_ptr<Sample>;

    Sample() = delete;
    Sample(AudioFile* audioFile, int start = 0, int stop = 0);
    Sample(const Sample&) = delete;
    Sample& operator =(const Sample&) = delete;
    ~Sample();

    /**
     * Returns hash code for this sample.
     * Two samples coming from the same file and having identical
     * start and stop positions will also have identical hashes.
     */
    std::size_t getHash() const;

    AudioFile& getAudioFile() { return *file; }
    const core::AudioBuffer<float>& getPreloadedSamples() const noexcept { return preloadBuffer; }

    core::Error preload(int numFrames);

    bool isPreloaded() const noexcept { return nPreloadedFrames > 0; }
    int getNumPreloadedFrames() const noexcept { return nPreloadedFrames; }
    int getStartPosition() const noexcept { return startPos; }
    int getStopPosition() const noexcept { return stopPos; }

private:
    std::unique_ptr<AudioFile> file;
    core::AudioBuffer<float> preloadBuffer;
    std::atomic<int> nPreloadedFrames;
    int startPos;
    int stopPos;
};

//==============================================================================

/**
 * A collection of samples.
 * 
 * This class handles a set of samples. There should
 * be normally just one instance of the samples pool which
 * lives in the GlobalEngine singleton.
 * 
 * @see GlobalEngine.
 */
class SamplePool final : public core::Worker::Job
{
public:

    struct Status
    {
        int preloadedSamples;
        int totalSamples;
    };

    SamplePool();
    SamplePool(const SamplePool&) = delete;
    SamplePool& operator =(const SamplePool&) = delete;
    ~SamplePool();

    /**
     * Add a sample to the pool but do not preload it yet.
     * This will not add a sample if it's been already added (based on the file
     * path and start/stop positions), however currently we don't check for the
     * uniquness of the IDs.
     */
    void addSample(int sampleId, const std::string& filePath, int startPos = 0, int stopPos = 0);
    void clear();
    int getNumPreloadedSamples() const noexcept { return numPreloadedSamples; }
    int getNumSamples() const noexcept { return numSamples; }

    /**
     * Returns sample by its ID.
     * If no sample can be found a nullptr is returned.
     */
    Sample::Ptr getSampleById(int id);

    Sample::Ptr getSampleByHash(std::size_t hash);

    void preload(int numFrames);

    // Worker::Job
    void run() override;

private:
    std::vector<Sample::Ptr> samples;
    std::unordered_map<int, Sample::Ptr> idToSampleMap;
    std::unordered_map<std::size_t, Sample::Ptr> hashToSampleMap;

    std::atomic<int> numPreloadedSamples;
    std::atomic<int> numSamples;

    std::mutex mutex;

    core::Worker preloadWorker;
    std::atomic<int> numPreloadFrames;    
};

TW_NAMESPACE_END
