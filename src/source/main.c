// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   main.c
 *		This file defines the entry point for the console application.
 *  Project:
 *		UartUpdateTool
 *---------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "uut_types.h"
#include "main.h"
#include "ComPort.h"
#include "opr.h"

/*----------------------------------------------------------------------------
 * Constant definitions
 *---------------------------------------------------------------------------
 */

#define MAX_FILE_NAME_SIZE	512
#define MAX_PARAM_SIZE		32
#define MAX_MSG_SIZE		128

/* Default values */
#define DEFAULT_BAUD_RATE	115200
#define DEFAULT_PORT_NAME	"ttyS0"
#define DEFAULT_DEV_NUM		0

#define GET_BASE(str)	(((str[0] == 'x') || (str[1] == 'x')) ? \
					BASE_HEXADECIMAL : BASE_DECIMAL)

/*---------------------------------------------------------------------------
 * Global variables
 *---------------------------------------------------------------------------
 */

char		port_name[MAX_PARAM_SIZE];
struct COMPORT_FIELDS	portCfg;
UINT32		baudRate;

char		opr_name[MAX_PARAM_SIZE];
UINT32		devNum;

char		fileName[MAX_FILE_NAME_SIZE];
char		addrStr[MAX_PARAM_SIZE];
char		sizeStr[MAX_PARAM_SIZE];

BOOLEAN		verbose;
BOOLEAN		console;

/*---------------------------------------------------------------------------
 * Local variables
 *---------------------------------------------------------------------------
 */
static const char TOOL_NAME[]	  = { "LINUX UART Update Tool" };
static const char TOOL_VERSION[] = { "2.0.2" };


/*---------------------------------------------------------------------------
 * Functions prototypes
 *---------------------------------------------------------------------------
 */

static void	PARAM_ParseCmdLine(int argc, char *argv[]);
static void	PARAM_CheckPortNum(char *port_name);
static void	PARAM_CheckOprNum(const char *opr);
static UINT32	PARAM_GetFileSize(const char *fileName);
static UINT32	PARAM_GetStrSize(char *string);
static void	MAIN_PrintVersion(void);
static void	ToolUsage(void);
static void	ExitUartApp(UINT32 exitStatus);
static void	ToolUsage(void);
static int	str_cmp_no_case(const char *s1, const char *s2);

enum EXIT_CODE {
	EC_OK			= 0x00,
	EC_PORT_ERR		= 0x01,
	EC_BAUDRATE_ERR		= 0x02,
	EC_SYNC_ERR		= 0x03,
	EC_DEV_NUM_ERR		= 0x04,
	EC_OPR_MUM_ERR		= 0x05,
	EC_ALIGN_ERR		= 0x06,
	EC_FILE_ERR		= 0x07,
	EC_UNSUPPORTED_CMD_ERR	= 0x08
};

/*---------------------------------------------------------------------------
 * Function implementation
 *---------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------
 * Function:	main
 *
 * Parameters:		argc - Argument Count.
 *			argv - Argument Vector.
 * Returns:		1 for a successful operation, 0 otherwise.
 * Side effects:
 * Description:
 *		Console application main operation.
 *---------------------------------------------------------------------------
 */
