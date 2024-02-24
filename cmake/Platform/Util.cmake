#=============================================================================#
# print_list
# [PRIVATE/INTERNAL]
#
# print_list(SETTINGS_LIST)
#
#      SETTINGS_LIST - Variables name of settings list
#
# Print list settings and names (see load_arduino_syle_settings()).
#=============================================================================#
function(print_list SETTINGS_LIST)
    if (${SETTINGS_LIST})
        set(MAX_LENGTH 0)
        foreach (ENTRY_NAME ${${SETTINGS_LIST}})
            string(LENGTH "${ENTRY_NAME}" CURRENT_LENGTH)
            if (CURRENT_LENGTH GREATER MAX_LENGTH)
                set(MAX_LENGTH ${CURRENT_LENGTH})
            endif ()
        endforeach ()
        foreach (ENTRY_NAME ${${SETTINGS_LIST}})
            string(LENGTH "${ENTRY_NAME}" CURRENT_LENGTH)
            math(EXPR PADDING_LENGTH "${MAX_LENGTH}-${CURRENT_LENGTH}")
            set(PADDING "")
            foreach (X RANGE ${PADDING_LENGTH})
                set(PADDING "${PADDING} ")
            endforeach ()
            message(STATUS "   ${PADDING}${ENTRY_NAME}: ${${ENTRY_NAME}.name}")
        endforeach ()
    endif ()
endfunction()

#=============================================================================#
# _check_path_exists_case_sensitive_brute_force
# [PRIVATE/INTERNAL]
#
# _check_path_exists_case_sensitive_brute_force(result_var_ absolute_path_)
#
#        result_var_ - A variable in parent scope that is assigned the result
#                      of the check. The result is TRUE if the file
#                      is found with a case sensitive check, otherwise FALSE.
#        absolute_path_
#                    - The absoute path of a file or directory.
#
# Checks if a path exists in a case sensitive fashion.
# This is the brute force version of the check that is used when
# there are no other means available.
#
# This is necessary because CMake's if(EXISTS ...) is not case sensitive
# on Windows (at least not up to CMake version 3.9.6).
#
# Important: Do not use this function directly but prefer to use
#            _check_path_exists_case_sensitive
#=============================================================================#
function(_check_path_exists_case_sensitive_brute_force result_var_ absolute_path_)
    # We recursively traverse the absolute_path_ from its root and
    # check it any path token cannot be found (early exit)
    string(REPLACE "/" ";" path_tokens "${absolute_path_}")

    list(LENGTH path_tokens n_tokens__)
    math(EXPR n_tokens "${n_tokens__} - 1")
    set(cur_path "/")

    foreach (id RANGE 0 ${n_tokens})
        list(GET path_tokens ${id} cur_token)
        if ("${cur_token}" STREQUAL "")
            continue()
        endif ()
        file(GLOB dir_entries RELATIVE "${cur_path}" "${cur_path}*")
        list(FIND dir_entries "${cur_token}" index)
        if (NOT ${index} GREATER -1)
            message("- ${absolute_path_}")
            set(${result_var_} FALSE PARENT_SCOPE)
            return()
        endif ()
        set(cur_path "${cur_path}${cur_token}/")
    endforeach ()
    message(VERBOSE "+ ${absolute_path_}")
    set(${result_var_} TRUE PARENT_SCOPE)
endfunction()

#=============================================================================#
# _check_path_exists_case_sensitive
# [PRIVATE/INTERNAL]
#
# _check_path_exists_case_sensitive(result_var_ absolute_path_)
#
#        result_var_ - A variable in parent scope that is assigned the result
#                      of the check. The result is TRUE if the file
#                      is found with a case sensitive check, otherwise FALSE.
#        absolute_path_
#                    - The absoute path of a file or directory.
#
# Checks if a path exists in a case sensitive fashion.
#
# This is necessary because CMake's if(EXISTS ...) is not case sensitive
# on Windows (at least not up to CMake version 3.9.6).
#=============================================================================#
function(_check_path_exists_case_sensitive result_var_ absolute_path_)
    # Important: First check for APPLE as CMAKE_HOST_UNIX reports true even
    #            if CMAKE_HOST_APPLE is true
    #
    if (CMAKE_HOST_APPLE)
        # On MacOS there is no appropriate command that enables a safe check
        # for a path to exist given a case sensitive name. This is because
        # MacOS file systems are mostly case insensitive by default.
        # That's why we fall back to brute force directory comparison.
        #
        _check_path_exists_case_sensitive_brute_force(tmp_result "${absolute_path_}")
        set(${result_var_} ${tmp_result} PARENT_SCOPE)
        return()
    elseif (CMAKE_HOST_UNIX)
        if (EXISTS "${absolute_path_}")
            set(${result_var_} TRUE PARENT_SCOPE)
        else ()
            set(${result_var_} FALSE PARENT_SCOPE)
        endif ()
        return()
    elseif (CMAKE_HOST_WIN32)
        # From this point on, we only deal with Windows
        file(TO_NATIVE_PATH "${absolute_path_}" native_path)
        string(REPLACE "/" "\\" native_path "${native_path}")
        # Check what is found when a globbing expression is used to
        # find the file. The output (actual_path) will be the case
        # sensitive path name or nothing.
        #
        execute_process(
                ERROR_QUIET
                OUTPUT_VARIABLE actual_path
                OUTPUT_STRIP_TRAILING_WHITESPACE
                COMMAND cmd /C dir /S /B "${native_path}"
        )
        if ("${actual_path}" STREQUAL "${native_path}")
            set(${result_var_} TRUE PARENT_SCOPE)
        else ()
            set(${result_var_} FALSE PARENT_SCOPE)
        endif ()
        return()
    endif ()
    message(FATAL_ERROR "Strange host system")
