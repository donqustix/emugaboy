#ifndef CPU_H
#define CPU_H

#include "mmu.h"

namespace gameboy::emulator
{
    class CPU
    {
        friend MMU;
        struct
        {
#define MAKE_REGISTER(a, b)             \
            struct                      \
            {                           \
                unsigned char a;        \
                unsigned char b;        \
                auto& operator=(unsigned v) noexcept    \
                {                                       \
                    a = v >> 8 & 0xFF;                  \
                    b = v      & 0xFF;                  \
                    return *this;                       \
                }                                       \
                operator unsigned() const noexcept {return b | a << 8;}   \
            } a##b;
            MAKE_REGISTER(A, F)
            MAKE_REGISTER(B, C)
            MAKE_REGISTER(D, E)
            MAKE_REGISTER(H, L)
#undef MAKE_REGISTER

            unsigned short SP;
            unsigned short PC;
        } regs;

    public:
        CPU() noexcept
        {
            regs.AF = 0x01B0;
            regs.BC = 0x0013;
            regs.DE = 0x00D8;
            regs.HL = 0x014D;
            regs.PC = 0x0100;
            regs.SP = 0xFFFE;
        }

    private:
        unsigned char IE = 0;
        unsigned char IF = 0;
        bool interrupt_master_enable;

        enum FlagMasks {
            MC = 0b0001'0000,
            MH = 0b0010'0000,
            MN = 0b0100'0000,
            MZ = 0b1000'0000
        };
        enum FlagShifts {SC = 4, SH, SN, SZ};
        bool get_flag(FlagMasks flag) const noexcept {return regs.AF.F & flag;}
        void set_flags(unsigned flags, unsigned mask = 0xF0) noexcept {regs.AF.F = (regs.AF.F & ~mask) | flags;}

        template<bool prefix/*CB*/, unsigned opcode>
        struct ExecuteInstruction;

