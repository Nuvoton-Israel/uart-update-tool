# uart-update-tool
UUT - Uart Update Tool

## Prerequisites
-------------
# Linux:
To build the deliverables for this project, you need to have the "gcc" compiler
in your Linux machine.

# Windows:
 To build the Windows version install:
    * Microsoft Visual Studio Professional 2017


## Build process
--------------		  
	* On the Linux machine:
			* "make clean" - In order to clean the "Release" directory.
			* "make all"   - In order to build ".\Release\Uartupdatetool"

## Deliverables
------------
- ./Release/Uartupdatetool		  - Uart Update Tool for Linux (compiled on CentOS 3.10.0-1062.4.1.el7.x86_64) 
						    executable file.
						      

					      

- .\Windows\Release\UartUpdateTool.exe                 - Uart update tool for windows.
- .\Windows\ReleaseDLL\UartUpdateTool.dll              - Uart update tool for windows.

## Usage
----------	
>UartUpdateTool.exe -help

LINUX UART Update Tool version 2.1.3

General switches:
       -version         - Print version
       -help            - Help menu
       -silent          - Supress verbose mode (default is verbose ON)
       -console         - Print data to console (default is print to file)
       -port <name>     - Serial port name (default is ttyS0)
       -baudrate <num>  - COM Port baud-rate (default is 115200)
       -crc <num>       - CRC type [16, 32]. Default 16.

Operation specific switches:
       -opr   <name>    - Operation number (see list below)
       -file  <name>    - Input/output file name
       -addr  <num>     - Start memory address
       -size  <num>     - Size of data to read

Operations:
       wr               - Write To Memory/Flash
       rd               - Read From Memory/Flash
       go               - Execute a non-return code
       call             - Execute a returnable code
       scan             - Scan all ports. Output is saved to  SerialPortNumber.txt
       srhigh           - Set device port to hight baudrate.

       
       
## Release notes:
-----------------
UUT 2.1.3
=========
Supports the following Nuvoton projects:
- Arbel (BMC8XX).
- EC: Gamla, Alon, Arbel.

New features:
============
- Supports testing CAVP as a DLL.
- CRC32 used for longer packets.
- Add port scan option. Try
	Uartupdatetool.exe -opr scan -baudrate 115200
	output is saved to  .\SerialPortNumber.txt
- Increase timeout to allow full chip format.
- Add set CR_UART baud-rate to high rate (A0).
- Upgrade to Microsoft Visual Studio Professional 2017

