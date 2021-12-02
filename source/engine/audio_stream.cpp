// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "audio_stream.h"
#include "global_engine.h"
#include <cassert>

TW_NAMESPACE_BEGIN

AudioStream::AudioStream(int bufferSize)
    : state{ State::Idle }
    , sample{ nullptr }
    , worker{ nullptr }
    , buffer(MIX_BUFFER_NUM_CHANNELS, bufferSize)
    , xfadeBuffer(MIX_BUFFER_NUM_CHANNELS, DEFAULT_XFADE_BUFFER_SIZE)
    , xfadeEnvelope(MIX_BUFFER_NUM_CHANNELS, DEFAULT_XFADE_BUFFER_SIZE)
    , samplesInBuffer{ 0 }
    , samplesInXfadeBuffer{ 0 }
    , readIndex{ 0 }
    , writeIndex{ 0 }
    , samplePos{ 0 }
    , streamPos{ 0 }
    , offset{ 0 }
    , loopBegin{ -1 }
    , loopEnd{ -1 }
    , loopXfadeSize{ 128 }
    , file{ nullptr }
{
}

AudioStream::~AudioStream() = default;

void AudioStream::trigger(Sample::Ptr streamingSample, core::Worker* streamingWorker)
{
    assert(streamingSample != nullptr);
    assert(streamingWorker != nullptr);

    sample = streamingSample;
    worker = streamingWorker;

    samplesInBuffer = 0;
    samplesInXfadeBuffer = 0;
    readIndex = 0;
    writeIndex = 0;
    samplePos = offset;
    loopBegin = -1;
    loopEnd = -1;
    state = State::Init;

    worker->addJob(this);
}

float AudioStream::getSampleRate()
{
    assert(sample != nullptr);
    return sample->getAudioFile().getSampleRate();
}

void AudioStream::setLoop(int begin, int end, int xfade)
{
    if (begin < 0 || end < 0) {
        // Turn the looping off
        loopBegin = -1;
        loopEnd = -1;
        return;
    }

    loopBegin = std::min(begin, end);
    loopEnd = std::max(begin, end);
    loopXfadeSize = std::max(DEFAULT_XFADE_BUFFER_SIZE, xfade);

    // Loop end must be outside of the preloaded region
    loopEnd = std::max(sample->getNumPreloadedFrames(), loopEnd);

    assert(loopBegin < loopEnd);
}

void AudioStream::setOffset(int offs)
{
    offset = offs;
    samplePos = offset;
}

int AudioStream::fillBuffers(float* left, float* right, int nFrames)
{
    assert(left != nullptr);
    assert(right != nullptr);

    int generatedFrames{ 0 };
    const auto preloadedFramesAvailable{ sample->getNumPreloadedFrames() - samplePos };

    if (preloadedFramesAvailable > 0)
    {
        // First deliver preloaded samples
        const auto& prebuffer{ sample->getPreloadedSamples() };
        const auto n{ std::min(preloadedFramesAvailable, nFrames) };

        ::memcpy(left, &prebuffer.getChannelData(0)[samplePos], sizeof(float) * n);
        ::memcpy(right, &prebuffer.getChannelData(1)[samplePos], sizeof(float) * n);
        samplePos += n;

        if (nFrames == n)
            return nFrames;

        nFrames -= n;
        left += n;
        right += n;
        generatedFrames += n;
    }

    // Delivering from the streaming buffer
    int framesAvailable{ samplesInBuffer };
    int framesToCopy{ std::min (framesAvailable, nFrames) };

    while (framesToCopy > 0) {
        int copyThisTime = std::min((int) buffer.getNumFrames() - readIndex, framesToCopy);
        ::memcpy(left, &buffer.getChannelData(0)[readIndex], sizeof(float) * copyThisTime);
        ::memcpy(right, &buffer.getChannelData(1)[readIndex], sizeof(float) * copyThisTime);

        readIndex = (readIndex.load() + copyThisTime) % (int)buffer.getNumFrames();
        samplesInBuffer -= copyThisTime;
        framesToCopy -= copyThisTime;

        samplePos += copyThisTime;
        nFrames -= copyThisTime;
        generatedFrames += copyThisTime;
    }

    // Schedule to read more samples if half of the buffer is empty
    if (state == State::Streaming && (samplesInBuffer <= buffer.getNumFrames() / 2))
        worker->addJob (this);

    return generatedFrames;
}