        template<unsigned opcode>
        struct ExecuteInstruction<0, opcode> 
        {
            // return a result of the opcode execution
            static unsigned exec(CPU& cpu, const MMU& mmu) noexcept
            {
                [[maybe_unused]] auto rb = [&mmu](unsigned index) noexcept {return mmu.read_byte(index);};
                [[maybe_unused]] auto rw = [&mmu](unsigned index) noexcept {return mmu.read_word(index);};
                [[maybe_unused]] auto wb = [&mmu](unsigned index, unsigned value) noexcept {mmu.write_byte(index, value);};
                [[maybe_unused]] auto ww = [&mmu](unsigned index, unsigned value) noexcept {mmu.write_word(index, value);};

                [[maybe_unused]]
                auto &A = cpu.regs.AF.A, &B = cpu.regs.BC.B, &C = cpu.regs.BC.C, &D = cpu.regs.DE.D,
                     &E = cpu.regs.DE.E, &H = cpu.regs.HL.H, &L = cpu.regs.HL.L;
                
                [[maybe_unused]] auto& HL = cpu.regs.HL;
                [[maybe_unused]] auto& BC = cpu.regs.BC;
                [[maybe_unused]] auto& DE = cpu.regs.DE;
                [[maybe_unused]] auto& AF = cpu.regs.AF;
                [[maybe_unused]] auto& PC = cpu.regs.PC;
                [[maybe_unused]] auto& SP = cpu.regs.SP;

                unsigned cycles = 0;

#define ST(mnemonic, id, init_cycles, code)       \
                if constexpr(opcode == id) {cycles = init_cycles; code;} else
                    // misc/control instructions
                    ST("NOP",      0x00, 1, )
                    ST("STOP",     0x10, 0, ) // !!!
                    ST("HALT",     0x76, 0, ) // !!!
                    ST("DI",       0xF3, 1, cpu.interrupt_master_enable = 0)
                    ST("EI",       0xFB, 1, cpu.interrupt_master_enable = 1) // FIX

                    // 8-bit loads
                    ST("LD  B  , B",     0x40, 1, B = B) // !!!
                    ST("LD  D  , B",     0x50, 1, D = B)
                    ST("LD  H  , B",     0x60, 1, H = B)

                    ST("LD  B  , C",     0x41, 1, B = C)
                    ST("LD  D  , C",     0x51, 1, D = C)
                    ST("LD  H  , C",     0x61, 1, H = C)
                    
                    ST("LD (HL ), C",     0x71, 2, wb(HL, C))
                    ST("LD (HL ), B",     0x70, 2, wb(HL, B))
                    ST("LD (BC ), A",     0x02, 2, wb(BC, A))
                    ST("LD (DE ), A",     0x12, 2, wb(DE, A))
                    ST("LD (HL+), A",     0x22, 2, wb(HL, A); HL = HL + 1)
                    ST("LD (HL-), A",     0x32, 2, wb(HL, A); HL = HL - 1)

                    ST("LD  B  , D",     0x42, 1,    B = D)
                    ST("LD  D  , D",     0x52, 1,    D = D) // !!!
                    ST("LD  H  , D",     0x62, 1,    H = D)
                    ST("LD (HL), D",     0x72, 2, wb(HL, D))

                    ST("LD  B  , E",     0x43, 1,    B = E)
                    ST("LD  D  , E",     0x53, 1,    D = E)
                    ST("LD  H  , E",     0x63, 1,    H = E)
                    ST("LD (HL), E",     0x73, 2, wb(HL, E))

                    ST("LD  B  , H",     0x44, 1,    B = H)
                    ST("LD  D  , H",     0x54, 1,    D = H)
                    ST("LD  H  , H",     0x64, 1,    H = H) // !!!
                    ST("LD (HL), H",     0x74, 2, wb(HL, H))

                    ST("LD  B  , L",     0x45, 1,    B = L)
                    ST("LD  D  , L",     0x55, 1,    D = L)
                    ST("LD  H  , L",     0x65, 1,    H = L)
                    ST("LD (HL), L",     0x75, 2, wb(HL, L))

                    ST("LD  B  , n",     0x06, 2,    B = rb(PC++))
                    ST("LD  D  , n",     0x16, 2,    D = rb(PC++))
                    ST("LD  H  , n",     0x26, 2,    H = rb(PC++))
                    ST("LD (HL), n",     0x36, 3, wb(HL, rb(PC++)))

                    ST("LD B, (HL)",     0x46, 2, B = rb(HL))
                    ST("LD D, (HL)",     0x56, 2, D = rb(HL))
                    ST("LD H, (HL)",     0x66, 2, H = rb(HL))

                    ST("LD  B  , A",     0x47, 1,    B = A)
                    ST("LD  D  , A",     0x57, 1,    D = A)
                    ST("LD  H  , A",     0x67, 1,    H = A)
                    ST("LD (HL), A",     0x77, 2, wb(HL, A))

                    ST("LD C, B",     0x48, 1, C = B)
                    ST("LD E, B",     0x58, 1, E = B)
                    ST("LD L, B",     0x68, 1, L = B)
                    ST("LD A, B",     0x78, 1, A = B)

                    ST("LD C, C",     0x49, 1, C = C) // !!!
                    ST("LD E, C",     0x59, 1, E = C)
                    ST("LD L, C",     0x69, 1, L = C)
                    ST("LD A, C",     0x79, 1, A = C)

                    ST("LD A, (BC )",     0x0A, 2, A = rb(BC))
                    ST("LD A, (DE )",     0x1A, 2, A = rb(DE))
                    ST("LD A, (HL+)",     0x2A, 2, A = rb(HL); HL = HL + 1)
                    ST("LD A, (HL-)",     0x3A, 2, A = rb(HL); HL = HL - 1)

                    ST("LD C, D",     0x4A, 1, C = D)
                    ST("LD E, D",     0x5A, 1, E = D)
                    ST("LD L, D",     0x6A, 1, L = D)
                    ST("LD A, D",     0x7A, 1, A = D)

                    ST("LD C, E",     0x4B, 1, C = E)
                    ST("LD E, E",     0x5B, 1, E = E) // !!!
                    ST("LD L, E",     0x6B, 1, L = E)
                    ST("LD A, E",     0x7B, 1, A = E)

                    ST("LD C, H",     0x4C, 1, C = H)
                    ST("LD E, H",     0x5C, 1, E = H)
                    ST("LD L, H",     0x6C, 1, L = H)
                    ST("LD A, H",     0x7C, 1, A = H)

                    ST("LD C, L",     0x4D, 1, C = L)
                    ST("LD E, L",     0x5D, 1, E = L)
                    ST("LD L, L",     0x6D, 1, L = L) // !!!
                    ST("LD A, L",     0x7D, 1, A = L)

                    ST("LD C, n",     0x0E, 2, C = rb(PC++))
                    ST("LD E, n",     0x1E, 2, E = rb(PC++))
                    ST("LD L, n",     0x2E, 2, L = rb(PC++))
                    ST("LD A, n",     0x3E, 2, A = rb(PC++))

                    ST("LD C, (HL)",     0x4E, 2, C = rb(HL))
                    ST("LD E, (HL)",     0x5E, 2, E = rb(HL))
                    ST("LD L, (HL)",     0x6E, 2, L = rb(HL))
                    ST("LD A, (HL)",     0x7E, 2, A = rb(HL))

                    ST("LD C, A",     0x4F, 1, C = A)
                    ST("LD E, A",     0x5F, 1, E = A)
                    ST("LD L, A",     0x6F, 1, L = A)
                    ST("LD A, A",     0x7F, 1, A = A) // !!!

                    ST("LDH (n), A",     0xE0, 3, wb(0xFF00 | rb(PC++), A))
                    ST("LD  (C), A",     0xE2, 2, wb(0xFF00 |     C,    A))
                    ST("LD (nn), A",     0xEA, 4, wb(rw(PC),            A); PC += 2)

                    ST("LDH A, (n )",     0xF0, 3, A = rb(0xFF00 | rb(PC++)))
                    ST("LD  A, (C )",     0xF2, 2, A = rb(0xFF00 |     C  ))
                    ST("LD  A, (nn)",     0xFA, 4, A = rb(rw(PC)          ); PC += 2)

                    // 16-bit loads
                    ST("LD BC, nn",     0x01, 3, BC = rw(PC); PC += 2)
                    ST("LD DE, nn",     0x11, 3, DE = rw(PC); PC += 2)
                    ST("LD HL, nn",     0x21, 3, HL = rw(PC); PC += 2)
                    ST("LD SP, nn",     0x31, 3, SP = rw(PC); PC += 2)

                    ST("LD (nn), SP",     0x08, 5, ww(rw(PC), SP); PC += 2)
                    ST("LD  SP , HL",     0xF9, 2, SP = HL)

                    ST("POP BC",     0xC1, 3, BC = rw(SP);          SP += 2)
                    ST("POP DE",     0xD1, 3, DE = rw(SP);          SP += 2)
                    ST("POP HL",     0xE1, 3, HL = rw(SP);          SP += 2)
                    ST("POP AF",     0xF1, 3, AF = rw(SP) & 0xFFF0; SP += 2)

                    ST("PUSH BC",     0xC5, 4, ww(SP -= 2, BC))
                    ST("PUSH DE",     0xD5, 4, ww(SP -= 2, DE))
                    ST("PUSH HL",     0xE5, 4, ww(SP -= 2, HL))
                    ST("PUSH AF",     0xF5, 4, ww(SP -= 2, AF))

                    ST("LDHL SP, n",     0xF8, 3,
                            const auto        n = (signed char) rb(PC++);
                            const unsigned temp =  SP + n;
                            const unsigned mask = (SP ^ n ^ temp) & 0x110;
                                                   HL     = temp;
                            cpu.set_flags((mask >> 8) << SC | (mask >> 4) << SH)
                    )

                    // jumps/calls
                    ST("JR NZ, n",     0x20, 2, if (!cpu.get_flag(MZ)) {PC += (signed char) rb(PC); ++cycles;} ++PC)
                    ST("JR NC, n",     0x30, 2, if (!cpu.get_flag(MC)) {PC += (signed char) rb(PC); ++cycles;} ++PC)

                    ST("JR Z, n",     0x28, 2, if (cpu.get_flag(MZ)) {PC += (signed char) rb(PC); ++cycles;} ++PC)
                    ST("JR C, n",     0x38, 2, if (cpu.get_flag(MC)) {PC += (signed char) rb(PC); ++cycles;} ++PC)

                    ST("JR n",     0x18, 3, PC += (signed char) rb(PC++))

                    ST("RET NZ",     0xC0, 2, if (!cpu.get_flag(MZ)) {PC = rw(SP); SP += 2; cycles += 3;})
                    ST("RET NC",     0xD0, 2, if (!cpu.get_flag(MC)) {PC = rw(SP); SP += 2; cycles += 3;})

                    ST("JP NZ, nn",     0xC2, 3, if (!cpu.get_flag(MZ)) {PC = rw(PC); ++cycles;} else PC += 2)
                    ST("JP NC, nn",     0xD2, 3, if (!cpu.get_flag(MC)) {PC = rw(PC); ++cycles;} else PC += 2)

                    ST("JP nn",     0xC3, 4, PC = rw(PC))

                    ST("CALL NZ, nn",     0xC4, 3,
                            if (!cpu.get_flag(MZ)) {ww(SP -= 2, PC + 2); PC = rw(PC); cycles += 3;} else PC += 2)
                    ST("CALL NC, nn",     0xD4, 3,
                            if (!cpu.get_flag(MC)) {ww(SP -= 2, PC + 2); PC = rw(PC); cycles += 3;} else PC += 2)

                    ST("RST 00H",     0xC7, 4, ww(SP -= 2, PC); PC = 0x00)
                    ST("RST 10H",     0xD7, 4, ww(SP -= 2, PC); PC = 0x10)
                    ST("RST 20H",     0xE7, 4, ww(SP -= 2, PC); PC = 0x20)
                    ST("RST 30H",     0xF7, 4, ww(SP -= 2, PC); PC = 0x30)

                    ST("RET Z",     0xC8, 2, if (cpu.get_flag(MZ)) {PC = rw(SP); SP += 2; cycles += 3;})
                    ST("RET C",     0xD8, 2, if (cpu.get_flag(MC)) {PC = rw(SP); SP += 2; cycles += 3;})

                    ST("RET",      0xC9, 4, PC = rw(SP); SP += 2                                 )
                    ST("RETI",     0xD9, 4, PC = rw(SP); SP += 2; cpu.interrupt_master_enable = 1)

                    ST("JP (HL)",     0xE9, 1, PC = HL)

                    ST("JP Z, nn",     0xCA, 3, if (cpu.get_flag(MZ)) {PC = rw(PC); ++cycles;} else PC += 2)
                    ST("JP C, nn",     0xDA, 3, if (cpu.get_flag(MC)) {PC = rw(PC); ++cycles;} else PC += 2)

                    ST("CALL Z, nn",     0xCC, 3,
                            if (cpu.get_flag(MZ)) {ww(SP -= 2, PC + 2); PC = rw(PC); cycles += 3;} else PC += 2)
                    ST("CALL C, nn",     0xDC, 3,
                            if (cpu.get_flag(MC)) {ww(SP -= 2, PC + 2); PC = rw(PC); cycles += 3;} else PC += 2)

                    ST("CALL nn",     0xCD, 6, ww(SP -= 2, PC + 2); PC = rw(PC))

                    ST("RST 08H",     0xCF, 4, ww(SP -= 2, PC); PC = 0x08)
                    ST("RST 18H",     0xDF, 4, ww(SP -= 2, PC); PC = 0x18)
                    ST("RST 28H",     0xEF, 4, ww(SP -= 2, PC); PC = 0x28)
                    ST("RST 38H",     0xFF, 4, ww(SP -= 2, PC); PC = 0x38)

                    // 8-bit arithmetic/logical instructions
#define ADD(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles,           \
                            const unsigned seco = o;                            \
                            const unsigned temp = A + seco;                             \
                            const unsigned half_carry = (A ^ seco ^ temp) >> 4 & 1;     \
                                                         A        = temp;               \
                            cpu.set_flags(!A << SZ | (temp >> 8) << SC | half_carry << SH))
                    ADD("ADD A, A",     0x87, 1,     A)
                    ADD("ADD A, B",     0x80, 1,     B)
                    ADD("ADD A, C",     0x81, 1,     C)
                    ADD("ADD A, D",     0x82, 1,     D)
                    ADD("ADD A, E",     0x83, 1,     E)
                    ADD("ADD A, H",     0x84, 1,     H)
                    ADD("ADD A, L",     0x85, 1,     L)
                    ADD("ADD A, n",     0xC6, 2, rb(PC++))
                    ADD("ADD A, (HL)",  0x86, 2, rb(HL  ))
#undef ADD

#define ADC(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles,           \
                            const bool     flag = cpu.get_flag(MC);             \
                            const unsigned seco = o;                            \
                            const unsigned temp = A + seco + flag;                      \
                            const unsigned half_carry = (A ^ seco ^ temp) >> 4 & 1;     \
                                                         A        = temp;               \
                            cpu.set_flags(!A << SZ | (temp >> 8) << SC | half_carry << SH))
                    ADC("ADC A, A",     0x8F, 1, A)
                    ADC("ADC A, B",     0x88, 1, B)
                    ADC("ADC A, C",     0x89, 1, C)
                    ADC("ADC A, D",     0x8A, 1, D)
                    ADC("ADC A, E",     0x8B, 1, E)
                    ADC("ADC A, H",     0x8C, 1, H)
                    ADC("ADC A, L",     0x8D, 1, L)
                    ADC("ADC A, n",     0xCE, 2, rb(PC++))
                    ADC("ADC A, (HL)",  0x8E, 2, rb(HL  ))
#undef ADC

#define SUB(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles,       \
                            const unsigned seco = o;                        \
                            const unsigned temp = A - seco;                             \
                            const unsigned mask = (A ^ seco ^ temp) & 0x110;            \
                                                   A        = temp;                     \
                            cpu.set_flags(!A << SZ | (mask >> 8) << SC | (mask >> 4) << SH | MN))
                    SUB("SUB A",     0x97, 1,     A)
                    SUB("SUB B",     0x90, 1,     B)
                    SUB("SUB C",     0x91, 1,     C)
                    SUB("SUB D",     0x92, 1,     D)
                    SUB("SUB E",     0x93, 1,     E)
                    SUB("SUB H",     0x94, 1,     H)
                    SUB("SUB L",     0x95, 1,     L)
                    SUB("SUB n",     0xD6, 2, rb(PC++))
                    SUB("SUB (HL)",  0x96, 2, rb(HL  ))
#undef SUB

#define SBC(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles,           \
                            const bool     flag = cpu.get_flag(MC);             \
                            const unsigned seco = o;                            \
                            const unsigned temp = A - seco - flag;              \
                            const unsigned mask = (A ^ seco ^ temp) & 0x110;    \
                                                   A        = temp;             \
                            cpu.set_flags(!A << SZ | (mask >> 8) << SC | (mask >> 4) << SH | MN))
                    SBC("SBC A, A",     0x9F, 1,     A)
                    SBC("SBC A, B",     0x98, 1,     B)
                    SBC("SBC A, C",     0x99, 1,     C)
                    SBC("SBC A, D",     0x9A, 1,     D)
                    SBC("SBC A, E",     0x9B, 1,     E)
                    SBC("SBC A, H",     0x9C, 1,     H)
                    SBC("SBC A, L",     0x9D, 1,     L)
                    SBC("SBC A, n",     0xDE, 2, rb(PC++))
                    SBC("SBC A, (HL)",  0x9E, 2, rb(HL  ))
