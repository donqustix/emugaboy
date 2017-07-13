#include "cpu.h"

using gameboy::emulator::CPU;

void CPU::reset() noexcept
{
    regs.AF = 0x01B0;
    regs.BC = 0x0013;
    regs.DE = 0x00D8;
    regs.HL = 0x014D;
    regs.PC = 0x0100;
    regs.SP = 0xFFFE;
}

unsigned CPU::next_step(const MMU& mmu) noexcept
{
    if (interrupt_master_enable)
    {
        interrupt_master_enable = false;
        const auto mask = IF & IE;
        for (unsigned i = 0; i < 5; ++i)
        {
            if (mask >> i & 1)
            {
                static constexpr unsigned values[]{0x0040, 0x0048, 0x0050, 0x0058, 0x0060};
                mmu.write_word(regs.SP -= 2, regs.PC); regs.PC = values[i];
                mmu.write_byte(0xFF0F, IF & ~(1 << i));
                return 5;
            }
        }
    }
    // build a table of opcodes
    static constexpr unsigned (*insts[512])(CPU&, const MMU&) noexcept
    {
#define a(p,n) ExecuteInstruction<p, n    >::exec,   \
               ExecuteInstruction<p, n + 1>::exec,

#define b(p,n) a(p,n)a(p,n+2)a(p,n+04)a(p,n+06)//vorecci
#define c(p,n) b(p,n)b(p,n+8)b(p,n+16)b(p,n+24)b(p,n+32)b(p,n+40)b(p,n+48)b(p,n+56)

        c(0,0)c(0,64)c(0,128)c(0,192)
        c(1,0)c(1,64)c(1,128)c(1,192)

#undef c
#undef b
#undef a
    };

    unsigned opcode = mmu.read_byte(regs.PC++);
    if ((opcode & 0xFF) == 0xCB)
         opcode = mmu.read_byte(regs.PC++) + 256;

    std::clog << regs.PC << std::endl;

    return (insts[opcode])(*this, mmu);
}

