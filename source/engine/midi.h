// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#pragma once

#include "globals.h"

#include <array>
#include <bitset>
#include <functional>

TW_NAMESPACE_BEGIN

/**
 * This class is used to track the MIDI keys
 * state across the keyboard. It captures the keys
 * on/off state and tracks the sustain pedal.
 */
class MidiKeyboardState final
{
public:
    enum KeyState
    {
        KeyIdle        = 0,
        KeyPressed     = 1,
        KeySustained   = 2
    };

    MidiKeyboardState();

    void reset();

    void noteOn(int key);
    void noteOff(int key);
    void sustainOn();
    void sustainOff();

    bool isKeyIdle(int key) const;
    bool isKeyPressed(int key) const;
    bool isKeySustained(int key) const;

    bool isSustainOn() const noexcept;

private:
    bool sustainState{ false };

    constexpr static int totalKeys = 128;
    std::array<KeyState, (size_t)totalKeys> keysState{ KeyIdle };
};

TW_NAMESPACE_END