#undef SBC

#define AND(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, A = A & o; cpu.set_flags(!A << SZ | MH))
                    AND("AND A",     0xA7, 1,     A)
                    AND("AND B",     0xA0, 1,     B)
                    AND("AND C",     0xA1, 1,     C)
                    AND("AND D",     0xA2, 1,     D)
                    AND("AND E",     0xA3, 1,     E)
                    AND("AND H",     0xA4, 1,     H)
                    AND("AND L",     0xA5, 1,     L)
                    AND("AND n",     0xE6, 2, rb(PC++))
                    AND("AND (HL)",  0xA6, 2, rb(HL  ))
#undef AND

#define XOR(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, A = A ^ o; cpu.set_flags(!A << SZ))
                    XOR("XOR A",     0xAF, 1,     A)
                    XOR("XOR B",     0xA8, 1,     B)
                    XOR("XOR C",     0xA9, 1,     C)
                    XOR("XOR D",     0xAA, 1,     D)
                    XOR("XOR E",     0xAB, 1,     E)
                    XOR("XOR H",     0xAC, 1,     H)
                    XOR("XOR L",     0xAD, 1,     L)
                    XOR("XOR n",     0xEE, 2, rb(PC++))
                    XOR("XOR (HL)",  0xAE, 2, rb(HL  ))