bool AudioStream::readOne(float& left, float& right)
{
    const auto prebufferedFramesAvailable{ sample->getNumPreloadedFrames() - samplePos };

    // Read from pre-buffer
    if (prebufferedFramesAvailable > 0)
    {
        const auto& prebuffer{ sample->getPreloadedSamples() };
        left = prebuffer.getChannelData(0)[samplePos];
        right = prebuffer.getChannelData(1)[samplePos];

        ++samplePos;
        return true;
    }

    // Read from stream
    if (samplesInBuffer == 0) {
        left = 0.0f;
        right = 0.0f;

        if (state == State::Finishing)
            state = State::Over;

        return false;
    }

    const auto ri{ readIndex.load() };
    left = buffer.getChannelData(0)[ri];
    right = buffer.getChannelData(1)[ri];

    readIndex = (ri + 1) % (int)buffer.getNumFrames();
    --samplesInBuffer;
    ++samplePos;

    // Schedule to read more samples if half of the buffer is empty
    if (state == State::Streaming && (samplesInBuffer <= buffer.getNumFrames() / 2))
        worker->addJob (this);

    return true;

}

void AudioStream::release()
{
    if (!isOver()) {
        state = State::Finishing;
        worker->addJob(this);
    }
}

void AudioStream::returnToPool()
{
    assert(sample != nullptr);

    auto* g{ GlobalEngine::getInstance() };

    // This moves the sample pointer so that we don't delete it here.
    g->releaseObject(sample);
    g->getAudioStreamPool().returnToIdle(this);
}