int main(int   argc, char *argv[])
{
	/*-----------------------------------------------------------------
	 * CoreInterruptHandler(CR_Handler) enable CR interrupt handler
	 * Core interrupts will call CR_Handler function
	 *  TODO: remove the remarks "\\" if you want to use it
	 *
	 * CoreInterruptHandler(CR_Handler);
	 *
	 * PcInterruptHandler(PcHandler) enable PC interrupt handler
	 * PC interrupts will call PcHandler function
	 * TODO: remove the remarks "\\" if you want to use it
	 *
	 * PcInterruptHandler(PcHandler);
	 *
	 * TODO: Add here your main procedure
	 * Avoid using printf() or fprintf(), use LogMessage() instead
	 * LogMessage() parameters are the same as in printf()
	 *
	 * Use the following Macro for test cases:
	 * COND_FAILED(condition_exression, (*fail_format,..),
	 *       (*pass_format,..))
	 * if condition_exression==0 the fail_format error message will appear
	 * and both the error counter and the test counter will increment, if
	 * condition_exression==1 the pass_format message will appear and only
	 * the test counter will increment
	 * COND_FAILED() returns 1 if condition is false, and 0 otherwise.
	 * Example:
	 * COND_FAILED(Error==0, ("Failed: Error=%lu\n",Error), ("Passed\n"));
	 */

	char		*stopStr;
	char		auxBuf[MAX_FILE_NAME_SIZE];
	UINT32		size	= 0;
	UINT32		addr	= 0;
	enum SYNC_RESULT sr;

	if (argc <= 1)
		exit(EC_UNSUPPORTED_CMD_ERR);

	/* Setup defaults */
	strncpy(port_name, DEFAULT_PORT_NAME, sizeof(port_name));
	baudRate     = DEFAULT_BAUD_RATE;
	devNum	     = DEFAULT_DEV_NUM;
	opr_name[0]  = '\0';
	verbose  = TRUE;
	console  = FALSE;

	PARAM_ParseCmdLine(argc, argv);

	PARAM_CheckPortNum(port_name);

	/* Configure COM Port parameters */
	portCfg.BaudRate	= MAX(baudRate, BR_LOW_LIMIT);
	portCfg.ByteSize	= 8;
	portCfg.FlowControl	= 0;
	portCfg.Parity		= 0;
	portCfg.StopBits	= 0;

	/*
	 * Open a ComPort device. If user haven't specified such, use ComPort 1
	 */
	if (OPR_OpenPort(port_name, portCfg) != TRUE)
		exit(EC_PORT_ERR);

	if (baudRate == 0) { /* Scan baud rate range */
		OPR_ScanBaudRate();
		exit(EC_OK);
	}

	/* Verify Host and Device are synchronized */
	DISPLAY_MSG(("Performing a Host/Device synchronization check...\n"));
	sr = OPR_CheckSync(baudRate);
	if (sr != SR_OK) {
		displayColorMsg(FAIL,
		     "Host/Device synchronization failed, error = %lu.\n", sr);
		ExitUartApp(EC_SYNC_ERR);
	}


	PARAM_CheckOprNum(opr_name);

	/* Write buffer data to chosen address */
	if (strcmp(opr_name, OPR_WRITE_MEM) == 0) {

		addr = strtoul(addrStr, &stopStr, GET_BASE(addrStr));

		/*
		 * Copy the input string to an auxiliary buffer, since string
		 * is altered by PARAM_GetStrSize
		 */
		memcpy(auxBuf, fileName, sizeof(fileName));

		/* Retrieve input size */
		if (console)
			size = PARAM_GetStrSize(auxBuf);
		else
			size = PARAM_GetFileSize(fileName);

		/* Ensure non-zero size */
		if (size == 0)
			ExitUartApp(EC_FILE_ERR);

		OPR_WriteMem(fileName, addr, size);
	} else if (strcmp(opr_name, OPR_READ_MEM) == 0) {
		/* Read data to chosen address */

		addr = strtoul(addrStr, &stopStr, GET_BASE(addrStr));
		size = strtoul(sizeStr, &stopStr, GET_BASE(sizeStr));

		OPR_ReadMem(fileName, addr, size);
	} else if (strcmp(opr_name, OPR_EXECUTE_EXIT) == 0) {
		/* Execute From Address a non-return code */

		addr = strtoul(addrStr, &stopStr, GET_BASE(addrStr));

		OPR_ExecuteExit(addr);
		ExitUartApp(EC_OK);
	} else if (strcmp(opr_name, OPR_EXECUTE_CONT) == 0) {
		/* Execute From Address a returnable code */

		addr = strtoul(addrStr, &stopStr, GET_BASE(addrStr));

		OPR_ExecuteReturn(addr);
	} else
		ExitUartApp(EC_UNSUPPORTED_CMD_ERR);


#ifdef __WATCOMC__
	return 0;
#endif /*__WATCOMC__*/
	ExitUartApp(EC_OK);
	return 0;
}

/*---------------------------------------------------------------------------
 * Function:	PARAM_ParseCmdLine
 *
 * Parameters:		argc - Argument Count.
 *			argv - Argument Vector.
 * Returns:		None.
 * Side effects:
 * Description:
 *		Parse command line parameters.
 *---------------------------------------------------------------------------
 */
