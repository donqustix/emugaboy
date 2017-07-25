#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <string_view>
#include <vector>

namespace gameboy::emulator
{
    enum class MBCs;

    class Cartridge
    {
        std::vector<unsigned char> rom;
        std::vector<unsigned char> ram;
        MBCs type;

    public:
        static Cartridge load(std::string_view filepath);

        explicit Cartridge(std::vector<unsigned char> rom) noexcept;

        void write_ram(unsigned index, unsigned value) noexcept {ram[index] = value;}
        MBCs get_mbc_type() const noexcept {return type;}

        unsigned char read_rom(unsigned index) const noexcept {return rom[index];}
        unsigned char read_ram(unsigned index) const noexcept {return ram[index];}
    };
}

#endif
