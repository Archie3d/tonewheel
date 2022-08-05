// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include <cstddef>

#define TW_NAMESPACE_BEGIN namespace tonewheel {
#define TW_NAMESPACE_END }
#define TW_USING_NAMESPACE using namespace tonewheel;

TW_NAMESPACE_BEGIN

constexpr size_t DEFAULT_RELEASE_POOL_SIZE = 4096;

constexpr float DEFAULT_SAMPLE_RATE_F = 44100.0f;

constexpr int MIX_BUFFER_NUM_CHANNELS = 2;
constexpr int MIX_BUFFER_NUM_FRAMES = 32;

constexpr int DEFAULT_VOICE_POOL_SIZE = 256;
constexpr int DEFAULT_AUDIO_STREAM_POOL_SIZE = 256;

constexpr int MAX_PRELOAD_BUFFER_SIZE = 65536;
constexpr int DEFAULT_STREAM_BUFFER_SIZE = 16384;
constexpr int DEFAULT_XFADE_BUFFER_SIZE = 32;

constexpr int NUM_CC_PARAMETERS = 128;

constexpr int NUM_STREAM_WORKERS = 4;

constexpr int NUM_BUSES = 16;
constexpr int DEFAULT_TRIGGER_BUFFER_SIZE = 1024;
constexpr int DEFAULT_RELEASE_BUFFER_SIZE = 1024;
constexpr int DEFAULT_ACTUATOR_BUFFER_SUZE = 1024;

TW_NAMESPACE_END