endfunction()
#=============================================================================#
# arduino_debug_msg
# [PRIVATE/INTERNAL]
#
# arduino_debug_msg(MSG)
#
#        MSG - Message to print
#
# Print Arduino debugging information. In order to enable printing
# use arduino_debug_on() and to disable use arduino_debug_off().
#=============================================================================#
function(arduino_debug_msg MSG)
    if (ARDUINO_DEBUG)
        message("## ${MSG}")
    endif ()
endfunction()
#=============================================================================#
# _fix_redundant_target_compile_flags
# [PRIVATE/INTERNAL]
#
# _fix_redundant_target_compile_flags(TARGET)
#
#       TARGET - A CMake target
#
# Removes any redundantly defined entries from TARGET's COMPILE_FLAGS
# property.
# This is a fix that is necessary as the collection of include directories
# during establishing targets is partially redundant, causing
# many redundant -I... entries to appear in compiler command lines.
#=============================================================================#
function(_fix_redundant_target_compile_flags target_)
    # Get the current compile flags of a target
    #
    get_target_property(compile_flags ${target_} COMPILE_FLAGS)
    # Turn the space separated list of compile flags into a semicolon
    # separated list. Also consider single quotes arguments.
    #
    string(REGEX REPLACE " +('*-)" ";\\1" new_compile_flags "${compile_flags}")
    # Remove any duplicate flags (includes directory specifications, ...)
    #
    list(REMOVE_DUPLICATES new_compile_flags)
    # Convert the semicolon separated list back into a space separated one
    #
    string(REGEX REPLACE ";('*-)" " \\1" new_compile_flags "${new_compile_flags}")
    # Replace the bad (redundant) compile flags
    #
    set_target_properties(${target_} PROPERTIES COMPILE_FLAGS "${new_compile_flags}")
endfunction()
#=============================================================================#
# load_generator_settings
# [PRIVATE/INTERNAL]
#
# load_generator_settings(TARGET_NAME PREFIX [SUFFIX_1 SUFFIX_2 .. SUFFIX_N])
#
#         TARGET_NAME - The base name of the user settings
#         PREFIX      - The prefix name used for generator settings
#         SUFFIX_XX   - List of suffixes to load
#
#  Loads a list of user settings into the generators scope. User settings have
#  the following syntax:
#
#      ${BASE_NAME}${SUFFIX}
#
#  The BASE_NAME is the target name and the suffix is a specific generator settings.
#
#  For every user setting found a generator setting is created of the follwoing fromat:
#
#      ${PREFIX}${SUFFIX}
#
#  The purpose of loading the settings into the generator is to not modify user settings
#  and to have a generic naming of the settings within the generator.
#
#=============================================================================#
function(load_generator_settings TARGET_NAME PREFIX)
    foreach (GEN_SUFFIX ${ARGN})
        if (${TARGET_NAME}_${GEN_SUFFIX} AND NOT ${PREFIX}_${GEN_SUFFIX})
            set(${PREFIX}_${GEN_SUFFIX} ${${TARGET_NAME}_${GEN_SUFFIX}} PARENT_SCOPE)
        endif ()
    endforeach ()
endfunction()
#=============================================================================#
# parse_generator_arguments
# [PRIVATE/INTERNAL]
#
# parse_generator_arguments(TARGET_NAME PREFIX OPTIONS ARGS MULTI_ARGS [ARG1 ARG2 .. ARGN])
#
#         PREFIX     - Parsed options prefix
#         OPTIONS    - List of options
#         ARGS       - List of one value keyword arguments
#         MULTI_ARGS - List of multi value keyword arguments
#         [ARG1 ARG2 .. ARGN] - command arguments [optional]
#
# Parses generator options from either variables or command arguments
#
#=============================================================================#
macro(parse_generator_arguments TARGET_NAME PREFIX OPTIONS ARGS MULTI_ARGS)
    cmake_parse_arguments(${PREFIX} "${OPTIONS}" "${ARGS}" "${MULTI_ARGS}" ${ARGN})
    error_for_unparsed(${PREFIX})
    load_generator_settings(${TARGET_NAME} ${PREFIX} ${OPTIONS} ${ARGS} ${MULTI_ARGS})
endmacro()
#=============================================================================#
# get_mcu
# [PRIVATE/INTERNAL]
#
# get_mcu(FULL_MCU_NAME, OUTPUT_VAR)
#
#         FULL_MCU_NAME - Board's full mcu name, including a trailing 'p' if present
#         OUTPUT_VAR - String value in which a regex match will be stored
#
# Matches the board's mcu without leading or trailing characters that would rather mess
# further processing that requires the board's mcu.
#
#=============================================================================#
macro(GET_MCU FULL_MCU_NAME OUTPUT_VAR)
    string(REGEX MATCH "^.+[^p]" ${OUTPUT_VAR} "FULL_MCU_NAME" PARENT_SCOPE)
endmacro()
#=============================================================================#
# increment_example_category_index
# [PRIVATE/INTERNAL]
#
# increment_example_category_index(OUTPUT_VAR)
#
#         OUTPUT_VAR - A number representing an example's category prefix
#
# Increments the given number by one, taking into consideration the number notation
# which is defined (Some SDK's or OSs use a leading '0' in single-digit numbers.
#
#=============================================================================#
macro(increment_example_category_index OUTPUT_VAR)
    math(EXPR INC_INDEX "${${OUTPUT_VAR}}+1")
    if (EXAMPLE_CATEGORY_INDEX_LENGTH GREATER 1 AND INC_INDEX LESS 10)
        set(${OUTPUT_VAR} "0${INC_INDEX}")
    else ()
        set(${OUTPUT_VAR} ${INC_INDEX})
    endif ()
endmacro()







