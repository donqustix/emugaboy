#ifndef GPU_H
#define GPU_H

#include <list>

namespace gameboy::emulator
{
    class GPU
    {
        friend class MMU;
        enum StatModes {STAT_MODE_HBLANK, STAT_MODE_VBLANK, STAT_MODE_READ_OAM, STAT_MODE_READ_OAM_VRAM};
        enum StatMasks {
            STAT_MASK_MODE_FLAG        = 0b0000011,
            STAT_MASK_COINCIDENCE_FLAG = 0b0000100,
            STAT_MASK_HBLANK_INTERRUPT = 0b0001000,
            STAT_MASK_VBLANK_INTERRUPT = 0b0010000,
            STAT_MASK_OAM_INTERRUPT    = 0b0100000,
            STAT_MASK_LYC_INTERRUPT    = 0b1000000
        };
        enum InterruptMasks {
            INTERRUPT_MASK_VBLANK      = 0b00001,
            INTERRUPT_MASK_LCDC_STATUS = 0b00010
        };
        enum ControlMasks {
            CONTROL_MASK_BG_DISPLAY                     = 0b00000001,
            CONTROL_MASK_OBJ_DISPLAY_ENABLE             = 0b00000010,
            CONTROL_MASK_OBJ_SIZE                       = 0b00000100,
            CONTROL_MASK_BG_TILE_MAP_DISPLAY_SELECT     = 0b00001000,
            CONTROL_MASK_BG_WINDOW_TILE_DATA_SELECT     = 0b00010000,
            CONTROL_MASK_WINDOW_DISPLAY_ENABLE          = 0b00100000,
            CONTROL_MASK_WINDOW_TILE_MAP_DISPLAY_SELECT = 0b01000000,
            CONTROL_MASK_LCD_DISPLAY_ENABLE             = 0b10000000
        };

        unsigned char vram[0x2000];
        unsigned char  oam[0x00A0];

        unsigned char control = 0x91, stat = 0x06;
        unsigned char scy = 0,
                      scx = 0, ly = 0, lyc = 0, wy = 0,
                                                wx = 0;
        unsigned char bgp = 0xFC, obp0 = 0xFF,
                                  obp1 = 0xFF;

        unsigned char framebuffer[160 * 144 / 8 * 2];
        
        std::list<int> oam_indices;
        unsigned mode_clock = 0;

        void set_framebuffer_pixel(int index, unsigned px) noexcept
        {
            framebuffer[index / 4] &= ~( 3 << (6 - index % 4 * 2));
            framebuffer[index / 4] |=   px << (6 - index % 4 * 2);
        }

        void draw_background() noexcept;
        void draw_sprites() noexcept;
        void scanline() noexcept;
    public:
        void write_oam(unsigned index, unsigned value) noexcept;
        void write_lcd_control(unsigned value) noexcept;
        void write_lcd_stat(unsigned value) noexcept {stat &= 7; stat |= value & ~7;}
        unsigned tick(unsigned cycles) noexcept;
        unsigned get_framebuffer_pixel(int index) const noexcept {return framebuffer[index / 4] >> (6 - index % 4 * 2) & 3;}
    };
}

#endif
