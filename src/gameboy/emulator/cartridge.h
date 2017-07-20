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

        static MBCs mbc_type(const std::vector<unsigned char>& rom) noexcept {return static_cast<MBCs>(rom[0x0147]);}

    public:
        static Cartridge load(std::string_view filepath);

        Cartridge(std::vector<unsigned char> rom,
                  std::vector<unsigned char> ram) noexcept;

        void write_ram(unsigned index, unsigned value) noexcept {if (ram.size()) {ram[index] = value;}}
        MBCs get_mbc_type() const noexcept {return mbc_type(rom);}

        unsigned char read_rom(unsigned index) const noexcept {return rom[index];}
        unsigned char read_ram(unsigned index) const noexcept {return ram[index];}
    };
}

#endif
