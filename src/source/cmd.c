// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018-2019 Nuvoton Technologies, All Rights Reserved
 *<<<-------------------------------------------------------------------------
 * File Contents:
 *   cmd.c
 *            This file creates the UART Program Protocol API commands.
 *  Project:
 *            UartUpdateTool
 *---------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>

#include "uut_types.h"
#include "program.h"
#include "cmd.h"
#include "lib_crc.h"

/*----------------------------------------------------------------------------
 *  SPI Flash commands
 *---------------------------------------------------------------------------
 */
#define SPI_READ_JEDEC_ID_CMD       0x9F
#define SPI_WRITE_ENABLE_CMD        0x06
#define SPI_WRITE_DISABLE_CMD       0x04
#define SPI_READ_STATUS_REG_CMD     0x05
#define SPI_WRITE_STATUS_REG_CMD    0x01
#define SPI_READ_DATA_CMD           0x03
#define SPI_PAGE_PRGM_CMD           0x02
#define SPI_SECTOR_ERASE_CMD        0xD8
#define SPI_BULK_ERASE_CMD          0xC7
#define SPI_READ_PID_CMD            0x90

/*---------------------------------------------------------------------------
 * Global types
 *---------------------------------------------------------------------------
 */
union cmd_addr {
	UINT8  c_adr[4];
	UINT32 h_adr;
};

extern UINT32          crc_type;  // 16/32

/*----------------------------------------------------------------------------
 * Functions implementation
 *---------------------------------------------------------------------------
 */
 /*----------------------------------------------------------------------------
 * Function:	CMD_CreateSetDevPortToHigh
 *
 * Parameters:	   cmdInfo - Pointer to a command buffer.
 *		   cmdLen  - Pointer to command length.
 * Returns:	   none.
 * Side effects:
 * Description:
 *		Create a set device port to high baudrate command.
 *		The total command length is written to 'cmdLen'.
 *---------------------------------------------------------------------------
 */
void CMD_CreateSetDevPortToHigh(UINT8  *cmdInfo, UINT32 *cmdLen)
{
	UINT32 len = 0;

	/* Build the command buffer */
	cmdInfo[len++] = UFPP_SET_HIGH_RATE_CMD;

	/* Return total command length */
	*cmdLen = len;
}

/*----------------------------------------------------------------------------
 * Function:	CMD_CreateSync
 *
 * Parameters:	   cmdInfo - Pointer to a command buffer.
 *		   cmdLen  - Pointer to command length.
 * Returns:	   none.
 * Side effects:
 * Description:
 *		Create a Host to Device SYNC protocol command.
 *		The total command length is written to 'cmdLen'.
 *---------------------------------------------------------------------------
 */
void CMD_CreateSync(UINT8  *cmdInfo, UINT32 *cmdLen)
{
	UINT32 len = 0;

	/* Build the command buffer */
	cmdInfo[len++] = UFPP_H2D_SYNC_CMD;

	/* Return total command length */
	*cmdLen = len;
}

/*---------------------------------------------------------------------------
 * Function:        CMD_CreateWrite
 *
 * Parameters:	addr    - Memory address to write to.
 *		size    - Size of daya (in bytes) to write to memory.
 *		dataBuf - Pointer to data buffer containing raw data to write.
 *		cmdInfo - Pointer to a command buffer.
 *		cmdLen  - Pointer to command length.
 * Returns:     none.
 * Side effects:
 * Description:
 *	Create a WRITE protocol command.
 *	The command buffer is enclosed by CRC, calculated on command
 *	information and raw data.
 *	The total command length is written to 'cmdLen'.
 *---------------------------------------------------------------------------
 */
void CMD_CreateWrite(UINT32  addr,
		     UINT32  size,
		     UINT8  *dataBuf,
		     UINT8  *cmdInfo,
		     UINT32 *cmdLen)
{
	UINT32		i;
	union cmd_addr	adr_tr;
	UINT16		crc = 0;
	UINT32		len = 0;

	/* Build the command buffer */
	cmdInfo[len++] = UFPP_WRITE_CMD;
	cmdInfo[len++] = (UINT8)(size - 1);

	/* Insert Address */
	adr_tr.h_adr = addr;
	cmdInfo[len++] = adr_tr.c_adr[3];
	cmdInfo[len++] = adr_tr.c_adr[2];
	cmdInfo[len++] = adr_tr.c_adr[1];
	cmdInfo[len++] = adr_tr.c_adr[0];

	/* Insert data */
	memcpy(&cmdInfo[len], dataBuf, size);
	len += size;

	/* Calculate CRC */

	for (i = 0; i < len; i++) {
		if (crc_type == 32)
			crc = update_crc32(crc, (char)cmdInfo[i]);
		else
			crc = update_crc16((UINT16)crc, (char)cmdInfo[i]);
	}

	/* Insert CRC */
	cmdInfo[len++] = MSB((UINT16)crc);
	cmdInfo[len++] = LSB((UINT16)crc);

	/* Return total command length */
	*cmdLen = len;
}

