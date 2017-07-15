#include "mmu.h"

#include "cartridge.h"
#include "gpu.h"
#include "cpu.h"
#include "dma.h"

using gameboy::emulator::MMU;

void MMU::write_byte(unsigned address, unsigned value) const noexcept
{
         if (address < 0x8000);
    else if (address < 0xA000) mem_pointers.gpu->vram[address - 0x8000] = value;
    else if (address < 0xC000) mem_pointers.cartridge->write_ram(address - 0xA000, value);
    else if (address < 0xE000) mem_pointers.wram[address - 0xC000] = value;
    else if (address < 0xFE00) mem_pointers.wram[address - 0xE000] = value;
    else if (address < 0xFEA0) mem_pointers.gpu->oam[address - 0xFE00] = value;
    else if (address < 0xFF00);
    else if (address < 0xFF80)
        switch (address)
        {
            case 0xFF0F: mem_pointers.cpu->IF = value;                  break;
            case 0xFF40: mem_pointers.gpu->write_lcd_control(value);    break;
            case 0xFF41: mem_pointers.gpu->stat = value;                break;
            case 0xFF42: mem_pointers.gpu->scy = value;                 break;
            case 0xFF43: mem_pointers.gpu->scx = value;                 break;
            case 0xFF44: mem_pointers.gpu->ly = 0;                      break;
            case 0xFF45: mem_pointers.gpu->lyc = value;                 break;
            case 0xFF46: mem_pointers.dma->enable_transfer(value);      break;
            case 0xFF4A: mem_pointers.gpu->wy = value;                  break;
            case 0xFF4B: mem_pointers.gpu->wx = value;                  break;
        }
    else if (address  < 0xFFFF) mem_pointers.hram[address - 0xFF80] = value;
    else mem_pointers.cpu->IE = value;
}

void MMU::write_word(unsigned address, unsigned value) const noexcept
{
    write_byte(address    , value            );
    write_byte(address + 1, value >> 8 & 0xFF);
}

unsigned MMU::read_byte(unsigned address) const noexcept
{
         if (address < 0x8000) return mem_pointers.cartridge->read_rom(address);
    else if (address < 0xA000) return mem_pointers.cartridge->read_ram(address - 0x8000);
    else if (address < 0xC000) return mem_pointers.cartridge->read_ram(address - 0xA000);
    else if (address < 0xE000) return mem_pointers.wram[address - 0xC000];
    else if (address < 0xFE00) return mem_pointers.wram[address - 0xE000];
    else if (address < 0xFEA0) return mem_pointers.gpu->oam[address - 0xFE00];
    else if (address < 0xFF00) return 0;
    else if (address < 0xFF80)
        switch (address)
        {
            case 0xFF0F: return mem_pointers.cpu->IF        | 0xE0;
            case 0xFF40: return mem_pointers.gpu->control;
            case 0xFF41: return mem_pointers.gpu->stat      | 0x80;
            case 0xFF42: return mem_pointers.gpu->scy;
            case 0xFF43: return mem_pointers.gpu->scx;
            case 0xFF44: return mem_pointers.gpu->ly;
            case 0xFF45: return mem_pointers.gpu->lyc;
            case 0xFF4A: return mem_pointers.gpu->wy;
            case 0xFF4B: return mem_pointers.gpu->wx;
            default:
                return 0;
        }
    else if (address < 0xFFFF) return mem_pointers.hram[address - 0xFF80];
    else return mem_pointers.cpu->IE;
}

unsigned MMU::read_word(unsigned address) const noexcept
{
    return read_byte(address) | read_byte(address + 1) << 8;
}

