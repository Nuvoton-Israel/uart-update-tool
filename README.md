# uart-update-tool
UUT - Uart Update Tool

## Prerequisites
-------------
To build the deliverables for this project, you need to have the "gcc" compiler
in your Linux machine.

## Build process
--------------		  
	* On the Linux machine:
		* Activate the following in the ".\linux"  directory
			* "make clean" - In order to clean the "Release" directory.
			* "make all"   - In order to build ".\Release\Uartupdatetool"
			
## Deliverables
------------
- ./Linux/Release/Uartupdatetool		  - Uart Update Tool Windows 
						    executable file.
						      
- ./Linux/src/source: main.c, cmd.c, opr.c, 
                      l_com_port.c & lib_crc.c    - Uart Update Tool source 
						    files for Linux OS.
						      
- ./Linux/src/include: main.h, cmd.h, opr.h, 
                       ComPort.h, uut_types.h, lib_crc.h   - Uart Update Tool header 
						             files for Linux OS.			

					      
- /Windows/Release/UartUpdateTool.exe                   - Uart update tool for windows.

## Usage
----------	
>UartUpdateTool.exe -help

WINDOWS UART Update Tool version 2.0.2

General switches:

       -version         - Print version       
       -help            - Help menu       
       -silent          - Supress verbose mode (default is verbose ON)       
       -console         - Print data to console (default is print to file)       
       -port <name>     - Serial port name (default is COM1)       
       -baudrate <num>  - COM Port baud-rate (default is 115200)       

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
       
       
       
