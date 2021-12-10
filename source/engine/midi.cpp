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

MidiMessage::MidiMessage()
    : data{ 0 }
    , timestamp{ 0.0 }
{
}

MidiMessage::MidiMessage(const uint8_t* rawData, size_t size, double time)
    : data{ 0 }
    , timestamp{ time }
{
    for (size_t i = 0; i < std::min(sizeof(data), size); ++i)
        data = (data << 8) | rawData[i];
}

MidiMessage::MidiMessage(uint32_t rawData, double time)
    : data{ rawData }
    , timestamp{ time }
{
}

MidiMessage::MidiMessage(const MidiMessage& other)
    : data{ other.data }
    , timestamp{ other.timestamp }
{
}

MidiMessage& MidiMessage::operator =(const MidiMessage& other)
{
    if (this != &other) {
        data = other.data;
        timestamp = other.timestamp;
    }

    return *this;
}

MidiMessage::Type MidiMessage::getType() const noexcept
{
    switch ((data & 0x00F00000) >> 16) {
        case 0x80: return Type::NoteOff;
        case 0x90: return Type::NoteOn;
        case 0xA0: return Type::PolyphonicAftertouch;
        case 0xB0: return Type::Controller;
        case 0xC0: return Type::ProgramChange;
        case 0xD0: return Type::ChannelAftertouch;
        case 0xE0: return Type::PitchBend;
        case 0xF0: return Type::System;
        default: break;
    }

    return Type::Invalid;
}

bool MidiMessage::isValid() const noexcept
{
    return getType() != Type::Invalid;
}

bool MidiMessage::isNoteOn() const noexcept
{
    return getType() == Type::NoteOn;
}

bool MidiMessage::isNoteOff() const noexcept
{
    return getType() == Type::NoteOff;
}

bool MidiMessage::isController() const noexcept
{
    return getType() == Type::Controller;
}

bool MidiMessage::isPitchBend() const noexcept
{
    return getType() == Type::PitchBend;
}

int MidiMessage::getChannel() const noexcept
{
    return int(((data & 0x000F0000) >> 16) + 1);
}

int MidiMessage::getNoteNumber() const noexcept
{
    return int((data &0x00007F00) >> 8);
}

int MidiMessage::getVelocity() const noexcept
{
    return int(data & 0x7F);
}

float MidiMessage::getVelocityAsFloat() const noexcept
{
    return float(getVelocity()) * (1.0f / 127.0f);
}

int MidiMessage::getPressure() const noexcept
{
    switch (getType()) {
        case Type::PolyphonicAftertouch: return int(data & 0x7F);
        case Type::ChannelAftertouch: return int((data & 0x7F00) >> 8);
        default: break;
    }

    return 0;
}

int MidiMessage::getControllerNumber() const noexcept
{
    return int((data & 0x7F00) >> 8);
}

int MidiMessage::getControllerValue() const noexcept
{
    return int(data & 0x7F);
}

float MidiMessage::getControllerValueAsFloat() const noexcept
{
    return float(getControllerValue()) * (1.0f / 127.0f);
}

int MidiMessage::getProgramNumber() const noexcept
{
    return int((data & 0x7F00) >> 8);
}

int MidiMessage::getPitchBend() const noexcept
{
    uint32_t lsb{ (data & 0x7F00) >> 8 };
    uint32_t msb{ data & 0x7F };

    return (msb << 7) | lsb;
}

float MidiMessage::getPitchBendAsFloat() const noexcept
{
    return float(getPitchBend()) * (1.0f / 16383.0f);
}

//==============================================================================

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
