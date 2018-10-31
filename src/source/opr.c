// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   opr.cpp
 *	This file implements the UART console application operations.
 *  Project:
 *	UartUpdateTool
 *---------------------------------------------------------------------------
 */

#include <stdio.h>
#ifndef WIN32
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif
#include <time.h>

#include "uut_types.h"
#include "ComPort.h"
#include "main.h"
#include "opr.h"
#include "cmd.h"

/*----------------------------------------------------------------------------
 * Global Variables
 *---------------------------------------------------------------------------
 */
HANDLE portHandle = INVALID_HANDLE_VALUE;


/*----------------------------------------------------------------------------
 * Constant definitions
 *---------------------------------------------------------------------------
 */
#define MAX_PORT_NAME_SIZE	32
#define OPR_TIMEOUT		10L		/* 10  seconds */
#define FLASH_ERASE_TIMEOUT	120L	/* 120 seconds */

#define STS_MSG_MIN_SIZE	8
#define STS_MSG_APP_END		0x09
#define DUMMY_SIZE		2

#define MAX_SYNC_TRIALS		3

/*----------------------------------------------------------------------------
 * Internal types
 *---------------------------------------------------------------------------
 */
struct STATUS_MSG {
	UINT32	status;
	UINT32	dataSize;
	UINT8	data[DUMMY_SIZE];
};


/*----------------------------------------------------------------------------
 * Global variables
 *---------------------------------------------------------------------------
 */
struct ComandNode	cmdBuf[MAX_CMD_BUF_SIZE];
UINT8			respBuf[MAX_RESP_BUF_SIZE];


/*---------------------------------------------------------------------------
 * Functions prototypes
 *---------------------------------------------------------------------------
 */
static BOOLEAN OPR_SendCmds(struct ComandNode *cmdBuf, UINT32 cmdNum);

/*----------------------------------------------------------------------------
 * Function implementation
 *----------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------
 * Function:	OPR_Usage
 *
 * Parameters:	none.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Prints the console application operation menu.
 *---------------------------------------------------------------------------
 */
void OPR_Usage(void)
{
	printf("Operations:\n");
	printf("       %s\t\t- Write To Memory/Flash\n", OPR_WRITE_MEM);
	printf("       %s\t\t- Read From Memory/Flash\n", OPR_READ_MEM);
	printf("       %s\t\t- Execute a non-return code\n", OPR_EXECUTE_EXIT);
	printf("       %s\t\t- Execute a returnable code\n", OPR_EXECUTE_CONT);
	printf("       %s\t\t- Scan all ports. Output is saved to  SerialPortNumber.txt\n", OPR_SCAN);
}

/*----------------------------------------------------------------------------
 * Function:	OPR_ClosePort
 *
 * Parameters:	none
 * Returns:
 * Side effects:
 * Description:
 *		This routine closes the opened COM port by the application
 *---------------------------------------------------------------------------
 */
BOOLEAN OPR_ClosePort(void)
{
	return ComPortClose(portHandle);
}


/*----------------------------------------------------------------------------
 * Function:        OPR_OpenPort
 *
 * Parameters:	port_name - COM Port name.
 *		portCfg - COM Port configuration structure.
 * Returns:	1 if successful, 0 in the case of an error.
 * Side effects:
 * Description:
 *		Open a specified ComPort device.
 *---------------------------------------------------------------------------
 */
BOOLEAN OPR_OpenPort(const char  *port_name, struct COMPORT_FIELDS portCfg)
{
	char full_port_name[MAX_PORT_NAME_SIZE];

#ifdef WIN32
	strcpy(full_port_name, "\\\\.\\");
#else
	strcpy(full_port_name, "/dev/");
#endif

	strcat(full_port_name, port_name);

	if ((INT32)portHandle > 0)
		ComPortClose(portHandle);

	portHandle = ComPortOpen((const char *) full_port_name, portCfg);

	if ((INT32)portHandle <= 0) {
		displayColorMsg(FAIL, "\nERROR: COM Port failed to open.\n");
		DISPLAY_MSG(
	   ("Please select the right serial port or check if other serial\n"));
		DISPLAY_MSG(("communication applications are opened.\n"));
		return FALSE;
	}

	displayColorMsg(SUCCESS, "Port %s Opened\n", full_port_name);

	return TRUE;
}



