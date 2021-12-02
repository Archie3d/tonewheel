// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "../globals.h"
#include "aligned_memory.h"
#include <cassert>

TW_NAMESPACE_BEGIN

namespace core {

template <typename SampleType = float, size_t Align = 32>
class AudioBuffer final
{
public:

    static constexpr size_t alignment = Align;
    using Memory = AlignedMemory<Align>;

    AudioBuffer(int numChannels, int numFrames, SampleType* preallocatedData = nullptr)
        : nChannels{ 0 }
        , nFrames{ 0 }
        , dataPtr{ preallocatedData }
        , allocatedData(nullptr, &Memory::free)
    {
        if (preallocatedData == nullptr) {
            allocate(numChannels, numFrames);
        } else {
            nChannels = numChannels;
            nFrames = numFrames;
        }

        assert(dataPtr != nullptr);
    }

    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer& operator=(const AudioBuffer&) = delete;

    ~AudioBuffer() = default;

    AudioBuffer(AudioBuffer&& other)
        : nChannels{ std::move(other.nChannels) }
        , nFrames{ std::move(other.nFrames) }
        , dataPtr{ std::move(other.dataPtr) }
        , allocatedData{ std::move(other.allocatedData) }
    {
        assert(dataPtr != nullptr);
    }
    
    void allocate(int numChannels, int numFrames)
    {
        assert(numChannels > 0);
        assert(numFrames > 0);

        if (nChannels != numChannels || nFrames != numFrames)
        {
            allocatedData.reset((SampleType*) Memory::alloc(sizeof(SampleType) * numChannels * numFrames));
            dataPtr = allocatedData.get();
            nChannels = numChannels;
            nFrames = numFrames;
        }

        assert(dataPtr != nullptr);
    }

    const SampleType* data() const noexcept { return dataPtr; }
    SampleType* data() noexcept { return dataPtr; }

    int getNumChannels() const noexcept { return nChannels; }
    int getNumFrames() const noexcept { return nFrames; }

    const SampleType* getChannelData(int channel) const
    {
        assert(channel >= 0 && channel < nChannels);
        return &dataPtr[channel * nFrames];
    }

    SampleType* getChannelData(int channel)
    {
        assert(channel >= 0 && channel < nChannels);
        return &dataPtr[channel * nFrames];
    }

    void clear()
    {
        assert(dataPtr != nullptr);
        ::memset(dataPtr, 0, sizeof(SampleType) * nChannels * nFrames);
    }

    void fill(SampleType value)
    {
        assert(dataPtr != nullptr);
        std::fill_n(dataPtr, nChannels * nFrames, value);
    }

    void mix(const AudioBuffer<SampleType>& other)
    {
        assert(other.nChannels == nChannels);
        assert(other.nFrames == nFrames);

        // @todo Vectorize
        for (int i = 0; i < nChannels * nFrames; ++i)
            dataPtr[i] += other.dataPtr[i];
    }

    void mix(const AudioBuffer<SampleType>& other, SampleType gain)
    {
        assert(other.nChannels == nChannels);
        assert(other.nFrames == nFrames);

        // @todo Vectorize
        for (int i = 0; i < nChannels * nFrames; ++i)
            dataPtr[i] += other.dataPtr[i] * gain;
    }

private:
    int nChannels;
    int nFrames;
    SampleType* dataPtr;
    std::unique_ptr<SampleType, decltype(&Memory::free)> allocatedData;
};

} // namespace core

TW_NAMESPACE_END
