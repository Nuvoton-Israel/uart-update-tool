// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018 Nuvoton Technologies, All Rights Reserved
 *<<<------------------------------------------------------------------------
 * File Contents:
 *   ComPort.h
 *            This file defines the ComPort interface header file.
 *  Project:
 *            UartUpdateTool
 *---------------------------------------------------------------------------
 */

#ifndef _COM_PORT_H_
#define _COM_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif
/*---------------------------------------------------------------------------
 *                                      ComPort INTERFACE
 *---------------------------------------------------------------------------
 */

#if WIN32
#ifdef __WATCOMC__

/*---------------------------------------------------------------------------
 * Constant definitions
 *---------------------------------------------------------------------------
 */
#define INVALID_HANDLE_VALUE    0

/*----------------------------------------------------------------------------
 * Type definitions
 *---------------------------------------------------------------------------*/
typedef unsigned long  HANDLE; /* for compatability with WIN API */

#endif /* __WATCOMC__ */
#else
/*---------------------------------------------------------------------------
 * Constant definitions
 *---------------------------------------------------------------------------
 */
#ifdef WIN32
#define INVALID_HANDLE_VALUE    0
#else
#define INVALID_HANDLE_VALUE    -1
#endif

/*---------------------------------------------------------------------------
 * Type definitions
 *---------------------------------------------------------------------------
 */
typedef unsigned long  HANDLE; /* for compatability with operating system */

#endif


#define MAX_COMPORT_DEVICES     10
#ifdef WIN32
#define COMP_PORT_PREFIX_1      "COM"
#define COMP_PORT_PREFIX_2      "COM"
#else
#define COMP_PORT_PREFIX_1      "ttyS"
#define COMP_PORT_PREFIX_2      "ttyUSB"
#endif

struct COMPORT_FIELDS {
	UINT32	BaudRate;	/* Baudrate at which running               */
	UINT8	ByteSize;	/* Number of bits/byte, 4-8                */
	UINT8	Parity;		/* 0-4=None,Odd,Even,Mark,Space            */
	UINT8	StopBits;	/* 0,1,2 = 1, 1.5, 2                       */
	UINT8	FlowControl;	/* 0-none, 1-SwFlowControl,2-HwFlowControl */
};

#ifndef COMPORT_IF_H

/*---------------------------------------------------------------------------
 * Function: HANDLE ComPortOpen()
 *
 * Purpose:  Open the specified ComPort device.
 *
 *  Params:   ComPortDeviceName - The name of the device to open
 *           ComPortFildes - a struct filled with Comport settings, see
 *                           definition above.
 *
 * Returns:  INVALID_HANDLE_VALUE (-1) - invalid handle.
 *           Other value - Handle to be used in other Comport APIs
 *
 * Comments: The returned handle can be used for other Win32 API communication
 *           function by casting it to HANDLE.
 *
 *---------------------------------------------------------------------------
 */
HANDLE ComPortOpen(const char *ComPortDeviceName,
		   struct COMPORT_FIELDS ComPortFildes);

/*---------------------------------------------------------------------------
 * Function: HANDLE ConfigureUart()
 *
 * Purpose:  Configures the Uart port properties.
 *
 * Params:   hDevice_Driver - the opened handle returned by ComPortOpen()
 *           ComPortFildes - a struct filled with Comport settings, see
 *                           definition above.
 *
 * Returns:  1 if successful
 *           0 in the case of an error.
 *
 *---------------------------------------------------------------------------
 */
BOOLEAN ConfigureUart(HANDLE hDevice_Driver,
		      struct COMPORT_FIELDS ComPortFields);

/*---------------------------------------------------------------------------
 * Function: BOOLEAN ComPortClose()
 *
 * Purpose:  Close the ComPort device specified by Handle
 *
 * Params:   nDeviceID - the opened handle returned by ComPortOpen()
 *
 * Returns:  1 if successful
 *           0 in the case of an error.
 *
 *---------------------------------------------------------------------------
 */
BOOLEAN ComPortClose(HANDLE nDeviceID);

/*---------------------------------------------------------------------------
 * Function: BOOLEAN ComPortWriteBin()
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
 * Comments: The caller must ensure that Size is not bigger than Buffer size.
 *
 *---------------------------------------------------------------------------
 */
BOOLEAN ComPortWriteBin(HANDLE nDeviceID, const UINT8 *Buffer, UINT32 BufSize);

/*---------------------------------------------------------------------------
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
 *---------------------------------------------------------------------------
 */
#ifdef WIN32
UINT32 ComPortReadBin(HANDLE nDeviceID, const UINT8 *Buffer, UINT32 BufSize);
#else
UINT32 ComPortReadBin(HANDLE nDeviceID, UINT8 *Buffer, UINT32 BufSize);
#endif

/*---------------------------------------------------------------------------
 * Function: UINT32 ComPortWaitForRead()
 *
 * Purpose:  Wait until a byte is received for read
 *
 * Params:   nDeviceID - the opened handle returned by ComPortOpen()
 *
 * Returns:  The number of bytes that are waiting in RX queue.
 *
 *---------------------------------------------------------------------------
 */
UINT32 ComPortWaitForRead(HANDLE nDeviceID);

#endif  /* COMPORT_IF_H */

#ifdef __cplusplus
}
#endif

#endif /* _COM_PORT_H_ */