void AudioStream::run()
{
    if (state == State::Idle) {
        // Stream has not been triggered yet
        return;
    }

    if (state == State::Init) {
        // Initializing stream
        assert(sample != nullptr);
        file.reset(sample->getAudioFile().clone());

        if (file == nullptr) {
            state = State::Over;
            return;
        }

        if (file->open().failed()) {
            state = State::Over;
            return;
        }

        if (file->seek(sample->getStartPosition() + sample->getNumPreloadedFrames()).failed())
        {
            state = State::Finishing;
            return;
        }

        // Generate x-fade envelope if looping
        if (loopBegin >=0 && loopEnd >= 0)
        {
            xfadeBuffer.allocate(MIX_BUFFER_NUM_CHANNELS, loopXfadeSize);
            xfadeEnvelope.allocate(2, loopXfadeSize);
            generateXfadeEnvelope(0.5f);
        }

        state = State::Streaming;

        // Stream position is calculate dform the sample start pos.
        streamPos = sample->getNumPreloadedFrames();
    }

    if (state == State::Streaming) {
        // Active streaming
        int framesToRead{ buffer.getNumFrames() - samplesInBuffer };

        while (framesToRead > 0) {
            int readThisTime{ std::min(buffer.getNumFrames() - writeIndex, framesToRead) };

            bool loop{ false };

            if (sample->getStopPosition() > 0 && streamPos + sample->getStartPosition() + readThisTime > sample->getStopPosition())
                readThisTime = std::max(0, sample->getStopPosition() - sample->getStartPosition() - streamPos);

            if (readThisTime == 0) {
                // Stream is over
                state = State::Finishing;
                break;
            }

            if (loopEnd > 0) {
                if (streamPos + readThisTime >= loopEnd) {
                    // Hitting the loop
                    readThisTime = loopEnd - streamPos;
                    loop = true;
                }
            }

            float* left{ &buffer.getChannelData(0)[writeIndex] };
            float* right{ &buffer.getChannelData(1)[writeIndex] };

            assert(file->isOpen());
            const int framesRead = file->read(readThisTime, left, right);

            if (framesRead == 0) {
                // Stream is depleted
                state = State::Finishing;
                break;
            }

            // Loop x-fade
            if (samplesInXfadeBuffer > 0) {
                // Cannot x-fade on loop edge
                assert(!loop);
                const int n{ std::min(samplesInXfadeBuffer, framesRead) };
                int j{ xfadeBuffer.getNumFrames() - samplesInXfadeBuffer };

                for (int i = 0; i < n; ++i) {
                    const auto a{ xfadeEnvelope.getChannelData(0)[j] };
                    const auto b{ xfadeEnvelope.getChannelData(1)[j] };

                    left[i] = left[i] * a + xfadeBuffer.getChannelData(0)[j] * b;
                    right[i] = right[i] * a + xfadeBuffer.getChannelData(1)[j] * b;

                    ++j;
                }

                samplesInXfadeBuffer -= n;
            }

            writeIndex = (writeIndex.load() + framesRead) % buffer.getNumFrames();
            framesToRead -= framesRead;
            samplesInBuffer += framesRead;
            streamPos += framesRead;

            if (loop) {
                assert(samplesInXfadeBuffer == 0);
                assert(streamPos == loopEnd);

                assert(xfadeBuffer.getNumFrames() == loopXfadeSize);
                assert(xfadeEnvelope.getNumFrames() == loopXfadeSize);

                xfadeBuffer.clear();

                int xfadeRead{ xfadeBuffer.getNumFrames() };
                int xfadeIdx{ 0 };

                while (xfadeRead > 0) {
                    const auto read{ file->read (xfadeRead,
                                                 &xfadeBuffer.getChannelData(0)[xfadeIdx],
                                                 &xfadeBuffer.getChannelData(1)[xfadeIdx]) };

                    if (read == 0)
                        break;

                    xfadeIdx += read;
                    xfadeRead -= read;
                }

                file->seek(sample->getStartPosition() + loopBegin);

                streamPos = loopBegin;
                samplesInXfadeBuffer = xfadeBuffer.getNumFrames();
            }
        }
    }

    if (state == State::Finishing) {
        // Close file on streaming thread
        if (file != nullptr && file->isOpen())
            file->close();

        if (samplesInBuffer == 0)
            state = State::Over;
    }

    if (state == State::Over) {
        // Reset file as stream will be returned to pool
        file.reset();
    }
}

void AudioStream::close()
{
    if (sample != nullptr)
        sample->getAudioFile().close();

    state = State::Over;
}

void AudioStream::generateXfadeEnvelope(float k)
{
    const float r{ 1.0f / (float)xfadeEnvelope.getNumFrames() };

    for (int i = 0; i < xfadeBuffer.getNumFrames(); ++i) {
        float a{ r * float (i) };
        float b{ (1.0f - a) };

        a = std::powf(a, k);
        b = std::powf(b, k);

        xfadeEnvelope.getChannelData(0)[i] = a;
        xfadeEnvelope.getChannelData(1)[i] = b;
    }
}

//==============================================================================

AudioStreamPool::AudioStreamPool(int numStreams)
    : streams(numStreams) // This will create streams with default buffer size
{
    for (auto& stream : streams)
        idleStreams.append(&stream);
}

AudioStreamPool::~AudioStreamPool() = default;

AudioStream* AudioStreamPool::getStream()
{
    if (auto* stream{ idleStreams.first() }) {
        idleStreams.remove(stream);
        return stream;
    }

    return nullptr;
}

void AudioStreamPool::returnToIdle(AudioStream* stream)
{
    assert(stream != nullptr);
    idleStreams.append(stream);
}

TW_NAMESPACE_END