static void PARAM_ParseCmdLine(int   argc, char *argv[])
{
	UINT32 i = 0;

	while (--argc > 0) {

		/* Skip application name */
		i++;

		/*------------------------------------------------------------
		 * Help Message
		 *-----------------------------------------------------------
		 */
		if (str_cmp_no_case(*(argv+i), "-version") == 0) {
			MAIN_PrintVersion();
			exit(EC_OK);
		}

		/*-----------------------------------------------------------
		 * Help Message
		 *-----------------------------------------------------------
		 */
		if (str_cmp_no_case(*(argv+i), "-help") == 0) {
			ToolUsage();
			OPR_Usage();
			exit(EC_OK);
		}
		/*-----------------------------------------------------------
		 * Verbose/Silent Mode
		 *-----------------------------------------------------------
		 */
		else if (str_cmp_no_case(*(argv+i), "-silent") == 0) {
			verbose = FALSE;
			continue;
		}
		/*-----------------------------------------------------------
		 * File/Console Mode
		 *-----------------------------------------------------------
		 */
		else if (str_cmp_no_case(*(argv+i), "-console") == 0) {
			console = TRUE;
			continue;
		}
		/*-----------------------------------------------------------
		 * Baud Rate Value
		 *-----------------------------------------------------------
		 */
		else if (str_cmp_no_case(*(argv+i), "-baudrate") == 0) {
			if (sscanf(*(argv+1+i), "%du", &baudRate) == 0)
				exit(EC_BAUDRATE_ERR);
		}
		/*-----------------------------------------------------------
		 * Operation Number
		 *-----------------------------------------------------------
		 */
		else if (str_cmp_no_case(*(argv+i), "-opr") == 0) {
			if (sscanf(*(argv+1+i), "%s", (char *) opr_name) == 0)
				exit(EC_OPR_MUM_ERR);
		}
		/*-----------------------------------------------------------
		 * Port Number
		 *-----------------------------------------------------------
		 */
		else if (str_cmp_no_case(*(argv+i), "-port") == 0) {
			if (sscanf(*(argv+1+i), "%s", (char *) port_name) == 0)
				exit(EC_PORT_ERR);
		}
		/*-----------------------------------------------------------
		 * File Name
		 *-----------------------------------------------------------
		 */
		else if (str_cmp_no_case(*(argv+i), "-file") == 0) {
			if (sscanf(*(argv+1+i), "%s", fileName) == 0)
				exit(EC_FILE_ERR);
		}
		/*-----------------------------------------------------------
		 * Start memory address
		 *-----------------------------------------------------------
		 */
		else if (str_cmp_no_case(*(argv+i), "-addr") == 0) {
			if (sscanf(*(argv+1+i), "%s", addrStr) == 0)
				;
		}
		/*-----------------------------------------------------------
		 * Size of data to read
		 *-----------------------------------------------------------
		 */
		else if (str_cmp_no_case(*(argv+i), "-size") == 0) {
			if (sscanf(*(argv+1+i), "%s", sizeStr) == 0)
				;
		}
		/*-----------------------------------------------------------
		 * Unknown Parameter
		 *-----------------------------------------------------------
		 */
		else {
			displayColorMsg(FAIL,
			"ERROR: Parameter '%s' is not supported\n", *(argv+i));
			DISPLAY_MSG(("Use '-help' for menu\n"));
			exit(1);
		}

		i++;
		argc--;
	}
}

/*---------------------------------------------------------------------------
 * Function:        PARAM_CheckPortNum
 *
 * Parameters:      port_name - Pointer to port name.
 * Returns:         none.
 * Side effects:
 * Description:
 *                  Verify the validity of input port Number.
 *---------------------------------------------------------------------------
 */
static void PARAM_CheckPortNum(char *port_name)
{

	if ((strncmp(port_name,
		     COMP_PORT_PREFIX_1,
		     strlen(COMP_PORT_PREFIX_1)) != 0) &&
	    (strncmp(port_name,
		     COMP_PORT_PREFIX_2,
		     strlen(COMP_PORT_PREFIX_2)) != 0)) {
		displayColorMsg(FAIL,
				"ERROR: %s is an invalid serial port\n",
				port_name);
		DISPLAY_MSG(("Serial port %s will be used as default\n\n",
		DEFAULT_PORT_NAME));
		strcpy(port_name, DEFAULT_PORT_NAME);
	}
}

/*---------------------------------------------------------------------------
 * Function:	PARAM_CheckOprNum
 *
 * Parameters:	opr - Operation Number.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Verify the validity of operation Number.
 *---------------------------------------------------------------------------
 */
static void PARAM_CheckOprNum(const char *opr)
{
	if ((strcasecmp(opr, OPR_WRITE_MEM) != 0)    &&
	    (strcasecmp(opr, OPR_READ_MEM) != 0)     &&
	    (strcasecmp(opr, OPR_EXECUTE_EXIT) != 0) &&
	    (strcasecmp(opr, OPR_EXECUTE_CONT) != 0)) {
		displayColorMsg(FAIL,
"ERROR: Operation %s not supported, Supported operations are %s, %s, %s & %s\n",
		opr,
		OPR_WRITE_MEM,
		OPR_READ_MEM,
		OPR_EXECUTE_EXIT,
		OPR_EXECUTE_CONT);
		ExitUartApp(EC_OPR_MUM_ERR);
	}
}

