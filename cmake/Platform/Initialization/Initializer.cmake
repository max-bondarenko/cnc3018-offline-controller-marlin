#=============================================================================#
#                          Initialization
#=============================================================================#

#=============================================================================#
#                              C Flags
#=============================================================================#
include(${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Initialization/DefaultCFlags_${PLATFORM}.cmake)

if (NOT DEFINED ARDUINO_C_FLAGS) # todo may be for adnv setup
    set(ARDUINO_C_FLAGS "${ARDUINO_DEFAULT_CFLAGS}" CACHE INTERNAL "")
endif ()
set(CMAKE_C_FLAGS "-g -Os ${ARDUINO_C_FLAGS}" CACHE STRING "")
set(CMAKE_C_FLAGS_DEBUG "-g" CACHE STRING "")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG" CACHE STRING "")
set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG -w" CACHE STRING "")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Os -g -w " CACHE STRING "")

#=============================================================================#
#                             C++ Flags
#=============================================================================#
#=============================================================================#
# setup_c_flags
# [PRIVATE/INTERNAL]
#
# setup_cxx_flags()
#
# Setups some basic flags for the g++ compiler to use later.
#=============================================================================#
if (NOT DEFINED ARDUINO_CXX_FLAGS)
    if (DEFINED ARDUINO_DEFAULT_CXXFLAGS)
        set(ARDUINO_CXX_FLAGS "${ARDUINO_DEFAULT_CXXFLAGS}" CACHE INTERNAL "")
    else ()
        set(ARDUINO_CXX_FLAGS "${ARDUINO_C_FLAGS} -fno-exceptions" CACHE INTERNAL "")
    endif ()
endif ()
set(CMAKE_CXX_FLAGS "-g -Os ${ARDUINO_CXX_FLAGS}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING "")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -g" CACHE STRING "")

if (NOT DEFINED ARDUINO_LINKER_FLAGS)
    set(ARDUINO_LINKER_FLAGS "-nostdlib -lgcc -Wl,--gc-sections -lm ")
endif ()
set(CMAKE_EXE_LINKER_FLAGS "${ARDUINO_LINKER_FLAGS}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE STRING "")

set(CMAKE_SHARED_LINKER_FLAGS "${ARDUINO_LINKER_FLAGS}" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "" CACHE STRING "")

set(CMAKE_MODULE_LINKER_FLAGS "${ARDUINO_LINKER_FLAGS}" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE STRING "")

SET(CMAKE_TRY_COMPILE_CONFIGURATION "Debug" CACHE STRING "")
#todo  this may depend on PLATFORM
set(ARDUINO_OBJCOPY_EEP_FLAGS -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load
        --no-change-warnings --change-section-lma .eeprom=0 CACHE STRING "")
set(ARDUINO_OBJCOPY_HEX_FLAGS -O ihex -R .eeprom CACHE STRING "")
set(ARDUINO_AVRDUDE_FLAGS -V CACHE STRING "")

include(DetectVersion)

#include(RegisterHardwarePlatform)

return()


include(FindPrograms)

set(ARDUINO_DEFAULT_BOARD uno CACHE STRING "Default Arduino Board ID when not specified.")
set(ARDUINO_DEFAULT_BOARD_CPU CACHE STRING "Default Arduino Board CPU when not specified.")
set(ARDUINO_DEFAULT_PORT CACHE STRING "Default Arduino port when not specified.")
set(ARDUINO_DEFAULT_SERIAL CACHE STRING "Default Arduino Serial command when not specified.")
set(ARDUINO_DEFAULT_PROGRAMMER CACHE STRING "Default Arduino Programmer ID when not specified.")
set(ARDUINO_CMAKE_RECURSION_DEFAULT FALSE CACHE BOOL "The default recursion behavior during library setup")

include(SetupFirmwareSizeScript) # TODO

set(ARDUINO_LIBRARY_BLACKLIST "" CACHE STRING
        "A list of absolute paths to Arduino libraries that are meant to be ignored during library search")

if (NOT ARDUINO_CMAKE_SKIP_TEST_SETUP)
    # Ensure that all required paths are found
    validate_variables_not_empty(VARS
            ARDUINO_PLATFORMS
            ARDUINO_CORES_PATH
            ARDUINO_BOOTLOADERS_PATH
            ARDUINO_LIBRARIES_PATH
            ARDUINO_BOARDS_PATH
            ARDUINO_PROGRAMMERS_PATH
            #                ARDUINO_VERSION_PATH TODO
            ARDUINO_AVRDUDE_FLAGS
            ARDUINO_AVRDUDE_PROGRAM
            ARDUINO_AVRDUDE_CONFIG_PATH
            AVRSIZE_PROGRAM
            ${ADDITIONAL_REQUIRED_VARS}
            MSG "Invalid Arduino SDK path (${ARDUINO_SDK_PATH}).\n")
endif ()
set(ARDUINO_FOUND True CACHE INTERNAL "Arduino Found")


