#include "gpu.h"

#include <iostream>

using gameboy::emulator::GPU;

void GPU::scanline() noexcept
{
    if (control & CONTROL_MASK_BG_DISPLAY)
    {
        const int st = control & CONTROL_MASK_BG_WINDOW_TILE_DATA_SELECT ? 0x0000 : 0x0800;
        const int sm = control & CONTROL_MASK_BG_TILE_MAP_DISPLAY_SELECT ? 0x1C00 : 0x1800;
        for (int i = 0; i < 160; ++i)
        {
            const int im = sm + (scx + i) / 8 % 32 + (scy + ly) / 8 % 32 * 32;
            const int vm = st == 0x0000 ?               vram[im] :
                                          (signed char) vram[im] + 128;
            const unsigned px =
                (vram[st + vm * 16 + (scy + ly) % 8 * 2    ] >> (7 - (scx + i) % 8) & 1) << 1 |
                (vram[st + vm * 16 + (scy + ly) % 8 * 2 + 1] >> (7 - (scx + i) % 8) & 1);

            framebuffer[(i + ly * 160) / 8 * 2] &= ~(( 3 << 6) >> i % 8 * 2);
            framebuffer[(i + ly * 160) / 8 * 2] |=   (px << 6) >> i % 8 * 2;

            framebuffer[(i + ly * 160) / 8 * 2 + 1] &= ~( 3 << (7 - i % 8) * 2);
            framebuffer[(i + ly * 160) / 8 * 2 + 1] |=   px << (7 - i % 8) * 2;
        }
    }
}

void GPU::write_lcd_control(unsigned value) noexcept
{
    std::clog << value << std::endl;
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

