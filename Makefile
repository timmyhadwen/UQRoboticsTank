# put your *.o targets here, make should handle the rest!
# THIS IS THE MAKEFILE FOR NUCLEOF401RE STM32 BOARD

NUC_PATH=$(NP2_ROOT)/src/boards/nucleof401
STDPERPH_PATH=$(NP2_ROOT)/src/Libraries/stm32f4xx_periph
COMMON_PATH=$(NP2_ROOT)/src/Libraries/common
CMSIS_PATH=$(NP2_ROOT)/src/Libraries/cmsis
FREERTOS_PATH=$(NP2_ROOT)/src/FreeRTOS
CLI_PATH=$(NP2_ROOT)/src/FreeRTOSplus/FreeRTOS-Plus-CLI
PERIPH_PATH=$(NP2_ROOT)/src/peripherals

PROJ_NAME=main

SRCS = $(PROJ_NAME).c *.c
SRCS += $(NUC_PATH)/src/*.c
SRCS += $(STDPERPH_PATH)/src/*.c $(COMMON_PATH)/*.c
#SRCS += $(FREERTOS_PATH)/*.c $(FREERTOS_PATH)/portable/GCC/ARM_CM4F/*.c $(FREERTOS_PATH)/portable/MemMang/heap_1.c
#SRCS += $(CLI_PATH)/*.c
# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)

###################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

CFLAGS  = -g -O1 -T$(NUC_PATH)/STM32F401XE.ld -Wmaybe-uninitialized
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

###################################################

vpath %.c src
vpath %.a lib

ROOT=$(shell pwd)

CFLAGS += -I. -I..
CFLAGS += -I$(CMSIS_PATH) -I$(NUC_PATH)/inc -I$(STDPERPH_PATH)/inc -I$(COMMON_PATH)
#CFLAGS += -I$(FREERTOS_PATH)/include -I$(FREERTOS_PATH)/portable/GCC/ARM_CM4F
CFLAGS += -I$(CLI_PATH)
CFLAGS += -I$(PERIPH_PATH)/nrf24l01plus/
#CFLAGS += -DENABLE_VCP #Enable USB VCP for debug_printf
#CFLAGS += -DENABLE_FRVCP #Enable USB VCP for debug_printf with FreeRTOS
CFLAGS += -DENABLE_DEBUG_UART	#Enable UART4 for debug printf
CFLAGS += -DSTM32F401xE
CFLAGS += -DHAVE_PLATFORMCONF_H

SRCS += $(NUC_PATH)/startup_stm32f401xe.s # add startup file to build

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: clean proj

lib:
	$(MAKE) -C lib

proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -Llib -lm -lc -lgcc -lnosys #-lstm32f4
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

prog:
	sudo dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000 -D $(PROJ_NAME).bin

pron:
	sudo st-flash write main.bin 0x8000000					#Program Nucleo

clean:
	rm -f *.o
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
