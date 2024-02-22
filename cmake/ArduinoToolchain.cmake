#=============================================================================#
# Author: Tomasz Bogdal (QueezyTheGreat)
# Home:   https://github.com/queezythegreat/arduino-cmake
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#=============================================================================#
if (_IS_TOOLCHAIN_PROCESSED)
    return()
endif ()
set(_IS_TOOLCHAIN_PROCESSED True)

macro(dump_all)
    get_cmake_property(_variableNames VARIABLES)
    list(SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(DEBUG "${_variableName}=${${_variableName}}")
    endforeach ()
endmacro()

function(FATAL_BANNER msg)
    message(STATUS "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
    message(STATUS "${msg}")
    message(STATUS "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
    message(FATAL_ERROR "${msg}")
endfunction(FATAL_BANNER msg)


set(CMAKE_SYSTEM_NAME Arduino)
# Add current directory to CMake Module path automatically
if (NOT DEFINED ARDUINO_CMAKE_TOP_FOLDER)
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/Platform/Arduino.cmake)
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")
    endif ()
    set(ARDUINO_CMAKE_TOP_FOLDER ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")
endif (NOT DEFINED ARDUINO_CMAKE_TOP_FOLDER)

#=============================================================================#
#                         System Paths                                        #
#=============================================================================#
if (CMAKE_HOST_UNIX)
    include(Platform/UnixPaths)
    if (CMAKE_HOST_APPLE)
        list(APPEND CMAKE_SYSTEM_PREFIX_PATH ~/Applications
                /Applications
                /Developer/Applications
                /sw        # Fink
                /opt/local) # MacPorts
    endif ()
elseif (CMAKE_HOST_WIN32)
    include(Platform/WindowsPaths)
endif ()

#=============================================================================#
#                         Detect Arduino SDK                                  #
#=============================================================================#
if (DEFINED PLATFORM_PATH)
    set(ARDUINO_SDK_PATH ${PLATFORM_PATH})
    set(ARDUINO_CMAKE_SKIP_DETECT_VERSION True CACHE BOOL "Skip from detect")
    if (PLATFORM_PATH MATCHES ".*avr|stm32|esp32$")
        string(REGEX REPLACE ".*(avr|stm32|esp32)$" "\\1" PLATFORM_ARCHITECTURE ${PLATFORM_PATH})
    endif ()
endif ()

if (NOT ARDUINO_SDK_PATH AND NOT DEFINED ENV{_ARDUINO_CMAKE_WORKAROUND_ARDUINO_SDK_PATH})
    set(ARDUINO_PATHS)

    foreach (DETECT_VERSION_MAJOR 1)
        foreach (DETECT_VERSION_MINOR RANGE 5 0)
            list(APPEND ARDUINO_PATHS arduino-${DETECT_VERSION_MAJOR}.${DETECT_VERSION_MINOR})
            foreach (DETECT_VERSION_PATCH RANGE 3 0)
                list(APPEND ARDUINO_PATHS arduino-${DETECT_VERSION_MAJOR}.${DETECT_VERSION_MINOR}.${DETECT_VERSION_PATCH})
            endforeach ()
        endforeach ()
    endforeach ()

    foreach (VERSION RANGE 23 19)
        list(APPEND ARDUINO_PATHS arduino-00${VERSION})
    endforeach ()

    if (CMAKE_HOST_UNIX)
        file(GLOB SDK_PATH_HINTS
                /usr/share/arduino*
                /opt/local/arduino*
                /opt/arduino*
                /usr/local/share/arduino*)
    elseif (CMAKE_HOST_WIN32)
        set(SDK_PATH_HINTS
                "C:\\Program Files\\Arduino"
                "C:\\Program Files (x86)\\Arduino")
    endif ()
    list(SORT SDK_PATH_HINTS)
    list(REVERSE SDK_PATH_HINTS)

    if (DEFINED ENV{ARDUINO_SDK_PATH})
        list(APPEND SDK_PATH_HINTS $ENV{ARDUINO_SDK_PATH})
    endif ()

    find_path(ARDUINO_SDK_PATH
            NAMES lib/version.txt
            PATH_SUFFIXES share/arduino Arduino.app/Contents/Resources/Java/ Arduino.app/Contents/Java/ ${ARDUINO_PATHS}
            HINTS ${SDK_PATH_HINTS}
            DOC "Arduino SDK base directory")
elseif (NOT ARDUINO_SDK_PATH AND DEFINED ENV{_ARDUINO_CMAKE_WORKAROUND_ARDUINO_SDK_PATH})
    set(ARDUINO_SDK_PATH "$ENV{_ARDUINO_CMAKE_WORKAROUND_ARDUINO_SDK_PATH}")
endif ()


if (NOT DEFINED PLATFORM_ARCHITECTURE)
    set(PLATFORM_ARCHITECTURE "avr")
endif ()


message(STATUS "Architecture is ${PLATFORM_ARCHITECTURE}, System is ${CMAKE_SYSTEM_NAME}")
string(TOUPPER "${PLATFORM_ARCHITECTURE}" upper_arch)
if ("${upper_arch}" MATCHES "STM32.*")
    set(PLATFORM_ARCHITECTURE_POSTFIX "Stm32")
elseif (PLATFORM_ARCHITECTURE STREQUAL "avr")
    set(PLATFORM_ARCHITECTURE_POSTFIX "Avr")
elseif (PLATFORM_ARCHITECTURE STREQUAL "ESP32")
    set(PLATFORM_ARCHITECTURE_POSTFIX "Esp32")
else (PLATFORM_ARCHITECTURE STREQUAL "ESP32")
    message(FATAL_ERROR "Unknown architecture : ${PLATFORM_ARCHITECTURE} (${upper_arch})")
endif ()

include(${CMAKE_CURRENT_LIST_DIR}/ArduinoToolchain${PLATFORM_ARCHITECTURE_POSTFIX}.cmake)
