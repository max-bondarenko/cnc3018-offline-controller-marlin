message(VERBOSE "Setting up STM32 arduino cmake environment")

if (NOT DEFINED TOOLCHAIN_PATH)
    fatal_banner("No TOOLCHAIN_PATH specified")
endif ()

if (DEFINED PLATFORM_PATH)
    set(STM32_TOOLCHAIN_PREFIX arm-none-eabi-)

    file(GLOB PLATFORM_TOOLCHAIN_PATH_HINTS
            ${PLATFORM_PATH}/*
            ${TOOLCHAIN_PATH}/*
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
    get_filename_component(PATH ${PLATFORM_TOOLCHAIN_GCC} DIRECTORY)
    get_filename_component(PATH ${PATH} DIRECTORY)

    if (EXISTS ${PATH})
        set(PLATFORM_TOOLCHAIN_PATH ${PATH} CACHE PATH "" FORCE)
        list(APPEND CMAKE_SYSTEM_PREFIX_PATH "${PLATFORM_TOOLCHAIN_PATH}")
    endif ()

    set(CMAKE_SYSTEM_PROCESSOR "stm32" CACHE STRING "Stm32 platform")
    set(ARDUINO_SDK_VERSION "1.8.6" CACHE STRING "Arduino SDK Version") # todo workaround

    set(CMAKE_C_COMPILER_ID "GNU" CACHE INTERNAL "")
    set(CMAKE_C_COMPILER ${PLATFORM_TOOLCHAIN_GCC} CACHE PATH "" FORCE)
    set(CMAKE_ASM_COMPILER ${PLATFORM_TOOLCHAIN_GCC} CACHE PATH "" FORCE)
    set(CMAKE_CXX_COMPILER_ID "GNU" CACHE INTERNAL "")
    set(CMAKE_CXX_COMPILER ${PLATFORM_TOOLCHAIN_GPP} CACHE PATH "" FORCE)

    # TODO ???
    set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> -Wl,--start-group <OBJECTS> <LINK_LIBRARIES> -Wl,--end-group -o <TARGET> ")

    message(VERBOSE "C   compiler ${CMAKE_C_COMPILER}")
    message(VERBOSE "C++ compiler ${CMAKE_CXX_COMPILER}")
    message(VERBOSE "TOOLCHAIN is ${PLATFORM_TOOLCHAIN_PATH}")
endif ()

