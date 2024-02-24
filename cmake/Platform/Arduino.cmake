#=============================================================================#
# Original Author: Tomasz Bogdal (QueezyTheGreat)
# Current Author: Timor Gruber (MrPointer)
# Original Home: https://github.com/queezythegreat/arduino-cmake
# Current Home: https://github.com/arduino-cmake/arduino-cmake
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#=============================================================================#

cmake_minimum_required(VERSION 3.8)

set(CMAKE_MODULE_PATH
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Initialization
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Core
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Core/BoardFlags
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Core/Libraries
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Core/Targets
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Core/Sketch
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Core/Examples
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Extras
        ${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Generation
        )

include(CMakeParseArguments)

include(Util)
include(VariableValidator)
include(Initializer)


#include(BoardPropertiesReader)

return()
include(FlagsSetter)
#TODO ABI & Check for working C compiler EROORS

include(SourceFinder)
include(LibraryFinder)

include(ArduinoSketchToCppConverter)
include(ArduinoSketchFactory)

include(CoreLibraryFactory)
include(ArduinoLibraryFactory)
include(BlacklistedLibrariesRemover)

include(ArduinoExampleFactory)
include(ArduinoLibraryExampleFactory)

include(ArduinoBootloaderArgumentsBuilder)
include(ArduinoBootloaderBurnTargetCreator)
include(ArduinoBootloaderUploadTargetCreator)
include(ArduinoFirmwareTargetCreator)
include(ArduinoProgrammerArgumentsBuilder)
include(ArduinoProgrammerBurnTargetCreator)
include(ArduinoSerialTargetCreator)
include(ArduinoUploadTargetCreator)

include(AvrLibraryGenerator)
include(AvrFirmwareGenerator)
include(ArduinoLibraryGenerator)
include(ArduinoFirmwareGenerator)
include(ArduinoExampleGenerator)
include(ArduinoLibraryExampleGenerator)

# Define the 'RECURSE' flag for libraries known to depend on it
set(Wire_RECURSE True)
set(Ethernet_RECURSE True)
set(Robot_Control_RECURSE True)
set(SD_RECURSE True)
set(Servo_RECURSE True)
set(Temboo_RECURSE True)
set(TFT_RECURSE True)
set(WiFi_RECURSE True)

#TODO find place in target actions
#if (${PLATFORM}_BOARDS_PATH)
#    set(SETTINGS_LIST ${PLATFORM}_BOARDS)
#    set(SETTINGS_PATH "${${PLATFORM}_BOARDS_PATH}")
#    include(LoadArduinoPlatformSettings)
#endif ()
#
#if (${PLATFORM}_PROGRAMMERS_PATH)
#    set(SETTINGS_LIST ${PLATFORM}_PROGRAMMERS)
#    set(SETTINGS_PATH "${${PLATFORM}_PROGRAMMERS_PATH}")
#    include(LoadArduinoPlatformSettings)
#endif ()
#
#if (${PLATFORM}_VARIANTS_PATH)
#    file(GLOB sub-dir ${${PLATFORM}_VARIANTS_PATH}/*)
#    foreach (dir ${sub-dir})
#        if (IS_DIRECTORY ${dir})
#            get_filename_component(variant ${dir} NAME)
#            set(VARIANTS ${VARIANTS} ${variant} CACHE INTERNAL "A list of registered variant boards")
#            set(${variant}.path ${dir} CACHE INTERNAL "The path to the variant ${variant}")
#            message(VERBOSE "Registerd variants ${${variant}.path}")
#        endif ()
#    endforeach ()
#endif ()
#
#if (${PLATFORM}_CORES_PATH)
#    file(GLOB sub-dir ${${PLATFORM}_CORES_PATH}/*)
#    foreach (dir ${sub-dir})
#        if (IS_DIRECTORY ${dir})
#            get_filename_component(core ${dir} NAME)
#            set(CORES ${CORES} ${core} CACHE INTERNAL "A list of registered cores")
#            set(${core}.path ${dir} CACHE INTERNAL "The path to the core ${core}")
#
#            # See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5-3rd-party-Hardware-specification#referencing-another-core-variant-or-tool
#            # for an explanation why cores must also be available as <vendor_id>:<core_id>
#            # and <vendor_id>:<architecture_id>:<core_id>
#            set(CORES ${CORES} "${VENDOR_ID}:${core}" CACHE INTERNAL "A list of registered cores")
#            set(${VENDOR_ID}:${core}.path ${dir} CACHE INTERNAL "The path to the core ${core}")
#            set(CORES ${CORES} "${VENDOR_ID}:${ARCHITECTURE_ID}:${core}" CACHE INTERNAL "A list of registered cores")
#
#        endif ()
#    endforeach ()
#endif ()
#
#
#if (${PLATFORM}_PLATFORM_FILE_PATH)
#    set(SETTINGS_LIST ${PLATFORM}_PLATFORM)
#    set(SETTINGS_PATH "${${PLATFORM}_PLATFORM_FILE_PATH}")
#    include(LoadArduinoPlatformSettings)
#endif ()

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CACHE INTERNAL "")
# Setup libraries known to be recursive only once
