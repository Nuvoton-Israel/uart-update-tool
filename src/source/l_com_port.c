// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   ComPort.cpp
 *            This file defines the initialization routines for the DLL.
 *  Project:
 *            UartUpdateTool
 *--------------------------------------------------------------------------
 */


#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <sys/ioctl.h>

#include "uut_types.h"
#include "main.h"
#include "ComPort.h"

/*---------------------------------------------------------------------------
 * Constant definitions
 *---------------------------------------------------------------------------
 */

#define	INBUFSIZE		2048
#define	OUTBUFSIZE		2048
#define	LOWER_THRESHHOLD	16
#define	UPPER_THRESHHOLD	512
#define	XOFF_CHAR		0x13
#define	XON_CHAR		0x11

#define UART_FIFO_SIZE          16

#define COMMAND_TIMEOUT		10000 /* 10 seconds */


/*---------------------------------------------------------------------------
 * Global variables
 *---------------------------------------------------------------------------
 */
HANDLE DeviceDescriptor[MAX_COMPORT_DEVICES];
static struct termios  savetty;

/*---------------------------------------------------------------------------
 * Functions prototypes
 *---------------------------------------------------------------------------
 */

/*--------------------------------------------------------------------------
 * Local Function implementation
 *--------------------------------------------------------------------------
 */

/*--------------------------------------------------------------------------
 * Function:	convert_baudrate_to_baudrate_mask
 *
 * Parameters:
 *		baudrate - Bauderate value.
 *
 * Returns:	Baudrate mask.
 * Side effects:
 * Description:
 *		This routine convert from baudrate mode to paudrate mask.
 *--------------------------------------------------------------------------
 */
static speed_t convert_baudrate_to_baudrate_mask(UINT32 baudrate)
{
	switch (baudrate) {
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	default:
		return B0;
	}
}

/*--------------------------------------------------------------------------
 * Function:	convert_parity_to_parity_mask
 *
 * Parameters:
 *		parity - parity mode
 *
 * Returns:
 * Side effects:
 * Description:
 *		This routine convert from parity mode to parit mask.
 *--------------------------------------------------------------------------
 */
static tcflag_t convert_parity_to_parity_mask(UINT32 parity)
{
	switch (parity) {
	case 0: /* None         */
		return 0x0000;

	case 1: /* Odd parity   */
		return (PARENB | PARODD);

	case 2: /* Even parity  */
		return PARENB;

	default: /* None        */
		return 0x0000;
	}
}

/*--------------------------------------------------------------------------
 * Function:	convert_byte_size_to_byte_size_mask
 *
 * Parameters:
 *		byte_size - 5-8 bits
 *
 * Returns:
 * Side effects:
 * Description:
 *		This routine convert from byte-size mode to byte-size mask.
 *--------------------------------------------------------------------------
 */
static tcflag_t convert_byte_size_to_byte_size_mask(UINT32 byte_size)
{
	switch (byte_size) {
	case 5:
		return CS5;

	case 6:
		return CS6;

	case 7:
		return CS7;

	case 8:
		return CS8;

	default:
		return CS8;
	}
}


/*-------------------------------------------------------------------------
 * Function:	set_read_blocking
 *
 * Parameters:
 *		hDevice_Driver	- The opened handle returned by ComPortOpen()
 *		block		- TRUE means read in blocking mode
 *				  FALSE means read in non-blocking mode.
 *
 * Returns:	none
 * Side effects:
 * Description:
 *		This routine set/unset read blocking mode.
 *--------------------------------------------------------------------------
 */
void set_read_blocking(HANDLE  hDevice_Driver, BOOLEAN block)
{
	struct termios tty;

	memset(&tty, 0, sizeof(tty));

	if (tcgetattr(hDevice_Driver, &tty) != 0) {
		displayColorMsg(FAIL,
"set_read_blocking Error: %d Fail to get attribute from Device number %lu.\n",
		errno, hDevice_Driver);
		return;
	}

	tty.c_cc[VMIN]  = block;
	tty.c_cc[VTIME] = 5;	/* 0.5 seconds read timeout */

	if (tcsetattr(hDevice_Driver, TCSANOW, &tty) != 0) {
		displayColorMsg(FAIL,
"set_read_blocking Error: %d Fail to set attribute to Device number %lu.\n",
		errno, hDevice_Driver);
	}
}


