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

TW_NAMESPACE_BEGIN

class MidiMessage
{
public:

    enum class Type
    {
        Invalid,
        NoteOff,
        NoteOn,
        PolyphonicAftertouch,
        Controller,
        ProgramChange,
        ChannelAftertouch,
        PitchBend,
        System
    };

    MidiMessage();
    MidiMessage(const uint8_t* rawData, size_t size, double time = 0.0);
    MidiMessage(const uint32_t rawData, double time = 0.0);
    MidiMessage(const MidiMessage& other);
    MidiMessage& operator =(const MidiMessage& other);

    Type getType() const noexcept;
    bool isValid() const noexcept;

    bool isNoteOn() const noexcept;
    bool isNoteOff() const noexcept;
    bool isController() const noexcept;
    bool isPitchBend() const noexcept;
    int getChannel() const noexcept;
    int getNoteNumber() const noexcept;
    int getVelocity() const noexcept;
    float getVelocityAsFloat() const noexcept;
    int getPressure() const noexcept;
    int getControllerNumber() const noexcept;
    int getControllerValue() const noexcept;
    float getControllerValueAsFloat() const noexcept;
    int getProgramNumber() const noexcept;
    int getPitchBend() const noexcept;
    float getPitchBendAsFloat() const noexcept;

private:
    uint32_t data{};
    double timestamp{};
};

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
