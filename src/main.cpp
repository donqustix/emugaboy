#include "gameboy/emulator/cartridge.h"
#include "gameboy/emulator/cpu.h"
#include "gameboy/emulator/gpu.h"
#include "gameboy/emulator/mmu.h"
#include "gameboy/emulator/dma.h"

#include <SDL2/SDL.h>

#include <iostream>

int main()
{
    auto cartridge{gameboy::emulator::Cartridge::load("res/roms/Tetris (World).gb")};
    gameboy::emulator::CPU cpu;
    gameboy::emulator::GPU gpu;
    unsigned char wram[0x2000], hram[0x7F];
    gameboy::emulator::MMU mmu;
    gameboy::emulator::DMA dma{&gpu, &mmu};
    gameboy::emulator::MMU::MemPointers mem_pointers;
    mem_pointers.cartridge = &cartridge;
    mem_pointers.gpu       = &gpu;
    mem_pointers.cpu       = &cpu;
    mem_pointers.dma       = &dma;
    mem_pointers.wram = wram;
    mem_pointers.hram = hram;
    mmu.set_mem_pointers(mem_pointers);

    ::SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    SDL_Window* const window = ::SDL_CreateWindow("Emugaboy",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* const renderer = ::SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* const texture = ::SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING, 160, 144);
    const unsigned seconds_per_update = 1000 / 60, insts_per_update = 1048576 / 60;
    unsigned acc_update_time = 0;
    Uint32 previous_time = ::SDL_GetTicks();
    for (bool running = true; running;)
    {
        const Uint32 current_time = ::SDL_GetTicks();
        acc_update_time += current_time - previous_time;

        previous_time = current_time;

        for (static SDL_Event event; ::SDL_PollEvent(&event);)
            if (event.type == SDL_QUIT) running = false;

        for (; acc_update_time >= seconds_per_update; acc_update_time -= seconds_per_update)
        {
            static unsigned i = 0;
            while (i < insts_per_update)
            {
                const unsigned cycles = cpu.next_step(mmu); i += cycles;
                dma.tick(cycles);
                const unsigned interrupts = gpu.tick(cycles);
                cpu.request_interrupts(interrupts);
            }
            i -= insts_per_update;
        }
        Uint32* pixels;
        int pitch;

        ::SDL_LockTexture(texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);

        const unsigned char* const framebuffer = gpu.get_framebuffer();
        for (int i = 0; i < 160 * 144; ++i)
        {
            const unsigned px = 0xFF * (3 - (framebuffer[i / 4] >> (6 - i % 4 * 2) & 3)) / 3;
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

