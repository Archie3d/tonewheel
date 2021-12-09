// *****************************************************************************
//
//  Tonewheel Audio Engine
//
//  Copyright (C) 2021 Arthur Benilov <arthur.benilov@gmail.com>
//
// *****************************************************************************

#include "midi.h"
#include <cassert>

TW_NAMESPACE_BEGIN

MidiKeyboardState::MidiKeyboardState()
{
}

void MidiKeyboardState::reset()
{
    sustainState = false;

    for (auto& s : keysState)
        s = KeyIdle;
}

void MidiKeyboardState::noteOn(int key)
{
    assert(key >= 0 && key < totalKeys);

    keysState[key] = KeyPressed;
}

void MidiKeyboardState::noteOff(int key)
{
    assert(key >= 0 && key < totalKeys);

    keysState[key] = sustainState ? KeySustained : KeyIdle;
}

void MidiKeyboardState::sustainOn()
{
    sustainState = true;
}

void MidiKeyboardState::sustainOff()
{
    // Turn-off all sustained keys while keeping all the pressed keys
    for (auto& s : keysState) {
        if (s == KeySustained)
            s = KeyIdle;
    }
}

bool MidiKeyboardState::isKeyIdle(int key) const
{
    assert(key >= 0 && key < totalKeys);

    return keysState[key] == KeyIdle;
}

bool MidiKeyboardState::isKeyPressed(int key) const
{
    assert(key >= 0 && key < totalKeys);

    return keysState[key] == KeyPressed;
}

bool MidiKeyboardState::isKeySustained(int key) const
{
    assert(key >= 0 && key < totalKeys);

    return keysState[key] == KeySustained;
}

bool MidiKeyboardState::isSustainOn() const noexcept
{
    return sustainState;
}

TW_NAMESPACE_END
