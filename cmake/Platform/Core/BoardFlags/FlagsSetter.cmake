
#=============================================================================#
# _sanitize_quotes
# [PRIVATE/INTERNAL]
#
# _sanitize_quotes(CMD_LINE_VARIABLE)
#
#       CMD_LINE_VARIABLE - Variable holding a shell command line
#                           or command line flag(s) that potentially
#                           require(s) quotes to be fixed.
#
# Replaces Unix-style quotes with Windows-style quotes.
# '-DSOME_MACRO="foo"' would become "-DSOME_MACRO=\"foo\"".
#
#=============================================================================#
function(_sanitize_quotes CMD_LINE_VARIABLE)
    if (CMAKE_HOST_WIN32)

        # Important: The order of the statements below does matter!

        # First replace all occurences of " with \"
        #
        string(REPLACE "\"" "\\\"" output "${${CMD_LINE_VARIABLE}}")

        # Then replace all ' with "
        #
        string(REPLACE "'" "\"" output "${output}")

        set(${CMD_LINE_VARIABLE} "${output}" PARENT_SCOPE)
    endif ()
endfunction()

# ToDo: Comment
function(set_board_compiler_flags COMPILER_FLAGS NORMALIZED_SDK_VERSION BOARD_ID IS_MANUAL)
    include(${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Core/BoardFlags/CompilerFlagsSetter_${CMAKE_SYSTEM_PROCESSOR}.cmake)
    _try_get_board_property(${BOARD_ID} build.vid VID)
    _try_get_board_property(${BOARD_ID} build.pid PID)
    if (NOT ${VID} STREQUAL "")
        set(COMPILE_FLAGS "${COMPILE_FLAGS} -DUSB_VID=${VID}")
    endif ()
    if (NOT ${PID} STREQUAL "")
        set(COMPILE_FLAGS "${COMPILE_FLAGS} -DUSB_PID=${PID}")
    endif ()

    _try_get_board_property(${BOARD_ID} build.extra_flags EXTRA_FLAGS)

    if (NOT "${EXTRA_FLAGS}" STREQUAL "")
        _sanitize_quotes(EXTRA_FLAGS)
        set(COMPILE_FLAGS "${COMPILE_FLAGS} ${EXTRA_FLAGS}")
    endif ()
       # TODO this does not substitute usb flags form platform !! fix
    _try_get_board_property(${BOARD_ID} build.usb_flags USB_FLAGS)
    if (NOT "${USB_FLAGS}" STREQUAL "")
        _sanitize_quotes(USB_FLAGS)
        set(COMPILE_FLAGS "${COMPILE_FLAGS} ${USB_FLAGS}")
    endif ()

    if (NOT IS_MANUAL)
        _get_board_property(${BOARD_ID} build.core BOARD_CORE)
        set(COMPILE_FLAGS "${COMPILE_FLAGS} -I\"${${BOARD_CORE}.path}\" -I\"${${CMAKE_SYSTEM_PROCESSOR}_LIBRARIES_PATH}\"") # TODO _PATH
        if (${${CMAKE_SYSTEM_PROCESSOR}_PLATFORM_LIBRARIES_PATH})
            set(COMPILE_FLAGS "${COMPILE_FLAGS} -I\"${${CMAKE_SYSTEM_PROCESSOR}_PLATFORM_LIBRARIES_PATH}\"")
        endif ()
    endif ()
    if (ARDUINO_SDK_VERSION VERSION_GREATER 1.0 OR ARDUINO_SDK_VERSION VERSION_EQUAL 1.0)
        if (NOT IS_MANUAL)
            _get_board_property(${BOARD_ID} build.variant VARIANT)
            set(PIN_HEADER ${${VARIANT}.path}) # should resolve to path but not
            if (PIN_HEADER)
                set(COMPILE_FLAGS "${COMPILE_FLAGS} -I\"${PIN_HEADER}\"")
            endif ()
        endif ()
    endif ()

    set(${COMPILER_FLAGS} "${COMPILE_FLAGS}" PARENT_SCOPE)

endfunction()

# ToDo: Comment
function(set_board_linker_flags LINKER_FLAGS BOARD_ID IS_MANUAL)
    include(${ARDUINO_CMAKE_TOP_FOLDER}/Platform/Core/BoardFlags/LinkerFlagsSetter_${CMAKE_SYSTEM_PROCESSOR}.cmake)
endfunction()

#=============================================================================#
# set_board_flags
# [PRIVATE/INTERNAL]
#
# set_board_flags(COMPILER_FLAGS LINKER_FLAGS BOARD_ID IS_MANUAL)
#
#       COMPILER_FLAGS - Variable holding compiler flags
#       LINKER_FLAGS - Variable holding linker flags
#       BOARD_ID - The board id name
#       IS_MANUAL - (Advanced) Only use AVR Libc/Includes
#
# Configures the build settings for the specified Arduino Board.
#
#=============================================================================#
function(set_board_flags COMPILER_FLAGS LINKER_FLAGS BOARD_ID IS_MANUAL)

    _get_board_property(${BOARD_ID} build.core BOARD_CORE)
    if (BOARD_CORE)
        set_board_compiler_flags(COMPILE_FLAGS ${NORMALIZED_SDK_VERSION} ${BOARD_ID} ${IS_MANUAL})
        set_board_linker_flags(LINK_FLAGS ${BOARD_ID} ${IS_MANUAL})

        # output
        set(${COMPILER_FLAGS} "${COMPILE_FLAGS}" PARENT_SCOPE)
        set(${LINKER_FLAGS} "${LINK_FLAGS}" PARENT_SCOPE)

    else ()
        message(FATAL_ERROR "Invalid Arduino board ID (${BOARD_ID}), aborting.")
    endif ()

endfunction()