/*----------------------------------------------------------------------------
* Function:        OPR_ScanPort
*
* Parameters:	portCfg - COM Port configuration structure.
* Returns:	1 if successful, 0 in the case of an error.
* Side effects:
* Description:
*		Open a specified ComPort device.
*---------------------------------------------------------------------------
*/
BOOLEAN OPR_ScanPort(struct COMPORT_FIELDS portCfg, char * port)
{
	char full_port_name[MAX_PORT_NAME_SIZE] = { 0 };
	char env[6 + MAX_PORT_NAME_SIZE] = { 0 };  // PORT=...
	char num[3];
	int i;
	enum SYNC_RESULT sr;
	BOOLEAN ret_val = FALSE;
	FILE *file_pointer;

	displayColorMsg(SUCCESS, "\nscan ports...\n");

	for (i = 0; i < 256; i++) {
		sprintf(num, "%d", i);
#ifdef WIN32
		strcpy(full_port_name, "\\\\.\\COM");
#else
		strcpy(full_port_name, "/dev/tty");
#endif
		strcat(full_port_name, num);

		if (portHandle != INVALID_HANDLE_VALUE)
			ComPortClose(portHandle);

		displayColorMsg(SUCCESS, "\rTry to open port  %s", full_port_name);

		portHandle = ComPortOpen((const char *)full_port_name, portCfg);

		if ((INT32)portHandle > 0) {
			sr = OPR_CheckSync(portCfg.BaudRate);
			if (sr == SR_OK) {
				displayColorMsg(SUCCESS, "\nFound port  %s\n", full_port_name);
				strncpy(port, full_port_name, sizeof(full_port_name));
				ComPortClose(portHandle);
#ifdef WIN32
				strcpy(full_port_name, "COM");
				strcat(full_port_name, num);
#else
				strcpy(full_port_name, "tty");
				strcat(full_port_name, num);
#endif
				

				ret_val = TRUE;
				break;
			}
		}
	}
	// for Linux only: can be both tty0 or ttyUSBS0.
#ifndef WIN32
	if ((INT32)portHandle < 0) {
		displayColorMsg(SUCCESS, "\n\nscan usb to serial ports...\n");
		for (i = 0; i < 256; i++) {
			strcpy(full_port_name, "/dev/ttyUSB");

			sprintf(num, "%d", i);
			strcat(full_port_name, num);

			displayColorMsg(SUCCESS, "\rTry to open port  %s", full_port_name);

			if (portHandle != INVALID_HANDLE_VALUE)
				ComPortClose(portHandle);

			portHandle = ComPortOpen((const char *)full_port_name, portCfg);

			if ((INT32)portHandle > 0) {
				displayColorMsg(SUCCESS, "\nFound port  %s\n", full_port_name);
				strncpy(port, full_port_name, sizeof(full_port_name));
				ComPortClose(portHandle);
				strcpy(full_port_name, "ttyUSB");
				strcat(full_port_name, num);
				ret_val = TRUE;
				break;
			}
		}
	}
#endif

	// save the port number to environment:
	strcpy(env, "PORT=");
	strcat(env, full_port_name);
	putenv(env);

	//save the port to "SerialPortNumber.txt" for writing
	file_pointer = fopen("SerialPortNumber.txt", "w+");

	if (file_pointer) {
		// Write to the file
		fprintf(file_pointer, full_port_name);

		// Close the file
		fclose(file_pointer);
	}

	return ret_val;
}


/*----------------------------------------------------------------------------
 * Function:	OPR_WriteMem
 *
 * Parameters:	input	- Input (file-name/console), containing data to write.
 *		addr	- Memory address to write to.
 *		size	- Data size to write.
 * Returns:	none.
 * Side effects:
 * Description:
 *	Write data to memory, starting from a given address.
 *	Memory may be Flash (SPI), DRAM (DDR) or SRAM.
 *	The data is retrieved either from an input file or from a console.
 *	Data size is not limited.
 *	Data is sent in 256 bytes chunks (file mode) or 4 ytes chunks
 *	(console mode).
 *---------------------------------------------------------------------------
 */