/*----------------------------------------------------------------------------
 * Function:	CMD_CreateRead
 *
 * Parameters:	addr    - Memory address to read from.
 *		size    - Size of daya (in bytes) to read from memory.
 *		cmdInfo - Pointer to a command buffer.
 *		cmdLen  - Pointer to command length.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Create a READ protocol command.
 *		The command buffer is enclosed by CRC, calculated on command
 *		information and raw data.
 *		The total command length is written to 'cmdLen'.
 *---------------------------------------------------------------------------
 */
void CMD_CreateRead(UINT32  addr, UINT8   size, UINT8  *cmdInfo, UINT32 *cmdLen)
{
	UINT32		i;
	union cmd_addr	adr_tr;
	UINT16		crc = 0;
	UINT32		len = 0;

	/* Build the command buffer */
	cmdInfo[len++] = UFPP_READ_CMD;
	cmdInfo[len++] = (UINT8)size;

	/* Insert Address */
	adr_tr.h_adr    = addr;
	cmdInfo[len++] = adr_tr.c_adr[3];
	cmdInfo[len++] = adr_tr.c_adr[2];
	cmdInfo[len++] = adr_tr.c_adr[1];
	cmdInfo[len++] = adr_tr.c_adr[0];

	/* Calculate CRC */
	for (i = 0; i < len; i++) {
		if (crc_type == 32)
			crc = update_crc32(crc, (char)cmdInfo[i]);
		else
			crc = update_crc16((UINT16)crc, (char)cmdInfo[i]);
	}

	/* Insert CRC */
	cmdInfo[len++] = MSB((UINT16)crc);
	cmdInfo[len++] = LSB((UINT16)crc);

	/* Return total command length */
	*cmdLen = len;
}

/*----------------------------------------------------------------------------
 * Function:	CMD_CreateExec
 *
 * Parameters:	addr    - Memory address to execute from.
 *		cmdInfo - Pointer to a command buffer.
 *		cmdLen  - Pointer to command length.
 * Returns:     none.
 * Side effects:
 * Description:
 *		Create an FCALL protocol command.
 *		The command buffer is enclosed by CRC, calculated on command
 *		information and raw data.
 *		The total command length is written to 'cmdLen'.
 *---------------------------------------------------------------------------
 */
void CMD_CreateExec(UINT32  addr, UINT8  *cmdInfo, UINT32 *cmdLen)
{
	UINT32		i;
	union cmd_addr	adr_tr;
	UINT16		crc = 0;
	UINT32		len = 0;

	/* Build the command buffer */
	cmdInfo[len++] = UFPP_FCALL_CMD;
	cmdInfo[len++] = 0;

	/* Insert Address */
	adr_tr.h_adr   = addr;
	cmdInfo[len++] = adr_tr.c_adr[3];
	cmdInfo[len++] = adr_tr.c_adr[2];
	cmdInfo[len++] = adr_tr.c_adr[1];
	cmdInfo[len++] = adr_tr.c_adr[0];

	/* Calculate CRC */
	for (i = 0; i < len; i++) {
		if (crc_type == 32)
			crc = update_crc32(crc, (char)cmdInfo[i]);
		else
			crc = update_crc16((UINT16)crc, (char)cmdInfo[i]);
	}

	/* Insert CRC */
	cmdInfo[len++] = MSB((UINT16)crc);
	cmdInfo[len++] = LSB((UINT16)crc);

	/* Return total command length */
	*cmdLen = len;
}

/*---------------------------------------------------------------------------
* Function:        CMD_BuildSetDevPortToHigh
*
* Parameters:	cmdBuf - Pointer to a command buffer.
*		cmdLen - Pointer to command length.
* Returns:	none.
* Description:
*		Build a set device to high baudrate command buffer.
*		The total command number is written to 'cmdNum'.
*---------------------------------------------------------------------------
*/
void CMD_BuildSetDevPortToHigh(struct ComandNode *cmdBuf, UINT32 *cmdNum)
{
	UINT32 nCmd = 0;

	CMD_CreateSetDevPortToHigh(cmdBuf[nCmd].cmd, &cmdBuf[nCmd].cmdSize);
	cmdBuf[nCmd].respSize = 0;
	nCmd++;

	*cmdNum = nCmd;
}

