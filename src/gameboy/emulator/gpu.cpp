#include "gpu.h"

using gameboy::emulator::GPU;

void GPU::scanline_background() noexcept
{
    const int st = control & CONTROL_MASK_BG_WINDOW_TILE_DATA_SELECT ? 0x0000 : 0x0800;
    const int sm = control & CONTROL_MASK_BG_TILE_MAP_DISPLAY_SELECT ? 0x1C00 : 0x1800;
    for (int i = 0; i < 160; ++i)
    {
        const int im = sm + (scx + i) / 8 % 32 + (scy + ly) / 8 % 32 * 32;
        const int vm = st == 0x0000 ?               vram[im] :
                                      (signed char) vram[im] + 128;
        const unsigned px =
            (vram[st + vm * 16 + (scy + ly) % 8 * 2    ] >> (7 - (scx + i) % 8) & 1) |
            (vram[st + vm * 16 + (scy + ly) % 8 * 2 + 1] >> (7 - (scx + i) % 8) & 1) << 1;
        set_framebuffer_pixel(i + ly * 160, bgp >> (px * 2) & 3);
    }
}

void GPU::scanline_sprites() noexcept
{
    const int ss = control & CONTROL_MASK_OBJ_SIZE ? 16 : 8;
    for (int oam_index : oam_indices)
    {
        const int sy = oam[oam_index] - 16;
        if (ly >= sy && ly < sy + ss)
        {
            const unsigned sn = ss == 16 ? oam[oam_index + 2] & 0xFE : oam[oam_index + 2];
            const unsigned sa = oam[oam_index + 3];
            const unsigned s_bg_p = sa & 0x80, sfy = sa & 0x40, sfx = sa & 0x20, spn = sa & 0x10;
            const unsigned sp = spn ? obp1 : obp0;
            const int sx = oam[oam_index + 1] - 8;
            const int px_line = sfy ? ss * 2 - 2 - (ly - sy) % ss * 2 : (ly - sy) % ss * 2;
            const int si = 0x0000 + sn * 16 + px_line;

            for (int i = 0; i < 8; ++i)
            {
                     if (sx + i <   0) continue;
                else if (sx + i > 159) break;

                const int px_index = sx + i + ly * 160;

                if (!(s_bg_p && get_framebuffer_pixel(px_index)))
                {
                    const int px_shift = sfx ? i % 8 : 7 - i % 8;
                    const unsigned px =
                            (vram[si    ] >> px_shift & 1) |
                            (vram[si + 1] >> px_shift & 1) << 1;
                    if (px)
                        set_framebuffer_pixel(px_index, sp >> (px * 2) & 3);
                }
            }
        }
    }
}

void GPU::scanline() noexcept
{
    if (control & CONTROL_MASK_BG_DISPLAY)
        scanline_background();
    if (control & CONTROL_MASK_OBJ_DISPLAY_ENABLE)
        scanline_sprites();
}

void GPU::write_oam_dma(unsigned index, unsigned value) noexcept
{
    if ((index & 2) ^ 2)
    {
        oam_indices.remove(index / 4 * 4);
        if (value && value < (index & 1 ? 168 : 160))
        {
            const int oam_index = index / 4 * 4;
            auto   iter  = oam_indices.begin();
            for (; iter != oam_indices.end(); ++iter)
            {
                const unsigned x = oam[*iter + 1];
                const unsigned y = oam[*iter    ];
                if      (x  < value) oam_indices.insert(iter,  oam_index);
                else if (x == value)
                {
                    if (y == oam[oam_index])
                        *iter  = oam_index;
                    else
                        oam_indices.insert(iter, oam_index);
                }
                else continue;
                break;
            }
            if (iter == oam_indices.end()) oam_indices.insert(iter, oam_index);
        }
    }
    oam[index] = value;
}

void GPU::write_lcd_control(unsigned value) noexcept
{
    if ((control ^ value) & CONTROL_MASK_LCD_DISPLAY_ENABLE)
    {
        if (value & CONTROL_MASK_LCD_DISPLAY_ENABLE) {
            stat = (stat & ~STAT_MASK_MODE_FLAG) | STAT_MODE_READ_OAM;      mode_clock = 0;
        } else {
            stat &= ~(STAT_MASK_MODE_FLAG | STAT_MASK_COINCIDENCE_FLAG);    ly = 0;
        }
    }
    control = value;
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