#undef XOR

#define OR(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, A = A | o; cpu.set_flags(!A << SZ))
                    OR("OR A",     0xB7, 1,     A)
                    OR("OR B",     0xB0, 1,     B)
                    OR("OR C",     0xB1, 1,     C)
                    OR("OR D",     0xB2, 1,     D)
                    OR("OR E",     0xB3, 1,     E)
                    OR("OR H",     0xB4, 1,     H)
                    OR("OR L",     0xB5, 1,     L)
                    OR("OR n",     0xF6, 2, rb(PC++))
                    OR("OR (HL)",  0xB6, 2, rb(HL  ))
#undef OR

#define CP(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles,            \
                            const unsigned seco =         o;                    \
                            const unsigned temp =  A - seco;                    \
                            const unsigned mask = (A ^ seco ^ temp) & 0x110;    \
                            cpu.set_flags(!temp << SZ | (mask >> 8) << SC | (mask >> 4) << SH | MN))
                    CP("CP A",     0xBF, 1,     A)
                    CP("CP B",     0xB8, 1,     B)
                    CP("CP C",     0xB9, 1,     C)
                    CP("CP D",     0xBA, 1,     D)
                    CP("CP E",     0xBB, 1,     E)
                    CP("CP H",     0xBC, 1,     H)
                    CP("CP L",     0xBD, 1,     L)
                    CP("CP n",     0xFE, 2, rb(PC++))
                    CP("CP (HL)",  0xBE, 2, rb(HL  ))

#undef CP

#define INC(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,     \
                            const unsigned seco = o;                                     \
                            const unsigned temp = seco + 1; assign_method;               \
                            cpu.set_flags(!(temp & 0xFF) << SZ | ((seco ^ temp) >> 4 & 1) << SH, ~MC))
                    INC("INC A",     0x3C, 1,     A,      A  = temp)
                    INC("INC B",     0x04, 1,     B,      B  = temp)
                    INC("INC C",     0x0C, 1,     C,      C  = temp)
                    INC("INC D",     0x14, 1,     D,      D  = temp)
                    INC("INC E",     0x1C, 1,     E,      E  = temp)
                    INC("INC H",     0x24, 1,     H,      H  = temp)
                    INC("INC L",     0x2C, 1,     L,      L  = temp)
                    INC("INC (HL)",  0x34, 3, rb(HL), wb(HL,   temp))
#undef INC

#define DEC(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,     \
                            const unsigned seco = o;                                     \
                            const unsigned temp = seco - 1; assign_method;               \
                            cpu.set_flags(!temp << SZ | ((seco ^ temp) >> 4 & 1) << SH | MN, ~MC))
                    DEC("DEC A",     0x3D, 1,     A,      A  = temp)
                    DEC("DEC B",     0x05, 1,     B,      B  = temp)
                    DEC("DEC C",     0x0D, 1,     C,      C  = temp)
                    DEC("DEC D",     0x15, 1,     D,      D  = temp)
                    DEC("DEC E",     0x1D, 1,     E,      E  = temp)
                    DEC("DEC H",     0x25, 1,     H,      H  = temp)
                    DEC("DEC L",     0x2D, 1,     L,      L  = temp)
                    DEC("DEC (HL)",  0x35, 3, rb(HL), wb(HL,   temp))
#undef DEC

                    ST("CPL",  0x2F, 1, A = A ^ 0xFF; cpu.set_flags(MN | MH, MN | MH))
                    ST("CCF",  0x3F, 1,               cpu.set_flags((cpu.regs.AF.F ^ MC) & MC, ~MZ))
                    ST("SCF",  0x37, 1,               cpu.set_flags(                       MC, ~MZ))
                    ST("DDA",  0x27, 1,
                         if (!cpu.get_flag(MN))
                         {  
                             if (cpu.get_flag(MC) ||  A > 0x99)       { A += 0x60; cpu.regs.AF.F |= MC;}
                             if (cpu.get_flag(MH) || (A & 0x0F) > 0x09) A += 0x6;
                         }
                         else
                         { 
                             if (cpu.get_flag(MC)) A -= 0x60;
                             if (cpu.get_flag(MH)) A -= 0x6;
                         }
                         cpu.set_flags(!A << SZ, MZ | MH);
                    )

                    // 16bit arithmetic/logical instructions
#define ADD(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles,                   \
                            const unsigned temp = HL + o;                               \
                            const unsigned half_carry = (HL ^ o ^ temp) >> 12 & 1;      \
                                                         HL     = temp;                 \
                            cpu.set_flags((temp >> 16) << SC | half_carry << SH, ~MZ))
                    ADD("ADD HL, BC",  0x09, 2, BC)
                    ADD("ADD HL, DE",  0x19, 2, DE)
                    ADD("ADD HL, HL",  0x29, 2, HL)
                    ADD("ADD HL, SP",  0x39, 2, SP)