void OPR_WriteMem(char  *input, UINT32 addr, UINT32 size)
{
	FILE	      *inputFileID = NULL;
	UINT32	      curAddr = addr;
	UINT8	      dataBuf[256];
	UINT32	      writeSize;
	UINT32	      cmdIdx	= 1;
	char	      seps[]	= " ";
	char	      *token	= NULL;
	char	      *stopStr;
	UINT32	      blockSize = (console) ? sizeof(UINT32) : MAX_RW_DATA_SIZE;
	struct ComandNode wCmdBuf;

	if (!console) {
		inputFileID = fopen(input, "rb");

		if (inputFileID == NULL) {
			displayColorMsg(FAIL,
				"ERROR: could not open input file [%s]\n",
					input);
			return;
		}
	}

	/* Initialize response size */
	wCmdBuf.respSize = 1;

	DISPLAY_MSG(("Writing [%d] bytes in [%d] packets\n",
		     size, ((size + (blockSize - 1)) / blockSize)));

	/* Read first token from string */
	if (console)
		token = strtok(input, seps);

	/* Main write loop */
	while (TRUE) {
		if (console) {
			/* Check if last token in string is reached */
			if (token == NULL)
				break;

			/*
			 * Invert token to double-word and insert the value to
			 * data buffer
			 */
			(*(UINT32 *)dataBuf) =
				strtoul(token, &stopStr, BASE_HEXADECIMAL);

			/* Block size is fixed to a double-word */
			writeSize = sizeof(UINT32);

			/* Prepare the next iteration */
			token = strtok(NULL, seps);
		} else {
			/* Check if end of file is reached */
			if (feof(inputFileID))
				break;

			/* Read from file into data buffer */
			writeSize = (UINT32)fread(dataBuf, 1,
						  blockSize, inputFileID);

			/*
			 * In case we read the exact size of the file (e.g.,
			 * 256 bytes), feof will return 0 because, even though
			 * the file pointer is at the end of the file, we have
			 * not attempted to read beyond the end.
			 * Only after trying to read additional byte will feof
			 * return a nonzero value
			 */
			if (writeSize == 0)
				break;
		}

		CMD_CreateWrite(curAddr, writeSize, dataBuf,
				wCmdBuf.cmd, &wCmdBuf.cmdSize);
		if (OPR_SendCmds(&wCmdBuf, 1) != TRUE)
			break;

		CMD_DispWrite(respBuf, writeSize, cmdIdx,
			      ((size + (blockSize - 1)) / blockSize));

		curAddr += blockSize;
		cmdIdx++;
	}

	DISPLAY_MSG(("\n"));

	if (!console)
		fclose(inputFileID);
}

/*----------------------------------------------------------------------------
 * Function:	OPR_ReadMem
 *
 * Parameters:	output - Output file name, containing data that was read.
 *		addr   - Memory address to read from.
 *		size   - Data size to read.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Read data from memory, starting from a given address.
 *		Memory may be Flash (SPI), DRAM (DDR) or SRAM.
 *		The data is written into an output file, data size is limited
 *		as specified.
 *		Data is received in 256 bytes chunks.
 *---------------------------------------------------------------------------
 */
void OPR_ReadMem(char  *output, UINT32 addr, UINT32 size)
{
	FILE		*outputFileID = NULL;
	UINT32		curAddr;
	UINT32		bytesLeft;
	UINT32		readSize;
	UINT32		cmdIdx = 1;
	struct ComandNode	rCmdBuf;

	if (!console) {
		outputFileID = fopen(output, "w+b");

		if (outputFileID == NULL) {
			displayColorMsg(FAIL,
				"ERROR: could not open outout file [%s]\n",
					output);
			return;
		}
	}

	DISPLAY_MSG(("Reading [%d] bytes in [%d] packets\n", size,
		    ((size + (MAX_RW_DATA_SIZE - 1)) / MAX_RW_DATA_SIZE)));

	for (curAddr = addr;
	     curAddr < (addr + size);
	     curAddr += MAX_RW_DATA_SIZE) {
		bytesLeft = (UINT32)(addr + size - curAddr);
		readSize = MIN(bytesLeft, MAX_RW_DATA_SIZE);

		CMD_CreateRead(curAddr, ((UINT8)readSize - 1),
			       rCmdBuf.cmd, &rCmdBuf.cmdSize);
		rCmdBuf.respSize = readSize + 3;

		if (OPR_SendCmds(&rCmdBuf, 1) != TRUE)
			break;

		CMD_DispRead(respBuf, readSize, cmdIdx,
		     ((size + (MAX_RW_DATA_SIZE - 1)) / MAX_RW_DATA_SIZE));

		if (console)
			CMD_DispData((respBuf+1), readSize);
		else
			fwrite((respBuf+1), 1, readSize, outputFileID);

		cmdIdx++;
	}

	DISPLAY_MSG(("\n"));
	if (!console)
		fclose(outputFileID);
}

