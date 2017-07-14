#ifndef GPU_H
#define GPU_H

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
    public:
        unsigned char vram[0x2000];
        unsigned char  oam[0x00A0];
        unsigned char control = 0x91, stat;
        unsigned char scy = 0, scx = 0, ly, lyc = 0, wy = 0, wx = 0;
        unsigned char bgp = 0xFC;
        unsigned char obp0 = 0xFF, obp1 = 0xFF;
        unsigned char framebuffer[160 * 144 / 8];
        unsigned mode_clock = 0;

        void scanline() noexcept;
    public:
        void write_oam(unsigned index, unsigned value) noexcept {oam[index] = value;}
        unsigned tick(unsigned cycles) noexcept;
    };
}

#endif
