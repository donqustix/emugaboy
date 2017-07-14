#include "dma.h"

#include "gpu.h"
#include "mmu.h"

using gameboy::emulator::DMA;

void DMA::tick(unsigned cycles) noexcept
{
    if (enabled)
    {
        while (cycles --> 0)
        {
            if (init) init = false;
            else
            {
                gpu->write_oam(dst_address++, mmu->read_byte(src_address++));
                if (dst_address == 0xA0) enabled = false;
            }
        }
    }
}

void DMA::enable_transfer(unsigned value) noexcept
{
    src_address = value << 8;
    dst_address = 0; 
    enabled = init = true;
}