#undef ADD
                    ST("ADD SP, n",  0xE8, 4,
                            const auto        n = (signed char) rb(PC++);
                            const unsigned temp = SP + n;
                            const unsigned mask = (SP ^ n ^ temp) & 0x110;
                                                   SP     = temp;
                            cpu.set_flags((mask >> 8) << SC | (mask >> 4) << SH)
                    )

                    ST("INC BC", 0x03, 2, BC = BC + 1)
                    ST("INC DE", 0x13, 2, DE = DE + 1)
                    ST("INC HL", 0x23, 2, HL = HL + 1)
                    ST("INC SP", 0x33, 2, SP = SP + 1)

                    ST("DEC BC", 0x0B, 2, BC = BC - 1)
                    ST("DEC DE", 0x1B, 2, DE = DE - 1)
                    ST("DEC HL", 0x2B, 2, HL = HL - 1)
                    ST("DEC SP", 0x3B, 2, SP = SP - 1)

                    ST("RRCA",     0x0F, 1,
                            const unsigned carry = A  & 1; A = A >> 1 | carry << 7; cpu.set_flags(carry << SC))
                    ST("RLCA",     0x07, 1,
                            const unsigned carry = A >> 7; A = A << 1 | carry;      cpu.set_flags(carry << SC))

                    ST("RRA",     0x1F, 1,
                            const unsigned carry = A  & 1; A = A >> 1 | cpu.get_flag(MC) << 7; cpu.set_flags(carry << SC);)
                    ST("RLA",     0x17, 1,
                            const unsigned carry = A >> 7; A = A << 1 | cpu.get_flag(MC);      cpu.set_flags(carry << SC);)
                    {}

#undef ST
                return cycles;
            }
        };

        template<unsigned opcode>
        struct ExecuteInstruction<1, opcode>
        {
            static unsigned exec(CPU& cpu, const MMU& mmu) noexcept
            {
                [[maybe_unused]] auto rb = [&mmu](unsigned index) noexcept {return mmu.read_byte(index);};
                [[maybe_unused]] auto rw = [&mmu](unsigned index) noexcept {return mmu.read_word(index);};
                [[maybe_unused]] auto wb = [&mmu](unsigned index, unsigned value) noexcept {mmu.write_byte(index, value);};
                [[maybe_unused]] auto ww = [&mmu](unsigned index, unsigned value) noexcept {mmu.write_word(index, value);};

                [[maybe_unused]]
                auto &A = cpu.regs.AF.A, &B = cpu.regs.BC.B, &C = cpu.regs.BC.C, &D = cpu.regs.DE.D,
                     &E = cpu.regs.DE.E, &H = cpu.regs.HL.H, &L = cpu.regs.HL.L;

                [[maybe_unused]] auto& HL = cpu.regs.HL;

                unsigned cycles = 0;

#define ST(mnemonic, id, init_cycles, code)       \
                if constexpr(opcode == id) {cycles = init_cycles; code;} else

#define RLC(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,    \
                            const unsigned  seco =    o;                                \
                            const unsigned carry = seco >> 7;                           \
                            const unsigned  temp = seco << 1 | carry; assign_method;    \
                            cpu.set_flags(!(temp & 0xFF) << SZ | carry << SC))
                    RLC("RLC A",        0x07, 2,     A,     A = temp)
                    RLC("RLC B",        0x00, 2,     B,     B = temp)
                    RLC("RLC C",        0x01, 2,     C,     C = temp)
                    RLC("RLC D",        0x02, 2,     D,     D = temp)
                    RLC("RLC E",        0x03, 2,     E,     E = temp)
                    RLC("RLC H",        0x04, 2,     H,     H = temp)
                    RLC("RLC L",        0x05, 2,     L,     L = temp)
                    RLC("RLC (HL)",     0x06, 4, rb(HL), wb(HL, temp))
#undef RLC

#define RRC(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,        \
                            const unsigned seco = o;                                        \
                            const unsigned carry = seco & 1;                                \
                            const unsigned temp = seco >> 1 | carry << 7; assign_method;    \
                            cpu.set_flags(!temp << SZ | carry << SC))
                    RRC("RRC A",        0x0F, 2,     A,     A = temp)
                    RRC("RRC B",        0x08, 2,     B,     B = temp)
                    RRC("RRC C",        0x09, 2,     C,     C = temp)
                    RRC("RRC D",        0x0A, 2,     D,     D = temp)
                    RRC("RRC E",        0x0B, 2,     E,     E = temp)
                    RRC("RRC H",        0x0C, 2,     H,     H = temp)
                    RRC("RRC L",        0x0D, 2,     L,     L = temp)
                    RRC("RRC (HL)",     0x0E, 4, rb(HL), wb(HL, temp))
#undef RRC

#define RL(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,                     \
                            const unsigned seco = o;                                                    \
                            const unsigned temp = seco << 1 | cpu.get_flag(MC); assign_method;          \
                            cpu.set_flags(!(temp & 0xFF) << SZ | (seco >> 7) << SC))
                    RL("RL A",        0x17, 2,     A,     A = temp)
                    RL("RL B",        0x10, 2,     B,     B = temp)
                    RL("RL C",        0x11, 2,     C,     C = temp)
                    RL("RL D",        0x12, 2,     D,     D = temp)
                    RL("RL E",        0x13, 2,     E,     E = temp)
                    RL("RL H",        0x14, 2,     H,     H = temp)
                    RL("RL L",        0x15, 2,     L,     L = temp)
                    RL("RL (HL)",     0x16, 4, rb(HL), wb(HL, temp))
#undef RL

#define RR(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,                     \
                            const unsigned seco = o;                                                    \
                            const unsigned temp = seco >> 1 | cpu.get_flag(MC) << 7; assign_method;     \
                            cpu.set_flags(!temp << SZ | (seco & 1) << SC))
                    RR("RR A",        0x1F, 2,     A,     A = temp)
                    RR("RR B",        0x18, 2,     B,     B = temp)
                    RR("RR C",        0x19, 2,     C,     C = temp)
                    RR("RR D",        0x1A, 2,     D,     D = temp)
                    RR("RR E",        0x1B, 2,     E,     E = temp)
                    RR("RR H",        0x1C, 2,     H,     H = temp)
                    RR("RR L",        0x1D, 2,     L,     L = temp)
                    RR("RR (HL)",     0x1E, 4, rb(HL), wb(HL, temp))
#undef RR

#define SLA(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,    \
                            const unsigned seco = o;                                    \
                            const unsigned temp = seco << 1; assign_method;             \
                            cpu.set_flags(!(temp & 0xFF) << SZ | (seco >> 7) << SC))
                    SLA("SLA A",        0x27, 2,     A,     A = temp)
                    SLA("SLA B",        0x20, 2,     B,     B = temp)
                    SLA("SLA C",        0x21, 2,     C,     C = temp)
                    SLA("SLA D",        0x22, 2,     D,     D = temp)
                    SLA("SLA E",        0x23, 2,     E,     E = temp)
                    SLA("SLA H",        0x24, 2,     H,     H = temp)
                    SLA("SLA L",        0x25, 2,     L,     L = temp)
                    SLA("SLA (HL)",     0x26, 4, rb(HL), wb(HL, temp))
#undef SLA