/*--------------------------------------------------------------------------
 * Global Function implementation
 *--------------------------------------------------------------------------
 */


/******************************************************************************
 * Function: HANDLE ConfigureUart()
 *
 * Purpose:  Configures the Uart port properties.
 *
 * Params:   hDevice_Driver - the opened handle returned by ComPortOpen()
 *	    ComPortFildes  - a struct filled with Comport settings, see
 *			     definition above.
 *
 * Returns:  1 if successful
 *	    0 in the case of an error.
 *
 *****************************************************************************
 */
BOOLEAN ConfigureUart(HANDLE  hDevice_Driver,
		      struct COMPORT_FIELDS ComPortFields)
{
	struct termios tty;
	speed_t        baudrate;

	memset(&tty, 0, sizeof(tty));

	if (tcgetattr(hDevice_Driver, &tty) != 0) {
		displayColorMsg(FAIL,
	"ConfigureUart Error: Fail to get attribute from Device number %lu.\n",
		hDevice_Driver);
		return FALSE;
	}

	baudrate = convert_baudrate_to_baudrate_mask(ComPortFields.BaudRate);
	cfsetospeed(&tty, baudrate);
	cfsetispeed(&tty, baudrate);

	tty.c_cflag |= baudrate;

	tty.c_cflag |=
		convert_byte_size_to_byte_size_mask(ComPortFields.ByteSize);
	/*
	 * disable IGNBRK for mismatched speed tests; otherwise receive break
	 * as \000 chars
	 */

	/* Set port to be in a "raw" mode. */
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR
				| IGNCR | ICRNL | IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

	tty.c_oflag = ~OPOST;
	tty.c_cc[VMIN]  = 0;	/* read doesn't block		*/
	tty.c_cc[VTIME] = 5;	/* 0.5 seconds read timeout	*/

	tty.c_iflag |= (ComPortFields.FlowControl == 0x01) ?
			(IXON | IXOFF) : 0x00; /* xon/xoff ctrl */

	tty.c_cflag |= (CLOCAL | CREAD);/* ignore modem controls, */
	/* enable reading */
	tty.c_cflag &= ~(PARENB | PARODD);	/* shut off parity */
	tty.c_cflag |= convert_parity_to_parity_mask(ComPortFields.Parity);
	/* Stop bits */
	tty.c_cflag |= (ComPortFields.StopBits == 0x02) ? CSTOPB : 0x00;
	/* HW flow control */
	tty.c_cflag |= (ComPortFields.FlowControl == 0x02) ? CRTSCTS : 0x00;

	/* Flush Port, then applies attributes */
	tcflush(hDevice_Driver, TCIFLUSH);

	if (tcsetattr(hDevice_Driver, TCSANOW, &tty) != 0) {
		displayColorMsg(FAIL,
	"ConfigureUart Error: %d setting port handle %d: %s.\n",
		errno, hDevice_Driver, strerror(errno));
		return FALSE;
	}

	return TRUE;
}

/******************************************************************************
 * Function: HANDLE ComPortOpen()
 *
 * Purpose:  Open the specified ComPort device and return its handle.
 *
 * Params:   ComPortDeviceName - The name of the device to open
 *           ComPortFields - a struct filled with Comport settings
 *
 * Returns:  INVALID_HANDLE_VALUE (-1) - invalid handle.
 *           Other value - Handle to be used in other Comport APIs
 *
 * Comments: The returned handle can be used for other Win32 API communication
 *           function.
 *
 *****************************************************************************
 */
HANDLE ComPortOpen(const char *ComPortDeviceName,
		   struct COMPORT_FIELDS ComPortFields)
{
	INT32  port_handler;

	port_handler = open(ComPortDeviceName, O_RDWR | O_NOCTTY);

	if (port_handler < 0) {
		displayColorMsg(FAIL,
				"ComPortOpen Error %d opening %s: %s\n",
				errno, ComPortDeviceName, strerror(errno));
		return INVALID_HANDLE_VALUE;
	}

	tcgetattr(port_handler, &savetty);

	if (!ConfigureUart(port_handler, ComPortFields)) {
		displayColorMsg(FAIL,
		"ComPortOpen() Error %d, Failed on ConfigureUart() %s, %s\n",
				errno, ComPortDeviceName,  strerror(errno));
		return INVALID_HANDLE_VALUE;
	}

	return (HANDLE) port_handler;
}