/*----------------------------------------------------------------------------
 * Function:	OPR_ReadStatusMsg
 *
 * Parameters:
 *		outputFileName - name of the file to write the data to
 *
 * Returns:	none
 * Side effects:
 * Description:
 *  Reads status message from the core and outputs it to a file (binary format)
 *---------------------------------------------------------------------------
 */
void OPR_ReadStatusMsg(char *outputFileName)
{

	FILE	*outputFileID = NULL;
	UINT32	bytesToRead = 0;
	UINT32	bytesRead;
	UINT32	i;

	outputFileID = fopen(outputFileName, "w+b");

	if (outputFileID == NULL) {
		displayColorMsg(FAIL,
				"Error openning output file: %s\n",
				outputFileName);
		return;
	}

	DISPLAY_MSG(("Reading status message\n"));

	while (1) {
		UINT32  dataSize;

		bytesToRead = 0;
		while (bytesToRead < STS_MSG_MIN_SIZE)
			bytesToRead = ComPortWaitForRead(portHandle);

		bytesRead = ComPortReadBin(portHandle,
					   respBuf,
					   STS_MSG_MIN_SIZE);

		DISPLAY_MSG(("bytesRead = %d\n", bytesRead));

		for (i = 0; i < bytesRead; i++)
			DISPLAY_MSG(("0x%x ", respBuf[i]));

		DISPLAY_MSG(("\n"));

		fwrite(respBuf, 1, bytesRead, outputFileID);

		if (*((UINT32 *)respBuf) == (UINT32)STS_MSG_APP_END)
			break;

		/* Read additional data if exists */
		dataSize = ((struct STATUS_MSG *)respBuf)->dataSize;
		if (dataSize != 0) {
			bytesToRead = 0;
			while (bytesToRead < dataSize)
				bytesToRead = ComPortWaitForRead(portHandle);

			bytesRead = ComPortReadBin(portHandle,
						   respBuf,
						   dataSize);

			DISPLAY_MSG(("bytesRead = %d\n", bytesRead));
			for (i = 0; i < bytesRead; i++)
				DISPLAY_MSG(("0x%x ", respBuf[i]));

			DISPLAY_MSG(("\n"));

			fwrite(respBuf, 1, bytesRead, outputFileID);
		}
	}

	fclose(outputFileID);
}

/*----------------------------------------------------------------------------
 * Function:	OPR_ExecuteExit
 *
 * Parameters:	addr - Start address to execute from.
 * Returns:	none.
 * Side effects:	ROM-Code is not in UART command mode anymore.
 * Description:
 *	Execute code starting from a given address.
 *	Memory address may be in Flash (SPI), DRAM (DDR) or SRAM.
 *	No further communication with thr ROM-Code is expected at this point.
 *---------------------------------------------------------------------------
 */
void OPR_ExecuteExit(UINT32 addr)
{
	UINT32 cmdNum;

	CMD_BuildExecExit(addr, cmdBuf, &cmdNum);
	if (OPR_SendCmds(cmdBuf, cmdNum) != TRUE)
		return;

	CMD_DispExecExit(respBuf);
}

/*----------------------------------------------------------------------------
 * Function:	OPR_ExecuteReturn
 *
 * Parameters:	addr - Start address to execute from.
 * Returns:	none.
 * Side effects:
 * Description:
 *	Execute code starting from a given address.
 *	Memory address may be in Flash (SPI), DRAM (DDR) or SRAM.
 *	The executed code should return with the execution result.
 *---------------------------------------------------------------------------
 */
void OPR_ExecuteReturn(UINT32 addr)
{
	UINT32 cmdNum;

	CMD_BuildExecRet(addr, cmdBuf, &cmdNum);
	if (OPR_SendCmds(cmdBuf, cmdNum) != TRUE)
		return;

	CMD_DispExecRet(respBuf);
}

