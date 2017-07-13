#ifndef MMU_H
#define MMU_H

namespace gameboy::emulator
{
    class Cartridge;
    class GPU;
    class CPU;
    class DMA;

    class MMU
    {
    public:
        struct MemPointers
        {
            Cartridge* cartridge;
            GPU*       gpu;
            CPU*       cpu;
            DMA*       dma;
            unsigned char* wram;
            unsigned char* hram;
        };
    private:
        MemPointers mem_pointers;
        unsigned char bank_rom_index = 1;
        unsigned char bank_ram_index = 0;
    public:
        void set_mem_pointers(const MemPointers& mem_pointers) noexcept {this->mem_pointers = mem_pointers;}
        void write_byte(unsigned address, unsigned value) const noexcept;
        void write_word(unsigned address, unsigned value) const noexcept;
        unsigned read_byte(unsigned address) const noexcept;
        unsigned read_word(unsigned address) const noexcept;
    };
}

#endif
