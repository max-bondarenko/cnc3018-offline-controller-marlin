set(CMAKE_SYSTEM_NAME Arduino)
set(CMAKE_SYSTEM_VERSION 1.0)
set(PLATFORM "arduino" CACHE STRING "Arduino SDK platform")
if (DEFINED PLATFORM_PATH AND DEFINED PLATFORM_TOOLCHAIN_PATH)
    MESSAGE(STATUS "Skip setup in SubBuild")
    return()
endif ()

# Add current directory to CMake Module path automatically
if (NOT DEFINED ARDUINO_CMAKE_TOP_FOLDER)
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/Platform/Arduino.cmake)
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")
    endif ()
    set(ARDUINO_CMAKE_TOP_FOLDER ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")
endif ()

macro(fatal_banner msg)
    message(STATUS "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
    message(STATUS "${msg}")
    message(STATUS "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
    message(FATAL_ERROR)
endmacro()


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
if (NOT DEFINED PLATFORM_PATH)
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

    if (DEFINED ENV{ARDUINO_SDK_PATH})
        list(APPEND SDK_PATH_HINTS $ENV{ARDUINO_SDK_PATH})
    endif ()

    find_path(PATH
            NAMES lib/version.txt
            PATH_SUFFIXES share/arduino Arduino.app/Contents/Resources/Java/ Arduino.app/Contents/Java/
            HINTS ${SDK_PATH_HINTS})

    if (EXISTS ${PATH})
        set(ARDUINO_IDE TRUE CACHE BOOL "we  work with Arduino Default IDE")
        SET(PLATFORM_PATH ${PATH} CACHE PATH "Arduino SDK base directory")
        SET(PLATFORM_TOOLCHAIN_PATH ${PATH}/hardware/tools/ CACHE PATH "Arduino SDK base directory")
        SET(CMAKE_SYSTEM_PROCESSOR "avr" CACHE INTERNAL "")
        list(APPEND CMAKE_SYSTEM_PREFIX_PATH ${PLATFORM_TOOLCHAIN_PATH}/avr)
        list(APPEND CMAKE_SYSTEM_PREFIX_PATH ${PLATFORM_TOOLCHAIN_PATH}/avr/utils)

        set(CMAKE_C_COMPILER avr-gcc)
        set(CMAKE_ASM_COMPILER avr-gcc)
        set(CMAKE_CXX_COMPILER avr-g++)
    endif ()

    #elseif (NOT ARDUINO_SDK_PATH AND DEFINED ENV{_ARDUINO_CMAKE_WORKAROUND_ARDUINO_SDK_PATH})
    #  TODO   set(ARDUINO_SDK_PATH "$ENV{_ARDUINO_CMAKE_WORKAROUND_ARDUINO_SDK_PATH}")
else ()
    include(${CMAKE_CURRENT_LIST_DIR}/ArduinoToolchainStm32.cmake)
endif ()

if(NOT DEFINED PLATFORM_TOOLCHAIN_PATH)
    fatal_banner("NO platform specified")
endif()

SET(CMAKE_VERBOSE_MAKEFILE TRUE)
#
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
        CMAKE_SYSTEM_NAME
        ARDUINO_SDK_VERSION
        PLATFORM_PATH
        PLATFORM_TOOLCHAIN_PATH
        PLATFORM
        ARDUINO_IDE)
