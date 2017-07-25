#include "cartridge.h"
#include "mbcs.h"

#include <stdexcept>
#include <fstream>
#include <iterator>
#include <iostream>

using gameboy::emulator::Cartridge;

Cartridge Cartridge::load(std::string_view filepath)
{
    std::ifstream stream{filepath.data(), std::ios::in | std::ios::binary};
    if (!stream)
        throw std::runtime_error{"file reading error"};

    std::vector<unsigned char> rom{std::istreambuf_iterator<char>{stream},
                                   std::istreambuf_iterator<char>{}};
    std::clog << "MBC type: " << (unsigned) rom[0x0147] << std::endl;
    std::clog << "ROM size: " << (unsigned) rom[0x0148] << std::endl;

    return Cartridge{std::move(rom)};
}

Cartridge::Cartridge(std::vector<unsigned char> rom) noexcept : rom{std::move(rom)}
{
    switch (this->rom[0x0147]) {
        case 1:
        case 2:
        case 3:
            ram.resize(32768); type = MBCs::MBC1;
        break;
    }
}

