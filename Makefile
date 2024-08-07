Color_Off=\e[0m

Red=\e[31m
Green=\e[32m
Yellow=\e[33m
Blue=\e[34m
Purple=\e[35m
Cyan=\e[36m
White=\e[37m

default_target: all
.PHONY : default_target

# Delete the default suffixes
.SUFFIXES:
.SUFFIXES: .c .cpp .o .s .h
# Disable VCS-based implicit rules.
% : %,v
% : RCS/%
% : RCS/%,v
% : SCCS/s.%
% : s.%

# PLATFORMIO ======================================================
PIO-PATH 			:= ${HOME}/.platformio
PIO-LIB-PATH 		:= .pio/libdeps/controller
TOOL-PATH 			:= ${PIO-PATH}/packages/toolchain-gccarmnoneeabi
PLATFORM-PATH 		:= ${PIO-PATH}/packages/framework-arduinoststm32
CMSIS-PATH 			:= ${PIO-PATH}/packages/framework-cmsis/CMSIS
OCD-PATH 			:= ${PIO-PATH}/packages/tool-openocd
TOOL_PREFIX 		:= arm-none-eabi
# ARDUINO =========================================================
ARDUINO-LIB-PATH 	:= ${HOME}/.arduino15/libraries
ARDUINO_HW_LIB 		:= ${PLATFORM-PATH}/libraries


board := STM32F1
# STM32F1xx
variant := ${board}xx
name := F103C
# F103CBT
series := ${name}BT
#GENERIC_F103CBTX
board_name := GENERIC_${series}X

CDEFS := ARDUINO_ARCH_STM32
CDEFS += PLATFORMIO=60111
CDEFS += ARDUINO=10808
CDEFS += ${board}
CDEFS += ${variant}
CDEFS += ${board}03xB
CDEFS += BOARD_NAME=\"${board_name}\"
#ARDUINO_GENERIC_F103CBTX
CDEFS += ARDUINO_${board_name}
CDEFS += VARIANT_H=\"variant_generic.h\"
# dont make it 256, it fail HWSerial
CDEFS += SERIAL_RX_BUFFER_SIZE=128
CDEFS += SERIAL_TX_BUFFER_SIZE=128

CDEFS += USBCON
CDEFS += USBD_USE_CDC
CDEFS += USE_FULL_LL_DRIVER
CDEFS += ENABLE_HWSERIAL1
CDEFS += HAL_UART_MODULE_ENABLED
CDEFS += HAL_PCD_MODULE_ENABLED

TARGET_ELF = cnc_3018.elf

CC = $(TOOL-PATH)/bin/$(TOOL_PREFIX)-gcc
CXX = $(TOOL-PATH)/bin/$(TOOL_PREFIX)-g++
AS = $(TOOL-PATH)/bin/$(TOOL_PREFIX)-g++
AR = $(TOOL-PATH)/bin/$(TOOL_PREFIX)-ar

SHELL = /bin/sh
RM = /bin/rm
MKDIR = /bin/mkdir

BUILD_DIR := build
SRC_DIRS := src

LIB_SRC = board.c \
		hooks.c \
		itoa.c \
		pins_arduino.c \
		wiring_analog.c \
		wiring_digital.c \
		wiring_shift.c \
		wiring_time.c \
		PeripheralPins.c \
		generic_clock.c \
		spi_com.c \
		syscalls.c

LIB_SRC_WRAPPER = bootloader.c \
				clock.c \
				dwt.c \
				hw_config.c \
				otp.c \
				pinmap.c \
				PortNames.c \
				stm32_def.c \
				system_stm32yyxx.c \
				timer.c \
				uart.c
LIB_USBD_CDC = usbd_cdc_if.c \
			   usbd_cdc.c \
               cdc_queue.c \
			   usb_device_core.c \
               usb_device_ctlreq.c \
               usb_device_ioreq.c \
               usbd_conf.c \
               usbd_desc.c \
               usbd_ep_conf.c \
               usbd_if.c

LIB_HAL := stm32f1xx_hal.c \
			stm32f1xx_hal_cortex.c \
			stm32yyxx_hal_dma.c \
			stm32yyxx_hal_irda.c \
			stm32yyxx_hal_pcd.c \
			stm32yyxx_hal_pcd_ex.c \
			stm32yyxx_hal_rcc.c \
			stm32yyxx_hal_rcc_ex.c \
			stm32yyxx_hal_tim.c \
			stm32yyxx_hal_tim_ex.c \
			stm32yyxx_hal_spi.c \
			stm32yyxx_hal_uart.c \
			stm32yyxx_hal_uart_ex.c \
			stm32yyxx_hal_usart.c \
			stm32yyxx_hal_usart_ex.c \
			stm32yyxx_ll_usb.c

LIB_SRC += $(LIB_HAL) $(LIB_SRC_WRAPPER) $(LIB_USBD_CDC)

LIB_CXXSRC = HardwareSerial.cpp \
             HardwareTimer.cpp \
             IPAddress.cpp \
             Print.cpp \
             RingBuffer.cpp \
             SD.cpp \
             File.cpp \
             SPI.cpp \
             Stream.cpp \
             Tone.cpp \
             USBSerial.cpp \
             VirtIOSerial.cpp \
             WMath.cpp \
             WSerial.cpp \
             WString.cpp \
             analog.cpp \
             main.cpp \
             new.cpp \
             variant_generic.cpp \
             wiring_pulse.cpp \
			 abi.cpp \
             Sd2Card.cpp \
             SdFile.cpp \
             SdVolume.cpp

LIB_ASRC := startup_stm32yyxx.S

LIB_SRC += $(notdir $(shell find $(PIO-LIB-PATH)/U8g2/src/clib -name '*.c'))

LIB_CXXSRC += U8g2lib.cpp U8x8lib.cpp

CXXSRC := $(notdir $(shell find $(SRC_DIRS) -name '*.cpp'))

CSRC := ini.c

# Define all object files.
OBJ  = $(patsubst %.c,   $(BUILD_DIR)/arduino/%.c.o, ${LIB_SRC})
OBJ += $(patsubst %.cpp, $(BUILD_DIR)/arduino/%.cpp.o, ${LIB_CXXSRC})
OBJ += $(patsubst %.S,   $(BUILD_DIR)/arduino/%.s.o, ${LIB_ASRC})
OBJ += $(patsubst %.cpp, $(BUILD_DIR)/%.cpp.o, ${CXXSRC})
OBJ += $(patsubst %.c, 	 $(BUILD_DIR)/%.c.o, ${CSRC})

DEPS := $(OBJS:.o=.d)

VPATH =  $(PLATFORM-PATH)/cores/arduino
VPATH += $(PLATFORM-PATH)/cores/arduino/stm32
VPATH += $(PLATFORM-PATH)/cores/arduino/stm32/usb
VPATH += $(PLATFORM-PATH)/cores/arduino/stm32/usb/cdc
VPATH += $(PLATFORM-PATH)/cores/arduino/stm32/LL

VPATH += $(PLATFORM-PATH)/system/Middlewares/ST/STM32_USB_Device_Library/Core
VPATH += $(PLATFORM-PATH)/system/Middlewares/ST/STM32_USB_Device_Library/Core/Src
VPATH += $(PLATFORM-PATH)/system/Middlewares/ST/STM32_USB_Device_Library/Core/Inc

VPATH += $(CMSIS-PATH)/Core/Include
VPATH += $(PLATFORM-PATH)/system/$(variant)
VPATH += $(PLATFORM-PATH)/variants/$(variant)/$(name)8T_$(name)B(T-U)

VPATH += $(PLATFORM-PATH)/system/Drivers/CMSIS/Device/ST/$(variant)/Include
VPATH += $(PLATFORM-PATH)/system/Drivers/CMSIS/Device/ST/$(variant)/Source/Templates/gcc

VPATH += $(PLATFORM-PATH)/system/Drivers/$(variant)_HAL_Driver/Src
VPATH += $(PLATFORM-PATH)/system/Drivers/$(variant)_HAL_Driver/Inc

VPATH += $(ARDUINO_HW_LIB)/SPI/src
VPATH += $(ARDUINO_HW_LIB)/SPI/src/utility
VPATH += $(ARDUINO_HW_LIB)/Wire/src
VPATH += $(ARDUINO_HW_LIB)/SrcWrapper/src
VPATH += $(ARDUINO_HW_LIB)/SrcWrapper/src/LL
VPATH += $(ARDUINO_HW_LIB)/SrcWrapper/src/HAL
VPATH += $(ARDUINO_HW_LIB)/SrcWrapper/src/stm32

VPATH += $(PIO-LIB-PATH)/U8g2/src
VPATH += $(PIO-LIB-PATH)/U8g2/src/clib
VPATH += $(PIO-LIB-PATH)/SD/src
VPATH += $(PIO-LIB-PATH)/SD/src/utility
VPATH += .lib/lib_ini
VPATH += $(shell find $(SRC_DIRS) -type d)

ETL_INCLUDE := -I "$(PIO-LIB-PATH)/Embedded Template Library/include"

INC_FLAGS += $(addprefix "-I, $(addsuffix ",$(VPATH))) ${ETL_INCLUDE}
CDEFS_FLAGS = $(addprefix -D,$(CDEFS))


COMMON_FLAGS := -Os -Wall -ffunction-sections -fdata-sections -flto -mcpu=cortex-m3 -mthumb
COMMON_FLAGS += -fno-fat-lto-objects
CFLAGS = -std=gnu11 --param max-inline-insns-single=500
CXXFLAGS = -std=gnu++11 -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit -fno-rtti
ASFLAGS = -x assembler-with-cpp

LDFLAGS := -Wl,-Map=cnc_3018.map
LDFLAGS += "-T$(PLATFORM-PATH)/variants/$(variant)/$(name)8T_$(name)B(T-U)/ldscript.ld"
LDFLAGS += -Wl,--cref \
		   -Wl,--as-needed \
           -Wl,--gc-sections,--relax \
           -Wl,--check-sections \
           -Wl,--entry=Reset_Handler \
           -Wl,--unresolved-symbols=report-all \
           -Wl,--warn-common \
           -Wl,--defsym=LD_MAX_SIZE=128K \
           -Wl,--defsym=LD_MAX_DATA_SIZE=20K \
           -Wl,--defsym=LD_FLASH_OFFSET=0x0 \
           --specs=nosys.specs

LDLIB := $(CMSIS-PATH)/DSP/Lib/GCC/libarm_cortexM3l_math.a

# ========================= lib =============================
LIB_DIR = .lib/lib_ini
LIB_ini_github_version-URL = https://raw.githubusercontent.com/benhoyt/inih/r58

$(LIB_DIR) :
	$(MKDIR) -p $@

$(LIB_DIR)/ini.h $(LIB_DIR)/ini.c: $(LIB_DIR)
	curl -o $@ $(LIB_ini_github_version-URL)/$(notdir $@)

# ========================= end =============================
$(BUILD_DIR):
	$(MKDIR) -p $@
	$(MKDIR) -p $@/arduino

clean:
	$(RM) -rf $(BUILD_DIR)
.PHONY : clean

$(BUILD_DIR)/arduino/%.s.o: %.S | $(BUILD_DIR)
	@echo -e "$(Cyan)  CXX   $< $(Color_Off)"
	@ $(AS) $(ASFLAGS) $(COMMON_FLAGS) $(CDEFS_FLAGS) $(INC_FLAGS) -c '$<' -o $@

$(BUILD_DIR)/arduino/%.c.o: %.c | $(BUILD_DIR)
	@echo -e "$(Cyan)  CC    $< $(Color_Off)"
	@ $(CC) $(CFLAGS) $(COMMON_FLAGS) $(CDEFS_FLAGS) $(INC_FLAGS) -c '$<' -o $@

$(BUILD_DIR)/arduino/%.txt: %.c | $(BUILD_DIR)
	@echo -e "$(Blue)  CPP    $< $(Color_Off)"
	@ $(CXX) -E $(CXXFLAGS) $(COMMON_FLAGS) $(CDEFS_FLAGS) $(INC_FLAGS) -c '$<' -o $@

# Build step for C++ source
$(BUILD_DIR)/arduino/%.cpp.o: %.cpp | $(BUILD_DIR)
	@echo -e "$(Blue)  CPP    $< $(Color_Off)"
	@ $(CXX)  $(CXXFLAGS) $(COMMON_FLAGS) $(CDEFS_FLAGS) $(INC_FLAGS) -c '$<' -o $@

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c | $(BUILD_DIR)
	@echo -e "$(Green)  CC    $< $(Color_Off)"
	@ $(CC)  $(CFLAGS) $(COMMON_FLAGS) $(CDEFS_FLAGS) $(INC_FLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp | $(BUILD_DIR)
	@echo -e "$(Green)  CC    $< $(Color_Off)"
	@ $(CXX) $(CXXFLAGS) $(COMMON_FLAGS) $(CDEFS_FLAGS) $(INC_FLAGS) -c $< -o $@

# This step is necessary. If link object itself it pull some extra symbols and fail a build.
# checked 100500 times. todo find what symbol and fix!!
$(BUILD_DIR)/%.a: $(OBJ)
	@echo -e "$(Green)  ARCH   $@ $(Color_Off)"
	$(RM) -f $@ ;  $(AR) qcv $@ $(OBJ)

$(BUILD_DIR)/$(TARGET_ELF): $(BUILD_DIR)/libcnc_3018.a | Makefile
	@echo -e "$(Green)  Link   $@ $(Color_Off)"
	$(CXX) -g $(LDFLAGS) $(CXXFLAGS) $(COMMON_FLAGS) -o $@ $(LDLIB) $<

upload: $(BUILD_DIR)/$(TARGET_ELF)
	/usr/bin/openocd  -s /usr/share/openocd/scripts \
-c "set FLASH_SIZE 0x20000" \
-f interface/stlink.cfg \
-c "transport select hla_swd" \
-f target/stm32f1x.cfg \
-c "program $(BUILD_DIR)/$(TARGET_ELF) verify reset; shutdown;"
.PHONY : upload

get_dep: $(LIB_DIR)/ini.c $(LIB_DIR)/ini.h
.PHONY: get_dep

all: $(BUILD_DIR)/$(TARGET_ELF)
.PHONY : all

help:
	$(CC) --version
	@echo -e "$(Red)Be aware that project depends on platform-io artifacts $(Color_Off)"
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "all    ... build $(TARGET_ELF)"
	@echo "clean  ... clean"
	@echo "upload ... upload elf to controller  "
	@echo "get_dep... download lib dependencies"
.PHONY : help