#define SRA(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,                    \
                            const unsigned seco = o;                                                    \
                            const unsigned temp = seco >> 1 | (seco & 0x80); assign_method;             \
                            cpu.set_flags(!temp << SZ | (seco & 1) << SC))
                    SRA("SRA A",        0x2F, 2,     A,     A = temp)
                    SRA("SRA B",        0x28, 2,     B,     B = temp)
                    SRA("SRA C",        0x29, 2,     C,     C = temp)
                    SRA("SRA D",        0x2A, 2,     D,     D = temp)
                    SRA("SRA E",        0x2B, 2,     E,     E = temp)
                    SRA("SRA H",        0x2C, 2,     H,     H = temp)
                    SRA("SRA L",        0x2D, 2,     L,     L = temp)
                    SRA("SRA (HL)",     0x2E, 4, rb(HL), wb(HL, temp))
#undef SRA

#define SRL(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,    \
                            const unsigned seco = o;                                    \
                            const unsigned temp = seco >> 1; assign_method;             \
                            cpu.set_flags(!temp << SZ | (seco & 1) << SC))
                    SRL("SRL A",        0x3F, 2,     A,     A = temp)
                    SRL("SRL B",        0x38, 2,     B,     B = temp)
                    SRL("SRL C",        0x39, 2,     C,     C = temp)
                    SRL("SRL D",        0x3A, 2,     D,     D = temp)
                    SRL("SRL E",        0x3B, 2,     E,     E = temp)
                    SRL("SRL H",        0x3C, 2,     H,     H = temp)
                    SRL("SRL L",        0x3D, 2,     L,     L = temp)
                    SRL("SRL (HL)",     0x3E, 4, rb(HL), wb(HL, temp))
#undef SRL

#define SWAP(mnemonic, opcode, cycles, o, assign_method) ST(mnemonic, opcode, cycles,               \
                            const unsigned seco = o;                                                \
                            const unsigned temp = seco >> 4 | (seco & 0xF) << 4; assign_method;     \
                            cpu.set_flags(!temp << SZ))
                    SWAP("SWAP A",        0x37, 2,     A,     A = temp)
                    SWAP("SWAP B",        0x30, 2,     B,     B = temp)
                    SWAP("SWAP C",        0x31, 2,     C,     C = temp)
                    SWAP("SWAP D",        0x32, 2,     D,     D = temp)
                    SWAP("SWAP E",        0x33, 2,     E,     E = temp)
                    SWAP("SWAP H",        0x34, 2,     H,     H = temp)
                    SWAP("SWAP L",        0x35, 2,     L,     L = temp)
                    SWAP("SWAP (HL)",     0x36, 4, rb(HL), wb(HL, temp))
#undef SWAP

#define BIT0(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, cpu.set_flags(!(o & 0x01) << SZ | MH, ~MC))
                    BIT0("BIT 0, A",        0x47, 2,     A)
                    BIT0("BIT 0, B",        0x40, 2,     B)
                    BIT0("BIT 0, C",        0x41, 2,     C)
                    BIT0("BIT 0, D",        0x42, 2,     D)
                    BIT0("BIT 0, E",        0x43, 2,     E)
                    BIT0("BIT 0, H",        0x44, 2,     H)
                    BIT0("BIT 0, L",        0x45, 2,     L)
                    BIT0("BIT 0, (HL)",     0x46, 3, rb(HL))
#undef BIT0

#define BIT2(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, cpu.set_flags(!(o & 0x04) << SZ | MH, ~MC))
                    BIT2("BIT 2, A",        0x57, 2,     A)
                    BIT2("BIT 2, B",        0x50, 2,     B)
                    BIT2("BIT 2, C",        0x51, 2,     C)
                    BIT2("BIT 2, D",        0x52, 2,     D)
                    BIT2("BIT 2, E",        0x53, 2,     E)
                    BIT2("BIT 2, H",        0x54, 2,     H)
                    BIT2("BIT 2, L",        0x55, 2,     L)
                    BIT2("BIT 2, (HL)",     0x56, 3, rb(HL))
#undef BIT2

#define BIT4(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, cpu.set_flags(!(o & 0x10) << SZ | MH, ~MC))
                    BIT4("BIT 4, A",        0x67, 2,     A)
                    BIT4("BIT 4, B",        0x60, 2,     B)
                    BIT4("BIT 4, C",        0x61, 2,     C)
                    BIT4("BIT 4, D",        0x62, 2,     D)
                    BIT4("BIT 4, E",        0x63, 2,     E)
                    BIT4("BIT 4, H",        0x64, 2,     H)
                    BIT4("BIT 4, L",        0x65, 2,     L)
                    BIT4("BIT 4, (HL)",     0x66, 3, rb(HL))
#undef BIT4

#define BIT6(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, cpu.set_flags(!(o & 0x40) << SZ | MH, ~MC))
                    BIT6("BIT 6, A",        0x77, 2,     A)
                    BIT6("BIT 6, B",        0x70, 2,     B)
                    BIT6("BIT 6, C",        0x71, 2,     C)
                    BIT6("BIT 6, D",        0x72, 2,     D)
                    BIT6("BIT 6, E",        0x73, 2,     E)
                    BIT6("BIT 6, H",        0x74, 2,     H)
                    BIT6("BIT 6, L",        0x75, 2,     L)
                    BIT6("BIT 6, (HL)",     0x76, 3, rb(HL))
#undef BIT6

#define BIT1(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, cpu.set_flags(!(o & 0x02) << SZ | MH, ~MC))
                    BIT1("BIT 1, A",        0x4F, 2,     A)
                    BIT1("BIT 1, B",        0x48, 2,     B)
                    BIT1("BIT 1, C",        0x49, 2,     C)
                    BIT1("BIT 1, D",        0x4A, 2,     D)
                    BIT1("BIT 1, E",        0x4B, 2,     E)
                    BIT1("BIT 1, H",        0x4C, 2,     H)
                    BIT1("BIT 1, L",        0x4D, 2,     L)
                    BIT1("BIT 1, (HL)",     0x4E, 3, rb(HL))
#undef BIT1

#define BIT3(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, cpu.set_flags(!(o & 0x08) << SZ | MH, ~MC))
                    BIT3("BIT 3, A",        0x5F, 2,     A)
                    BIT3("BIT 3, B",        0x58, 2,     B)
                    BIT3("BIT 3, C",        0x59, 2,     C)
                    BIT3("BIT 3, D",        0x5A, 2,     D)
                    BIT3("BIT 3, E",        0x5B, 2,     E)
                    BIT3("BIT 3, H",        0x5C, 2,     H)
                    BIT3("BIT 3, L",        0x5D, 2,     L)
                    BIT3("BIT 3, (HL)",     0x5E, 3, rb(HL))
#undef BIT3

