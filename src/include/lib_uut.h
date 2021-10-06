// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2019 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   lib_uut.h
 *	This file defines the UART Programm Protocol API commands.
 *  Project:
 *	UartUpdateTool
 *--------------------------------------------------------------------------
 */
 #ifndef _LIB_UUT_H_
 #define _LIB_UUT_H_

 /*---------------------------------------------------------------------------
 * Functions prototypes
 *----------------------------------------------------------------------------
 */
#pragma once

__declspec(dllexport) int Init(UINT32 baudRate);
__declspec(dllexport) int OPR_WriteMem_DLL(UINT32 addr, const UINT8* buff, UINT32 size);
__declspec(dllexport) int OPR_ReadMem_DLL(UINT32 addr, UINT8* buff, UINT32 size);
__declspec(dllexport) int OPR_ExecuteReturn_DLL(UINT32 addr, UINT8* resp);

/*---------------------------------------------------------------------------
* Functions types
*----------------------------------------------------------------------------
*/
typedef int(*UUT_LIB_INIT)     (UINT32 baudRate);
typedef int(*UUT_LIB_WRITE)    (UINT32 addr, const UINT8* buff, UINT32 size);
typedef int(*UUT_LIB_READ)     (UINT32 addr, UINT8* buff, UINT32 size);
typedef int(*UUT_LIB_CALL)     (UINT32 addr, UINT8* resp);

#endif /* _LIB_UUT_H_ */
