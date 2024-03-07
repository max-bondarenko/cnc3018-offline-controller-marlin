#HARDWARE_MOTHERBOARD ?= 1020
#
#ifeq ($(OS),Windows_NT)
#  # Windows
#  ARDUINO_INSTALL_DIR ?= ${HOME}/Arduino
#  ARDUINO_USER_DIR ?= ${HOME}/Arduino
#else
#  UNAME_S := $(shell uname -s)
#  ifeq ($(UNAME_S),Linux)
#    # Linux
#    ARDUINO_INSTALL_DIR ?= /opt/arduino-1.8.19
#    ARDUINO_USER_DIR ?= ${HOME}/Arduino
#  endif
#  ifeq ($(UNAME_S),Darwin)
#    # Darwin (macOS)
#    ARDUINO_INSTALL_DIR ?= /Applications/Arduino.app/Contents/Java
#    ARDUINO_USER_DIR ?= ${HOME}/Documents/Arduino
#    AVR_TOOLS_PATH ?= /Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/
#  endif
#endif

# Arduino source install directory, and version number
# On most linuxes this will be /usr/share/arduino
#ARDUINO_INSTALL_DIR  ?=
#ARDUINO_VERSION      ?= 10819

TOOL_PATH = ${HOME}/.platformio/packages/toolchain-gccarmnoneeabi
PLATFORM_PATH = ${HOME}/.platformio/packages/framework-arduinoststm32
CMSIS_PATH = ${PLATFORM_PATH}/../framework-cmsis/CMSIS)


# This defines if U8GLIB is needed (may require RELOC_WORKAROUND)
U8GLIB             ?= 0


############
# Try to automatically determine whether RELOC_WORKAROUND is needed based
# on GCC versions:
#   https://www.avrfreaks.net/comment/1789106#comment-1789106

CC_MAJ:=$(shell $(CC) -dM -E - < /dev/null | grep __GNUC__ | cut -f3 -d\ )
CC_MIN:=$(shell $(CC) -dM -E - < /dev/null | grep __GNUC_MINOR__ | cut -f3 -d\ )
CC_PATCHLEVEL:=$(shell $(CC) -dM -E - < /dev/null | grep __GNUC_PATCHLEVEL__ | cut -f3 -d\ )
CC_VER:=$(shell echo $$(( $(CC_MAJ) * 10000 + $(CC_MIN) * 100 + $(CC_PATCHLEVEL) )))
ifeq ($(shell test $(CC_VER) -lt 40901 && echo 1),1)
  $(warning This GCC version $(CC_VER) is likely broken. Enabling relocation workaround.)
  RELOC_WORKAROUND = 1
endif


BOARD = STM32F1
VARIANT = STM32F1xx
NAME = F103C
SERIES = $(NAME)BT # F103CBT
BOARD_NAME = GENERIC_$(SERIES)X) #GENERIC_F103CBTX

DEFINES = ARDUINO_ARCH_STM32 \
PLATFORMIO=60111 \
ARDUINO=10808 \
$(BOARD) \
$(VARIANT) \
$(BOARD)03xB \
BOARD_NAME=\"$(BOARD_NAME)\" \
ARDUINO_$(BOARD_NAME) \
VARIANT_H=\"variant_generic.h\"


HARDWARE_VARIANT ?= arduino

CPU_FLAGS   = -mthumb -mcpu=$(MCPU)
SIZE_FLAGS  = -A

TARGET = $(notdir $(CURDIR))

VPATH = .
VPATH += $(HARDWARE_SRC)

VPATH += $(ARDUINO_INSTALL_DIR)/hardware/arduino/avr/libraries/SPI/src

FORMAT = ihex

# Name of this Makefile (used for "make depend").
MAKEFILE = Makefile

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
DEBUG = stabs

OPT = s



TOOL_PREFIX = arm-none-eabi
# Program settings
CC = $(TOOL_PATH)$(TOOL_PREFIX)-gcc
CXX = $(TOOL_PATH)$(TOOL_PREFIX)-g++
OBJCOPY = $(TOOL_PATH)$(TOOL_PREFIX)-objcopy
OBJDUMP = $(TOOL_PATH)$(TOOL_PREFIX)-objdump
AR  = $(TOOL_PATH)$(TOOL_PREFIX)-ar
SIZE = $(TOOL_PATH)$(TOOL_PREFIX)-size
NM = $(TOOL_PATH)$(TOOL_PREFIX)-nm

REMOVE = rm -f
MV = mv -f

# Place -D or -U options here
CDEFS    = $(CPU_FLAGS) ${addprefix -D , $(DEFINES)}
CXXDEFS  = $(CDEFS)



