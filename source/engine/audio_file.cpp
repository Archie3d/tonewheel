// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "audio_file.h"
#include "core/string_utils.h"
#include "vorbis/vorbisfile.h"
#if TONEWHEEL_WITH_OPUS
#   include "opusfile.h"
#endif
#include <fstream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>

TW_NAMESPACE_BEGIN

//==============================================================================

namespace errors {

const static core::Error invalidDecoder("Invalid audio file format decoder");
const static core::Error invalidFormat("Invalid audio file format");
const static core::Error failedToOpen("Failed to open file");
const static core::Error invalidFile("Invalid file");
const static core::Error invalidHeader("File format header is invalid");
const static core::Error unexpectedEof("Unexpected end of file");
const static core::Error unsupportedChannelsCount("Unsupported channels count");
const static core::Error unsupportedSampleFormat("Unsupported sample format");
const static core::Error outOfRange("Out of range");

} // namespace errors

//==============================================================================

namespace decoder {

/**
 * Decoder for uncompressed WAV PCM file format.
 */
struct WavPCM : public AudioFile::Decoder
{
    enum Chunk
    {
        Chunk_RiffHeader      = 0x46464952,
        Chunk_WavRiff         = 0x54651475,
        Chunk_Format          = 0x20746D66,
        Chunk_LabeledText     = 0x478747C6,
        Chunk_Instrumentation = 0x478747C6,
        Chunk_Sample          = 0x6C706D73,
        Chunk_Fact            = 0x47361666,
        Chunk_Data            = 0x61746164,
        Chunk_Junk            = 0x4B4E554A
    };

    enum Format
    {
        Format_PCM                = 0x01,
        Format_FloatingPoint      = 0x03,
        Format_ALaw               = 0x06,
        Format_MuLaw              = 0x07,
        Format_IMAADPCM           = 0x11,
        Format_YamahaITUG723ADPCM = 0x16,
        Format_GSM610             = 0x31,
        Format_ITUG721ADPCM       = 0x40,
        Format_MPEG               = 0x50,
        Format_Extensible         = 0xFFFE
    };

    constexpr static uint32_t RIFF = 0x45564157;

    std::unique_ptr<std::ifstream> file{ nullptr };
    Format format{ Format_PCM };
    int nChannels{ 0 };
    int sRate{ 0 };
    int blockAlign{ 0 };
    int bitsPerSample{ 0 };
    size_t dataChunkPos{ 0 };
    size_t numberOfSamples{ 0 };
    size_t readSamplePos{ 0 };

    // Raw data read buffer
    std::vector<uint8_t> buffer{};

    core::Error open (const std::string& path) override
    {
        file.reset(new std::ifstream(path, std::ios::in | std::ios::binary));
        if (!file->is_open())
            return errors::failedToOpen;

        auto res{ readHeader() };

        if (res.failed())
            return res;

        file->seekg(dataChunkPos);
        readSamplePos = 0;

        if (format != Format_PCM)
            return errors::invalidFormat;

        if (nChannels != 1 && nChannels != 2)
            return errors::unsupportedChannelsCount;

        if (bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 24)
            return errors::unsupportedSampleFormat;

        return {};
    }

    void close() override
    {
        if (file != nullptr && file->is_open()) {
            file->close();
            file.reset();
        }
    }

    bool isOpen() override
    {
        return file != nullptr && file->is_open();
    }

    core::Error seek(size_t frame) override
    {
        if (file == nullptr || ! file->is_open())
            return errors::invalidFile;

        if (frame >= numberOfSamples)
            return errors::outOfRange;

        const size_t pos{ dataChunkPos + frame * (bitsPerSample / 8) * nChannels };
        file->seekg(pos);

        readSamplePos = frame;

        return {};
    }

    int read(int nFrames, float* left, float* right) override
    {
        size_t n = std::min((size_t)nFrames, availableSamples());
        float* out[] = {left, right};

        switch (nChannels) {
            // Mono
            case 1:
            {
                switch (bitsPerSample)
                {
                case 8:
                    n = readAsFloats<1, 1>(out, n);
                    break;
                case 16:
                    n = readAsFloats<2, 1>(out, n);
                    break;
                case 24:
                    n = readAsFloats<3, 1>(out, n);
                    break;
                default:
                    return 0;
                }

                ::memcpy (right, left, sizeof (float) * n);
                break;
            }
            // Stereo
            case 2:
            {
                switch (bitsPerSample)
                {
                case 8:
                    n = readAsFloats<1, 2>(out, n);
                    break;
                case 16:
                    n = readAsFloats<2, 2>(out, n);
                    break;
                case 24:
                    n = readAsFloats<3, 2>(out, n);
                    break;
                default:
                    return 0;
                }

                break;
            }
            default:
                return 0;
        }

        readSamplePos += n;

        return (int) n;
    }

    float getSampleRate() const override
    {
        return (float)sRate;
    }

