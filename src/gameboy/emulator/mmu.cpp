#include "mmu.h"

#include "cartridge.h"
#include "joypad.h"
#include "timer.h"
#include "mbcs.h"
#include "gpu.h"
#include "cpu.h"
#include "dma.h"

using gameboy::emulator::MMU;

void MMU::write_byte(unsigned address, unsigned value) noexcept
{
    if (address < 0x8000)
    {
        switch (mem_pointers.cartridge->get_mbc_type())
        {
            case MBCs::MBC1:
                     if (address < 0x2000)     ram_enable =                            value & 0x0A;
                else if (address < 0x4000) bank_rom_index = (bank_rom_index & 0x60) | (value & 0x1F ? value & 0x1F : 1);
                else if (address < 0x6000)
                {
                    if (mode_select) bank_ram_index =                            value & 0x03;
                    else             bank_rom_index = (bank_rom_index & 0x1F) | (value & 0x60);
                }
                else if (address < 0x8000) mode_select = value & 1;
            break;
        }
    }
    else if (address < 0xA000) mem_pointers.gpu->write_ram(address - 0x8000, value);
    else if (address < 0xC000)
    {
        switch (mem_pointers.cartridge->get_mbc_type())
        {
            case MBCs::MBC1:
                if (ram_enable) 
                {
                    if (mode_select) mem_pointers.cartridge->write_ram(address - 0xA000 + bank_ram_index * 0x2000, value);
                    else             mem_pointers.cartridge->write_ram(address - 0xA000,                           value);
                }
            break;
        }
    }
    else if (address < 0xE000) mem_pointers.wram[address - 0xC000] = value;
    else if (address < 0xFE00) mem_pointers.wram[address - 0xE000] = value;
    else if (address < 0xFEA0) mem_pointers.gpu->write_oam_cpu(address - 0xFE00, value);
    else if (address < 0xFF00);
    else if (address < 0xFF80)
        switch (address)
        {
            case 0xFF00: mem_pointers.joypad->write(value);             break;
            case 0xFF04: mem_pointers.timer->internal_counter = 0;      break;
            case 0xFF05: mem_pointers.timer->counter = value;           break;
            case 0xFF06: mem_pointers.timer->modulo = value;            break;
            case 0xFF07: mem_pointers.timer->control = value;           break;
            case 0xFF0F: mem_pointers.cpu->IF = value;                  break;
            case 0xFF40: mem_pointers.gpu->write_lcd_control(value);    break;
            case 0xFF41: mem_pointers.gpu->write_lcd_stat(value);       break;
            case 0xFF42: mem_pointers.gpu->scy = value;                 break;
            case 0xFF43: mem_pointers.gpu->scx = value;                 break;
            case 0xFF44: mem_pointers.gpu->ly = 0;                      break;
            case 0xFF45: mem_pointers.gpu->lyc = value;                 break;
            case 0xFF46: mem_pointers.dma->enable_transfer(value);      break;
            case 0xFF47: mem_pointers.gpu->bgp = value;                 break;
            case 0xFF48: mem_pointers.gpu->obp0 = value;                break;
            case 0xFF49: mem_pointers.gpu->obp1 = value;                break;
            case 0xFF4A: mem_pointers.gpu->wy = value;                  break;
            case 0xFF4B: mem_pointers.gpu->wx = value;                  break;
        }
    else if (address < 0xFFFF) mem_pointers.hram[address - 0xFF80] = value;
    else mem_pointers.cpu->IE = value;
}

void MMU::write_word(unsigned address, unsigned value) noexcept
{
    write_byte(address    , value            );
    write_byte(address + 1, value >> 8 & 0xFF);
}

unsigned MMU::read_byte(unsigned address) const noexcept
{
    if (address < 0x8000)
    {
        switch (mem_pointers.cartridge->get_mbc_type())
        {
            case MBCs::MBC1: 
                if (address > 0x3FFF)
                {
                    if (mode_select) return mem_pointers.cartridge->read_rom(address - 0x4000 + (bank_rom_index & 0x1F) * 0x4000);
                    else             return mem_pointers.cartridge->read_rom(address - 0x4000 +  bank_rom_index         * 0x4000);
                }
            break;
        }
        return mem_pointers.cartridge->read_rom(address);
    }
    else if (address < 0xA000) return mem_pointers.gpu->read_ram(address - 0x8000);
    else if (address < 0xC000)
    {
        switch (mem_pointers.cartridge->get_mbc_type())
        {
            case MBCs::MBC1:
                if (ram_enable) 
                {
                    if (mode_select) return mem_pointers.cartridge->read_ram(address - 0xA000 + bank_ram_index * 0x2000);
                    else             return mem_pointers.cartridge->read_ram(address - 0xA000);
                }
            break;
        }
        return 0xFF;
    }
    else if (address < 0xE000) return mem_pointers.wram[address - 0xC000];
    else if (address < 0xFE00) return mem_pointers.wram[address - 0xE000];
    else if (address < 0xFEA0) return mem_pointers.gpu->read_oam(address - 0xFE00);
    else if (address < 0xFF00) return 0;
    else if (address < 0xFF80)
        switch (address)
        {
            case 0xFF00: return mem_pointers.joypad->read();
            case 0xFF04: return mem_pointers.timer->read_divider();
            case 0xFF05: return mem_pointers.timer->counter;
            case 0xFF06: return mem_pointers.timer->modulo;
            case 0xFF07: return mem_pointers.timer->control & 0x07;
            case 0xFF0F: return mem_pointers.cpu->IF        | 0xE0;
            case 0xFF40: return mem_pointers.gpu->control;
            case 0xFF41: return mem_pointers.gpu->stat      | 0x80;
            case 0xFF42: return mem_pointers.gpu->scy;
            case 0xFF43: return mem_pointers.gpu->scx;
            case 0xFF44: return mem_pointers.gpu->ly;
            case 0xFF45: return mem_pointers.gpu->lyc;
            case 0xFF47: return mem_pointers.gpu->bgp;
            case 0xFF48: return mem_pointers.gpu->obp0;
            case 0xFF49: return mem_pointers.gpu->obp1;
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

