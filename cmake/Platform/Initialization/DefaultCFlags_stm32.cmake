if (NOT DEFINED ARDUINO_MCU_FLAGS)
    set(ARDUINO_MCU_FLAGS "-mcpu=cortex-m3 -mthumb")
endif ()

#-DF_CPU=${MCU_SPEED}L \
#-DARDUINO_ARCH_STM32F1 \
set(ARDUINO_DEFAULT_CFLAGS "${ARDUINO_MCU_FLAGS} \
-DGENERIC_BOOTLOADER  \
-DCONFIG_MAPLE_MINI_NO_DISABLE_DEBUG")
#--verbose
set(ARDUINO_DEFAULT_CXXFLAGS "-pipe ${ARDUINO_DEFAULT_CFLAGS} \
-std=gnu++11 \
-fno-threadsafe-statics \
-fno-rtti \
-fno-exceptions \
-fno-use-cxa-atexit ")

set(ARDUINO_DEFAULT_CFLAGS "${ARDUINO_DEFAULT_CFLAGS} -flto -ffreestanding -nostdlib --specs=nosys.specs -std=gnu11")
if (ARDUINO_USE_NEWLIB) # smaller
    set(ARDUINO_DEFAULT_CFLAGS " ${ARDUINO_DEFAULT_CFLAGS} --specs=nano.specs")
endif (ARDUINO_USE_NEWLIB) # smaller

