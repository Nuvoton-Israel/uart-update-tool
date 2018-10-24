// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   main.h
 *	This file defines common definitions for the UART console application.
 *  Project:
 *	UartUpdateTool
 *--------------------------------------------------------------------------
 */

#ifndef _MAIN_H_
#define _MAIN_H_

/*---------------------------------------------------------------------------
 * Constant definitions
 *---------------------------------------------------------------------------
 */
/* Maximum Read/Write data size per packet */
#define MAX_RW_DATA_SIZE    256

/* Base for string conversion */
#define BASE_DECIMAL		10
#define BASE_HEXADECIMAL	16

/* Verbose control messages display */
#define DISPLAY_MSG(msg) {	\
	if (verbose)		\
		printf msg;	\
}

#define SUCCESS		TRUE
#define FAIL		FALSE

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------
 * Global variables
 *--------------------------------------------------------------------------
 */

extern BOOLEAN		verbose;
extern BOOLEAN		console;

/*--------------------------------------------------------------------------
 * Global functions
 *--------------------------------------------------------------------------
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
 * Side effects:	Using DISPLAY_MSG macro.
 * Description:
 *	This routine displays a message using color attributes:
 *		In case of a successful message, use green foreground text on
 *		black background.
 *		In case of an erroneous message, use red foreground text on
 *		black background.
 *--------------------------------------------------------------------------
 */
void displayColorMsg(BOOLEAN success, char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* _MAIN_H_ */
