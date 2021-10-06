// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2019 Nuvoton Technologies, All Rights Reserved
 *<<<-------------------------------------------------------------------------
 * File Contents:
 *   program.c
 *	This file creates common definitions for the UART console application.
 *  Project:
 *            UartUpdateTool
 *---------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <stdarg.h>
#endif

#include "uut_types.h"
#include "opr.h"
#include "cmd.h"
#include "ComPort.h"
#include "program.h"

/*---------------------------------------------------------------------------
 * Global variables
 *---------------------------------------------------------------------------
 */
UINT32					BaudRate;
char					PortName[MAX_PARAM_SIZE];
struct COMPORT_FIELDS	PortCfg;
BOOLEAN					Verbose;
BOOLEAN					Console;
UINT32					DevPortNum;
UINT32					crc_type;

/*----------------------------------------------------------------------------
 * Functions implementation
 *---------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------
 * Function:	displayColorMsg
 *
 * Parameters:
 *		success - SUCCESS for successful message, FAIL for erroneous
 *			  massage.
 *		fmt     - Massage to dispaly (format and arguments).
 *
 * Returns:	none
 * Side effects: Using DISPLAY_MSG macro.
 * Description:
 *		This routine displays a message using color attributes:
 *		In case of a successful message, use green foreground text on
 *		black background.
 *		In case of an erroneous message, use red foreground text on
 *		black background.
 *---------------------------------------------------------------------------
 */
void displayColorMsg(BOOLEAN success, char *fmt, ...)
{
	va_list argptr;

#ifdef WIN32
	HANDLE				h_console;
	CONSOLE_SCREEN_BUFFER_INFO	csbi;
	WORD				old_color = FOREGROUND_INTENSITY;
	WORD				new_color;

	h_console = GetStdHandle(STD_OUTPUT_HANDLE);

	if (h_console != INVALID_HANDLE_VALUE) {
		GetConsoleScreenBufferInfo(h_console, &csbi);

		/* Save the current text colors */
		old_color = csbi.wAttributes;

		if (success == TRUE)
			new_color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
		else
			new_color = FOREGROUND_RED | FOREGROUND_INTENSITY;

		SetConsoleTextAttribute(h_console, new_color);
	}
#endif

	va_start(argptr, fmt);
	vprintf(fmt, argptr);
	va_end(argptr);

#ifdef WIN32
	if (h_console != INVALID_HANDLE_VALUE)
		SetConsoleTextAttribute(h_console, old_color);
#endif
}

/*---------------------------------------------------------------------------
 * Function:    displayMsg
 *
 * Parameters:
 *      fmt     - Massage to dispaly (format and arguments).
 *
 * Returns: none
 * Side effects:
 * Description:
 *      This routine displays a message using color attributes:
 *      In case of a successful message, use green foreground text on
 *      black background.
 *      In case of an erroneous message, use red foreground text on
 *      black background.
 *---------------------------------------------------------------------------
 */
void displayMsg(char *fmt, ...)
{
	if (Verbose)
	{
        va_list argptr;
        va_start(argptr, fmt);
        vprintf(fmt, argptr);
        va_end(argptr);
	}
}


