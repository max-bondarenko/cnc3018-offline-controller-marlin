if (ARDUINO_IDE)
    find_file(${CMAKE_SYSTEM_PROCESSOR}_EXAMPLES_PATH
            NAMES examples
            PATHS ${PLATFORM_PATH}
            DOC "Path to directory containg the Arduino built-in examples."
            NO_DEFAULT_PATH)

    find_file(${CMAKE_SYSTEM_PROCESSOR}_LIBRARIES_PATH
            NAMES libraries
            PATHS ${PLATFORM_PATH}
            DOC "Path to directory containing the Arduino libraries."
            NO_DEFAULT_PATH)

    find_program(${CMAKE_SYSTEM_PROCESSOR}_AVRDUDE_PROGRAM
            NAMES avrdude
            PATHS ${PLATFORM_PATH}
            PATH_SUFFIXES hardware/tools hardware/tools/avr/bin
            NO_DEFAULT_PATH)

    find_program(${CMAKE_SYSTEM_PROCESSOR}_AVRDUDE_PROGRAM
            NAMES avrdude
            DOC "Path to avrdude programmer binary.")

    find_program(AVRSIZE_PROGRAM
            NAMES avr-size)
    if (AVRSIZE_PROGRAM)
        set(FIRMWARE_SIZE_SCRIPT_PATH "${CMAKE_CURRENT_LIST_DIR}/../Extras/CalculateFirmwareSize.cmake")
        file(READ ${FIRMWARE_SIZE_SCRIPT_PATH} FIRMWARE_SIZE_SCRIPT)

        # Replace placeholders with matching arguments
        string(REGEX REPLACE "PLACEHOLDER_1" "${AVRSIZE_PROGRAM}" FIRMWARE_SIZE_SCRIPT "${FIRMWARE_SIZE_SCRIPT}")

        # Create the replaced file in the build's cache directory
        set(CACHED_FIRMWARE_SCRIPT_PATH ${CMAKE_BINARY_DIR}/CMakeFiles/FirmwareSize.cmake)
        file(WRITE ${CACHED_FIRMWARE_SCRIPT_PATH} "${FIRMWARE_SIZE_SCRIPT}")

        set(ARDUINO_SIZE_SCRIPT ${CACHED_FIRMWARE_SCRIPT_PATH} CACHE INTERNAL "Arduino Size Script")
    endif ()

    find_file(${CMAKE_SYSTEM_PROCESSOR}_AVRDUDE_CONFIG_PATH
            NAMES avrdude.conf
            PATHS ${PLATFORM_PATH} /etc/avrdude /etc
            PATH_SUFFIXES hardware/tools
            hardware/tools/avr/etc
            DOC "Path to avrdude programmer configuration file.")

    if (ARDUINO_SDK_VERSION VERSION_LESS 1.0.0)
        find_file(${CMAKE_SYSTEM_PROCESSOR}_PLATFORM_HEADER_FILE_PATH
                NAMES WProgram.h
                PATHS ${PLATFORM_PATH}
                PATH_SUFFIXES hardware/arduino/avr/cores/arduino
                DOC "Path to Arduino platform's main header file"
                NO_DEFAULT_PATH)
    else ()
        find_file(${CMAKE_SYSTEM_PROCESSOR}_PLATFORM_HEADER_FILE_PATH
                NAMES Arduino.h
                PATHS ${PLATFORM_PATH}
                PATH_SUFFIXES hardware/arduino/avr/cores/arduino
                DOC "Path to Arduino platform's main header file"
                NO_DEFAULT_PATH)
    endif ()

    if (NOT CMAKE_OBJCOPY)
        find_program(AVROBJCOPY_PROGRAM
                avr-objcopy)
        set(ADDITIONAL_REQUIRED_VARS AVROBJCOPY_PROGRAM)
        set(CMAKE_OBJCOPY ${AVROBJCOPY_PROGRAM})
    endif (NOT CMAKE_OBJCOPY)
endif ()
