// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton UART Update Tool
 *
 * Copyright (C) 2018 Nuvoton Technologies
 *<<<------------------------------------------------------------------------------------------------------
 * File Contents:  
 *   ComPort.cpp
 *            This file defines the initialization routines for the DLL.
 *  Project:
 *            UartUpdateTool
 */

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "uut_types.h"
#include "main.h"
#include "ComPort.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Constant definitions                                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

#define	INBUFSIZE				2048
#define	OUTBUFSIZE				2048
#define	LOWER_THRESHHOLD		16
#define	UPPER_THRESHHOLD		512
#define	XOFF_CHAR				0x13
#define	XON_CHAR				0x11

#define UART_FIFO_SIZE          16

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
HANDLE DeviceDescriptor[MAX_COMPORT_DEVICES];

/*---------------------------------------------------------------------------------------------------------*/
/* Functions prototypes                                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

void InitGlobals             (void);
void CloseAllComports        (void);
void MasterCallbackError     (const char * ErrorString, UINT32 ErrorNum);
void (*ModifiedCallbackError)(const char * ErrorString, UINT32 ErrorNum);

/*---------------------------------------------------------------------------------------------------------*/
/* Function implementation                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/

/******************************************************************************
* Function: void MasterCallbackError()
*           
* Purpose:  Master Callback error routine
*           
* Params:   ErrorString - Error string
*           ErrorNum - Error number
*           
* Returns:  none
*           
******************************************************************************/
void MasterCallbackError (
	const char *ErrorString, 
	UINT32      ErrorNum
)
{
	if (ModifiedCallbackError)
		ModifiedCallbackError(ErrorString, ErrorNum);
}

/******************************************************************************
* Function: void InitGlobals()
*           
* Purpose:  Initialize ComPort global variables
*           
* Params:   none
*           
* Returns:  none
*           
******************************************************************************/
void InitGlobals (void)
{
	UINT32 i;

	ModifiedCallbackError = NULL;
	
	for (i = 0; i < MAX_COMPORT_DEVICES; i++)
	{
		DeviceDescriptor[i] = INVALID_HANDLE_VALUE;
	}
}

/******************************************************************************
* Function: void CloseAllComports()
*           
* Purpose:  Close the ComPort devices
*           
* Params:   none
*           
* Returns:  none
*           
******************************************************************************/
void CloseAllComports (void)
{
	UINT32 i;
	
	for (i = 0; i < MAX_COMPORT_DEVICES; i++)
	{
		if (DeviceDescriptor[i] != INVALID_HANDLE_VALUE)
			ComPortClose(DeviceDescriptor[i]);
	}
}

