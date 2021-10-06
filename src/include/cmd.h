// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018-2019 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   cmd.h
 *	This file defines the UART Programm Protocol API commands.
 *  Project:
 *	UartUpdateTool
 *--------------------------------------------------------------------------
 */

#ifndef _CMD_H_
#define _CMD_H_

/*---------------------------------------------------------------------------
 * Constant definitions
 *---------------------------------------------------------------------------
 */
#define MAX_CMD_BUF_SIZE    10
#define MAX_RESP_BUF_SIZE   512

/*---------------------------------------------------------------------------
 * Global types
 *---------------------------------------------------------------------------
 */
enum uart_protocol_cmd {
	UFPP_H2D_SYNC_CMD		= 0x55,	/* Single-Byte Host to Device		*/
									/* synchronization command			*/
	UFPP_D2H_SYNC_CMD		= 0x5A,	/* Single-Byte Device to Host		*/
									/* synchronization response			*/
	UFPP_WRITE_CMD			= 0x07,	/* Write command and response		*/
	UFPP_READ_CMD			= 0x1C,	/* Read command and response		*/
	UFPP_READ_CRC_CMD		= 0x89,	/* Read CRC command and response	*/
	UFPP_FCALL_CMD			= 0x70,	/* Call function command			*/
	UFPP_FCALL_RSLT_CMD		= 0x73,	/* Call function response			*/
	UFPP_SPI_CMD			= 0x92,	/* SPI specific command				*/
	UFPP_SET_HIGH_RATE_CMD	= 0xA0, /* Set device rate command.			*/
	UFPP_ERROR_CMD			= 0xFF	/* Error response					*/
};

struct ComandNode {
	UINT8  cmd[512];
	UINT32 cmdSize;
	UINT32 respSize;
};

/*---------------------------------------------------------------------------
 * Functions prototypes
 *---------------------------------------------------------------------------
 */
void    CMD_CreateSetDevPortToHigh(UINT8  *cmdInfo, UINT32 *cmdLen);
void    CMD_CreateSync(UINT8 *cmdInfo, UINT32 *cmdLen);
void    CMD_CreateWrite(UINT32 addr, UINT32 size, UINT8 *dataBuf,
			UINT8 *cmdInfo, UINT32 *cmdLen);
void    CMD_CreateRead(UINT32 addr, UINT8 size, UINT8 *cmdInfo, UINT32 *cmdLen);
void    CMD_CreateExec(UINT32 addr, UINT8 *cmdInfo, UINT32 *cmdLen);

void    CMD_BuildSetDevPortToHigh(struct ComandNode *cmdBuf, UINT32 *cmdNum);
void    CMD_BuildSync(struct ComandNode *cmdBuf, UINT32 *cmdNum);
void    CMD_BuildExecExit(UINT32 addr,
			  struct ComandNode *cmdBuf,
			  UINT32 *cmdNum);
void    CMD_BuildExecRet(UINT32 addr,
			 struct ComandNode *cmdBuf,
			 UINT32 *cmdNum);
void    CMD_BuildRomCfg(struct ComandNode *cmdBuf, UINT32 *cmdNum);

BOOLEAN CMD_DispSync(UINT8 *respBuf);
BOOLEAN CMD_DispWrite(UINT8 *respBuf, UINT32 respSize,
		      UINT32 respNum, UINT32 totalSize);
BOOLEAN CMD_DispRead(UINT8 *respBuf, UINT32 respSize,
		     UINT32 respNum, UINT32 totalSize);
void    CMD_DispData(UINT8 *respBuf, UINT32 respSize);
void    CMD_DispFlashEraseDev(UINT8 *respBuf, UINT32 devNum);
void    CMD_DispFlashEraseSect(UINT8 *respBuf, UINT32 devNum);
void    CMD_DispFlashId(UINT8 *respBuf, UINT32 devNum);
void    CMD_DispFlashSts(UINT8 *respBuf, UINT32 devNum);
void    CMD_DispExecExit(UINT8 *respBuf);
void    CMD_DispExecRet(UINT8 *respBuf);
void    CMD_DispRomCfg(UINT8 *respBuf, UINT32 respType);

#endif /* _CMD_H_ */