    int getNumChannels() const override
    {
        return nChannels;
    }

private:

    core::Error readHeader()
    {
        assert(file != nullptr);
        assert(file->is_open());

        // Make sure we're at the start of the file before reading the header.
        file->seekg(0);

        uint32_t chunkId = 0;
        uint32_t chunkSize = 0;
        bool foundData = false;

        // Reading up to the DATA chunk
        while (!foundData) {
            if (file->eof())
                return errors::unexpectedEof;

            file->read((char*)&chunkId, sizeof(chunkId));
            file->read((char*)&chunkSize, sizeof(chunkId));

            switch (static_cast<Chunk>(chunkId)) {
                case Chunk_RiffHeader:
                {
                    uint32_t wav = 0;
                    file->read((char*) &wav, sizeof (wav));

                    if (wav != RIFF)
                        return errors::invalidFormat;

                    break;
                }
                case Chunk_Format:
                {
                    uint16_t fmt = 0;
                    uint16_t ch;    // number of channels
                    uint32_t sr;    // sample rate
                    uint32_t br;    // byte rate
                    uint16_t ba;    // block align
                    uint16_t bps;   // bits per sample

                    file->read((char*)&fmt, sizeof(fmt));
                    file->read((char*)&ch, sizeof(ch));
                    file->read((char*)&sr, sizeof(sr));
                    file->read((char*)&br, sizeof(br));
                    file->read((char*)&ba, sizeof(ba));
                    file->read((char*)&bps, sizeof(bps));

                    format = static_cast<Format>(fmt);
                    nChannels = ch;
                    sRate = sr;
                    blockAlign = ba;
                    bitsPerSample = bps;

                    if (ba != ch * bps / 8)
                        return errors::invalidFormat;

                    if (chunkSize > 0x10)
                        file->seekg(chunkSize - 0x10, std::ios_base::cur);

                    break;
                }
                case Chunk_Data:
                {
                    foundData = true;
                    numberOfSamples = chunkSize / blockAlign;
                    dataChunkPos = file->tellg();
                    break;
                }
                default:
                    // Skip unknown chunk
                    file->seekg((size_t)file->tellg() + (size_t) chunkSize);
                    break;
            }
        }

        if (!foundData)
            return errors::invalidFormat;

        return {};
    }

    size_t availableSamples()
    {
        return numberOfSamples - readSamplePos;
    }

    template<int BytesPerSample>
    inline static float convert (uint8_t* data)
    {
        constexpr int maxValue = 1 << (BytesPerSample * 8);
        constexpr int midValue = maxValue >> 1;
        constexpr float norm = 1.0f / (float) midValue;

        int value = 0;

        if constexpr (BytesPerSample == 1)
            value = (int) *data - midValue;
        else if constexpr (BytesPerSample == 2)
            value = (int) *((int16_t*) data);
        else if constexpr (BytesPerSample == 3)
        {
            value = int (data[0]) | (int (data[1]) << 8) | ((int (data[2]) << 16));

            // Restore sign
            if ((value & 0x800000) != 0)
                value |= 0xFF000000;
        }

        return float (value) * norm;
    }

    template<int BytesPerSample, int Channels>
    size_t readAsFloats (float** out, size_t n)
    {
        size_t bufferSize = n * BytesPerSample * Channels;

        // Grow the buffer if needed.
        if (buffer.size() < bufferSize)
            buffer.resize (bufferSize);

        file->read ((char*) buffer.data(), bufferSize);
        auto readBytes = file->gcount();
        size_t readSamples = readBytes / BytesPerSample / Channels;

        size_t bufferIdx = 0;

        for (size_t i = 0; i < readSamples; ++i)
        {
            for (int c = 0; c < Channels; ++c)
            {
                out[c][i] = convert<BytesPerSample> (&buffer.data()[bufferIdx]);
                bufferIdx += BytesPerSample;
            }
        }

        return readSamples;
    }

};

//==============================================================================

struct OggVorbis : public AudioFile::Decoder
{
    OggVorbis_File vorbisFile{};
    bool fileIsOpen{ false };
    float sampleRate{};

    core::Error open(const std::string& path) override
    {
        const auto res{ ov_fopen(path.c_str(), &vorbisFile) };

        fileIsOpen = false;

        if (res != 0)
            return core::Error("Failed to open file");

        fileIsOpen = true;
        sampleRate = (float)vorbisFile.vi->rate;

        return {};
    }

    void close() override
    {
        if (fileIsOpen) {
            ov_clear (&vorbisFile);
            fileIsOpen = false;
        }
    }

    bool isOpen() override
    {
        return fileIsOpen;
    }

    core::Error seek(size_t frame) override
    {
        assert(fileIsOpen);

        const auto res{ ov_pcm_seek(&vorbisFile, frame) };

        if (res != 0)
            return core::Error("File seek failed");

        return {};
    }

