#ifndef MMU_H
#define MMU_H

namespace gameboy::emulator
{
    class Cartridge;
    class Joypad;
    class Timer;
    class GPU;
    class CPU;
    class DMA;

    class MMU
    {
    public:
        struct MemPointers
        {
            Cartridge*      cartridge;
            GPU*            gpu;
            CPU*            cpu;
            DMA*            dma;
            Joypad*         joypad;
            Timer*          timer;
            unsigned char* wram;
            unsigned char* hram;
        };
    private:
        unsigned char bank_rom_index, bank_ram_index;
        MemPointers mem_pointers;
        bool ram_enable = false, mode_select = false;
    public:
        void set_mem_pointers(const MemPointers& mem_pointers) noexcept {this->mem_pointers = mem_pointers;}
        void write_byte(unsigned address, unsigned value) noexcept;
        void write_word(unsigned address, unsigned value) noexcept;
        unsigned read_byte(unsigned address) const noexcept;
        unsigned read_word(unsigned address) const noexcept;
    };
}

#endif
