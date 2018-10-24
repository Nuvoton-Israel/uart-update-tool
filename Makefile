# SPDX-License-Identifier: GPL-2.0
#/*------------------------------------------------------------------------*/
#/*  Copyright (c) 2015-2018 by Nuvoton Technology Corporation             */
#/*  All Rights Reserved                                                   */
#/*                                                                        */
#/*<<<---------------------------------------------------------------------*/
#/* File Contents:                                                         */
#/*   makefile                                                             */
#/*            This file contains UUT makefile                             */
#/* Project:                                                               */
#/*            UUT                                                         */
#/*------------------------------------------------------------------------*/

#----------------------------------------------------------------------------
# tools used in makefile
#----------------------------------------------------------------------------


#----------------------------------------------------------------------------
# Suffixes
#----------------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .h .c .cpp .o

#----------------------------------------------------------------------------
# Directories
#----------------------------------------------------------------------------

SRC_DIR		= ./src/source
OUTPUT_DIR      = release

#----------------------------------------------------------------------------
# Files
#----------------------------------------------------------------------------

Uartupdatetool_SRC    =    $(SRC_DIR)/main.c $(SRC_DIR)/cmd.c $(SRC_DIR)/lib_crc.c $(SRC_DIR)/opr.c $(SRC_DIR)/l_com_port.c

#----------------------------------------------------------------------------
# Object files of the project
#----------------------------------------------------------------------------


#----------------------------------------------------------------------------
# C preprocessor flags
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# C compilation flags
#----------------------------------------------------------------------------

CC		= gcc
MAKEDIR		= mkdir -p
INCLUDE 	= -I $(SRC_DIR) -I ./src/include/
TARGET  	= Uartupdatetool
CFLAGS  	= -g -Wall
# Google-specific compilation
#CFLAGS  	= -O3 -g -Wall -Werror -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack


Uartupdatetool:
	@echo Creating \"$(TARGET)\" in directory \"$(OUTPUT_DIR)\" ...
	@$(MAKEDIR)	$(OUTPUT_DIR)
	@echo $(CC) $(CFLAGS) $(INCLUDE) $(Uartupdatetool_SRC) -o $(OUTPUT_DIR)/Uartupdatetool
	@$(CC) $(CFLAGS) $(INCLUDE) $(Uartupdatetool_SRC) -o $(OUTPUT_DIR)/Uartupdatetool
	
all:
	@echo Creating \"$(TARGET)\" in directory \"$(OUTPUT_DIR)\" ...
	@$(MAKEDIR)	$(OUTPUT_DIR)
	@echo $(CC) $(CFLAGS) $(INCLUDE) $(Uartupdatetool_SRC) -o $(OUTPUT_DIR)/Uartupdatetool
	@$(CC) $(CFLAGS) $(INCLUDE) $(Uartupdatetool_SRC) -o $(OUTPUT_DIR)/Uartupdatetool


#----------------------------------------------------------------------------
# Clean
#----------------------------------------------------------------------------
RMDIR		= rm
RMFLAGS		= -rf

clean:
	@echo Removing Directory \"$(OUTPUT_DIR)\" ...
	@$(RMDIR) $(RMFLAGS) $(OUTPUT_DIR)
	