/*---------------------------------------------------------------------------
 * Function:        CMD_BuildSync
 *
 * Parameters:	cmdBuf - Pointer to a command buffer.
 *		cmdLen - Pointer to command length.
 * Returns:	none.
 * Description:
 *		Build a synchronization command buffer.
 *		The total command number is written to 'cmdNum'.
 *---------------------------------------------------------------------------
 */
void CMD_BuildSync(struct ComandNode *cmdBuf, UINT32       *cmdNum)
{
	UINT32 nCmd = 0;

	CMD_CreateSync(cmdBuf[nCmd].cmd, &cmdBuf[nCmd].cmdSize);
	cmdBuf[nCmd].respSize = 1;
	nCmd++;

	*cmdNum = nCmd;
}

/*----------------------------------------------------------------------------
 * Function:	CMD_BuildExecExit
 *
 * Parameters:	addr   - Memory address to execute from.
 *		cmdBuf - Pointer to a command buffer.
 *		cmdLen - Pointer to command length.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Build an Excute command buffer.
 *		Command does not expect the executed code to return, that is,
 *		only FCALL protocol
 *		command code is expected.
 *		Determine the expected response size per each command.
 *		The total command number is written to 'cmdNum'.
 *---------------------------------------------------------------------------
 */
void CMD_BuildExecExit(UINT32  addr, struct ComandNode *cmdBuf, UINT32 *cmdNum)
{
	UINT32 nCmd = 0;

	CMD_CreateExec(addr, cmdBuf[nCmd].cmd, &cmdBuf[nCmd].cmdSize);
	cmdBuf[nCmd].respSize = 1;
	nCmd++;

	*cmdNum = nCmd;
}

/*----------------------------------------------------------------------------
 * Function:	CMD_BuildExecRet
 *
 * Parameters:	addr   - Memory address to execute from.
 *		cmdBuf - Pointer to a command buffer.
 *		cmdLen - Pointer to command length.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Build an Excute command buffer.
 *		Command expects the executed code to return, that is,
 *		FCALL_RSLT protocol command
 *		code is expected, together with the execution result.
 *		Determine the expected response size per each command.
 *		The total command number is written to 'cmdNum'.
 *---------------------------------------------------------------------------
 */
void CMD_BuildExecRet(UINT32  addr, struct ComandNode *cmdBuf, UINT32  *cmdNum)
{
	UINT32 nCmd = 0;

	CMD_CreateExec(addr, cmdBuf[nCmd].cmd, &cmdBuf[nCmd].cmdSize);
	cmdBuf[nCmd].respSize = 3;
	nCmd++;

	*cmdNum = nCmd;
}

/*----------------------------------------------------------------------------
 * Function:	CMD_DispSync
 *
 * Parameters:	respBuf - Pointer to a response buffer.
 * Returns:	TRUE if successful, FALSE in the case of an error.
 * Side effects:
 * Description:
 *		Display SYNC command response information.
 *---------------------------------------------------------------------------
 */
BOOLEAN CMD_DispSync(UINT8 *respBuf)
{
	if (respBuf[0] == (UINT8)(UFPP_D2H_SYNC_CMD)) {
		displayColorMsg(SUCCESS, "Host/Device are syncronized\n");
		return TRUE;
	}

	displayColorMsg(FAIL, "Host/Device synchronization failed!!!\n");
	return FALSE;
}

/*----------------------------------------------------------------------------
 * Function:	CMD_DispWrite
 *
 * Parameters:	respBuf - Pointer to a response buffer.
 *		respSize - Response size.
 *		respNum - Response packet number.
 * Returns:	TRUE if successful, FALSE in the case of an error.
 * Side effects:
 * Description:
 *		Display WRITE command response information.
 *---------------------------------------------------------------------------
 */
BOOLEAN CMD_DispWrite(UINT8 *respBuf,
		      UINT32 respSize,
		      UINT32 respNum,
		      UINT32 totalSize)
{
	if (respBuf[0] == (UINT8)(UFPP_WRITE_CMD)) {
		displayColorMsg(SUCCESS,
	"\rTransmitted packet of size %lu bytes, packet [%lu]out of [%lu]",
			respSize, respNum, totalSize);
		return TRUE;
	}

	displayColorMsg(FAIL, "\nWrite packet [%lu] Failed\n", respNum);
	return FALSE;
}

