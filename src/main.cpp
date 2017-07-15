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

    auto tick = [&] {
        const unsigned cycles = cpu.next_step(mmu);
        dma.tick(cycles);
        const unsigned interrupts = gpu.tick(cycles);
        cpu.request_interrupts(interrupts);
    };

    char c;
    do
    {
        std::cin.get(c);
        if (c == 'f')
        {
            for (int i = 0; i < 1500; ++i)
                tick();
        }
        else if (c == 'd')
        {
            const unsigned char* const framebuffer = gpu.get_framebuffer();
            for (int i = 0; i < 144; ++i)
            {
                for (int j = 0; j < 160; ++j)
                {
                    std::clog << unsigned((framebuffer[(j + i * 160) / 8 * 2] >> (6 - (j % 4) * 2)) & 3);
                }
                std::clog << std::endl;
            }
            std::clog << std::endl;
            //for (int i = 0; i < 0x800; ++i)
            //    std::clog << std::hex << 0x8000 + i << " = " << (int) gpu.vram[i] << std::endl;
        }
        else
        {
            tick();
        }
    }
    while (c != 'e');

        /*
        Uint32 previous_time = ::SDL_GetTicks();

        for (bool running = true; running;)
        {
            const Uint32 current_time = ::SDL_GetTicks();
            const Uint32 elapsed_time = current_time - previous_time;
            previous_time = current_time;

            for (unsigned cycles = 1048576 * elapsed_time / 1000; cycles; cycles -= cpu.next_step(mmu))
            {
            }
            ::SDL_Delay(1);
        }
        ::SDL_Quit();
    }*/
    /*
    if (::SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_VIDEO) >= 0)
    {
        SDL_Window* const window = ::SDL_CreateWindow("GameBoy", 100, 100, 640, 480, SDL_WINDOW_RESIZABLE);
        SDL_Renderer* const renderer = ::SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        SDL_Event event;

        Uint32 previous_time = ::SDL_GetTicks();

        for (bool running = true; running;)
        {
            const Uint32 current_time = ::SDL_GetTicks();
            const Uint32 elapsed_time = current_time - previous_time;

            previous_time = current_time;

            while (::SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                    running = false;
            }

            for (unsigned cycles = 1048576 * elapsed_time / 1000; cycles; --cycles)
            {
                // ...
            }

            ::SDL_RenderClear(renderer);
            ::SDL_RenderPresent(renderer);

            ::SDL_Delay(1);
        }

        ::SDL_DestroyRenderer(renderer);
        ::SDL_DestroyWindow(window);
        ::SDL_Quit();
    }
    else
        std::cerr << "SDL initialization error: " << ::SDL_GetError() << std::endl;
*/
    return 0;
}

