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
PIO-PATH 		:= ${HOME}/.platformio
PIO-LIB-PATH 	:= .pio/libdeps/controller
TOOL-PATH 		:= ${PIO-PATH}/packages/toolchain-gccarmnoneeabi
PLATFORM-PATH 	:= ${PIO-PATH}/packages/framework-arduinoststm32
CMSIS-PATH 		:= ${PIO-PATH}/packages/framework-cmsis/CMSIS
OCD-PATH 		:= ${PIO-PATH}/packages/tool-openocd
TOOL_PREFIX 	:= arm-none-eabi
# ARDUINO =========================================================
ARDUINO-PATH 	:= ${HOME}/.arduino15
ARDUINO_HW_LIB 	:= ${PLATFORM-PATH}/libraries
ARDUINO_LIB 	:= ${ARDUINO-PATH}/libraries


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
CDEFS += ENABLE_HWSERIAL1
CDEFS += HAL_UART_MODULE_ENABLED
CDEFS += HAL_PCD_MODULE_ENABLED
CDEFS += USE_FULL_LL_DRIVER

TARGET_ELF = cnc_3018.elf


CXXDEFS  = $(CDEFS)

CC = $(TOOL-PATH)/bin/$(TOOL_PREFIX)-gcc
CXX = $(TOOL-PATH)/bin/$(TOOL_PREFIX)-g++
AS = $(TOOL-PATH)/bin/$(TOOL_PREFIX)-g++

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
			syscalls.c \
			bootloader.c \
			clock.c \
			core_callback.c \
			dwt.c \
			hw_config.c \
			otp.c \
			pinmap.c \
			PortNames.c \
			stm32_def.c \
			system_stm32yyxx.c \
			timer.c \
			uart.c \
			usb_device_core.c \
			usb_device_ctlreq.c \
			usb_device_ioreq.c \
			usbd_conf.c \
			usbd_desc.c \
			usbd_ep_conf.c \
			usbd_if.c \
			cdc_queue.c \
			usbd_cdc.c \
			usbd_cdc_if.c \
			PeripheralPins.c \
			spi_com.c


LIB_HAL := 	stm32f1xx_hal_cortex.c \
		  	stm32f1xx_hal_uart.c \
          	generic_clock.c \
          	stm32f1xx_hal_pcd.c \
          	stm32f1xx_hal_pcd_ex.c \
          	stm32f1xx_hal_rcc.c \
          	stm32f1xx_hal_rcc_ex.c \
          	stm32f1xx_hal_spi.c \
          	stm32f1xx_ll_spi.c \
            stm32f1xx_ll_tim.c \
            stm32f1xx_ll_usart.c \
            stm32f1xx_ll_usb.c \
			stm32f1xx_hal.c \
			stm32f1xx_hal_tim.c \
            stm32f1xx_hal_tim_ex.c

LIB_SRC += $(LIB_HAL)

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
             interrupt.cpp \
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
OBJ  = $(patsubst %.c,   $(BUILD_DIR)/arduino/%.o, ${LIB_SRC})
OBJ += $(patsubst %.cpp, $(BUILD_DIR)/arduino/%.o, ${LIB_CXXSRC})
OBJ += $(patsubst %.S,   $(BUILD_DIR)/arduino/%.o, ${LIB_ASRC})
OBJ += $(patsubst %.cpp, $(BUILD_DIR)/%.o, ${CXXSRC})
OBJ += $(patsubst %.c, 	 $(BUILD_DIR)/%.o, ${CSRC})

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
VPATH += $(ARDUINO_HW_LIB)/SrcWrapper/src/stm32

VPATH += $(PIO-LIB-PATH)/U8g2/src
VPATH += $(PIO-LIB-PATH)/U8g2/src/clib
VPATH += $(PIO-LIB-PATH)/SD/src
VPATH += $(PIO-LIB-PATH)/SD/src/utility
VPATH += .lib/lib_ini
VPATH += $(shell find $(SRC_DIRS) -type d)

ETL_INCLUDE := -I "$(PIO-LIB-PATH)/Embedded Template Library/include"

INC_FLAGS += $(addprefix -I ", $(addsuffix ",$(VPATH))) ${ETL_INCLUDE}

COMMON_FLAGS := -O2 -Wall -ffunction-sections -fdata-sections -flto -mcpu=cortex-m3 -mthumb ${INC_FLAGS} $(addprefix -D,$(CDEFS))
CFLAGS = -fno-fat-lto-objects -nostdlib --param max-inline-insns-single=500 ${COMMON_FLAGS}
CXXFLAGS = -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit -fno-rtti -std=gnu++11 ${COMMON_FLAGS}
ASFLAGS = -x assembler-with-cpp ${COMMON_FLAGS}

LDFLAGS = -Wl,-Map=$(BUILD_DIR)/cnc_3018.map
LDFLAGS += -T '$(PLATFORM-PATH)/variants/$(variant)/$(name)8T_$(name)B(T-U)/ldscript.ld'
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

LD_LIB = $(CMSIS-PATH)/DSP/Lib/GCC/libarm_cortexM3l_math.a

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

# Build step for A source
$(BUILD_DIR)/arduino/%.o: %.S | $(BUILD_DIR)
	@echo -e "$(Cyan)  CC    $< $(Color_Off)"
	@ $(CXX)  $(ASFLAGS) -c '$<' -o $@

# Build step for C source
$(BUILD_DIR)/arduino/%.o: %.c | $(BUILD_DIR)
	@echo -e "$(Cyan)  CC    $< $(Color_Off)"
	@ $(CC)  $(CFLAGS) -c '$<' -o $@

# Build step for C++ source
$(BUILD_DIR)/arduino/%.o: %.cpp | $(BUILD_DIR)
	@echo -e "$(Blue)  CPP    $< $(Color_Off)"
	@ $(CXX)  $(CXXFLAGS) -c '$<' -o $@

# Build step for C source
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo -e "$(Green)  CC    $< $(Color_Off)"
	@ $(CC)  $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@echo -e "$(Green)  CC    $< $(Color_Off)"
	@ $(CXX) $(CXXFLAGS) -c $< -o $@


$(BUILD_DIR)/$(TARGET_ELF): $(OBJ)
	@echo -e "$(Green)  Link   $@ $(Color_Off)"
	@ $(CXX) $(CXXFLAGS) -Os -o $@ $(OBJ) $(LDFLAGS) -L$(BUILD_DIR)  $(addprefix -L ,$(LD_LIB))


all: $(BUILD_DIR)/$(TARGET_ELF)
.PHONY : all

upload: $(BUILD_DIR)/$(TARGET_ELF)
	/usr/bin/openocd  -s /usr/share/openocd/scripts -c "set FLASH_SIZE 0x20000"  -f interface/stlink.cfg \
-c "transport select hla_swd" \
-f target/stm32f1x.cfg \
-d0 \
-c "program $(BUILD_DIR)/$(TARGET_ELF) verify reset; shutdown;"
.PHONY : upload

get_dep: $(LIB_DIR)/ini.c $(LIB_DIR)/ini.h
.PHONY: get_dep

# Help Target
help:
	$(CC) --version
	@echo -e "$(Red)Be aware that project depends on platform-io artifacts $(Color_Off)"
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "all    ... (the default if no target is provided)"
	@echo "clean  ... clean"
	@echo "upload ... upload elf to controller  "
	@echo "get_dep... download lib dependencies"
.PHONY : help
