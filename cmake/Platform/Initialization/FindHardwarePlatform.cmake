#============================================================
#       Register Platform
#============================================================
# this may work with multiple platforms
#============================================================
if (CUSTOM_PLATFORM_REGISTRATION_SCRIPT)
    include("${CUSTOM_PLATFORM_REGISTRATION_SCRIPT}")
    return()
endif ()

set(HINTS ${PLATFORM_PATH}/hardware/${PLATFORM}/${CMAKE_SYSTEM_PROCESSOR}
        ${PLATFORM_PATH}/${PLATFORM}
        ${PLATFORM_PATH}/*
        ${PLATFORM_PATH})

find_file(${CMAKE_SYSTEM_PROCESSOR}_CORES_PATH
        NAMES cores
        PATHS ${HINTS}
        DOC "Path to directory containing the Arduino core sources.")

find_file(${CMAKE_SYSTEM_PROCESSOR}_VARIANTS_PATH
        NAMES variants
        PATHS ${HINTS}
        DOC "Path to directory containing the Arduino variant sources.")
find_file(${CMAKE_SYSTEM_PROCESSOR}_LIBS_PATH
        NAMES libraries
        PATHS ${HINTS}
        DOC "Path to directory containing the Arduino variant sources.")

find_file(${CMAKE_SYSTEM_PROCESSOR}_BOOTLOADERS_PATH
        NAMES bootloaders
        PATHS ${HINTS}
        DOC "Path to directory containing the Arduino bootloader images and sources.")

find_file(${CMAKE_SYSTEM_PROCESSOR}_PROGRAMMERS_PATH
        NAMES programmers.txt
        PATHS ${HINTS}
        DOC "Path to Arduino programmers definition file.")

find_file(${CMAKE_SYSTEM_PROCESSOR}_BOARDS_PATH
        NAMES boards.txt
        PATHS ${HINTS}
        DOC "Path to Arduino boards definition file."
        REQUIRED)

find_file(${CMAKE_SYSTEM_PROCESSOR}_PLATFORM_FILE_PATH
        NAMES platform.txt
        PATHS ${HINTS}
        DOC "Path to Arduino platform definition file.")

# some libraries are in platform path in versions 1.5 and greater
if (ARDUINO_SDK_VERSION VERSION_GREATER 1.0.5) # FOR non arduino LIBS & LIBRARIES is SAME
    find_file(ARDUINO_PLATFORM_LIBRARIES_PATH
            NAMES libraries
            PATHS ${PLATFORM_PATH}
            REQUIRED
            DOC "Path to platform directory containing the Arduino libraries.")

    if (${ARDUINO_PLATFORM_LIBRARIES_PATH} STREQUAL ${${CMAKE_SYSTEM_PROCESSOR}_LIBS_PATH})
        unset(ARDUINO_PLATFORM_LIBRARIES_PATH CACHE)
    endif ()
endif ()
message(VERBOSE "Found HW platform components")