/*----------------------------------------------------------------------------
 * Function:	OPR_CheckSync
 *
 * Parameters:
 *		bdRate - baud rate to check
 *
 * Returns:
 * Side effects:
 * Description:
 *	Checks whether the Host and the Core are synchoronized in the
 *	specified baud rate
 *---------------------------------------------------------------------------
 */
enum SYNC_RESULT OPR_CheckSync(UINT32 bdRate)
{

	UINT32			cmdNum;
	struct ComandNode	*curCmd = cmdBuf;
#ifdef WIN32
#if !defined(__WATCOMC__)
	UINT32		errors;
	COMSTAT		comstat;
#endif
#endif
	UINT32		bytesRead = 0;
	UINT32		i;

	portCfg.BaudRate = bdRate;
	if (!ConfigureUart(portHandle, portCfg))
		return SR_ERROR;

	CMD_BuildSync(cmdBuf, &cmdNum);

#ifdef WIN32
#if !defined(__WATCOMC__)
	ClearCommError(portHandle, (LPDWORD)&errors, &comstat);
#endif
#endif

	if (!ComPortWriteBin(portHandle, curCmd->cmd, curCmd->cmdSize))
		return SR_ERROR;

	/* Allow several SYNC trials */
	for (i = 0; i < MAX_SYNC_TRIALS; i++) {
		bytesRead = ComPortReadBin(portHandle, respBuf, 1);

		/* Quit if succeeded to read a response */
		if (bytesRead == 1)
			break;
		/* Otherwise give the ROM-Code time to answer */
#if WIN32
#if !defined(__WATCOMC__)
		Sleep(300);
#else
		sleep(1);
#endif
#else
		sleep(1);
#endif
	}

	if (bytesRead == 0)
		/*
		 * Unable to read a response from ROM-Code in a reasonable
		 * time
		 */
		return SR_TIMEOUT;

	if (respBuf[0] != (UINT8)(UFPP_D2H_SYNC_CMD))
		/* ROM-Code response is not as expected */
		return SR_WRONG_DATA;

	/* Good response */
	return SR_OK;

}

/*----------------------------------------------------------------------------
 * Function:	OPR_ScanBaudRate
 *
 * Parameters:	none
 * Returns:
 * Side effects:
 * Description:
 *		Scans the baud rate range by sending sync request to the core
 *		and prints the response
 *---------------------------------------------------------------------------
 */
BOOLEAN OPR_ScanBaudRate(void)
{
	UINT32          bdRate = 0;
	UINT32          brStep;
	enum SYNC_RESULT     sr;
	BOOLEAN         synched = FALSE;
	BOOLEAN         dataReceived = FALSE;

	/* Scan with HUGE STEPS */
	brStep = (BR_LOW_LIMIT*BR_BIG_STEP) / 100; /* BR_BIG_STEP is percents */
	for (bdRate = BR_LOW_LIMIT; bdRate < BR_HIGH_LIMIT; bdRate += brStep) {
		sr = OPR_CheckSync(bdRate);
		brStep = (bdRate * BR_BIG_STEP) / 100;
		if (sr == SR_OK) {
			printf("SR_OK: Baud rate - %d, respBuf - 0x%x\n",
			       bdRate,
			       respBuf[0]);
			synched = TRUE;
			brStep = (bdRate * BR_SMALL_STEP) / 100;
		} else if (sr == SR_WRONG_DATA) {
			printf(
			     "SR_WRONG_DATA: Baud rate - %d, respBuf - 0x%x\n",
			     bdRate,
			     respBuf[0]);
			dataReceived = TRUE;
			brStep = (bdRate * BR_MEDIUM_STEP) / 100;
		} else if (sr == SR_TIMEOUT) {
			printf("SR_TIMEOUT: Baud rate - %d, respBuf - 0x%x\n",
			       bdRate, respBuf[0]);

			if (synched || dataReceived)
				break;
		} else if (sr == SR_ERROR) {
			printf("SR_ERROR: Baud rate - %d, respBuf - 0x%x\n",
			       bdRate, respBuf[0]);
			if (synched || dataReceived)
				break;
		} else
		printf("Unknown error code: Baud rate - %d, respBuf - 0x%x\n",
			bdRate, respBuf[0]);
	}

	return TRUE;
}


