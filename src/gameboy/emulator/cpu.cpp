#include "cpu.h"

using gameboy::emulator::CPU;

unsigned CPU::next_step(MMU& mmu) noexcept
{
    {
        const unsigned mask = IF & IE & 0x1F;
        if (mask)
        {
            halt_mode = false;
            if (interrupt_master_enable)
            {
                interrupt_master_enable = false;
                int i = 0;
                for (; (mask >> i & 1) ^ 1; ++i)
                     ;
                IF &= ~(1 << i);

                static constexpr unsigned values[]{0x0040, 0x0048, 0x0050, 0x0058, 0x0060};
                mmu.write_word(regs.SP -= 2, regs.PC); regs.PC = values[i];

                return 5;
            }
        }
    }
    // build a table of opcodes
    static constexpr unsigned (*insts[512])(CPU& cpu, MMU&) noexcept
    {
#define a(p,n) ExecuteInstruction<p, n    >::exec,   \
               ExecuteInstruction<p, n + 1>::exec,

#define b(p,n) a(p,n)a(p,n+2)a(p,n+04)a(p,n+06)// v o r e cc ii .(o v  o ). v o r aa n t i
#define c(p,n) b(p,n)b(p,n+8)b(p,n+16)b(p,n+24)b(p,n+32)b(p,n+40)b(p,n+48)b(p,n+56)

        c(0,0)c(0,64)c(0,128)c(0,192)
        c(1,0)c(1,64)c(1,128)c(1,192)

#undef c
#undef b
#undef a
    };

    if (halt_mode) return 1;

    unsigned opcode = mmu.read_byte(regs.PC++);
    if (opcode == 0xCB)
        opcode = mmu.read_byte(regs.PC++) + 256;

    const unsigned cycles = (insts[opcode])(*this, mmu);
    if (ime_enable)
        interrupt_master_enable = !(ime_enable = false);

    return cycles;
}