/******************************************************************************
 * Function: ComPortClose()
 *
 * Purpose:  Close the ComPort device specified by Handle
 *
 * Params:   Handle - the opened handle returned by ComPortOpen()
 *
 * Returns:  1 if successful
 *           0 in the case of an error.
 *
 *****************************************************************************
 */
BOOLEAN ComPortClose(HANDLE nDeviceID)
{

	tcsetattr(nDeviceID, TCSANOW, &savetty);

	if (close(nDeviceID) == INVALID_HANDLE_VALUE) {
		displayColorMsg(FAIL,
		"ComPortClose() Error: %d Device com%u was not opened, %s.\n",
				errno, (UINT32)nDeviceID,  strerror(errno));
		return FALSE;
	}

	return TRUE;
}

/******************************************************************************
 * Function: ComPortWriteBin()
 *
 * Purpose:  Send binary data through Comport
 *
 * Params:   nDeviceID - the opened handle returned by ComPortOpen()
 *           Buffer - contains the binary data to send
 *           BufSize - the size of data to send
 *
 * Returns:  1 if successful
 *           0 in the case of an error.
 *
 * Comments: The caller must ensure that BufSize is not bigger than Buffer size.
 *
 *****************************************************************************
 */
BOOLEAN ComPortWriteBin(HANDLE		nDeviceID,
			const UINT8	*Buffer,
			UINT32		BufSize)
{
	UINT32  bytes_written;

	bytes_written = write(nDeviceID, Buffer, BufSize);
	if (bytes_written != BufSize) {
		displayColorMsg(FAIL,
"ComPortWriteBin() Error: %d  Failed to write data to Uart Port %d, %s.\n",
		errno, (UINT32)nDeviceID,  strerror(errno));

		return FALSE;
	}

	return TRUE;
}

/******************************************************************************
 * Function: UINT32 ComPortReadBin()
 *
 * Purpose:  Read a binary data from Comport
 *
 * Params:   nDeviceID - the opened handle returned by ComPortOpen()
 *           Buffer - this buffer will contain the arrived data
 *           BufSize - maximum data size to read
 *
 * Returns:  The number of bytes read.
 *
 * Comments: The caller must ensure that Size is not bigger than Buffer size.
 *
 *****************************************************************************
 */
UINT32 ComPortReadBin(HANDLE	nDeviceID,
		      UINT8	*Buffer,
		      UINT32	BufSize)
{
	INT32   read_bytes;

	/* Reset read blocking mode */
	set_read_blocking(nDeviceID, FALSE);

	read_bytes = read(nDeviceID, Buffer, BufSize);

	if (read_bytes == -1) {
		displayColorMsg(FAIL,
	"ComPortReadBin() Error: %d Device number %lu was not opened, %s.\n",
				errno, (UINT32)nDeviceID,  strerror(errno));
	}

	return read_bytes;
}

/******************************************************************************
 * Function: UINT32 ComPortWaitForRead()
 *
 * Purpose:  Wait until a byte is received for read
 *
 * Params:   nDeviceID - the opened handle returned by ComPortOpen()
 *
 * Returns:  The number of bytes that are waiting in RX queue.
 *
 *****************************************************************************
 */
UINT32 ComPortWaitForRead(HANDLE nDeviceID)
{
	INT32           bytes;
	INT32           ret_val;
	struct pollfd   fds;

	/* Set read blocking mode */
	set_read_blocking(nDeviceID, TRUE);

	/* Wait up to 10 sec untile byte is received for read. */
	fds.fd      = nDeviceID;
	fds.events  = POLLIN;
	ret_val = poll(&fds, 1, COMMAND_TIMEOUT);
	if (ret_val < 0) {
		displayColorMsg(FAIL,
		"ComPortWaitForRead() Error: %d Device number %lu %s\n",
				errno, (UINT32)nDeviceID,  strerror(errno));
		return 0;
	}

	bytes = 0;

	/* If data is ready for read. */
	if (ret_val > 0) {
		/* Get number of bytes that are ready to be read. */
		if (ioctl(nDeviceID, FIONREAD, &bytes) < 0) {
			displayColorMsg(FAIL,
		"ComPortWaitForRead() Error: %d Device number %lu %s\n",
					errno, (UINT32)nDeviceID,
					strerror(errno));
			return 0;
		}
	}

	return bytes;
}