#define BIT5(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, cpu.set_flags(!(o & 0x20) << SZ | MH, ~MC))
                    BIT5("BIT 5, A",        0x6F, 2,     A)
                    BIT5("BIT 5, B",        0x68, 2,     B)
                    BIT5("BIT 5, C",        0x69, 2,     C)
                    BIT5("BIT 5, D",        0x6A, 2,     D)
                    BIT5("BIT 5, E",        0x6B, 2,     E)
                    BIT5("BIT 5, H",        0x6C, 2,     H)
                    BIT5("BIT 5, L",        0x6D, 2,     L)
                    BIT5("BIT 5, (HL)",     0x6E, 3, rb(HL))
#undef BIT5

#define BIT7(mnemonic, opcode, cycles, o) ST(mnemonic, opcode, cycles, cpu.set_flags(!(o & 0x80) << SZ | MH, ~MC))
                    BIT7("BIT 7, A",        0x7F, 2,     A)
                    BIT7("BIT 7, B",        0x78, 2,     B)
                    BIT7("BIT 7, C",        0x79, 2,     C)
                    BIT7("BIT 7, D",        0x7A, 2,     D)
                    BIT7("BIT 7, E",        0x7B, 2,     E)
                    BIT7("BIT 7, H",        0x7C, 2,     H)
                    BIT7("BIT 7, L",        0x7D, 2,     L)
                    BIT7("BIT 7, (HL)",     0x7E, 3, rb(HL))
#undef BIT7

#define RES0(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o & ~0x01; assign_method)
                    RES0("RES 0, A",        0x87, 2,     A,     A = temp)
                    RES0("RES 0, B",        0x80, 2,     B,     B = temp)
                    RES0("RES 0, C",        0x81, 2,     C,     C = temp)
                    RES0("RES 0, D",        0x82, 2,     D,     D = temp)
                    RES0("RES 0, E",        0x83, 2,     E,     E = temp)
                    RES0("RES 0, H",        0x84, 2,     H,     H = temp)
                    RES0("RES 0, L",        0x85, 2,     L,     L = temp)
                    RES0("RES 0, (HL)",     0x86, 4, rb(HL), wb(HL, temp))
#undef RES0

#define RES2(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o & ~0x04; assign_method)
                    RES2("RES 2, A",        0x97, 2,     A,     A = temp)
                    RES2("RES 2, B",        0x90, 2,     B,     B = temp)
                    RES2("RES 2, C",        0x91, 2,     C,     C = temp)
                    RES2("RES 2, D",        0x92, 2,     D,     D = temp)
                    RES2("RES 2, E",        0x93, 2,     E,     E = temp)
                    RES2("RES 2, H",        0x94, 2,     H,     H = temp)
                    RES2("RES 2, L",        0x95, 2,     L,     L = temp)
                    RES2("RES 2, (HL)",     0x96, 4, rb(HL), wb(HL, temp))
#undef RES2

#define RES4(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o & ~0x10; assign_method)
                    RES4("RES 4, A",        0xA7, 2,     A,     A = temp)
                    RES4("RES 4, B",        0xA0, 2,     B,     B = temp)
                    RES4("RES 4, C",        0xA1, 2,     C,     C = temp)
                    RES4("RES 4, D",        0xA2, 2,     D,     D = temp)
                    RES4("RES 4, E",        0xA3, 2,     E,     E = temp)
                    RES4("RES 4, H",        0xA4, 2,     H,     H = temp)
                    RES4("RES 4, L",        0xA5, 2,     L,     L = temp)
                    RES4("RES 4, (HL)",     0xA6, 4, rb(HL), wb(HL, temp))
#undef RES4

#define RES6(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o & ~0x40; assign_method)
                    RES6("RES 6, A",        0xB7, 2,     A,     A = temp)
                    RES6("RES 6, B",        0xB0, 2,     B,     B = temp)
                    RES6("RES 6, C",        0xB1, 2,     C,     C = temp)
                    RES6("RES 6, D",        0xB2, 2,     D,     D = temp)
                    RES6("RES 6, E",        0xB3, 2,     E,     E = temp)
                    RES6("RES 6, H",        0xB4, 2,     H,     H = temp)
                    RES6("RES 6, L",        0xB5, 2,     L,     L = temp)
                    RES6("RES 6, (HL)",     0xB6, 4, rb(HL), wb(HL, temp))
#undef RES6

#define RES1(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o & ~0x02; assign_method)
                    RES1("RES 1, A",        0x8F, 2,     A,     A = temp)
                    RES1("RES 1, B",        0x88, 2,     B,     B = temp)
                    RES1("RES 1, C",        0x89, 2,     C,     C = temp)
                    RES1("RES 1, D",        0x8A, 2,     D,     D = temp)
                    RES1("RES 1, E",        0x8B, 2,     E,     E = temp)
                    RES1("RES 1, H",        0x8C, 2,     H,     H = temp)
                    RES1("RES 1, L",        0x8D, 2,     L,     L = temp)
                    RES1("RES 1, (HL)",     0x8E, 4, rb(HL), wb(HL, temp))
#undef RES1

#define RES3(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o & ~0x08; assign_method)
                    RES3("RES 3, A",        0x9F, 2,     A,     A = temp)
                    RES3("RES 3, B",        0x98, 2,     B,     B = temp)
                    RES3("RES 3, C",        0x99, 2,     C,     C = temp)
                    RES3("RES 3, D",        0x9A, 2,     D,     D = temp)
                    RES3("RES 3, E",        0x9B, 2,     E,     E = temp)
                    RES3("RES 3, H",        0x9C, 2,     H,     H = temp)
                    RES3("RES 3, L",        0x9D, 2,     L,     L = temp)
                    RES3("RES 3, (HL)",     0x9E, 4, rb(HL), wb(HL, temp))
#undef RES3

#define RES5(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o & ~0x20; assign_method)
                    RES5("RES 5, A",        0xAF, 2,     A,     A = temp)
                    RES5("RES 5, B",        0xA8, 2,     B,     B = temp)
                    RES5("RES 5, C",        0xA9, 2,     C,     C = temp)
                    RES5("RES 5, D",        0xAA, 2,     D,     D = temp)
                    RES5("RES 5, E",        0xAB, 2,     E,     E = temp)
                    RES5("RES 5, H",        0xAC, 2,     H,     H = temp)
                    RES5("RES 5, L",        0xAD, 2,     L,     L = temp)
                    RES5("RES 5, (HL)",     0xAE, 4, rb(HL), wb(HL, temp))
#undef RES5

