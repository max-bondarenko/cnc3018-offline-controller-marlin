cmake_minimum_required(VERSION 3.16)
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

project(cnc_3018)

function(ard_lib lib_name lib_path_name lib_path)
    if (lib_path)
        set(${lib_name}_path ${lib_path}/libraries/${lib_name}/src)
    else ()
        set(${lib_name}_path ${ARDUINO_SDK_PATH}/libraries/${lib_name}/src)
    endif ()

    file(GLOB_RECURSE _src ${${lib_name}_path}/*.c ${${lib_name}_path}/*.cpp)
    add_library(${lib_name} ${_src})
    target_include_directories(${lib_name} PUBLIC ${${lib_name}_path})


    set(${lib_path_name} ${${lib_name}_path} PARENT_SCOPE)
endfunction()


set(board STM32F1)
set(variant ${board}xx)
set(name F103C)
set(series ${name}BT)
set(board_name GENERIC_${series}X)

add_compile_definitions(ARDUINO_ARCH_STM32)
add_compile_definitions(PLATFORMIO=60111)
add_compile_definitions(ARDUINO=10808)
add_compile_definitions(${board})
add_compile_definitions(${variant})
add_compile_definitions(${board}03xB)
add_compile_definitions(BOARD_NAME=\"${board_name}\")
add_compile_definitions(ARDUINO_${board_name})
add_compile_definitions(VARIANT_H=\"variant_generic.h\")


set(core_path ${PLATFORM-PATH}/cores/arduino)
set(stm_core_path ${core_path}/stm32)

file(GLOB ARDUINO_SRC
        ${core_path}/*.c
        ${core_path}/*.cpp
        ${stm_core_path}/*.S)

set(HAL_PATH ${PLATFORM-PATH}/system/Drivers/${variant}_HAL_Driver)
set(HAL_INCLUDE ${HAL_PATH}/Inc)
file(GLOB HAL_SRC ${HAL_PATH}/Src/*.c)
add_library(HAL STATIC ${HAL_SRC})
target_include_directories(HAL PUBLIC
        ${HAL_INCLUDE}
        ${PLATFORM-PATH}/system/${variant}
        "${PLATFORM-PATH}/variants/${variant}/${name}8T_${F103C}B(T-U)")


add_compile_definitions(USBCON)
add_compile_definitions(USBD_USE_CDC)
add_compile_definitions(ENABLE_HWSERIAL1)
add_compile_definitions(HAL_UART_MODULE_ENABLED)
add_compile_definitions(HAL_PCD_MODULE_ENABLED)
add_compile_definitions(USE_FULL_LL_DRIVER)

set(usb_device_path ${PLATFORM-PATH}/system/Middlewares/ST/STM32_USB_Device_Library/Core)
set(usb_path ${stm_core_path}/usb)

file(GLOB usb_device_src ${usb_device_path}/Src/*.c)
list(FILTER usb_device_src EXCLUDE REGEX "usbd_(desc|conf)_template[.]c")
file(GLOB usb_src ${usb_path}/*.c)
add_library(usb_device STATIC ${usb_src} ${usb_device_src})

target_include_directories(usb_device PUBLIC
        ${HAL_INCLUDE}
        ${usb_path}
        ${usb_path}/cdc
        ${usb_device_path}/Src
        ${usb_device_path}/Inc)


add_library(core STATIC ${ARDUINO_SRC})
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
        ${HAL_INCLUDE}
        ${CMSIS-PATH}/Drive/Include
        ${CMSIS-PATH}/Core/Include
        ${CMSIS-PATH}/DSP/Include
        ${CMSIS-PATH}/DSP/PrivateInclude
)
target_compile_options(core PRIVATE -Wdeprecated-declarations)

#todo CMSIS as independent


set(openAmp_path ${stm_core_path}/OpenAMP)
file(GLOB openAmp_src ${openAmp_path}/*.c)
add_library(OpenAMP STATIC ${openAmp_src})
target_include_directories(OpenAMP PUBLIC ${openAmp_path})


SET(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_C_COMPILER "${TOOL-PATH}/bin/arm-none-eabi-gcc")
SET(CMAKE_CXX_COMPILER "${TOOL-PATH}/bin/arm-none-eabi-g++")
SET(OBJCOPY ${TOOL-PATH}/bin/arm-none-eabi-objcopy)


add_subdirectory(lib)

ard_lib(SD SD_path "")
ard_lib(SPI SPI_path ${PLATFORM-PATH})
ard_lib(Wire Wire_path ${PLATFORM-PATH})
target_include_directories(SD PUBLIC ${SPI_path})

set(u8g2_path ${ETL_PATH}/../U8g2)
file(GLOB_RECURSE U8g2_src ${u8g2_path}/src/*.c ${u8g2_path}/src/*.cpp)
add_library(U8g2 ${U8g2_src})

target_compile_definitions(U8g2 PRIVATE U8G2_16BIT)
target_compile_options(U8g2 PRIVATE -Wno-deprecated -Wno-deprecated-declarations)
target_include_directories(U8g2 PUBLIC ${u8g2_path}/src)
target_link_libraries(U8g2 PUBLIC SPI Wire)

file(GLOB_RECURSE SRC_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)
list(FILTER SRC_LIST EXCLUDE REGEX "src/main.cpp")
add_library(controller ${SRC_LIST})
set(main_include src sec/ui src/gcode src/)
target_include_directories(controller PUBLIC ${main_include})
target_compile_definitions(controller PUBLIC LOG_DEBUG)

target_include_directories(controller PUBLIC ${ETL_PATH}/include ${ETL_PATH}/../SD/src ${u8g2_path}/src lib/printfloat/)



#
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${FRAMEWORK}/system/ldscript.ld")

#link_directories(BEFORE ${PLATFORMIO_HOME}/toolchain-gccarmnoneeabi/lib)
#link_directories(${PLATFORMIO_HOME}/toolchain-gccarmnoneeabi/lib/gcc/arm-none-eabi/9.2.1)


SET(COMMON_FLAGS "-Os -Wall -ffunction-sections -fdata-sections -flto")
SET(CMAKE_CXX_FLAGS "-fno-exceptions -fno-threadsafe-statics -std=gnu++11 ${COMMON_FLAGS}")
SET(CMAKE_C_FLAGS "-std=gnu11 -fno-fat-lto-objects  ${COMMON_FLAGS}")

#set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F446RETx_FLASH.ld)
#set(CMAKE_EXE_LINKER_FLAGS "-Wl,-gc-sections -T ${LINKER_SCRIPT}")


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map")
set(HEX_FILE ${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_NAME}.bin)

add_executable(${PROJECT_NAME}.elf src/main.cpp)
target_link_libraries(${PROJECT_NAME}.elf core controller printfloat U8g2 SD SPI)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}"
        VERBATIM)