/******************************************************************************
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
******************************************************************************/
BOOLEAN ConfigureUart(
    HANDLE         hDevice_Driver, 
    COMPORT_FIELDS ComPortFields
)
{
	DCB		     dcb;
	UINT32		 RetVal;	
	COMMTIMEOUTS CommTimeouts;
	UINT		 numOfBits;

    // Set hardware flow control options
	dcb.DCBlength         = sizeof(DCB);
	dcb.BaudRate          = ComPortFields.BaudRate;
	dcb.ByteSize          = ComPortFields.ByteSize;
	dcb.fAbortOnError     = FALSE;
	dcb.fBinary           = TRUE;
	dcb.fDsrSensitivity   = FALSE;
    dcb.fDtrControl       = DTR_CONTROL_DISABLE;	// DTR is controled manually: default is OFF
	dcb.fInX              = (ComPortFields.FlowControl == 1);
	dcb.fNull             = FALSE;
	dcb.fOutX             = (ComPortFields.FlowControl == 1);
	dcb.fErrorChar 		  = FALSE;
	dcb.fOutxCtsFlow      = (ComPortFields.FlowControl == 2);
	dcb.fOutxDsrFlow      = FALSE;					// no automatic monitoring of DSR
	dcb.fParity           = ComPortFields.Parity? TRUE: FALSE;
	dcb.fRtsControl       = (ComPortFields.FlowControl == 2) ? RTS_CONTROL_HANDSHAKE : RTS_CONTROL_DISABLE;
	dcb.fTXContinueOnXoff = FALSE; 					//transmission continues after the driver has transmitted the XoffChar character
	dcb.XonLim            = LOWER_THRESHHOLD;		//number of bytes allowed in the input buffer before the XON character is sent
	dcb.XoffLim           = UPPER_THRESHHOLD;		//XOFF character is sent when less then this number of bytes are empty in the input buffer
	dcb.Parity            = ComPortFields.Parity;
	dcb.StopBits          = ComPortFields.StopBits;
	dcb.XoffChar          = XOFF_CHAR;				//Xoff char on reception
	dcb.XonChar           = XON_CHAR;				//Xon char on reception
	dcb.EofChar           = 0;
	dcb.EvtChar           = 0;

#define BAUD_RATE_ERR_NUM   87
    // Enable the options
    RetVal = ::SetCommState(hDevice_Driver, &dcb);
    while (!RetVal && (GetLastError() == BAUD_RATE_ERR_NUM))
    {
    	dcb.BaudRate -= 10;
        RetVal = ::SetCommState(hDevice_Driver, &dcb);
    }

    if (ComPortFields.BaudRate != dcb.BaudRate)
    {
        printf("Note: failed to set baud rate %lu, reduced to %lu\n", ComPortFields.BaudRate, dcb.BaudRate);
    }

	if (! RetVal)
	{
		displayColorMsg(FAIL, "ComPort_If ConfigureUart() Error:\nFailed on SetCommState()\nGetLastError() returned %lu\n",
			::GetLastError());
        printf("bdRate = %lu\n", dcb.BaudRate);
		return FALSE;
	}

	RetVal = ::SetupComm(hDevice_Driver, INBUFSIZE, OUTBUFSIZE);
	if (! RetVal)
	{
		displayColorMsg(FAIL, "ComPort_If ConfigureUart() Error:\nFailed on SetupComm()\nGetLastError() returned %lu\n",
			::GetLastError());
		return FALSE;
	}
	
    // Calculate number of bits in single UART transaction
    numOfBits  = dcb.ByteSize;					// Number of bits per byte
	numOfBits += ((dcb.StopBits+2)/2);			// Number of stop bits
	numOfBits += (dcb.Parity != 0) ? 1 : 0;		// Parity bit

    // Set the write timeout values
    CommTimeouts.ReadIntervalTimeout         = 200 + ((1000 * numOfBits)/dcb.BaudRate); //threshold(ms) + TimeForAllBits(ms)
    CommTimeouts.ReadTotalTimeoutMultiplier  = CommTimeouts.ReadIntervalTimeout;
    CommTimeouts.ReadTotalTimeoutConstant    = 0;
    CommTimeouts.WriteTotalTimeoutMultiplier = CommTimeouts.ReadIntervalTimeout;
    CommTimeouts.WriteTotalTimeoutConstant   = 0;

    RetVal = ::SetCommTimeouts(hDevice_Driver, &CommTimeouts);
	if (! RetVal)
	{
		displayColorMsg(FAIL, "ComPortOpen() Error:\nFailed on SetCommTimeouts()\nGetLastError() returned %lu\n", 
			::GetLastError());
		return FALSE;
	}

	::PurgeComm(hDevice_Driver, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	::SetCommMask(hDevice_Driver, EV_RXCHAR);
	
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
******************************************************************************/
HANDLE ComPortOpen (
	const char    *ComPortDeviceName, 
	COMPORT_FIELDS ComPortFields
)
{
	UINT32	PrimaryAddress = 1;
	HANDLE 	hDevice_Driver;

	InitGlobals();

	hDevice_Driver = CreateFile(ComPortDeviceName, GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);

	if (hDevice_Driver == INVALID_HANDLE_VALUE)
	{
		//displayColorMsg(FAIL, "ComPort_If ComPortOpen() Failed on CreateFile() GetLastError() returned %lu\n", 
		//	::GetLastError());
		return INVALID_HANDLE_VALUE;
	}

    if (!ConfigureUart(hDevice_Driver, ComPortFields))
	{
		displayColorMsg(FAIL, "ComPort_If ComPortOpen() Error:\nFailed on ConfigureUart()\nGetLastError() returned %lu\n", 
			::GetLastError());
		return INVALID_HANDLE_VALUE;
	}
      
	DeviceDescriptor[PrimaryAddress] = hDevice_Driver;
	return hDevice_Driver;
}

/******************************************************************************
* Function: BOOLEAN ComPortClose()
*           
* Purpose:  Close the ComPort device specified by Handle
*           
* Params:   Handle - the opened handle returned by ComPortOpen()
*           
* Returns:  1 if successful
*           0 in the case of an error.
*           
******************************************************************************/
BOOLEAN ComPortClose (HANDLE nDeviceID)
{
	UINT32 i;
	
	if (nDeviceID == INVALID_HANDLE_VALUE)
	{
		for (i = 0; i < MAX_COMPORT_DEVICES; i++)
		{
			if (DeviceDescriptor[i] != INVALID_HANDLE_VALUE)
				ComPortClose(DeviceDescriptor[i]);
		}
		return TRUE;
	}

	for (i = 0; i < MAX_COMPORT_DEVICES; i++)
	{
		if (DeviceDescriptor[i] == nDeviceID)
		{
			CloseHandle(nDeviceID);
			DeviceDescriptor[i] = INVALID_HANDLE_VALUE;
			break;
		}
	}

	return TRUE;
}

/******************************************************************************
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
* Comments: The caller must ensure that BufSize is not bigger than Buffer size.
*           
******************************************************************************/
BOOLEAN ComPortWriteBin (
    HANDLE       nDeviceID, 
    const UINT8 *Buffer, 
    UINT32       BufSize
)
{
	UINT32   NumberOfBytesWritten;
    UINT32  initialBufSize = BufSize;
	UINT32  i;
		
	for (i = 0; i < MAX_COMPORT_DEVICES; i++)
	{
		if (DeviceDescriptor[i] == nDeviceID)
		{
			break;
		}
	}

	if (i == MAX_COMPORT_DEVICES)
	{
		displayColorMsg(FAIL, "ComPortWriteBin() Error: Device number %lu was not opened.\n", (UINT32)nDeviceID);
		return FALSE;
	}


	do
	{
	    for (i = 0; i < initialBufSize; i += UART_FIFO_SIZE)
        {   
        	if (NULL == WriteFile(nDeviceID, (PVOID)Buffer, MIN(BufSize, UART_FIFO_SIZE), (LPDWORD)&NumberOfBytesWritten, NULL))
        	{
		        displayColorMsg(FAIL, "ComPortWriteBin() Error: Failed to write data to Uart Port.\n");
        		return FALSE;
        	}

        	BufSize -= NumberOfBytesWritten;
        	Buffer  += NumberOfBytesWritten;
        }
	} while (BufSize != 0);

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
******************************************************************************/
UINT32	ComPortReadBin (
	HANDLE       nDeviceID, 
    const UINT8 *Buffer, 
    UINT32       BufSize
)
{
	UINT32 NumberOfBytesRead;
	UINT32   i;

	for (i = 0; i < MAX_COMPORT_DEVICES; i++)
	{
		if (DeviceDescriptor[i] == nDeviceID)
		{
			break;
		}
	}

	if (i == MAX_COMPORT_DEVICES)
	{
		
		displayColorMsg(FAIL, "ComPortReadBin() Error: Device number %lu was not opened.\n", (UINT32)nDeviceID);
		return FALSE;
	}
	
	if (NULL == ReadFile(nDeviceID, (PVOID)Buffer, BufSize, (LPDWORD)&NumberOfBytesRead, NULL))
	{
		return FALSE;
	}

	return (UINT32)NumberOfBytesRead;
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
******************************************************************************/
UINT32 ComPortWaitForRead (HANDLE nDeviceID)
{
	UINT32   EvtMask = EV_RXCHAR;
	UINT32   Errors;
	COMSTAT Comstat;
	UINT32  i;

	for (i = 0; i < MAX_COMPORT_DEVICES; i++)
	{
		if (DeviceDescriptor[i] == nDeviceID)
		{
			break;
		}
	}

	if (i == MAX_COMPORT_DEVICES)
	{
		
		displayColorMsg(FAIL, "ComPortWaitForRead() Error: Device number %lu was not opened.\n", (UINT32)nDeviceID);
		return FALSE;
	}

	while (1)
	{
		ClearCommError(nDeviceID, (LPDWORD)&Errors, &Comstat);

		if (Errors)
		{
			displayColorMsg(FAIL, "ComPortWaitForRead() Error: ClearCommError Error %lu was detected.\n", Errors);
			return 0;
		}

		if (Comstat.cbInQue)
		{
			return (UINT32)Comstat.cbInQue;
		}

		WaitCommEvent(nDeviceID, (LPDWORD)&EvtMask, NULL);
	}
}

#endif //_WIN32