# Add all the source directories as include directories too
CINCS = ${addprefix -I ,${VPATH}}
CXXINCS = ${addprefix -I ,${VPATH}}

# Silence warnings for library code (won't work for .h files, unfortunately)
LIBWARN = -w -Wno-packed-bitfield-compat

# Compiler flag to set the C/CPP Standard level.
CSTANDARD = -std=gnu11
CXXSTANDARD = -std=gnu++11
CDEBUG = -g$(DEBUG)
CWARN   = -Wall -Wstrict-prototypes -Wno-packed-bitfield-compat -Wno-pragmas -Wunused-parameter
CXXWARN = -Wall                     -Wno-packed-bitfield-compat -Wno-pragmas -Wunused-parameter
CTUNING = -fsigned-char -funsigned-bitfields -fno-exceptions \
          -fshort-enums -ffunction-sections -fdata-sections



CXXEXTRA = -fno-use-cxa-atexit -fno-threadsafe-statics -fno-rtti
CFLAGS := $(CDEBUG) $(CDEFS) $(CINCS) -O$(OPT) $(CEXTRA)   $(CTUNING) $(CSTANDARD)
CXXFLAGS :=         $(CDEFS) $(CINCS) -O$(OPT) $(CXXEXTRA) $(CTUNING) $(CXXSTANDARD)
ASFLAGS :=          $(CDEFS)
#ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs

ifeq ($(HARDWARE_VARIANT), archim)
  LD_PREFIX = -Wl,--gc-sections,-Map,Marlin.ino.map,--cref,--check-sections,--entry=Reset_Handler,--unresolved-symbols=report-all,--warn-common,--warn-section-align
  LD_SUFFIX = $(LDLIBS)

  LDFLAGS   = -lm -T$(LDSCRIPT) -u _sbrk -u link -u _close -u _fstat -u _isatty
  LDFLAGS  += -u _lseek -u _read -u _write -u _exit -u kill -u _getpid
else
  LD_PREFIX = -Wl,--gc-sections,--relax
  LDFLAGS   = -lm
  CTUNING   += -flto
endif

# Programming support using avrdude. Settings and variables.
AVRDUDE_PORT = $(UPLOAD_PORT)
AVRDUDE_WRITE_FLASH = -Uflash:w:$(BUILD_DIR)/$(TARGET).hex:i
ifeq ($(shell uname -s), Linux)
  AVRDUDE_CONF = /etc/avrdude/avrdude.conf
else
  AVRDUDE_CONF = $(ARDUINO_INSTALL_DIR)/hardware/tools/avr/etc/avrdude.conf
endif
AVRDUDE_FLAGS = -D -C$(AVRDUDE_CONF) \
  -p$(PROG_MCU) -P$(AVRDUDE_PORT) -c$(AVRDUDE_PROGRAMMER) \
  -b$(UPLOAD_RATE)

# Since Marlin 2.0, the source files may be distributed into several
# different directories, so it is necessary to find them recursively

SRC    = $(shell find src -name '*.c'   -type f)
CXXSRC = $(shell find src -name '*.cpp' -type f)

# Define all object files.
OBJ  = ${patsubst %.c,   $(BUILD_DIR)/arduino/%.o, ${LIB_SRC}}
OBJ += ${patsubst %.cpp, $(BUILD_DIR)/arduino/%.o, ${LIB_CXXSRC}}
OBJ += ${patsubst %.S,   $(BUILD_DIR)/arduino/%.o, ${LIB_ASRC}}
OBJ += ${patsubst %.c,   $(BUILD_DIR)/%.o, ${SRC}}
OBJ += ${patsubst %.cpp, $(BUILD_DIR)/%.o, ${CXXSRC}}

# Define all listing files.
LST = $(LIB_ASRC:.S=.lst) $(LIB_CXXSRC:.cpp=.lst) $(LIB_SRC:.c=.lst)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS   = $(MCU_FLAGS) $(CPU_FLAGS) $(CFLAGS) -I.
ALL_CXXFLAGS = $(MCU_FLAGS) $(CPU_FLAGS) $(CXXFLAGS)
ALL_ASFLAGS  = $(MCU_FLAGS) $(CPU_FLAGS) $(ASFLAGS) -x assembler-with-cpp

# set V=1 (eg, "make V=1") to print the full commands etc.
ifneq ($V,1)
  Pecho=@echo
  P=@
else
  Pecho=@:
  P=
endif

# Create required build hierarchy if it does not exist

$(shell mkdir -p $(dir $(OBJ)))

