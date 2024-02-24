#_get_board_property(${BOARD_ID} menu.cpu.speed_72mhz.build.f_cpu FCPU)
##
macro(DBG)
    message(STATUS ${ARGN})
endmacro(DBG)


macro(ADD_IF_DEFINED key)
    dbg("Checking ${key}")
    if (DEFINED ${BOARD_ID}.${key})
        dbg("Yes ${key}: ${${BOARD_ID}.${key}}")
        set(COMPILE_FLAGS "${COMPILE_FLAGS} ${${BOARD_ID}.${key}}")
    endif (DEFINED ${BOARD_ID}.${key})
endmacro(ADD_IF_DEFINED key)

macro(SET_IF_DEFINED key out)
    dbg("Checking ${key}")
    if (DEFINED ${BOARD_ID}.${key})
        dbg("Yes  ${key}=>${${BOARD_ID}.${key}} ")
        set(${out} "${${BOARD_ID}.${key}} ")
    endif (DEFINED ${BOARD_ID}.${key})
endmacro(SET_IF_DEFINED key out)


macro(ADD_TO_COMPILE_FLAGS key prefix)
    dbg("Checking ${key}")
    if (DEFINED ${BOARD_ID}.${key})
        dbg("Yes ${key} -D${prefix}${${BOARD_ID}.${key}} ")
        set(COMPILE_FLAGS "${COMPILE_FLAGS} -D${prefix}${${BOARD_ID}.${key}} ")
    endif (DEFINED ${BOARD_ID}.${key})
endmacro(ADD_TO_COMPILE_FLAGS key prefix)

_try_get_board_property(${BOARD_ID} menu.cpu.${ARDUINO_CPU}.build.flags TRY_CPU_FLAGS)
if (TRY_CPU_FLAGS)
    set(CPU_FLAGS ${TRY_CPU_FLAGS})
else ()
    _try_get_board_property(${BOARD_ID} build.flags TRY_CPU_FLAGS)
    if (TRY_CPU_FLAGS)
        set(CPU_FLAGS ${TRY_CPU_FLAGS})
    else ()
        set(CPU_FLAGS "")
    endif ()
endif ()

# dont set the mcu speed, it is done elsewhere
# set(COMPILE_FLAGS "-DF_CPU=${FCPU} ${CPU_FLAGS} -DARDUINO=${NORMALIZED_SDK_VERSION} ")
set(COMPILE_FLAGS " ${CPU_FLAGS} -DARDUINO=${NORMALIZED_SDK_VERSION} ")
# This should be derived from the arduino config files
# hardcode them for the moment


#
set(STM32_SYSTEM_ROOT "-I\"${ARDUINO_PLATFORM_PATH}/system\"")

#set(COMPILE_FLAGS "${COMPILE_FLAGS} ${STM32_SYSTEM_ROOT}/libmaple/\" ") # Hack, there is a better way to get the system path
#set(COMPILE_FLAGS "${COMPILE_FLAGS} ${STM32_SYSTEM_ROOT}/libmaple/include\" ") # Hack
#set(COMPILE_FLAGS "${COMPILE_FLAGS} ${STM32_SYSTEM_ROOT}/libmaple/include/libmaple\" ") # Hack
#set(COMPILE_FLAGS "${COMPILE_FLAGS} ${STM32_SYSTEM_ROOT}/libmaple/usb/usb_lib/\" ") # Hack
#set(COMPILE_FLAGS "${COMPILE_FLAGS} ${STM32_SYSTEM_ROOT}/libmaple/usb/stm32f1/\" ") # Hack
#set(COMPILE_FLAGS "${COMPILE_FLAGS} ${STM32_SYSTEM_ROOT}/libmaple/stm32f1/include/\" ")
#set(COMPILE_FLAGS "${COMPILE_FLAGS} ${STM32_SYSTEM_ROOT}/../libraries/SPI/src/\" ")

add_to_compile_flags(build.vect "")
add_to_compile_flags(menu.cpu.${ARDUINO_UPLOAD_METHOD}Method.build.vect "")

# upload flags if any
add_if_defined(menu.cpu.${ARDUINO_UPLOAD_METHOD}Method.build.upload_flags)
add_if_defined(menu.cpu.${ARDUINO_CPU}.build.cpu_flags)

add_to_compile_flags(build.error_led_port "ERROR_LED_PORT=")
add_to_compile_flags(build.error_led_pin "ERROR_LED_PIN=")
add_to_compile_flags(build.board "ARDUINO_")
add_to_compile_flags(build.variant "BOARD_")


dbg("Final Compile flags for STM32: ${COMPILE_FLAGS}")
#
#set(COMPILE_FLAGS "${COMPILE_FLAGS} -std=gnu11 -MMD -DDEBUG_LEVEL=DEBUG_NONE ")


