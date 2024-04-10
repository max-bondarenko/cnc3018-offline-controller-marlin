cmake_minimum_required(VERSION 3.24)
set(TOOL-PATH $ENV{HOME}/.platformio/packages/toolchain-gccarmnoneeabi)
set(PLATFORM-PATH $ENV{HOME}/.platformio/packages/framework-arduinoststm32)
set(CMSIS-PATH ${PLATFORM-PATH}/../framework-cmsis/CMSIS)

if (UNIX)
    file(GLOB SDK_PATH_HINTS /usr/share/arduino*
            /opt/local/arduino*
            /opt/arduino*
            /usr/local/share/arduino*)
elseif (WIN32)
    set(SDK_PATH_HINTS "C:\\Program Files\\Arduino"
            "C:\\Program Files (x86)\\Arduino"
            )
endif ()

find_path(ARDUINO_SDK_PATH
        NAMES lib/version.txt
        PATH_SUFFIXES share/arduino
        Arduino.app/Contents/Resources/Java/
        ${ARDUINO_PATHS}
        HINTS ${SDK_PATH_HINTS}
        DOC "Arduino SDK path.")
set(CMAKE_FIND_DEBUG_MODE TRUE)

find_file(ETL_PATH
        NAMES "Embedded Template Library"
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/libdeps/debug
        HINTS "${CMAKE_CURRENT_SOURCE_DIR}/.pio/libdeps/debug/")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

project(cnc_3018 C CXX ASM)
if (CMAKE_BUILD_TYPE)
    STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
else ()
    set(CMAKE_BUILD_TYPE debug)
endif ()

set(OPT_FLAG "-Os")
if (CMAKE_BUILD_TYPE STREQUAL "debug")
    set(DEBUG_OPT_FLAG "-ggdb")
else ()
    set(DEBUG_OPT_FLAG "-Os")
endif ()