    int read(int nFrames, float* left, float* right) override
    {
        assert(fileIsOpen);

        int framesRead{ 0 };

        float** pcm{ nullptr };
        int currentSection{ 0 };
        int framesRemained{ nFrames - framesRead };

        while (framesRemained > 0) {
            auto currentFrames{ ov_read_float(&vorbisFile, &pcm, framesRemained, &currentSection) };

            if (currentFrames <= 0)
                break;

            ::memcpy(&left[framesRead], pcm[0], sizeof(float) * currentFrames);
            ::memcpy(&right[framesRead], pcm[vorbisFile.vi->channels < 2 ? 0 : 1], sizeof(float) * currentFrames);

            framesRead += currentFrames;
            framesRemained -= currentFrames;
        }

        return framesRead;
    }

    float getSampleRate() const override
    {
        return sampleRate;
    }

    int getNumChannels() const override
    {
        return vorbisFile.vi->channels;
    }
};

//==============================================================================

#if TONEWHEEL_WITH_OPUS

struct Opus : public AudioFile::Decoder
{
    OggOpusFile* opusFile = nullptr;

    Opus()
    {
    }

    ~Opus()
    {
        close();
    }

    core::Error open(const std::string& path) override
    {
        close();

        int err{};
        opusFile = op_open_file(path.c_str(), &err);

        if (opusFile == nullptr)
            return core::Error("Failed to open opus file");

        return {};
    }

    void close() override
    {
        if (opusFile != nullptr)
        {
            op_free(opusFile);
            opusFile = nullptr;
        }
    }

    bool isOpen() override
    {
        return opusFile != nullptr;
    }

    core::Error seek(size_t frame) override
    {
        assert(opusFile != nullptr);

        if (frame > 0) {
            constexpr size_t offset = 2048;
            size_t frameOffset = 0;

            if (frame > offset)
                frameOffset = frame - offset;

            size_t scratchFrames = frame - frameOffset;
            const auto res = op_pcm_seek(opusFile, frameOffset);

            if (res != 0)
                return core::Error("Opus seek failed");

            const auto scratchRead = read((int)scratchFrames, nullptr, nullptr);
            if (scratchRead != scratchFrames)
                return core::Error("Opus read failed");

            return {};
        }

        if (op_pcm_seek(opusFile, frame) != 0)
            return core::Error("Opus seek failed");

        return {};
    }

    int read(int nFrames, float* left, float* right) override
    {
        // @todo
        return 0;
    }

    float getSampleRate() const override
    {
        // @todo
        return 48000.0f;
    }

    int getNumChannels() const override
    {
        // @todo
        return 2;
    }
};

#endif // TONEWHEEL_WITH_OPUS

} // namespace decoder

//==============================================================================

AudioFile::AudioFile(const std::string& filePath, Format fileFormat)
    : path{ filePath }
    , format{ fileFormat }
    , decoder{ nullptr }
{
    // Create decoder for given file format
    switch (fileFormat) {
        case Format::WavPCM:
            decoder = std::make_unique<decoder::WavPCM>();
            break;
        case Format::OggVorbis:
            decoder = std::make_unique<decoder::OggVorbis>();
            break;
#if TONEWHEEL_WITH_OPUS
        case Format::Opus:
            decoder = std::make_unique<decoder::Opus>();
            break;
#endif
        default:
            break;
    }
}

AudioFile::~AudioFile()
{
    close();
}

AudioFile* AudioFile::clone() const
{
    return new AudioFile(path, format);
}

core::Error AudioFile::open()
{
    if (decoder == nullptr)
        return errors::invalidDecoder;

    auto res{ decoder->open(path) };

    if (res.failed())
        return res;

    sampleRate = decoder->getSampleRate();
    numChannels = decoder->getNumChannels();

    return {};
}

void AudioFile::close()
{
    if (decoder != nullptr) {
        decoder->close();
        decoder.reset();
    }
}

core::Error AudioFile::seek(int frame)
{
    assert(decoder != nullptr);

    if (decoder == nullptr)
        return errors::invalidDecoder;

    return decoder->seek(frame);
}

int AudioFile::read(int numFrames, float* left, float* right)
{
    assert(decoder != nullptr);

    if (decoder == nullptr)
        return 0;

    return decoder->read(numFrames, left, right);
}

AudioFile::Format AudioFile::guessFormatFromFileName(const std::string& path)
{
    const auto lowerCasePath{ core::str::toLower(path) };

    if (core::str::endsWith(lowerCasePath, ".wav"))
        return Format::WavPCM;

    if (core::str::endsWith(lowerCasePath, ".ogg"))
        return Format::OggVorbis;

#if TONEWHEEL_WITH_OPUS
    if (core::str::endsWith(lowerCasePath, ".opus"))
        return Format::Opus;
#endif

    return Format::Unknown;
}

TW_NAMESPACE_END
