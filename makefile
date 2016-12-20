THIS_DIR := $(realpath $(dir $(realpath $(lastword $(MAKEFILE_LIST)))))
ROOT := $(THIS_DIR)

PHOX_MESSENGER = $(ROOT)/libs/PhoxTools/messenger/cliMessenger.js

# OTA IP
ESP_ADDR ?= 192.168.43.20

ESP_MAKE=$(HOME)/src/makeEspArduino
ESP_LIBS=$(HOME)/src/ArduinoEsp/libraries

# makeEspArduino overrides
BUILD_DIR = $(ROOT)/tmp
FLASH_DEF=4M1M
ESP_ROOT=$(HOME)/src/ArduinoEsp
SKETCH=$(shell ls $(ROOT)/src/*.ino | head -n 1)
LIBS = $(ROOT)/libs \
	   $(ESP_LIBS)

DEVICE ?= 0

include $(ESP_MAKE)/makeEspArduino.mk

debug: 
	DEVICE=$(DEVICE) CFLAGS=-DDEBUG_MODE=1 make upload && \
	make listen

build: all
	mkdir -p $(ROOT)/build && \
	cp $(MAIN_EXE) $(ROOT)/build

send_message:
	cat $(MESSAGE) | netcat $(MESSAGE_RECIPIENT_IP) $(MESSAGE_SERVER_PORT) 

listen:
	picocom --imap lfcrlf -e c -b 115200 /dev/ttyUSB$(DEVICE) 

erase_flash:
	tools/esptool.py -p /dev/ttyUSB$(DEVICE) erase_flash

# see if this device is alive
ping:
	$(PHOX_MESSENGER) --ip $(ESP_ADDR)

# get device identity
who:
	$(PHOX_MESSENGER) --ip $(ESP_ADDR) --message who

.PHONY: send_message listen erase_flash