/*---------------------------------------------------------------------------
 * Function:	PARAM_GetFileSize
 *
 * Parameters:	fileName - input file name.
 * Returns:	size of file (in bytes).
 * Side effects:
 * Description:
 *		Retrieve the size (in bytes) of a given file.
 *--------------------------------------------------------------------------
 */
static UINT32 PARAM_GetFileSize(const char *fileName)
{
	FILE    *hFile;
	UINT32	fileSize = 0;

	/* Open the source file */
	hFile = fopen(fileName, "rb");
	if (hFile == NULL) {
		displayColorMsg(FAIL,
				"ERROR: Could not open source file [%s]\n",
				fileName);
		return 0;
	}

	/* Get the file size */
	fseek(hFile, 0, SEEK_END);
	fileSize = ftell(hFile);

	fclose(hFile);
	return fileSize;
}

/*---------------------------------------------------------------------------
 * Function:	PARAM_GetStrSize
 *
 * Parameters:	string - input string.
 * Returns:	size of doubel-words (in bytes).
 * Side effects:
 * Description:
 *	Retrieve the size (in bytes) of double-word values in a given string.
 *	E.g., given the string "1234 AB5678 FF", return 12 (for three
 *	double-words).
 *---------------------------------------------------------------------------
 */
static UINT32 PARAM_GetStrSize(char *string)
{
	UINT32 strSize	= 0;
	char  seps[]	= " ";
	char *token	= NULL;

	/* Verify string is non-NULL */
	if ((strlen(string) == 0) || (string == NULL)) {
		displayColorMsg(FAIL,
				"ERROR: Zero length input string provided\n",
				string);
		return 0;
	}

	/* Read first token from string */
	token = strtok(string, seps);

	/* Loop while there are tokens in "string" */
	while (token != NULL) {
		strSize++;
		token = strtok(NULL, seps);
	}

	/* Refer to each token as a double-word */
	strSize *= sizeof(UINT32);
	return strSize;
}

/*--------------------------------------------------------------------------
 * Function:	ToolUsage
 *
 * Parameters:	none.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Prints the console application help menu.
 *--------------------------------------------------------------------------
 */
static void ToolUsage(void)
{
	printf("%s version %s\n\n", TOOL_NAME, TOOL_VERSION);
	printf("General switches:\n");
	printf("       -version         - Print version\n");
	printf("       -help            - Help menu\n");
	printf(
"       -silent          - Supress verbose mode (default is verbose ON)\n");
	printf(
"       -console         - Print data to console (default is print to file)\n");
	printf(
"       -port <name>     - Serial port name (default is %s)\n",
DEFAULT_PORT_NAME);
	printf(
"       -baudrate <num>  - COM Port baud-rate (default is %d)\n",
DEFAULT_BAUD_RATE);
	printf("\n");
	printf("Operation specific switches:\n");
	printf("       -opr   <name>    - Operation number (see list below)\n");
	printf("       -file  <name>    - Input/output file name\n");
	printf("       -addr  <num>     - Start memory address\n");
	printf("       -size  <num>     - Size of data to read\n");
	printf("\n");
}


/*--------------------------------------------------------------------------
 * Function:	MAIN_PrintVersion
 *
 * Parameters:	none
 * Returns:	none
 * Side effects:
 * Description:
 *		This routine prints the tool version
 *--------------------------------------------------------------------------
 */
static void MAIN_PrintVersion(void)
{
	printf("%s version %s\n\n", TOOL_NAME, TOOL_VERSION);
}


/*---------------------------------------------------------------------------
 * Function:	ExitUartApp
 *
 * Parameters:	none.
 * Returns:	none.
 * Side effects:
 * Description:
 *		Exit "nicely" the application.
 *---------------------------------------------------------------------------
 */
static void ExitUartApp(UINT32 exitStatus)
{
	if (OPR_ClosePort() != TRUE)
		displayColorMsg(FAIL, "ERROR: Port close failed.\n");

	exit(exitStatus);
}

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


	va_start(argptr, fmt);
	vprintf(fmt, argptr);
	va_end(argptr);

}


/*--------------------------------------------------------------------------
 * Function:	 str_cmp_no_case
 * Parameters:	 s1, s2: Strings to compare.
 * Return:	 function returns an integer less than, equal to, or
 *		 greater than zero if s1 (or the first n bytes thereof) is
 *		 found, respectively, to be less than, to match, or be
 *		 greater than s2.
 * Description:	 Compare two string without case sensitive.
 *--------------------------------------------------------------------------
 */
static int str_cmp_no_case(const char *s1, const char *s2)
{
	return strcasecmp(s1, s2);
}
