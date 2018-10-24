// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018 Nuvoton Technologies, All Rights Reserved
 * --------------------------------------------------------------------------
 * File Contents:
 *   uut_types.c
 *            This file defines general types definition
 *  Project:
 *            UartUpdateTool
 *---------------------------------------------------------------------------
 */
#ifndef UUT_TYPES_H
#define UUT_TYPES_H


/*---------------------------------------------------------------------------------------------------------*/
/*                                        GENERIC TYPES DEFINITIONS                                        */
/*---------------------------------------------------------------------------------------------------------*/
typedef unsigned char  UINT8;                       /* Unsigned  8 bit quantity                            */
typedef signed   char  INT8;                        /* Signed    8 bit quantity                            */
typedef unsigned short UINT16;                      /* Unsigned 16 bit quantity                            */
typedef short          INT16;                       /* Signed   16 bit quantity                            */
typedef float          FP32;                        /* Single precision floating point                     */
typedef double         FP64;                        /* Double precision floating point                     */


#if ((~0ULL>>1) == 0x7FFFFFFFLL)
/*-----------------------------------------------------------------------------------------------------*/
/* unsigned long is 32bit (32bit Linux, 32bit Windows and 64bit Windows                                */
/*-----------------------------------------------------------------------------------------------------*/
typedef unsigned long   UINT32;                 /* Unsigned 32 bit quantity                            */
typedef signed   long   INT32;                  /* Signed   32 bit quantity                            */

#else
/*-----------------------------------------------------------------------------------------------------*/
/* unsigned long is 64bit (mostly on 64bit Linux systems)                                              */
/*-----------------------------------------------------------------------------------------------------*/
typedef unsigned int    UINT32;                 /* Unsigned 32 bit quantity                            */
typedef signed   int    INT32;                  /* Signed   32 bit quantity                            */

#endif


/*----------------------------------------------------------------------------
* Extracting Byte - 8 bit: MSB, LSB
*----------------------------------------------------------------------------
*/
#define MSB(u16)        ((UINT8)((UINT16)(u16) >> 8))
#define LSB(u16)        ((UINT8)(u16))

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------
*		UTILITY MACROS
*----------------------------------------------------------------------------
*----------------------------------------------------------------------------
*/

/*----------------------------------------------------------------------------
* calculate maximum value
*----------------------------------------------------------------------------
*/
#define MAX(a, b)               ((a) > (b) ? (a) : (b))

/*----------------------------------------------------------------------------
* calculate minimum value
*----------------------------------------------------------------------------
*/
#define MIN(a, b)               ((a) < (b) ? (a) : (b))

#if defined (_WIN32)
#include <windows.h>

/*---------------------------------------------------------------------------------------------------------*/
/* OS dependent types                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
typedef unsigned int        UINT;                       /* Native type of the core that fits the core's    */
typedef int                 INT;                        /* internal registers                              */

/* Windows already define BOOLEAN basic type - no need to define it here                                   */

/*---------------------------------------------------------------------------------------------------------*/
/* OS dependent PTR definitions                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
#define REG8                volatile  UINT8  *
#define REG16               volatile  UINT16 *
#define REG32               volatile  UINT32 *

#define PTR8                REG8 *
#define PTR16               REG16 *
#define PTR32               REG32 *


/*---------------------------------------------------------------------------------------------------------*/
/* Variables alignment                                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
#define _ALIGN_(x, decl)                        __declspec(align(x))  decl

/*---------------------------------------------------------------------------------------------------------*/
/* Variables packing                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
#define _PACK_(decl)                            decl

#else // LINUX

/*---------------------------------------------------------------------------------------------------------*/
/*                                                CONSTANTS                                                */
/*---------------------------------------------------------------------------------------------------------*/
#define BOOLEAN		int

#ifndef  FALSE
#define  FALSE      (BOOLEAN)0
#endif

#ifndef  TRUE
#define  TRUE       (BOOLEAN)1
#endif

#ifndef  NULL
#define  NULL       0
#endif

#define ENABLE      1
#define DISABLE     0


#endif // LINUX

#endif // UUT_TYPES_H