#ifdef __WATCOMC__
/*----------------------------------------------------------------------------
 * Function:	OPR_SendCmds    (DOS version)
 *
 * Parameters:	cmdBuf - Pointer to a Command Buffer.
 *		cmdNum - Number of commands to send.
 * Returns:	1 if successful, 0 in the case of an error.
 * Side effects:
 * Description:
 *	Send a group of commands through COM port.
 *	A command is sent only after a valid response for the previous command
 *	was recieved.
 *---------------------------------------------------------------------------
 */
static BOOLEAN OPR_SendCmds(struct ComandNode *cmdBuf, UINT32        cmdNum
)
{
	struct ComandNode	*curCmd = cmdBuf;
	UINT32		nCmd;
	UINT32		nRead;
	time_t		start;
	FP64		elapsed_time;
	UINT32		bytesRead = 0;

	for (nCmd = 0; nCmd < cmdNum; nCmd++, curCmd++) {
		bytesRead = 0;
		if (ComPortWriteBin(portHandle, curCmd->cmd, curCmd->cmdSize)
								== TRUE) {
			time(&start);

			/* Yarkon Z1 BYPASS */
			if (chipNum == Yarkon) {
				do {
					nRead = ComPortWaitForRead(portHandle);
					elapsed_time = difftime(time(NULL),
								start);
					/*
					 * We don't know how many bytes to read
					 * since more bytes can arrive than
					 * planned, therefore we read whatever
					 * is available:
					 */
					bytesRead += ComPortReadBin(portHandle,
							    respBuf+bytesRead,
							    MAX_RESP_BUF_SIZE);
				} while ((bytesRead < curCmd->respSize) &&
					 (elapsed_time <= OPR_TIMEOUT));
			} else {
				do {
					nRead = ComPortWaitForRead(portHandle);
					elapsed_time = difftime(time(NULL),
								start);
				} while ((nRead == 0) &&
					 (elapsed_time <= OPR_TIMEOUT));
				ComPortReadBin(portHandle,
					       respBuf,
					       curCmd->respSize);
			}

			if (elapsed_time > OPR_TIMEOUT)
				displayColorMsg(FAIL,
	"ERROR: [%d] bytes received for read, [%d] bytes are expected\n",
						nRead, curCmd->respSize);
		} else {
			displayColorMsg(FAIL,
				"ERROR: Failed to send Command number %d\n",
					nCmd);
			return FALSE;
		}
	}

	return TRUE;
}


#else /* __WATCOMC__ */

/*----------------------------------------------------------------------------
 * Function:	OPR_SendCmds  (Windows version)
 *
 * Parameters:	cmdBuf - Pointer to a Command Buffer.
 *		cmdNum - Number of commands to send.
 * Returns:	1 if successful, 0 in the case of an error.
 * Side effects:
 * Description:
 *	Send a group of commands through COM port.
 *	A command is sent only after a valid response for the previous command
 *	was recieved.
 *---------------------------------------------------------------------------
 */
static BOOLEAN OPR_SendCmds(struct ComandNode *cmdBuf, UINT32  cmdNum)
{
	struct ComandNode	*curCmd = cmdBuf;
	UINT32		nCmd;
	UINT32		nRead;
	time_t		start;
	FP64		elapsed_time;

	for (nCmd = 0; nCmd < cmdNum; nCmd++, curCmd++) {
		if (ComPortWriteBin(portHandle, curCmd->cmd,
						curCmd->cmdSize) == TRUE) {
			time(&start);

			do {
				nRead = ComPortWaitForRead(portHandle);
				elapsed_time = difftime(time(NULL), start);
			} while ((nRead < curCmd->respSize) &&
				 (elapsed_time <= OPR_TIMEOUT));
			ComPortReadBin(portHandle, respBuf, curCmd->respSize);

			if (elapsed_time > OPR_TIMEOUT)
				displayColorMsg(FAIL,
	"ERROR: [%d] bytes received for read, [%d] bytes are expected\n",
						nRead, curCmd->respSize);
		} else {
			displayColorMsg(FAIL,
				"ERROR: Failed to send Command number %d\n",
					nCmd);
			return FALSE;
		}
	}

	return TRUE;
}


#endif /* __WATCOMC__ */
