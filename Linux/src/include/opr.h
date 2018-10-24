// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   opr.h
 *	This file defines the UART console application operations.
 *  Project:
 *	UartUpdateTool
 *---------------------------------------------------------------------------
 */

#ifndef _OPR_H_
#define _OPR_H_

#include "uut_types.h"
/*---------------------------------------------------------------------------
 * Constant definitions
 *---------------------------------------------------------------------------
 */

/* Baud rate scan steps: */
#define BR_BIG_STEP	20     /*in percents from current baud rate	      */
#define BR_MEDIUM_STEP	10     /* in percents from current baud rate	      */
#define BR_SMALL_STEP	1      /* in percents from current baud rate	      */
#define BR_MIN_STEP	5      /* in absolut baud rate units		      */
#define BR_LOW_LIMIT	400    /* Automatic BR detection starts at this value */
#define BR_HIGH_LIMIT	150000 /* Automatic BR detection ends at this value   */


#define OPR_WRITE_MEM		"wr"	/* Write To Memory/Flash	*/
#define OPR_READ_MEM		"rd"	/* Read From Memory/Flash	*/
#define OPR_EXECUTE_EXIT	"go"	/* Execute a non-return code	*/
#define OPR_EXECUTE_CONT	"call"	/* Execute returnable code	*/

enum SYNC_RESULT {
	SR_OK           =   0x00,
	SR_WRONG_DATA   =   0x01,
	SR_TIMEOUT      =   0x02,
	SR_ERROR        =   0x03
};

/*----------------------------------------------------------------------------
 * External Variables
 *---------------------------------------------------------------------------
 */
extern struct COMPORT_FIELDS	portCfg;

/*---------------------------------------------------------------------------
 * Functions prototypes
 *---------------------------------------------------------------------------
 */

void		OPR_Usage(void);
BOOLEAN		OPR_ClosePort(void);
BOOLEAN		OPR_OpenPort(const char *port_name,
			     struct COMPORT_FIELDS portCfg);
void		OPR_WriteMem(char *inputFileName, UINT32 addr, UINT32 size);
void		OPR_ReadMem(char *outputFileName, UINT32 addr, UINT32 size);
void		OPR_FlashEraseDevice(UINT32 devNum);
void		OPR_FlashEraseSector(UINT32 devNum, UINT32 addr);
void		OPR_ExecuteExit(UINT32 addr);
void		OPR_ExecuteReturn(UINT32 addr);
void		OPR_PrintFlashId(UINT32 devNum);
void		OPR_PrintFlashSts(UINT32 devNum);
void		OPR_UnlockFlash(UINT32 devNum);
BOOLEAN		OPR_WaitTillReady(UINT32 devNum);
void		OPR_GetFlashSts(UINT32 devNum);
BOOLEAN		OPR_ScanBaudRate(void);
enum SYNC_RESULT	OPR_CheckSync(UINT32 bdRate);
void		OPR_ReadStatusMsg(char *outputFileName);
#endif /* _OPR_H_ */
