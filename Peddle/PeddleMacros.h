// -----------------------------------------------------------------------------
// This file is part of Peddle - A MOS 65xx CPU emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include <cassert>

#define LO_BYTE(x) (u8)((x) & 0xFF)
#define HI_BYTE(x) (u8)((x) >> 8)

#define LO_HI(x,y) (u16)((y) << 8 | (x))
#define HI_LO(x,y) (u16)((x) << 8 | (y))

#ifdef _MSC_VER

#define unreachable    __assume(false)
#define likely(x)      (x)
#define unlikely(x)    (x)

#else

#define unreachable    __builtin_unreachable()
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#endif

#define fatalError     assert(false); unreachable


//
// Performing overflow-prone arithmetic
//

// Sanitizer friendly macros for adding signed offsets to integer values
#define U8_ADD(x,y) (u8)((i64)(x) + (i64)(y))
#define U8_SUB(x,y) (u8)((i64)(x) - (i64)(y))
#define U8_ADD3(x,y,z) (u8)((i64)(x) + (i64)(y) + (i64)(z))
#define U8_SUB3(x,y,z) (u8)((i64)(x) - (i64)(y) - (i64)(z))
#define U8_INC(x,y) x = U8_ADD(x,y)
#define U8_DEC(x,y) x = U8_SUB(x,y)

#define U16_ADD(x,y) (u16)((i64)(x) + (i64)(y))
#define U16_SUB(x,y) (u16)((i64)(x) - (i64)(y))
#define U16_ADD3(x,y,z) (u16)((i64)(x) + (i64)(y) + (i64)(z))
#define U16_SUB3(x,y,z) (u16)((i64)(x) - (i64)(y) - (i64)(z))
#define U16_INC(x,y) x = U16_ADD(x,y)
#define U16_DEC(x,y) x = U16_SUB(x,y)

#define U32_ADD(x,y) (u32)((i64)(x) + (i64)(y))
#define U32_SUB(x,y) (u32)((i64)(x) - (i64)(y))
#define U32_ADD3(x,y,z) (u32)((i64)(x) + (i64)(y) + (i64)(z))
#define U32_SUB3(x,y,z) (u32)((i64)(x) - (i64)(y) - (i64)(z))
#define U32_INC(x,y) x = U32_ADD(x,y)
#define U32_DEC(x,y) x = U32_SUB(x,y)

#define U64_ADD(x,y) (u64)((i64)(x) + (i64)(y))
#define U64_SUB(x,y) (u64)((i64)(x) - (i64)(y))
#define U64_ADD3(x,y,z) (u64)((i64)(x) + (i64)(y) + (i64)(z))
#define U64_SUB3(x,y,z) (u64)((i64)(x) - (i64)(y) - (i64)(z))
#define U64_INC(x,y) x = U64_ADD(x,y)
#define U64_DEC(x,y) x = U64_SUB(x,y)
