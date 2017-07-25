#include "timer.h"

using gameboy::emulator::Timer;

unsigned Timer::tick(unsigned cycles) noexcept
{
    internal_counter += cycles;
    internal_counter &= 0xFFFF;

    unsigned interrupts = 0;
    if (control & CONTROL_MASK_TIMER_ENABLE)
    {
        counter_clock += cycles;

        static constexpr unsigned clocks[]{256, 4, 16, 64};
        const auto cclock = clocks[control & CONTROL_MASK_INPUT_CLOCK_SELECT];

        if (counter_clock >= cclock)
        {
            const unsigned elapsed = 1 + (counter_clock - cclock) / 4;
            counter_clock %= cclock;
            const unsigned temp = counter + elapsed;
            counter = temp;
            if (temp > 255)
            {
                counter    = modulo;
                interrupts = INTERRUPT_MASK;
            }
        }
    }
    return interrupts;
}