# Default target.
all: sizeafter

build: elf hex bin

elf: $(BUILD_DIR)/$(TARGET).elf
bin: $(BUILD_DIR)/$(TARGET).bin
hex: $(BUILD_DIR)/$(TARGET).hex
eep: $(BUILD_DIR)/$(TARGET).eep
lss: $(BUILD_DIR)/$(TARGET).lss
sym: $(BUILD_DIR)/$(TARGET).sym

# Program the device.
# Do not try to reset an Arduino if it's not one
upload: $(BUILD_DIR)/$(TARGET).hex
ifeq (${AVRDUDE_PROGRAMMER}, arduino)
	stty hup < $(UPLOAD_PORT); true
endif
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)
ifeq (${AVRDUDE_PROGRAMMER}, arduino)
	stty -hup < $(UPLOAD_PORT); true
endif

# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(BUILD_DIR)/$(TARGET).hex
ELFSIZE = $(SIZE)  $(SIZE_FLAGS) $(BUILD_DIR)/$(TARGET).elf; \
          $(SIZE)  $(BUILD_DIR)/$(TARGET).elf
sizebefore:
	$P if [ -f $(BUILD_DIR)/$(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(HEXSIZE); echo; fi

sizeafter: build
	$P if [ -f $(BUILD_DIR)/$(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi


# Convert ELF to COFF for use in debugging / simulating in AVR Studio or VMLAB.
COFFCONVERT=$(OBJCOPY) --debugging \
  --change-section-address .data-0x800000 \
  --change-section-address .bss-0x800000 \
  --change-section-address .noinit-0x800000 \
  --change-section-address .eeprom-0x810000


coff: $(BUILD_DIR)/$(TARGET).elf
	$(COFFCONVERT) -O coff-avr $(BUILD_DIR)/$(TARGET).elf $(TARGET).cof


extcoff: $(TARGET).elf
	$(COFFCONVERT) -O coff-ext-avr $(BUILD_DIR)/$(TARGET).elf $(TARGET).cof


.SUFFIXES: .elf .hex .eep .lss .sym .bin
.PRECIOUS: .o

.elf.hex:
	$(Pecho) "  COPY  $@"
	$P $(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

.elf.bin:
	$(Pecho) "  COPY  $@"
	$P $(OBJCOPY) -O binary -R .eeprom $< $@

.elf.eep:
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	  --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
.elf.lss:
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
.elf.sym:
	$(NM) -n $< > $@

# Link: create ELF output file from library.

$(BUILD_DIR)/$(TARGET).elf: $(OBJ) Configuration.h
	$(Pecho) "  CXX   $@"
	$P $(CXX) $(LD_PREFIX) $(ALL_CXXFLAGS) -o $@ -L. $(OBJ) $(LDFLAGS) $(LD_SUFFIX)

# Object files that were found in "src" will be stored in $(BUILD_DIR)
# in directories that mirror the structure of "src"

$(BUILD_DIR)/%.o: %.c Configuration.h Configuration_adv.h $(MAKEFILE)
	$(Pecho) "  CC    $<"
	$P $(CC) -MMD -c $(ALL_CFLAGS) $(CWARN) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Configuration.h Configuration_adv.h $(MAKEFILE)
	$(Pecho) "  CXX   $<"
	$P $(CXX) -MMD -c $(ALL_CXXFLAGS) $(CXXWARN) $< -o $@

# Object files for Arduino libs will be created in $(BUILD_DIR)/arduino

$(BUILD_DIR)/arduino/%.o: %.c Configuration.h Configuration_adv.h $(MAKEFILE)
	$(Pecho) "  CC    $<"
	$P $(CC) -MMD -c $(ALL_CFLAGS) $(LIBWARN) $< -o $@

$(BUILD_DIR)/arduino/%.o: %.cpp Configuration.h Configuration_adv.h $(MAKEFILE)
	$(Pecho) "  CXX   $<"
	$P $(CXX) -MMD -c $(ALL_CXXFLAGS)  $(LIBWARN) $< -o $@

$(BUILD_DIR)/arduino/%.o: %.S $(MAKEFILE)
	$(Pecho) "  CXX   $<"
	$P $(CXX) -MMD -c $(ALL_ASFLAGS) $< -o $@

# Target: clean project.
clean:
	$(Pecho) "  RMDIR $(BUILD_DIR)/"
	$P rm -rf $(BUILD_DIR)


.PHONY: all build elf hex eep lss sym program coff extcoff clean depend sizebefore sizeafter

# Automatically include the dependency files created by gcc
-include ${patsubst %.o, %.d, ${OBJ}}
