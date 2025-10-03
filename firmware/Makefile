PROJECT = toymatic_fw
BUILD_DIR = bin

CFILES = main.c utils.c setup.c vm.c plc.c crc16.c modbus.c modbus_func.c mb_diag.c blink.c
DEVICE=stm32g031f6

VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
