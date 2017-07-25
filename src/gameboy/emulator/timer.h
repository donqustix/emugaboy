#ifndef TIMER_H
#define TIMER_H

namespace gameboy::emulator
{
    class Timer
    {
        friend class MMU;

        static constexpr unsigned INTERRUPT_MASK = 0b00100;
        enum ControlMasks {
            CONTROL_MASK_INPUT_CLOCK_SELECT     = 3,
            CONTROL_MASK_TIMER_ENABLE           = 4
        };

        unsigned char counter = 0, modulo = 0, control = 0;
        unsigned internal_counter = 0xABCC;
        unsigned counter_clock = 0;

    public:
        unsigned tick(unsigned cycles) noexcept;
        unsigned read_divider() const noexcept {return internal_counter >> 8;}
    };
}

#endif
