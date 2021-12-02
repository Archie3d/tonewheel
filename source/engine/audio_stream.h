// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"
#include "sample.h"
#include "audio_file.h"
#include "core/list.h"
#include "core/worker.h"
#include "core/audio_buffer.h"
#include <vector>
#include <atomic>

TW_NAMESPACE_BEGIN

class AudioStream final : public core::ListItem<AudioStream>,
                          public core::Worker::Job
{
public:

    // Stream state
    enum class State
    {
        Idle, 
        Init,
        Streaming,
        Finishing,
        Over
    };

    AudioStream(int bufferSize = DEFAULT_STREAM_BUFFER_SIZE);
    AudioStream(const AudioStream&) = delete;
    AudioStream& operator =(const AudioStream&) = delete;
    ~AudioStream();

    void trigger(Sample::Ptr streamingSample, core::Worker* streamingWorker);
    Sample::Ptr getSample() noexcept { return sample; }
    float getSampleRate();

    void setLoop(int begin, int end, int xfade);
    void setOffset(int offs);

    int fillBuffers(float* left, float* right, int nFrames);
    bool readOne(float& left, float& right);

    bool isOver() const noexcept { return state == State::Over; }

    void release();
    void returnToPool();

    // Worker::Job
    void run() override;

private:
    void close();
    void generateXfadeEnvelope(float k = 1.0f);

    std::atomic<State> state;

    Sample::Ptr sample;
    core::Worker* worker;

    core::AudioBuffer<float> buffer;        ///< Streaming buffer.
    core::AudioBuffer<float> xfadeBuffer;   ///< Loop cross-fade buffer.
    core::AudioBuffer<float> xfadeEnvelope; ///< Cross-fade amplitude envelope.

    std::atomic<int> samplesInBuffer;       ///< Number of samples in the streaming buffer.
    int samplesInXfadeBuffer;               ///< Number of samples in the loop cross-fade buffer.
    std::atomic<int> readIndex;             ///< Read position within the streaming buffer.
    std::atomic<int> writeIndex;            ///< Write position within the streaming buffer.
    int samplePos;                          ///< Read index within the sample.
    int streamPos;                          ///< Streaming position.
    int offset;                             ///< Offset within the preload buffer.

    int loopBegin;
    int loopEnd;
    int loopXfadeSize;

    std::unique_ptr<AudioFile> file;    ///< Audio file to stream from.
};

//==============================================================================

/**
 * A collection of streams.
 */
class AudioStreamPool final
{
public:
    AudioStreamPool(int numStreams = DEFAULT_AUDIO_STREAM_POOL_SIZE);
    AudioStreamPool(const AudioStreamPool&) = delete;
    AudioStreamPool& operator =(const AudioStreamPool&) = delete;
    ~AudioStreamPool();

    AudioStream* getStream();
    void returnToIdle(AudioStream* stream);

private:
    std::vector<AudioStream> streams;
    core::List<AudioStream> idleStreams;
};

TW_NAMESPACE_END
