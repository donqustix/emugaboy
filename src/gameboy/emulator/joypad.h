#ifndef JOYPAD_H
#define JOYPAD_H

namespace gameboy::emulator
{
    class Joypad
    {
        unsigned char reg = 0x0F, keys_maks = 0xFF;

    public:
        void push_key_states(unsigned mask) noexcept {keys_maks = ~mask;}
        void write(unsigned value) noexcept {reg = value & 0x30;}
        unsigned read() noexcept {return reg | (keys_maks >> ((~reg >> 4 & 3) * 4 - 4) & 15) | 0xC0;}
    };
}

#endif
