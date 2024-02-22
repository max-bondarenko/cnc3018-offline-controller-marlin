# -fno-rtti 
# -fno-exceptions
# -x c++

if (NOT DEFINED ARDUINO_UPLOAD_METHOD)
    set(ARDUINO_UPLOAD_METHOD "DFUUpload")
endif ()

if (NOT DEFINED ARDUINO_CFLAGS_SET)
    set(DEFINED ARDUINO_CFLAGS_SET 1)
    if (NOT DEFINED MCU_SPEED)
        set(MCU_SPEED 72000000L)
    endif (NOT DEFINED MCU_SPEED)

    if (NOT DEFINED ARDUINO_MCU_FLAGS)
        set(ARDUINO_MCU_FLAGS "  -mcpu=cortex-m3 -mthumb")
    endif ()

    set(ARDUINO_DEFAULT_CFLAGS "-ffunction-sections -fdata-sections  --param max-inline-insns-single=500 -w ${ARDUINO_MCU_FLAGS} -g")
    set(ARDUINO_DEFAULT_CFLAGS "${ARDUINO_DEFAULT_CFLAGS} -DGENERIC_BOOTLOADER -DF_CPU=${MCU_SPEED}L -DARDUINO_ARCH_STM32F1 -DCONFIG_MAPLE_MINI_NO_DISABLE_DEBUG")

    set(ARDUINO_DEFAULT_CXXFLAGS "${ARDUINO_DEFAULT_CFLAGS} -fno-rtti -fno-exceptions -std=gnu++11 -Werror=return-type")
    set(ARDUINO_DEFAULT_CFLAGS "${ARDUINO_DEFAULT_CFLAGS} -std=gnu11")
    if (ARDUINO_USE_NEWLIB) # smaller
        set(ARDUINO_DEFAULT_CFLAGS " ${ARDUINO_DEFAULT_CFLAGS} --specs=nano.specs")
    endif (ARDUINO_USE_NEWLIB) # smaller
endif ()
