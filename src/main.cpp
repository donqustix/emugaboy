#include "gameboy/emulator/cartridge.h"
#include "gameboy/emulator/cpu.h"
#include "gameboy/emulator/gpu.h"
#include "gameboy/emulator/mmu.h"
#include "gameboy/emulator/dma.h"
#include "gameboy/emulator/joypad.h"
#include "gameboy/emulator/timer.h"

#include <SDL2/SDL.h>

#include <fstream>

int main()
{
    auto cartridge{gameboy::emulator::Cartridge::load("res/roms/SuperMarioLand.gb")};
    gameboy::emulator::CPU cpu;
    gameboy::emulator::GPU gpu;
    unsigned char wram[0x2000], hram[0x7F];
    gameboy::emulator::MMU mmu;
    gameboy::emulator::DMA dma{&gpu, &mmu};
    gameboy::emulator::Joypad joypad;
    gameboy::emulator::Timer timer;
    gameboy::emulator::MMU::MemPointers mem_pointers;
    mem_pointers.cartridge = &cartridge;
    mem_pointers.gpu       = &gpu;
    mem_pointers.cpu       = &cpu;
    mem_pointers.dma       = &dma;
    mem_pointers.joypad    = &joypad;
    mem_pointers.timer     = &timer;
    mem_pointers.wram = wram;
    mem_pointers.hram = hram;
    mmu.set_mem_pointers(mem_pointers);

    ::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    SDL_Window* const window = ::SDL_CreateWindow("Emugaboy",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * 2, 144 * 2, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* const renderer = ::SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* const texture = ::SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, 160, 144);
    constexpr unsigned seconds_per_update = 1000 / 60, insts_per_update = 1048576 / 60;
    unsigned acc_update_time = 0;
    Uint32 previous_time = ::SDL_GetTicks();
    for (bool running = true; running;)
    {
        const Uint32 current_time = ::SDL_GetTicks();
        acc_update_time += current_time - previous_time;

        previous_time = current_time;

        ::SDL_PumpEvents();

        const Uint8* const keyboard_state = ::SDL_GetKeyboardState(nullptr);
        if (keyboard_state[SDL_SCANCODE_ESCAPE])
            running = false;
        const unsigned keys_mask = keyboard_state[SDL_SCANCODE_D] << 0 |
                                   keyboard_state[SDL_SCANCODE_A] << 1 |
                                   keyboard_state[SDL_SCANCODE_W] << 2 |
                                   keyboard_state[SDL_SCANCODE_S] << 3 |
                                   keyboard_state[SDL_SCANCODE_E] << 4 |
                                   keyboard_state[SDL_SCANCODE_Q] << 5 |
                                   keyboard_state[SDL_SCANCODE_X] << 6 |
                                   keyboard_state[SDL_SCANCODE_Z] << 7;
        joypad.push_key_states(keys_mask);

        static bool triggered = false;
             if (keyboard_state[SDL_SCANCODE_F]) triggered = true;
        else if (triggered)
        {
            std::ofstream stream{"res/images/screenshot.ppm", std::ios::out | std::ios::binary};
            stream << "P6\n" << 160 << ' ' << 144 << '\n' << 255 << '\n';
            for (int i = 0; i < 160 * 144; ++i)
            {
                const unsigned char px = 0xFF * (3 - gpu.get_framebuffer_pixel(i)) / 3;
                const unsigned char color[]{px, px, px};
                stream.write(reinterpret_cast<const char*>(color), 3);
            }
            triggered = false;
        }

        for (; acc_update_time >= seconds_per_update; acc_update_time -= seconds_per_update)
        {
            static unsigned i = 0;
            while (i < insts_per_update)
            {
                const unsigned cycles = cpu.next_step(mmu); i += cycles;
                dma.tick(cycles);

                unsigned interrupts = 0;
                interrupts      |= timer.tick(cycles);
                interrupts      |=   gpu.tick(cycles);

                cpu.request_interrupts(interrupts);
            }
            i -= insts_per_update;
        }
        Uint32* pixels;
        int pitch;

        ::SDL_LockTexture(texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);

        for (int i = 0; i < 160 * 144; ++i)
        {
            const unsigned px = 0xFF * (3 - gpu.get_framebuffer_pixel(i)) / 3;
            pixels[i] = 0xFF000000 | px | px << 8 | px << 16;
        }

        ::SDL_UnlockTexture(texture);

        ::SDL_RenderClear(renderer);
        ::SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        ::SDL_RenderPresent(renderer);
    }
    ::SDL_DestroyTexture(texture);
    ::SDL_DestroyRenderer(renderer);
    ::SDL_DestroyWindow(window);
    ::SDL_Quit();

    return 0;
}