function(ard_lib lib_name lib_path_name lib_path)
    if (lib_path)
        set(${lib_name}_path ${lib_path}/libraries/${lib_name}/src)
    else ()
        set(${lib_name}_path ${ARDUINO_SDK_PATH}/libraries/${lib_name}/src)
    endif ()
    file(GLOB_RECURSE _src ${${lib_name}_path}/*.c ${${lib_name}_path}/*.cpp)
    add_library(${lib_name} ${_src})
    target_include_directories(${lib_name} PUBLIC ${${lib_name}_path})
    target_compile_options(${lib_name} PRIVATE ${OPT_FLAG})

    set(${lib_path_name} ${${lib_name}_path} PARENT_SCOPE)
endfunction()

set(board STM32F1)
set(variant ${board}xx) # STM32F1xx
set(name F103C)
set(series ${name}BT) # F103CBT
set(board_name GENERIC_${series}X) #GENERIC_F103CBTX

add_compile_definitions(ARDUINO_ARCH_STM32)
add_compile_definitions(PLATFORMIO=60111)
add_compile_definitions(ARDUINO=10808)
add_compile_definitions(${board})
add_compile_definitions(${variant})
add_compile_definitions(${board}03xB)
add_compile_definitions(BOARD_NAME=\"${board_name}\")
add_compile_definitions(ARDUINO_${board_name}) #ARDUINO_GENERIC_F103CBTX
add_compile_definitions(VARIANT_H=\"variant_generic.h\")

add_compile_definitions(SERIAL_RX_BUFFER_SIZE=256)

set(core_path ${PLATFORM-PATH}/cores/arduino)
set(stm_core_path ${core_path}/stm32)
set(usb_path ${stm_core_path}/usb)
set(usb_device_path ${PLATFORM-PATH}/system/Middlewares/ST/STM32_USB_Device_Library/Core)

file(GLOB ARDUINO_SRC
        ${core_path}/*.c
        ${core_path}/*.cpp
        ${usb_path}/*.c
        ${usb_path}/cdc/*.c
        ${stm_core_path}/*.c
        ${stm_core_path}/*.S #startup code
        ${stm_core_path}/*.cpp

        "${PLATFORM-PATH}/variants/${variant}/${name}8T_${name}B(T-U)/*.cpp"
        "${PLATFORM-PATH}/variants/${variant}/${name}8T_${name}B(T-U)/*.c"
        ${PLATFORM-PATH}/system/Drivers/${variant}_HAL_Driver/Src/*.c

        ${PLATFORM-PATH}/libraries/SrcWrapper/src/*.c
        ${PLATFORM-PATH}/libraries/SrcWrapper/src/stm32/*.c
        ${PLATFORM-PATH}/libraries/SrcWrapper/src/stm32/*.cpp
        )
list(FILTER ARDUINO_SRC EXCLUDE REGEX ".*_template[.]c")

add_compile_definitions(USBCON)
add_compile_definitions(USBD_USE_CDC)
add_compile_definitions(ENABLE_HWSERIAL1)
add_compile_definitions(HAL_UART_MODULE_ENABLED)
add_compile_definitions(HAL_PCD_MODULE_ENABLED)
add_compile_definitions(USE_FULL_LL_DRIVER)


add_library(core ${ARDUINO_SRC})
include_directories(
        ${core_path}
        ${stm_core_path}
        ${stm_core_path}/LL
        ${usb_path}
        ${usb_path}/cdc
        ${usb_device_path}/Src
        ${usb_device_path}/Inc

        ${PLATFORM-PATH}/system/${variant}
        "${PLATFORM-PATH}/variants/${variant}/${name}8T_${name}B(T-U)"
        ${PLATFORM-PATH}/system/Drivers/CMSIS/Device/ST/${variant}/Include
        ${PLATFORM-PATH}/system/Drivers/CMSIS/Device/ST/${variant}/Source/Templates/gcc
        ${PLATFORM-PATH}/system/Drivers/${variant}_HAL_Driver/Inc
        ${CMSIS-PATH}/Core/Include
        ${PLATFORM-PATH}/libraries/SrcWrapper/src
        ${PLATFORM-PATH}/system/Drivers/${variant}_HAL_Driver/Src
        ${PLATFORM-PATH}/libraries/SrcWrapper/src/LL ## LL reference to _HAL_Driver/Src`s .c files
        ${PLATFORM-PATH}/libraries/SrcWrapper/src/stm32
)

set(LINKER_SCRIPT "${PLATFORM-PATH}/variants/${variant}/${name}8T_${name}B(T-U)/ldscript.ld")
target_compile_options(core PRIVATE ${OPT_FLAG} -Wno-deprecated-declarations)
target_link_libraries(core)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_COMPILER "${TOOL-PATH}/bin/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${TOOL-PATH}/bin/arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "${TOOL-PATH}/bin/arm-none-eabi-g++")
set(CMAKE_OBJCOPY ${TOOL-PATH}/bin/arm-none-eabi-objcopy)
set(CMAKE_AR ${TOOL-PATH}/bin/arm-none-eabi-ar)

set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_DEBUG "")


set(COMMON_FLAGS "-Wall -ffunction-sections -fdata-sections -flto -mcpu=cortex-m3 -mthumb")
set(CMAKE_CXX_FLAGS "-fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit -fno-rtti -std=gnu++11 ${COMMON_FLAGS}")
set(CMAKE_C_FLAGS "-std=gnu11 -fno-fat-lto-objects -nostdlib --param max-inline-insns-single=500 ${COMMON_FLAGS}")
set(CMAKE_ASM_FLAGS "-x assembler-with-cpp")

if (NOT EXISTS ARDUINO_SDK_PATH)
    set(SD_LIB_path ${CMAKE_CURRENT_SOURCE_DIR}/.pio/libdeps/controller/SD/src)

    file(GLOB_RECURSE _src ${SD_LIB_path}/*.c ${SD_LIB_path}/*.cpp)
    add_library(SD ${_src})
    target_include_directories(SD PUBLIC ${SD_LIB_path})
    target_compile_options(SD PRIVATE ${OPT_FLAG})
else ()
    ard_lib(SD SD_path ${SD_LIB_path})
endif ()


target_compile_options(SD PRIVATE -Wno-deprecated -Wno-deprecated-declarations)
ard_lib(SPI SPI_path ${PLATFORM-PATH})
ard_lib(Wire Wire_path ${PLATFORM-PATH})
target_include_directories(SD PUBLIC ${SPI_path})

set(u8g2_path ${ETL_PATH}/../U8g2)
file(GLOB_RECURSE U8g2_src ${u8g2_path}/src/*.c ${u8g2_path}/src/*.cpp)
add_library(U8g2 ${U8g2_src})
target_compile_definitions(U8g2 PUBLIC U8G2_16BIT)

target_compile_options(U8g2 PRIVATE ${OPT_FLAG} -Wno-deprecated -Wno-deprecated-declarations)
target_include_directories(U8g2 PRIVATE ${u8g2_path}/src)
target_link_libraries(U8g2 PUBLIC SPI Wire)

file(GLOB_RECURSE SRC_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
list(FILTER SRC_LIST EXCLUDE REGEX "src/main.cpp")
list(FILTER SRC_LIST EXCLUDE REGEX "src/devices/*")
add_library(controller ${SRC_LIST})
set(main_include src sec/ui src/gcode src/devices)
target_include_directories(controller PUBLIC ${main_include})
target_compile_options(controller PRIVATE ${OPT_FLAG})

add_library(drivers
        src/devices/GCodeDevice.cpp
        src/devices/GrblDevice.cpp
        src/devices/MarlinDevice.cpp
        )
target_include_directories(drivers PUBLIC ${main_include} ${ETL_PATH}/../SD/src ${ETL_PATH}/include )
target_compile_options(drivers PRIVATE ${DEBUG_OPT_FLAG})

if (CMAKE_BUILD_TYPE STREQUAL "debug")
    target_compile_definitions(controller PUBLIC
#            LOG_DEBUG
#            LOG_DETECTOR
#            USB_TO_SERIAL
            )

endif ()

target_include_directories(controller PUBLIC ${ETL_PATH}/include ${ETL_PATH}/../SD/src ${u8g2_path}/src)
target_link_libraries(controller PUBLIC U8g2 drivers)

if (EXISTS ${LINKER_SCRIPT})
    set(LINKER_SCRIPT "-T ${LINKER_SCRIPT}")
endif ()

set(CMAKE_EXE_LINKER_FLAGS "-g ${search_whole_arch} \
${LINKER_SCRIPT} \
-Wl,--as-needed \
-Wl,--gc-sections,--relax \
-Wl,--check-sections \
-Wl,--entry=Reset_Handler \
-Wl,--unresolved-symbols=report-all \
-Wl,--warn-common \
--specs=nosys.specs")

add_link_options(
        -Wl,--defsym=LD_MAX_SIZE=128K
        -Wl,--defsym=LD_MAX_DATA_SIZE=20K
        -Wl,--defsym=LD_FLASH_OFFSET=0x0)

find_library(cortexM3_math
        NAMES arm_cortexM3l_math
        PATHS ${CMSIS-PATH}/DSP/Lib/GCC
        REQUIRED)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--cref -Wl,-Map=${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.map ")
set(HEX_FILE ${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_NAME}.bin)

add_executable(${PROJECT_NAME}.elf src/main.cpp)
target_compile_options(${PROJECT_NAME}.elf PRIVATE ${DEBUG_OPT_FLAG})

target_link_libraries(${PROJECT_NAME}.elf ${cortexM3_math})
target_link_libraries(${PROJECT_NAME}.elf "$<LINK_LIBRARY:WHOLE_ARCHIVE,core>")
target_link_libraries(${PROJECT_NAME}.elf
        controller
        SD
        Wire)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> -R .eeprom ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        BYPRODUCTS ${HEX_FILE} ${BIN_FILE}
        COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}"
        VERBATIM)

add_custom_target(${PROJECT_NAME}-size
        COMMAND ${TOOL-PATH}/bin/arm-none-eabi-size -A -d ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
        COMMAND ${TOOL-PATH}/bin/arm-none-eabi-size -B -d ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf
        DEPENDS ${PROJECT_NAME}.elf
        )

find_program(OCD_PROG NAMES openocd)
if (NOT EXISTS ${OCD_PROG})
    set(OCD_PROG ${PLATFORM-PATH}/../tool-openocd/bin/openocd)
endif ()


add_custom_target(${PROJECT_NAME}-upload-ocd
        COMMAND ${OCD_PROG}
        -s ${OCD_PATH}/scripts
        -c \"set FLASH_SIZE 0x20000\"
        -f interface/stlink.cfg
        -c \"transport select hla_swd\"
        -f target/stm32f1x.cfg
        -d0
        -c \"program ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf verify reset\; shutdown\;\"
        DEPENDS ${PROJECT_NAME}.elf
        )
