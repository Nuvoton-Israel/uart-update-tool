// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2019 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   program.h
 *	This file defines common definitions for the UART console application.
 *  Project:
 *	UartUpdateTool
 *--------------------------------------------------------------------------
 */

#ifndef _PROGRAM_H_
#define _PROGRAM_H_

/*---------------------------------------------------------------------------
 * Global types
 *---------------------------------------------------------------------------
 */
enum EXIT_CODE {
	EC_OK                   = 0x00,
	EC_PORT_ERR             = 0x01,
	EC_BAUDRATE_ERR         = 0x02,
	EC_SYNC_ERR             = 0x03,
	EC_DEV_NUM_ERR          = 0x04,
	EC_OPR_MUM_ERR          = 0x05,
	EC_ALIGN_ERR            = 0x06,
	EC_FILE_ERR             = 0x07,
	EC_UNSUPPORTED_CMD_ERR  = 0x08,
	EC_SCAN_ERR             = 0x09,
	EC_SIZE_ERR             = 0x10,
	EC_SEND_CMD_ERR         = 0x11,
	EC_CRC_ERR              = 0x12
};

/*---------------------------------------------------------------------------
 * Constant definitions
 *---------------------------------------------------------------------------
 */
#define MAX_PARAM_SIZE		32

 /* Default values */
#define DEFAULT_BAUD_RATE	115200
#define DEFAULT_DEV_NUM		0
#ifdef WIN32
#define DEFAULT_PORT_NAME	"COM1"
#else
#define DEFAULT_PORT_NAME	"ttyS0"
#endif

#define SUCCESS		TRUE
#define FAIL		FALSE

/* Verbose control messages display */
#define DISPLAY_MSG(msg)	displayMsg msg


#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
* Functions prototypes
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
void displayColorMsg(BOOLEAN success, char *fmt, ...);

/*---------------------------------------------------------------------------
 * Function:    displayMsg
 *
 * Parameters:
 *      success - SUCCESS for successful message, FAIL for erroneous
 *            massage.
 *      fmt     - Massage to dispaly (format and arguments).
 *
 * Returns: none
 * Side effects: Using DISPLAY_MSG macro.
 * Description:
 *      This routine displays a message using color attributes:
 *      In case of a successful message, use green foreground text on
 *      black background.
 *      In case of an erroneous message, use red foreground text on
 *      black background.
 *---------------------------------------------------------------------------
 */
void displayMsg(char *fmt, ...);


#ifdef __cplusplus
}
#endif

#endif /* _PROGRAM_H_ */

