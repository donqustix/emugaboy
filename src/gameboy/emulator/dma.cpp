#include "dma.h"

#include "gpu.h"
#include "mmu.h"

using gameboy::emulator::DMA;

void DMA::tick(unsigned cycles) noexcept
{
    if (enabled)
    {
        proc_clock += cycles;
        if (proc_clock >= 4)
        {
            proc_clock -= 4;
            if (init) init = false;
            else
            {
                gpu->write_oam(dst_address++, mmu->read_byte(src_address++));
                enabled = dst_address != 0xA0;
            }
        }
    }
}

void DMA::enable_transfer(unsigned value) noexcept
{
    dst_address = proc_clock = 0; 
    src_address = value << 8;
    enabled = init = true;
}

