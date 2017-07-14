#include "gpu.h"

using gameboy::emulator::GPU;

void GPU::scanline() noexcept
{
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
    else
    {
        stat &= ~(STAT_MASK_MODE_FLAG | STAT_MASK_COINCIDENCE_FLAG);
        mode_clock = ly = 0;
    }
    return interrupts;
}

