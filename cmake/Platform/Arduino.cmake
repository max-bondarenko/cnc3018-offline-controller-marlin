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
if (IS_SCRIPT_PROCESSED)
    return()
endif ()

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

include(SetupRecursiveLibraries)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} CACHE INTERNAL "")
# Setup libraries known to be recursive only once

set(IS_SCRIPT_PROCESSED True CACHE BOOL
        "Indicates whether platform script has already been processed")
