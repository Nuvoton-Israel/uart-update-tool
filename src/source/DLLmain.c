// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2019 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   DLLmain.c
 *		This file defines the entry point for the DLL application.
 *  Project:
 *		UartUpdateToolDLL
 *---------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uut_types.h"
#include "program.h"
#include "ComPort.h"
#include "opr.h"
#include "lib_uut.h"

/*---------------------------------------------------------------------------
 * External variables
 *---------------------------------------------------------------------------
 */
extern UINT32					BaudRate;
extern char						PortName[MAX_PARAM_SIZE];
extern struct COMPORT_FIELDS	PortCfg;
extern BOOLEAN					Verbose;
extern BOOLEAN					Console;

/*---------------------------------------------------------------------------
 * Functions implementation
 *---------------------------------------------------------------------------
 */

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL, // handle to DLL module
	DWORD fdwReason,	// reason for calling function
	LPVOID lpReserved)	// reserved
{
	// Perform actions based on the reason for calling.

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			printf("Initialize once for each new process...\n");
			// Initialize once for each new process.
			// Return FALSE to fail DLL load.
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			printf("Do thread-specific initialization...\n");
			// Do thread-specific initialization.
			break;
		}
		case DLL_THREAD_DETACH:
		{
			printf("Do thread-specific cleanup...\n");
			// Do thread-specific cleanup.
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			printf("Perform any necessary cleanup...\n");
			// Perform any necessary cleanup.
			
			printf("Close port...\n");
			if (OPR_ClosePort() != TRUE)
				displayColorMsg(FAIL, "ERROR: Port close failed.\n");

			break;
		}
	}

	// Successful DLL_PROCESS_ATTACH.
	return TRUE;
}

int Init (UINT32 baudRate)
{
    enum EXIT_CODE ec = EC_OK;

	/* Setup defaults */
	Console     = FALSE;
	Verbose     = TRUE;

	/*
	* Initialize parameters
	*/
	BaudRate	= baudRate;

	/*
	* Configure COM Port parameters
	*/
	PortCfg.BaudRate	= MAX(BaudRate, BR_LOW_LIMIT);
	PortCfg.ByteSize	= 8;
	PortCfg.FlowControl	= 0;
	PortCfg.Parity		= 0;
	PortCfg.StopBits	= 0;

	/*
	* A scan is required, check each port, and then save the port num to the invironment
	*/
	if (OPR_ScanPort(PortCfg, PortName)) {
		displayColorMsg(SUCCESS,
			"\nScan ports pass, detected %s\n", PortName);
	}
	else {
		displayColorMsg(FAIL,
			"\nScan ports failed! Try to disconnect all terminal Apps, and make sure straps are OK\n");
        ec = EC_SCAN_ERR;
	}

    return ec;
}

