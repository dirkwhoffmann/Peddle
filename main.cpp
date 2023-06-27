// -----------------------------------------------------------------------------
// This file is part of Peddle - A MOS 65xx CPU emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "Peddle.h"
#include <iostream>
#include <cstdio>

using namespace peddle;

unsigned char ram[65536] = { };

unsigned char prog[] =
{
    0xA2, 0x01,             // LDX #$01
    0x8E, 0x00, 0x02,       // STX $0200
    0x8E, 0x01, 0x02,       // STX $0201
    0xCA,                   // DEX
    0xBD, 0x00, 0x02,       // LDA $0200,X
    0x7D, 0x01, 0x02,       // ADC $0201,X
    0x9D, 0x02, 0x02,       // STA $0202,X
    0xE8,                   // INX
    0xE0, 0x08,             // CPX #$08
    0xD0, 0xF2,             // BNE $0609
    0x00                    // BRK
};


class CPU : public peddle::Peddle {

    u8 read(u16 addr) override { return ram[addr]; }
    void write(u16 addr, u8 val) override { ram[addr] = val; }
    u8 readDasm(u16 addr) const override { return ram[addr]; }

public:

    void dump()
    {
        char instr[64];
        auto len = disassembler.disassemble(instr, getPC0());

        printf("%04X %02X %02X %02X %02X %02X  %d%d1%d%d%d%d%d %-15s",
               getPC0(), getP(), reg.a, reg.x, reg.y, reg.sp,
               getN(), getV(), getB(), getD(), getI(), getZ(), getC(), instr
               );

        for (int i = 0; i < 10; i++) {
            printf("%3d ", ram[0x200 + i]);
        }
        printf("\n");
    }
};

int main(int argc, const char * argv[]) {

    CPU cpu;

    // Copy the test program to memory, starting at 0x600
    memcpy(ram + 0x600, prog, sizeof(prog));

    // Initialize the reset vector at $FFFC / $FFFD
    ram[0xFFFC] = 0x00;
    ram[0xFFFD] = 0x06;

    // Reset the CPU
    cpu.reset();

    printf("Peddle - A MOS Technology 65xx CPU emulator\n\n");
    printf("Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de\n");
    printf("Published under the terms of the MIT License\n\n");

    printf("Test program:\n\n");


    // Disassemble the program
    u16 len;
    char instr[64], bytes[64];
    for (u16 addr = 0x600; addr < 0x600 + sizeof(prog); addr += len) {

        len = (u16)cpu.disassembler.disassemble(instr, addr);
        cpu.disassembler.dumpBytes(bytes, addr, len);

        printf("%04X: %-10s %s\n", addr, bytes, instr);
    }

    printf("\nInstruction trace:\n\n");
    printf(" PC  SR AC XR YR SP  NV-BDIZC\n");

    // Run the program
    while (ram[cpu.getPC0()]) {

        cpu.executeInstruction();
        cpu.dump();
    };

    // Verify the result
    u8 expected[] = { 1, 1, 2, 3, 5, 8, 13, 21, 34, 55 };
    for (int i = 0; i < sizeof(expected); i++) {

        u16 addr = 0x200 + i;

        if (ram[addr] != expected[i]) {

            printf("\nERROR: ram[%04x] = %02x. Expected: %02x\n", addr, ram[addr], expected[i]);
            return 1;
        }
    }

    printf("\nSUCCESS\n");
    return 0;
}