#define RES7(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o & ~0x80; assign_method)
                    RES7("RES 7, A",        0xBF, 2,     A,     A = temp)
                    RES7("RES 7, B",        0xB8, 2,     B,     B = temp)
                    RES7("RES 7, C",        0xB9, 2,     C,     C = temp)
                    RES7("RES 7, D",        0xBA, 2,     D,     D = temp)
                    RES7("RES 7, E",        0xBB, 2,     E,     E = temp)
                    RES7("RES 7, H",        0xBC, 2,     H,     H = temp)
                    RES7("RES 7, L",        0xBD, 2,     L,     L = temp)
                    RES7("RES 7, (HL)",     0xBE, 4, rb(HL), wb(HL, temp))
#undef RES7

#define SET0(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o | 0x01; assign_method)
                    SET0("SET 0, A",        0xC7, 2,     A,     A = temp)
                    SET0("SET 0, B",        0xC0, 2,     B,     B = temp)
                    SET0("SET 0, C",        0xC1, 2,     C,     C = temp)
                    SET0("SET 0, D",        0xC2, 2,     D,     D = temp)
                    SET0("SET 0, E",        0xC3, 2,     E,     E = temp)
                    SET0("SET 0, H",        0xC4, 2,     H,     H = temp)
                    SET0("SET 0, L",        0xC5, 2,     L,     L = temp)
                    SET0("SET 0, (HL)",     0xC6, 4, rb(HL), wb(HL, temp))
#undef SET0

#define SET2(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o | 0x04; assign_method)
                    SET2("SET 2, A",        0xD7, 2,     A,     A = temp)
                    SET2("SET 2, B",        0xD0, 2,     B,     B = temp)
                    SET2("SET 2, C",        0xD1, 2,     C,     C = temp)
                    SET2("SET 2, D",        0xD2, 2,     D,     D = temp)
                    SET2("SET 2, E",        0xD3, 2,     E,     E = temp)
                    SET2("SET 2, H",        0xD4, 2,     H,     H = temp)
                    SET2("SET 2, L",        0xD5, 2,     L,     L = temp)
                    SET2("SET 2, (HL)",     0xD6, 4, rb(HL), wb(HL, temp))
#undef SET2

#define SET4(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o | 0x10; assign_method)
                    SET4("SET 4, A",        0xE7, 2,     A,     A = temp)
                    SET4("SET 4, B",        0xE0, 2,     B,     B = temp)
                    SET4("SET 4, C",        0xE1, 2,     C,     C = temp)
                    SET4("SET 4, D",        0xE2, 2,     D,     D = temp)
                    SET4("SET 4, E",        0xE3, 2,     E,     E = temp)
                    SET4("SET 4, H",        0xE4, 2,     H,     H = temp)
                    SET4("SET 4, L",        0xE5, 2,     L,     L = temp)
                    SET4("SET 4, (HL)",     0xE6, 4, rb(HL), wb(HL, temp))
#undef SET4

#define SET6(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o | 0x40; assign_method)
                    SET6("SET 6, A",        0xF7, 2,     A,     A = temp)
                    SET6("SET 6, B",        0xF0, 2,     B,     B = temp)
                    SET6("SET 6, C",        0xF1, 2,     C,     C = temp)
                    SET6("SET 6, D",        0xF2, 2,     D,     D = temp)
                    SET6("SET 6, E",        0xF3, 2,     E,     E = temp)
                    SET6("SET 6, H",        0xF4, 2,     H,     H = temp)
                    SET6("SET 6, L",        0xF5, 2,     L,     L = temp)
                    SET6("SET 6, (HL)",     0xF6, 4, rb(HL), wb(HL, temp))
#undef SET6

#define SET1(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o | 0x02; assign_method)
                    SET1("SET 1, A",        0xCF, 2,     A,     A = temp)
                    SET1("SET 1, B",        0xC8, 2,     B,     B = temp)
                    SET1("SET 1, C",        0xC9, 2,     C,     C = temp)
                    SET1("SET 1, D",        0xCA, 2,     D,     D = temp)
                    SET1("SET 1, E",        0xCB, 2,     E,     E = temp)
                    SET1("SET 1, H",        0xCC, 2,     H,     H = temp)
                    SET1("SET 1, L",        0xCD, 2,     L,     L = temp)
                    SET1("SET 1, (HL)",     0xCE, 4, rb(HL), wb(HL, temp))
#undef SET1

#define SET3(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o | 0x08; assign_method)
                    SET3("SET 3, A",        0xDF, 2,     A,     A = temp)
                    SET3("SET 3, B",        0xD8, 2,     B,     B = temp)
                    SET3("SET 3, C",        0xD9, 2,     C,     C = temp)
                    SET3("SET 3, D",        0xDA, 2,     D,     D = temp)
                    SET3("SET 3, E",        0xDB, 2,     E,     E = temp)
                    SET3("SET 3, H",        0xDC, 2,     H,     H = temp)
                    SET3("SET 3, L",        0xDD, 2,     L,     L = temp)
                    SET3("SET 3, (HL)",     0xDE, 4, rb(HL), wb(HL, temp))
#undef SET3

#define SET5(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o | 0x20; assign_method)
                    SET5("SET 5, A",        0xEF, 2,     A,     A = temp)
                    SET5("SET 5, B",        0xE8, 2,     B,     B = temp)
                    SET5("SET 5, C",        0xE9, 2,     C,     C = temp)
                    SET5("SET 5, D",        0xEA, 2,     D,     D = temp)
                    SET5("SET 5, E",        0xEB, 2,     E,     E = temp)
                    SET5("SET 5, H",        0xEC, 2,     H,     H = temp)
                    SET5("SET 5, L",        0xED, 2,     L,     L = temp)
                    SET5("SET 5, (HL)",     0xEE, 4, rb(HL), wb(HL, temp))
#undef SET5

#define SET7(mnemonic, opcode, cycles, o, assign_method) \
                            ST(mnemonic, opcode, cycles, const unsigned temp = o | 0x80; assign_method)
                    SET7("SET 7, A",        0xFF, 2,     A,     A = temp)
                    SET7("SET 7, B",        0xF8, 2,     B,     B = temp)
                    SET7("SET 7, C",        0xF9, 2,     C,     C = temp)
                    SET7("SET 7, D",        0xFA, 2,     D,     D = temp)
                    SET7("SET 7, E",        0xFB, 2,     E,     E = temp)
                    SET7("SET 7, H",        0xFC, 2,     H,     H = temp)
                    SET7("SET 7, L",        0xFD, 2,     L,     L = temp)
                    SET7("SET 7, (HL)",     0xFE, 4, rb(HL), wb(HL, temp)) {}
#undef SET7

#undef ST
                return cycles;
            }
        };

    public:
        void request_interrupts(unsigned mask) noexcept {IF |= mask;}
        unsigned next_step(const MMU& mmu) noexcept;
    };
}

#endif
