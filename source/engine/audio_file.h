// *****************************************************************************
//
//  Tonewheel Audio Engine
// 
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"
#include "core/error.h"
#include <string>
#include <memory>

TW_NAMESPACE_BEGIN

/**
 * Audio file reader.
 * 
 * This class is used to read samples from audio files.
 * So far it is limited to a known file formats and supports
 * only mono and setreo (2 channels) signals.
 */
class AudioFile final
{
public:

    /**
     * Recognizabe audio formats.
     */
    enum class Format
    {
        Unknown,
        WavPCM,
        OggVorbis,
        Opus
    };

    /**
     * Interface to the audio format decoder.
     */
    class Decoder
    {
    public:
        virtual ~Decoder() = default;
        virtual core::Error open(const std::string& path) = 0;
        virtual void close() = 0;
        virtual bool isOpen() = 0;
        virtual core::Error seek(size_t frame) = 0;
        virtual int read(int numFrames, float* left, float* right) = 0;
        virtual float getSampleRate() const = 0;
        virtual int getNumChannels() const = 0;
    };

    //------------------------------------------------------

    AudioFile() = delete;
    AudioFile(const std::string& filePath, Format fileFormat);
    ~AudioFile();
    AudioFile(const AudioFile&) = delete;
    AudioFile& operator=(const AudioFile&) = delete;

    AudioFile* clone() const;

    const std::string& getPath() const noexcept { return path; }
    Format getFormat() const noexcept { return format; }

    bool isValid() const { return decoder != nullptr; }
    bool isOpen() const { return decoder != nullptr && decoder->isOpen(); }

    core::Error open();
    void close();
    core::Error seek(int frame);
    int read(int numFrames, float* left, float* right);
    float getSampleRate() const noexcept { return sampleRate; }
    int getNumChannels() const noexcept { return numChannels; }

    static Format guessFormatFromFileName(const std::string& filePath);

protected:
    std::string path;
    Format format;
    float sampleRate;
    int numChannels;
    std::unique_ptr<Decoder> decoder;
};

TW_NAMESPACE_END
