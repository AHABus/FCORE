# @file        Makefile
# @brief       FCORE GNU Make script.
# @author      Amy Parent
# @copyright   2017 Amy Parent

# Edit those two variables to reflect your installation.

ESPPORT 		:= /dev/tty.Repleo-CH341-00001014
SDK_DIR			:= $(ESP8266_HOME)/rtos-sdk

# DO NOT EDIT BELOW UNLESS YOU ARE MODIFYING FCORE.

PROGRAM			:= fcore
SRC_DIR			:= ./src
PROGRAM_SRC_DIR	:= $(SRC_DIR)/$(PROGRAM) $(SRC_DIR)/$(PROGRAM)/*
CFG_OUT			:= $(SRC_DIR)/$(PROGRAM)/__fcore_cfg
CFG_FILE		:= $(SRC_DIR)/$(PROGRAM).json
CFG_PARSER		:= $(SRC_DIR)/codegen/fcore_config.rb

COMPONENTS		:= FreeRTOS lwip core open_esplibs
SDK_LIBS		:= main net80211 phy pp wpa
LIBS 			:= hal gcc c
EXTRA_CFLAGS	:= -I$(SRC_DIR) -Dtimegm=mktime

include $(SDK_DIR)/common.mk

config: $(CFG_PARSER) $(CFG_FILE)
	@echo "CFG $(CFG_FILE)"
	@$(CFG_PARSER) $(CFG_FILE) $(CFG_OUT)