/*-----------------------------------------------------------------------------
 * Function:	CMD_DispRead
 *
 * Parameters:	respBuf  - Pointer to a response buffer.
 *		respSize - Response size.
 *		respNum  - Response packet number.
 * Returns:	TRUE if successful, FALSE in the case of an error.
 * Side effects:
 * Description:
 *		Display READ command response information.
 *---------------------------------------------------------------------------
 */
BOOLEAN CMD_DispRead(UINT8 *respBuf,
		     UINT32 respSize,
		     UINT32 respNum,
		     UINT32 totalSize)
{
	if (respBuf[0] == (UINT8)(UFPP_READ_CMD)) {
		displayColorMsg(SUCCESS,
	"\rReceived packet of size %lu bytes, packet [%lu] out of [%lu]",
	respSize, respNum, totalSize);
		return TRUE;
	}

	displayColorMsg(FAIL, "\nRead packet [%lu] Failed\n", respNum);
	return FALSE;
}

/*----------------------------------------------------------------------------
 * Function:	CMD_DispData
 *
 * Parameters:	respBuf  - Pointer to a response buffer.
 *		respSize - Response size.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Display raw data, read from memory.
 *---------------------------------------------------------------------------
 */
void CMD_DispData(UINT8 *respBuf, UINT32 respSize)
{
	UINT32 idx;
	UINT32 i;

	for (idx = 0; idx < respSize; idx += 4) {
		if ((idx%16) == 0)
			printf("\n");

		printf("0x");
		for (i = 4; i > 0; i--)
			printf("%02x", respBuf[idx+i-1]);

		if ((idx%4) == 0)
			printf(" ");
	}

	printf("\n");
}

/*----------------------------------------------------------------------------
 * Function:	CMD_DispFlashEraseDev
 *
 * Parameters:	respBuf - Pointer to a response buffer.
 *		devNum  - Flash Device Number.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Display BULK_ERASE command response information.
 *---------------------------------------------------------------------------
 */
void CMD_DispFlashEraseDev(UINT8 *respBuf, UINT32 devNum)
{
	if (respBuf[0] == (UINT8)(UFPP_WRITE_CMD))
		displayColorMsg(SUCCESS,
				"Flash Erase of device [%lu] Passed\n",
				devNum);
	else
		displayColorMsg(FAIL,
				"Flash Erase of device [%lu] Failed\n",
				devNum);

}

/*---------------------------------------------------------------------------
 * Function:	CMD_DispFlashEraseSect
 *
 * Parameters:	respBuf - Pointer to a response buffer.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Display BULK_ERASE command response information.
 *---------------------------------------------------------------------------
 */
void CMD_DispFlashEraseSect(UINT8 *respBuf, UINT32 devNum)
{
	if (respBuf[0] == (UINT8)(UFPP_WRITE_CMD))
		displayColorMsg(SUCCESS,
				"Sector Erase of device [%lu] Passed\n",
				devNum);
	else
		displayColorMsg(FAIL,
				"Sector Erase of device [%lu] Failed\n",
				devNum);
}

/*---------------------------------------------------------------------------
 * Function:	CMD_DispExecExit
 *
 * Parameters:	respBuf - Pointer to a response buffer.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Display Execute command response information.
 *---------------------------------------------------------------------------
 */
void CMD_DispExecExit(UINT8 *respBuf)
{
	if (respBuf[0] == (UINT8)(UFPP_FCALL_CMD))
		displayColorMsg(SUCCESS, "Execute Command Passed\n");
	else
		displayColorMsg(FAIL, "Execute Command Failed\n");
}

/*---------------------------------------------------------------------------
 * Function:	CMD_DispExecRet
 *
 * Parameters:	respBuf - Pointer to a response buffer.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Display Execute Result command response information.
 *---------------------------------------------------------------------------
 */
void CMD_DispExecRet(UINT8 *respBuf)
{
	if (respBuf[1] == (UINT8)(UFPP_FCALL_RSLT_CMD))	{
		displayColorMsg(SUCCESS,
			"Execute Command Passed, execution result is [0x%X]\n",
			respBuf[2]);
	} else {
		displayColorMsg(FAIL,
			"Execute Command Failed  [0x%X]  [0x%X], rslt=[0x%X]\n",
			respBuf[0], respBuf[1], respBuf[2]);
	}
}

