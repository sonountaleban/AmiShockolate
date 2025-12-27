/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.
Copyright (c) 2026 Giuseppe Perniola

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
/*
 * $Source: n:/project/lib/src/h/RCS/types.h $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/09/28 01:12:47 $
 *
 * extra typedefs and macros for use by all code.
 *
 * $Log: types.h $
 * Revision 1.2  1993/09/28  01:12:47  kaboom
 * Converted #include "xxx" to #include <xxx> for watcom.
 *
 * Revision 1.1  1993/03/19  18:19:27  matt
 * Initial revision
 */

#ifndef __TYPES_H
#define __TYPES_H

#ifdef AMIGA
#include <stdbool.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include <intuition/intuition.h>
#include <devices/timer.h>

#ifndef F_OK
#define F_OK 0
#endif

typedef UBYTE uchar;
typedef UBYTE Uint8;
typedef UBYTE uint8_t;
typedef BYTE byte;
typedef UBYTE ubyte;
typedef WORD int16_t;
typedef UWORD uint16_t;
typedef UWORD ushort;
typedef ULONG uint32_t;
typedef ULONG uint;
#define ulong UQUAD
typedef ULONG Uint32;
typedef LONG int32_t;
typedef SIPTR intptr_t;
typedef IPTR uintptr_t;
typedef QUAD int64_t;
typedef UQUAD uint64_t;
#else
#ifndef _H2INC		//don't redefine byte in assembly header
/* this is a signed byte */
typedef signed char byte;
#endif /* !_H2INC */

/* these are convenience typedefs so we don't always have to keep typing
   `unsigned.' */
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char ubyte;

//typedef unsigned char bool;
#endif

#ifndef NULL
#define NULL 0
#endif /* !NULL */

#ifndef TRUE
#define TRUE 1
#endif /* !TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* !FALSE */

#endif /* !__TYPES_H */
