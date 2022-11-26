// -----------------------------------------------------------------------------
// This file is part of Peddle - A MOS 65xx CPU emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#include "PeddleConfig.h"
#include "Peddle.h"

namespace peddle {

//
// Printing
//

static void
sprint8d(char *s, u8 value)
{
    for (int i = 2; i >= 0; i--) {

        u8 digit = value % 10;
        s[i] = '0' + digit;
        value /= 10;
    }
    s[3] = 0;
}

static void
sprint8x(char *s, u8 value)
{
    for (int i = 1; i >= 0; i--) {

        u8 digit = value % 16;
        s[i] = (digit <= 9) ? ('0' + digit) : ('A' + digit - 10);
        value /= 16;
    }
    s[2] = 0;
}

static void
sprint16d(char *s, u16 value)
{
    for (int i = 4; i >= 0; i--) {

        u8 digit = value % 10;
        s[i] = '0' + digit;
        value /= 10;
    }
    s[5] = 0;
}

static void
sprint16x(char *s, u16 value)
{
    for (int i = 3; i >= 0; i--) {

        u8 digit = value % 16;
        s[i] = (digit <= 9) ? ('0' + digit) : ('A' + digit - 10);
        value /= 16;
    }
    s[4] = 0;
}


//
// Guard
//

bool
Guard::eval(u32 addr)
{
    if (this->addr == addr && this->enabled) {
        if (++hits > skip) {
            return true;
        }
    }
    return false;
}

//
// Guards
//

Guards::~Guards()
{
    assert(guards);
    delete [] guards;
}

Guard *
Guards::guardWithNr(long nr) const
{
    return nr < count ? &guards[nr] : nullptr;
}

Guard *
Guards::guardAtAddr(u32 addr) const
{
    for (int i = 0; i < count; i++) {
        if (guards[i].addr == addr) return &guards[i];
    }

    return nullptr;
}

bool
Guards::isSetAt(u32 addr) const
{
    Guard *guard = guardAtAddr(addr);

    return guard != nullptr;
}

bool
Guards::isSetAndEnabledAt(u32 addr) const
{
    Guard *guard = guardAtAddr(addr);

    return guard != nullptr && guard->enabled;
}

bool
Guards::isSetAndDisabledAt(u32 addr) const
{
    Guard *guard = guardAtAddr(addr);

    return guard != nullptr && !guard->enabled;
}

bool
Guards::isSetAndConditionalAt(u32 addr) const
{
    Guard *guard = guardAtAddr(addr);

    return guard != nullptr && guard->skip != 0;
}

void
Guards::addAt(u32 addr, long skip)
{
    if (isSetAt(addr)) return;

    if (count >= capacity) {

        Guard *newguards = new Guard[2 * capacity];
        for (long i = 0; i < capacity; i++) newguards[i] = guards[i];
        delete [] guards;
        guards = newguards;
        capacity *= 2;
    }

    guards[count].addr = addr;
    guards[count].enabled = true;
    guards[count].hits = 0;
    guards[count].skip = skip;
    count++;
    setNeedsCheck(true);
}

void
Guards::remove(long nr)
{
    if (nr < count) removeAt(guards[nr].addr);
}

void
Guards::removeAt(u32 addr)
{
    for (int i = 0; i < count; i++) {

        if (guards[i].addr == addr) {

            for (int j = i; j + 1 < count; j++) guards[j] = guards[j + 1];
            count--;
            break;
        }
    }
    setNeedsCheck(count != 0);
}

void
Guards::replace(long nr, u32 addr)
{
    if (nr >= count || isSetAt(addr)) return;
    
    guards[nr].addr = addr;
    guards[nr].hits = 0;
}

bool
Guards::isEnabled(long nr) const
{
    return nr < count ? guards[nr].enabled : false;
}

void
Guards::setEnable(long nr, bool val)
{
    if (nr < count) guards[nr].enabled = val;
}

void
Guards::setEnableAt(u32 addr, bool value)
{
    Guard *guard = guardAtAddr(addr);
    if (guard) guard->enabled = value;
}

bool
Guards::eval(u32 addr)
{
    for (int i = 0; i < count; i++)
        if (guards[i].eval(addr)) return true;

    return false;
}

void
Breakpoints::setNeedsCheck(bool value)
{
    if (value) {
        cpu.flags |= CPU_CHECK_BP;
    } else {
        cpu.flags &= ~CPU_CHECK_BP;
    }
}

void
Watchpoints::setNeedsCheck(bool value)
{
    if (value) {
        cpu.flags |= CPU_CHECK_WP;
    } else {
        cpu.flags &= ~CPU_CHECK_WP;
    }
}

//
// Debugger
//

void
Debugger::reset()
{
    breakpoints.setNeedsCheck(breakpoints.elements() != 0);
    watchpoints.setNeedsCheck(watchpoints.elements() != 0);
    clearLog();
}

void
Debugger::registerInstruction(u8 opcode, const char *mnemonic, AddressingMode mode)
{
    this->mnemonic[opcode] = mnemonic;
    this->addressingMode[opcode] = mode;
}

void
Debugger::setSoftStop(u64 addr)
{
    softStop = addr;
    breakpoints.setNeedsCheck(true);
}

bool
Debugger::breakpointMatches(u32 addr)
{
    // Check if a soft breakpoint has been reached
    if (addr == softStop || softStop == UINT64_MAX) {

        // Soft breakpoints are deleted when reached
        softStop = UINT64_MAX - 1;
        breakpoints.setNeedsCheck(breakpoints.elements() != 0);

        return true;
    }

    if (!breakpoints.eval(addr)) return false;

    breakpointPC = cpu.reg.pc;
    return true;
}

bool
Debugger::watchpointMatches(u32 addr)
{
    if (!watchpoints.eval(addr)) return false;
    
    watchpointPC = cpu.reg.pc0;
    return true;
}

void
Debugger::enableLogging()
{
    cpu.flags |= CPU_LOG_INSTRUCTION;
}

void
Debugger::disableLogging()
{
    cpu.flags &= ~CPU_LOG_INSTRUCTION;
}

isize
Debugger::loggedInstructions() const
{
    return logCnt < LOG_BUFFER_CAPACITY ? logCnt : LOG_BUFFER_CAPACITY;
}

void
Debugger::logInstruction()
{
    u16 pc = cpu.getPC0();
    u8 opcode = cpu.readDasm(pc);
    isize length = getLengthOfInstruction(opcode);

    isize i = logCnt++ % LOG_BUFFER_CAPACITY;
    
    logBuffer[i].cycle = cpu.clock;
    logBuffer[i].pc = pc;
    logBuffer[i].sp = cpu.reg.sp;
    logBuffer[i].byte1 = opcode;
    logBuffer[i].byte2 = length > 1 ? cpu.readDasm(pc + 1) : 0;
    logBuffer[i].byte3 = length > 2 ? cpu.readDasm(pc + 2) : 0;
    logBuffer[i].a = cpu.reg.a;
    logBuffer[i].x = cpu.reg.x;
    logBuffer[i].y = cpu.reg.y;
    logBuffer[i].flags = cpu.getP();
}

const RecordedInstruction &
Debugger::logEntryRel(isize n) const
{
    assert(n < loggedInstructions());
    return logBuffer[(logCnt - 1 - n) % LOG_BUFFER_CAPACITY];
}

const RecordedInstruction &
Debugger::logEntryAbs(isize n) const
{
    assert(n < loggedInstructions());
    return logEntryRel(loggedInstructions() - n - 1);
}

u16
Debugger::loggedPC0Rel(isize n) const
{
    assert(n < loggedInstructions());
    return logBuffer[(logCnt - 1 - n) % LOG_BUFFER_CAPACITY].pc;
}

u16
Debugger::loggedPC0Abs(isize n) const
{
    assert(n < loggedInstructions());
    return loggedPC0Rel(loggedInstructions() - n - 1);
}

isize
Debugger::getLengthOfInstruction(u8 opcode) const
{
    switch(addressingMode[opcode]) {
        case ADDR_IMPLIED:
        case ADDR_ACCUMULATOR:
            return 1;
        case ADDR_IMMEDIATE:
        case ADDR_ZERO_PAGE:
        case ADDR_ZERO_PAGE_X:
        case ADDR_ZERO_PAGE_Y:
        case ADDR_INDIRECT_X:
        case ADDR_INDIRECT_Y:
        case ADDR_RELATIVE:
            return 2;
        case ADDR_ABSOLUTE:
        case ADDR_ABSOLUTE_X:
        case ADDR_ABSOLUTE_Y:
        case ADDR_DIRECT:
        case ADDR_INDIRECT:
            return 3;
    }
    return 1;
}

isize
Debugger::getLengthOfInstructionAtAddress(u16 addr) const
{
    return getLengthOfInstruction(cpu.readDasm(addr));
}

isize
Debugger::getLengthOfCurrentInstruction() const
{
    return getLengthOfInstructionAtAddress(cpu.getPC0());
}

u16
Debugger::getAddressOfNextInstruction() const
{
    return (u16)(cpu.getPC0() + getLengthOfCurrentInstruction());
}

const char *
Debugger::disassembleRecordedInstr(int i, long *len) const
{
    return disassembleInstr(logEntryAbs(i), len);
}

const char *
Debugger::disassembleRecordedBytes(int i) const
{
    return disassembleBytes(logEntryAbs(i));
}

const char *
Debugger::disassembleRecordedFlags(int i) const
{
    return disassembleRecordedFlags(logEntryAbs(i));
}

const char *
Debugger::disassembleRecordedPC(int i) const
{
    return disassembleAddr(logEntryAbs(i).pc);
}

const char *
Debugger::disassembleInstr(u16 addr, long *len) const
{
    RecordedInstruction instr;
    
    instr.pc = addr;
    instr.byte1 = cpu.readDasm(addr);
    instr.byte2 = cpu.readDasm(addr + 1);
    instr.byte3 = cpu.readDasm(addr + 2);
    
    return disassembleInstr(instr, len);
}

const char *
Debugger::disassembleBytes(u16 addr) const
{
    RecordedInstruction instr;

    instr.byte1 = cpu.readDasm(addr);
    instr.byte2 = cpu.readDasm(addr + 1);
    instr.byte3 = cpu.readDasm(addr + 2);

    return disassembleBytes(instr);
}

const char *
Debugger::disassembleAddr(u16 addr) const
{
    static char result[6];

    hex ? sprint16x(result, addr) : sprint16d(result, addr);
    return result;
}

const char *
Debugger::disassembleInstruction(long *len) const
{
    return disassembleInstr(cpu.getPC0(), len);
}

const char *
Debugger::disassembleDataBytes() const
{
    return disassembleBytes(cpu.getPC0());
}

const char *
Debugger::disassemblePC() const
{
    return disassembleAddr(cpu.getPC0());
}

const char *
Debugger::disassembleInstr(const RecordedInstruction &instr, long *len) const
{
    if (hex) {
        return disassembleInstr<true>(instr, len);
    } else {
        return disassembleInstr<false>(instr, len);
    }
}

template <bool hex> const char *
Debugger::disassembleInstr(const RecordedInstruction &instr, long *len) const
{
    static char result[16];

    u8 opcode = instr.byte1;
    if (len) *len = getLengthOfInstruction(opcode);

    // Convert command
    char operand[6];
    switch (addressingMode[opcode]) {
            
        case ADDR_IMMEDIATE:
        case ADDR_ZERO_PAGE:
        case ADDR_ZERO_PAGE_X:
        case ADDR_ZERO_PAGE_Y:
        case ADDR_INDIRECT_X:
        case ADDR_INDIRECT_Y:
        {
            u8 value = instr.byte2;
            hex ? sprint8x(operand, value) : sprint8d(operand, value);
            break;
        }
        case ADDR_DIRECT:
        case ADDR_INDIRECT:
        case ADDR_ABSOLUTE:
        case ADDR_ABSOLUTE_X:
        case ADDR_ABSOLUTE_Y:
        {
            u16 value = LO_HI(instr.byte2, instr.byte3);
            hex ? sprint16x(operand, value) : sprint16d(operand, value);
            break;
        }
        case ADDR_RELATIVE:
        {
            u16 value = (u16)(instr.pc + 2 + (i8)instr.byte2);
            hex ? sprint16x(operand, value) : sprint16d(operand, value);
            break;
        }
        default:
            break;
    }
    
    switch (addressingMode[opcode]) {
            
        case ADDR_IMPLIED:
        case ADDR_ACCUMULATOR:
            
            std::strcpy(result, "xxx");
            break;
            
        case ADDR_IMMEDIATE:
            
            std::strcpy(result, hex ? "xxx #hh" : "xxx #ddd");
            std::memcpy(&result[5], operand, hex ? 2 : 3);
            break;
            
        case ADDR_ZERO_PAGE:
            
            std::strcpy(result, hex ? "xxx hh" : "xxx ddd");
            std::memcpy(&result[4], operand, hex ? 2 : 3);
            break;
            
        case ADDR_ZERO_PAGE_X:
            
            std::strcpy(result, hex ? "xxx hh,X" : "xxx ddd,X");
            std::memcpy(&result[4], operand, hex ? 2 : 3);
            break;
            
        case ADDR_ZERO_PAGE_Y:
            
            std::strcpy(result, hex ? "xxx hh,Y" : "xxx ddd,Y");
            std::memcpy(&result[4], operand, hex ? 2 : 3);
            break;
            
        case ADDR_ABSOLUTE:
        case ADDR_DIRECT:
            
            std::strcpy(result, hex ? "xxx hhhh" : "xxx ddddd");
            std::memcpy(&result[4], operand, hex ? 4 : 5);
            break;
            
        case ADDR_ABSOLUTE_X:
            
            std::strcpy(result, hex ? "xxx hhhh,X" : "xxx ddddd,X");
            std::memcpy(&result[4], operand, hex ? 4 : 5);
            break;
            
        case ADDR_ABSOLUTE_Y:
            
            std::strcpy(result, hex ? "xxx hhhh,Y" : "xxx ddddd,Y");
            std::memcpy(&result[4], operand, hex ? 4 : 5);
            break;
            
        case ADDR_INDIRECT:
            
            std::strcpy(result, hex ? "xxx (hhhh)" : "xxx (ddddd)");
            std::memcpy(&result[5], operand, hex ? 4 : 5);
            break;
            
        case ADDR_INDIRECT_X:
            
            std::strcpy(result, hex ? "xxx (hh,X)" : "xxx (ddd,X)");
            std::memcpy(&result[5], operand, hex ? 2 : 3);
            break;
            
        case ADDR_INDIRECT_Y:
            
            std::strcpy(result, hex ? "xxx (hh),Y" : "xxx (ddd),Y");
            std::memcpy(&result[5], operand, hex ? 2 : 3);
            break;
            
        case ADDR_RELATIVE:
            
            std::strcpy(result, hex ? "xxx hhhh" : "xxx ddddd");
            std::memcpy(&result[4], operand, hex ? 4 : 5);
            break;
            
        default:
            
            std::strcpy(result, "???");
    }
    
    // Copy mnemonic
    strncpy(result, mnemonic[opcode], 3);
    
    return result;
}

const char *
Debugger::disassembleBytes(const RecordedInstruction &instr) const
{
    static char result[13]; char *ptr = result;
    
    isize len = getLengthOfInstruction(instr.byte1);
    
    if (hex) {
        
        if (len >= 1) { sprint8x(ptr, instr.byte1); ptr[2] = ' '; ptr += 3; }
        if (len >= 2) { sprint8x(ptr, instr.byte2); ptr[2] = ' '; ptr += 3; }
        if (len >= 3) { sprint8x(ptr, instr.byte3); ptr[2] = ' '; ptr += 3; }
        
    } else {
        
        if (len >= 1) { sprint8d(ptr, instr.byte1); ptr[3] = ' '; ptr += 4; }
        if (len >= 2) { sprint8d(ptr, instr.byte2); ptr[3] = ' '; ptr += 4; }
        if (len >= 3) { sprint8d(ptr, instr.byte3); ptr[3] = ' '; ptr += 4; }
    }
    ptr[0] = 0;
    
    return result;
}

const char *
Debugger::disassembleRecordedFlags(const RecordedInstruction &instr) const
{
    static char result[9];
    
    result[0] = (instr.flags & N_FLAG) ? 'N' : 'n';
    result[1] = (instr.flags & V_FLAG) ? 'V' : 'v';
    result[2] = '-';
    result[3] = (instr.flags & B_FLAG) ? 'B' : 'b';
    result[4] = (instr.flags & D_FLAG) ? 'D' : 'd';
    result[5] = (instr.flags & I_FLAG) ? 'I' : 'i';
    result[6] = (instr.flags & Z_FLAG) ? 'Z' : 'z';
    result[7] = (instr.flags & C_FLAG) ? 'C' : 'c';
    result[8] = 0;
    
    return result;
}

}
