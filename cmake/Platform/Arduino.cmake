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

include(BoardPropertiesReader)
include(FlagsSetter)
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

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CACHE INTERNAL "")
# Setup libraries known to be recursive only once
