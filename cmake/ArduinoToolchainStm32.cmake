#=============================================================================#
# Author: Tomasz Bogdal (QueezyTheGreat)
# Home:   https://github.com/queezythegreat/arduino-cmake
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#=============================================================================#
message(VERBOSE "Setting up STM32 arduino cmake environment")

if (NOT PLATFORM_TOOLCHAIN_PATH)
    message(FATAL_ERROR "PLATFORM_TOOLCHAIN_PATH is not defined\n set \"set(PLATFORM_TOOLCHAIN_PATH <path_to_platform_base_dir>)\"")
endif (NOT PLATFORM_TOOLCHAIN_PATH)

list(APPEND CMAKE_SYSTEM_PREFIX_PATH "${PLATFORM_TOOLCHAIN_PATH}")

set(STM32_TOOLCHAIN_PREFIX arm-none-eabi-)

file(GLOB PLATFORM_TOOLCHAIN_PATH_HINTS
        ${PLATFORM_TOOLCHAIN_PATH}/*
        )
find_program(PLATFORM_TOOLCHAIN_GCC
        ${STM32_TOOLCHAIN_PREFIX}gcc
        PATHS ${PLATFORM_TOOLCHAIN_PATH_HINTS}
        DOC "stm32 gcc"
        REQUIRED
        )
find_program(PLATFORM_TOOLCHAIN_GPP
        ${STM32_TOOLCHAIN_PREFIX}g++
        PATHS ${PLATFORM_TOOLCHAIN_PATH_HINTS}
        DOC "stm32 g++"
        REQUIRED
        )
set(CMAKE_C_COMPILER_ID "GNU" CACHE INTERNAL "")
set(CMAKE_C_COMPILER ${PLATFORM_TOOLCHAIN_GCC} CACHE PATH "" FORCE)
set(CMAKE_ASM_COMPILER ${PLATFORM_TOOLCHAIN_GCC} CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER_ID "GNU" CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER ${PLATFORM_TOOLCHAIN_GPP} CACHE PATH "" FORCE)


if (EXISTS "${PLATFORM_PATH}/boards.txt")
    set(STM32_FLAVOR "RGCLARCK" CACHE INTERNAL "")
else ()
    #SET(STM32_FLAVOR "VANILLA" CACHE INTERNAL "")
    fatal_banner("!! PLATFORM_PATH does not point to valid STM32 arduino files!! (${PLATFORM_PATH})")
endif ()

# TODO ???
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER>   <CMAKE_CXX_LINK_FLAGS>  <LINK_FLAGS> -lgcc -Wl,--start-group  <OBJECTS> <LINK_LIBRARIES> -Wl,--end-group   -o <TARGET> ")

message(STATUS "C   compiler ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler ${CMAKE_CXX_COMPILER}")

if (ARDUINO_SDK_PATH)
    set(ENV{_ARDUINO_CMAKE_WORKAROUND_ARDUINO_SDK_PATH} "${ARDUINO_SDK_PATH}")
else ()
    message(FATAL_ERROR "Could not find Arduino SDK (set ARDUINO_SDK_PATH)!")
endif ()

