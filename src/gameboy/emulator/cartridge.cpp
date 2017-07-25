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
    std::clog << (unsigned)rom[0x0147] << std::endl;
    std::vector<unsigned char> ram;
    switch (mbc_type(rom)) {
        case MBCs::MBC1: ram.resize(32768); break;
    }
    return {std::move(rom), std::move(ram)};
}

Cartridge::Cartridge(std::vector<unsigned char> rom,
                     std::vector<unsigned char> ram) noexcept :
    rom{std::move(rom)}, ram{std::move(ram)}
{
}

