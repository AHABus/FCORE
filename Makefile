PROGRAM			:= fcore
SRC_DIR			:= ./src
SDK_DIR			:= $(ESP8266_HOME)/rtos-sdk
PROGRAM_SRC_DIR	:= $(SRC_DIR)/$(PROGRAM) $(SRC_DIR)/$(PROGRAM)/*
CONFIG			:= $(SRC_DIR)/$(PROGRAM).json

#config: $(CONFIG)
#	# TODO: parse fcore.json and export C config

include $(SDK_DIR)/common.mk
