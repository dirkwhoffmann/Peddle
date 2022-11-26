// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/*

#pragma once

#include <sys/types.h>

//
// Booleans
//

#ifndef __cplusplus
#include <cstdbool>
#endif


//
// Strings
//

#ifdef __cplusplus
#include <string>
#include <cstring>
#endif


//
// Integers
//

namespace peddle {

// Signed integers
typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;
typedef signed long        isize;

// Unsigned integers
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef unsigned long      usize;

//
// Enumerations
//

#if defined(__SWIFT__)

// Definition for Swift
#define peddle_enum_generic(_name, _type) \
typedef enum __attribute__((enum_extensibility(open))) _name : _type _name; \
enum _name : _type

#define peddle_enum_long(_name) peddle_enum_generic(_name, long)

#else

// Definition for C
#define peddle_enum_generic(_name, _type) \
typedef _type _name; \
enum : _type

#define peddle_enum_long(_name) peddle_enum_generic(_name, long)

#endif

}


*/
