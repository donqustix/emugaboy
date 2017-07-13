#ifndef DMA_H
#define DMA_H

namespace gameboy::emulator
{
    class GPU;
    class MMU;

    class DMA
    {
        GPU* gpu;
        MMU* mmu;
        unsigned src_address, dst_address;
        unsigned proc_clock;
        bool enabled = false, init;
    public:
        DMA(GPU* gpu, MMU* mmu) noexcept : gpu{gpu}, mmu{mmu} {}
        void tick(unsigned cycles) noexcept;
        void enable_transfer(unsigned value) noexcept;
    };
}

#endif
