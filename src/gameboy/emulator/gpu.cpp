#include "gpu.h"

#include <iostream>

using gameboy::emulator::GPU;

void GPU::scanline_background() noexcept
{
}

void GPU::scanline_sprites() noexcept
{
}

void GPU::scanline() noexcept
{
    const unsigned bg_tiles_address = control & CONTROL_MASK_BG_WINDOW_TILE_DATA_SELECT ? 0x8000 : 0x8800;
    const unsigned bg_map_address   = control & CONTROL_MASK_BG_TILE_MAP_DISPLAY_SELECT ? 0x9C00 : 0x9800;

    for (int i = 0; i < 160; ++i)
    {
        const unsigned tile_address = 2;

        const unsigned pixel = 2;

        const unsigned pixel_mask = pixel << (6 - (i % 4) * 2);
        const int index = (i + ly * 160) / 8 * 2;

        framebuffer[index] &= ~pixel_mask;
        framebuffer[index] |=  pixel_mask;
    }
}

void GPU::write_lcd_control(unsigned value) noexcept
{
    if ((control = value) & CONTROL_MASK_LCD_DISPLAY_ENABLE) {
        stat = (stat & ~STAT_MASK_MODE_FLAG) | STAT_MODE_READ_OAM;   mode_clock = 0;
    }
    else {
        stat &= ~(STAT_MASK_MODE_FLAG | STAT_MASK_COINCIDENCE_FLAG);         ly = 0;
    }
}

unsigned GPU::tick(unsigned cycles) noexcept
{
    unsigned interrupts = 0;
    if (control & CONTROL_MASK_LCD_DISPLAY_ENABLE)
    {
        mode_clock += cycles;
        switch (stat & STAT_MASK_MODE_FLAG)
        {
            case STAT_MODE_HBLANK:
                if (mode_clock >= 51)
                {
                    mode_clock -= 51;
                    if (++ly == 144)
                    {
                        stat = (stat & ~STAT_MASK_MODE_FLAG) | STAT_MODE_VBLANK;
                        interrupts |= INTERRUPT_MASK_VBLANK;
                        if (stat & STAT_MASK_VBLANK_INTERRUPT)
                            interrupts |= INTERRUPT_MASK_LCDC_STATUS;
                    }
                    else
                    {
                        stat = (stat & ~STAT_MASK_MODE_FLAG) | STAT_MODE_READ_OAM;
                        if (stat & STAT_MASK_OAM_INTERRUPT)
                            interrupts |= INTERRUPT_MASK_LCDC_STATUS;
                    }
                }
                break;
            case STAT_MODE_VBLANK:
                if (mode_clock >= 114)
                {
                    mode_clock -= 114;
                    if (++ly == 154)
                    {
                        stat = (stat & ~STAT_MASK_MODE_FLAG) | STAT_MODE_READ_OAM;
                        ly = 0;
                        if (stat & STAT_MASK_OAM_INTERRUPT)
                            interrupts |= INTERRUPT_MASK_LCDC_STATUS;
                    }
                }
                break;
            case STAT_MODE_READ_OAM:
                if (mode_clock >= 20)
                {
                    mode_clock -= 20;
                    stat = (stat & ~STAT_MASK_MODE_FLAG) | STAT_MODE_READ_OAM_VRAM;
                }
                break;
            case STAT_MODE_READ_OAM_VRAM:
                if (mode_clock >= 43)
                {
                    mode_clock -= 43;
                    stat = (stat & ~STAT_MASK_MODE_FLAG) | STAT_MODE_HBLANK;
                    if (interrupts & STAT_MASK_HBLANK_INTERRUPT)
                        interrupts |= INTERRUPT_MASK_LCDC_STATUS;
                    scanline();
                }
                break;
        }
        if (ly      !=      lyc)
            stat &= ~STAT_MASK_COINCIDENCE_FLAG;
        else
        {
            stat |= STAT_MASK_COINCIDENCE_FLAG;
            if (stat & STAT_MASK_LYC_INTERRUPT)
                interrupts |= INTERRUPT_MASK_LCDC_STATUS;
        }
    }
    return interrupts;
}

