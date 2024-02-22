#=============================================================================#
#                              C Flags
#=============================================================================#
include(${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Initialization/DefaultCFlags${PLATFORM_ARCHITECTURE_POSTFIX}.cmake)
#=============================================================================#
# setup_c_flags
# [PRIVATE/INTERNAL]
#
# setup_c_flags()
#
# Setups some basic flags for the gcc compiler to use later.
#=============================================================================#
if (NOT DEFINED ARDUINO_C_FLAGS)
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

#=============================================================================#
#                       Executable Linker Flags                               #
#=============================================================================#
#=============================================================================#
# setup_exe_linker_flags
# [PRIVATE/INTERNAL]
#
# setup_exe_linker_flags()
#
# Setups some basic flags for the gcc linker to use when linking executables.
#=============================================================================#

if (NOT DEFINED ARDUINO_LINKER_FLAGS)
    set(ARDUINO_LINKER_FLAGS "-Wl,--gc-sections -lm")
endif ()
set(CMAKE_EXE_LINKER_FLAGS "${ARDUINO_LINKER_FLAGS}" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE STRING "")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE STRING "")


#=============================================================================#
#                       Shared Library Linker Flags                           #
#=============================================================================#
#=============================================================================#
# setup_shared_lib_flags
# [PRIVATE/INTERNAL]
#
# setup_shared_lib_flags()
#
# Setups some basic flags for the gcc linker to use when linking shared libraries.
#=============================================================================#

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



