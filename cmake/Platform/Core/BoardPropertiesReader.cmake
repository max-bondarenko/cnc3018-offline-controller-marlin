#=============================================================================#
# print_board_list
# [PUBLIC/USER]
#
# print_board_list()
#
# see documentation at top
#=============================================================================#
function(print_board_list)
    load_platform_settings()
    if (${CMAKE_SYSTEM_PROCESSOR}_BOARDS)
        message(STATUS "${CMAKE_SYSTEM_PROCESSOR} Boards:")
        print_list(${CMAKE_SYSTEM_PROCESSOR}_BOARDS)
        message(STATUS "")
    endif ()
endfunction()
#=============================================================================#
# print_programmer_list
# [PUBLIC/USER]
#
# print_programmer_list()
#
# see documentation at top
#=============================================================================#
function(print_programmer_list)
    load_platform_settings()
    foreach (PLATFORM ${${CMAKE_SYSTEM_PROCESSOR}_PLATFORMS})
        if (${CMAKE_SYSTEM_PROCESSOR}_PROGRAMMERS)
            message(STATUS "${CMAKE_SYSTEM_PROCESSOR} Programmers:")
            print_list(${CMAKE_SYSTEM_PROCESSOR}_PROGRAMMERS)
        endif ()
        message(STATUS "")
    endforeach ()
endfunction()
#=============================================================================#
# print_programmer_settings
# [PUBLIC/USER]
#
# print_programmer_settings(PROGRAMMER)
#
# see documentation at top
#=============================================================================#
function(print_programmer_settings PROGRAMMER)
    load_platform_settings()
    if (${PROGRAMMER}.SETTINGS)
        message(STATUS "Programmer ${PROGRAMMER} Settings:")
        print_settings(${PROGRAMMER})
    endif ()
endfunction()
#=============================================================================#
# print_board_settings
# [PUBLIC/USER]
#
# print_board_settings(ARDUINO_BOARD)
#
# see documentation at top
function(print_board_settings _BOARD)
    load_platform_settings()
    if (${_BOARD}.SETTINGS)
        message(STATUS "Arduino ${_BOARD} Board:")
        print_settings(${_BOARD})
    endif ()
endfunction()

function(load_platform_settings)
    if (NOT CORES)
        file(GLOB sub-dir ${${CMAKE_SYSTEM_PROCESSOR}_CORES_PATH}/*)
        #        get_filename_component(BASE_PATH ${${CMAKE_SYSTEM_PROCESSOR}_CORES_PATH} PATH)
        #        get_filename_component(VENDOR_ID ${${CMAKE_SYSTEM_PROCESSOR}_CORES_PATH} NAME)
        set(VENDOR_ID "arduino")
        foreach (dir ${sub-dir})
            if (IS_DIRECTORY ${dir})
                get_filename_component(core ${dir} NAME)
                set(CORES ${CORES} ${core})
                set(${core}.path ${dir} CACHE INTERNAL "The path to the core ${core}")
                # See https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5-3rd-party-Hardware-specification#referencing-another-core-variant-or-tool
                # for an explanation why cores must also be available as <vendor_id>:<core_id>
                # and <vendor_id>:<architecture_id>:<core_id>
                set(CORES ${CORES} "${VENDOR_ID}:${core}")
                set(${VENDOR_ID}:${core}.path ${dir} CACHE INTERNAL "The path to the core ${core}")
                set(CORES ${CORES} "${VENDOR_ID}:${CMAKE_SYSTEM_PROCESSOR}:${core}")
                message(VERBOSE "Registered core [" ${VENDOR_ID}:${CMAKE_SYSTEM_PROCESSOR}:${core}"]")
            endif ()
            list(SORT CORES)
            set(CORES ${CORES} CACHE INTERNAL "A list of registered cores")
        endforeach ()
    endif ()
    if (NOT VARIANTS)
        file(GLOB sub-dir ${${CMAKE_SYSTEM_PROCESSOR}_VARIANTS_PATH}/*)
        foreach (dir ${sub-dir})
            if (IS_DIRECTORY ${dir})
                get_filename_component(variant ${dir} NAME)
                set(VARIANTS ${VARIANTS} ${variant})
                set(${variant}.path ${dir} CACHE INTERNAL "The path to the variant ${variant}")
                message(VERBOSE "Registered variants ${${variant}.path}")
            endif ()
        endforeach ()
        list(SORT VARIANTS)
        set(VARIANTS ${VARIANTS} CACHE INTERNAL "A list of registered variant boards")
    endif ()
    if (NOT ${CMAKE_SYSTEM_PROCESSOR}_PLATFORM)
        set(SETTINGS_LIST ${CMAKE_SYSTEM_PROCESSOR}_PLATFORM)
        set(SETTINGS_PATH "${${CMAKE_SYSTEM_PROCESSOR}_PLATFORM_FILE_PATH}")
        include(LoadPlatformSettings)
    endif ()
    if (NOT ${CMAKE_SYSTEM_PROCESSOR}_BOARDS)
        set(SETTINGS_LIST ${CMAKE_SYSTEM_PROCESSOR}_BOARDS)
        set(SETTINGS_PATH "${${CMAKE_SYSTEM_PROCESSOR}_BOARDS_PATH}")
        include(LoadPlatformSettings)
    endif ()
    if (NOT ${CMAKE_SYSTEM_PROCESSOR}_PROGRAMMERS)
        set(SETTINGS_LIST ${CMAKE_SYSTEM_PROCESSOR}_PROGRAMMERS)
        set(SETTINGS_PATH "${${CMAKE_SYSTEM_PROCESSOR}_PROGRAMMERS_PATH}")
        include(LoadPlatformSettings)
    endif ()
endfunction()
#=============================================================================#
# print_settings
# [PRIVATE/INTERNAL]
#
# print_settings(ENTRY_NAME)
#
#      ENTRY_NAME - name of entry
#
# Print the entry settings (see load_arduino_syle_settings()).
#
#=============================================================================#
function(print_settings ENTRY_NAME) # TODO fix parser for STM32
    if (${ENTRY_NAME}.SETTINGS)
        foreach (ENTRY_SETTING ${${ENTRY_NAME}.SETTINGS})
            if (${ENTRY_NAME}.${ENTRY_SETTING})
                message(STATUS "   ${ENTRY_NAME}.${ENTRY_SETTING}=${${ENTRY_NAME}.${ENTRY_SETTING}}")
            endif ()
            if (${ENTRY_NAME}.${ENTRY_SETTING}.SUBSETTINGS)
                foreach (ENTRY_SUBSETTING ${${ENTRY_NAME}.${ENTRY_SETTING}.SUBSETTINGS})
                    if (${ENTRY_NAME}.${ENTRY_SETTING}.${ENTRY_SUBSETTING})
                        message(STATUS "   ${ENTRY_NAME}.${ENTRY_SETTING}.${ENTRY_SUBSETTING}=${${ENTRY_NAME}.${ENTRY_SETTING}.${ENTRY_SUBSETTING}}")
                    endif ()
                endforeach ()
            endif ()
            message(STATUS "")
        endforeach ()
    endif ()
endfunction()
#=============================================================================#
# _get_board_id
# [PRIVATE/INTERNAL]
#
# _get_board_id(BOARD_NAME BOARD_CPU TARGET_NAME OUTPUT_VAR)
#
#        BOARD_NAME - name of the board, eg.: nano, uno, etc...
#        BOARD_CPU - some boards have multiple versions with different cpus, eg.: nano has atmega168 and atmega328
#        TARGET_NAME - name of the build target, used to show clearer error message
#        OUT_VAR - BOARD_ID constructed from BOARD_NAME and BOARD_CPU
#
# returns BOARD_ID constructing from BOARD_NAME and BOARD_CPU, if board doesn't has multiple cpus then BOARD_ID = BOARD_NAME
# if board has multiple CPUS, and BOARD_CPU is not defined or incorrect, fatal error will be invoked.
#=============================================================================#
function(_get_board_id BOARD_NAME BOARD_CPU TARGET_NAME OUTPUT_VAR)
    load_platform_settings()
    if (${BOARD_NAME}.menu.CPUS)
        if (BOARD_CPU)
            list(FIND ${BOARD_NAME}.menu.CPUS ${BOARD_CPU} CPU_INDEX)
            if (CPU_INDEX EQUAL -1)
                message(FATAL_ERROR "Invalid BOARD_CPU (valid cpus: ${${BOARD_NAME}.menu.CPUS}).")
            endif ()
        else ()
            message(FATAL_ERROR "Board has multiple CPU versions (${${BOARD_NAME}.menu.CPUS}). BOARD_CPU must be defined for target ${TARGET_NAME}.")
        endif ()
        set(${OUTPUT_VAR} ${BOARD_NAME}.${BOARD_CPU} PARENT_SCOPE)
    else ()
        set(${OUTPUT_VAR} ${BOARD_NAME} PARENT_SCOPE)
    endif ()
endfunction()
#=============================================================================#
# _recursively_replace_properties
# [PRIVATE/INTERNAL]
#
# _recursively_replace_properties(BOARD_ID PROPERTY_VALUE_VAR)
#        BOARD_ID - return value from function "_get_board_id (BOARD_NAME, BOARD_CPU)".
#            It contains BOARD_NAME and BOARD_CPU
#        PROPERTY_VALUE_VAR - the value of a property that may contain
#           references to other properties.
#
# Recursively replaces references to other properties.
#
#=============================================================================#
function(_recursively_replace_properties BOARD_ID PROPERTY_VALUE_VAR)
    # The following regular expressions looks for arduino property references
    # that are {property_name} the [^\$] part is just there to ensure that
    # something like ${foo} is not matched as it could be a shell variable
    # or a cmake variable or whatever, but not a Arduino property.
    #
    while ("${${PROPERTY_VALUE_VAR}}" MATCHES "(^|[^\$]){([^}]*)}")

        set(variable "${CMAKE_MATCH_2}")

        # The following regular expression checks if the property (variable)
        # that was referenced is one of a board.
        #
        _try_get_board_property("${BOARD_ID}" "${variable}" repl_string)

        if ("${repl_string}" STREQUAL "")
            if (NOT "${${variable}}" STREQUAL "")
                # If it's not a board property, we try to find the variable
                # at global scope.
                #
                set(repl_string "${${variable}}")
            elseif (ARDUINO_CMAKE_ERROR_ON_UNDEFINED_PROPERTIES)
                message(SEND_ERROR "Variable ${variable} used in board property definition is undefined.")
                message(FATAL_ERROR "Property definition: ${${PROPERTY_VALUE_VAR}}")
            endif ()
        endif ()

        string(REGEX REPLACE "{${variable}}" "${repl_string}" ${PROPERTY_VALUE_VAR} "${${PROPERTY_VALUE_VAR}}")
    endwhile ()
    set(${PROPERTY_VALUE_VAR} "${${PROPERTY_VALUE_VAR}}" PARENT_SCOPE)
endfunction()

#=============================================================================#
# _get_board_property
# [PRIVATE/INTERNAL]
#
# _get_board_property(BOARD_ID PROPERTY_NAME OUTPUT_VAR)
#
#        BOARD_ID - return value from function "_get_board_id (BOARD_NAME, BOARD_CPU)". It contains BOARD_NAME and BOARD_CPU
#        PROPERTY_NAME - property name for the board, eg.: bootloader.high_fuses
#        OUT_VAR - variable holding value for the property
#
# Gets board property.
# Reconstructs BOARD_NAME and BOARD_CPU from BOARD_ID and tries to find value at ${BOARD_NAME}.${PROPERTY_NAME},
# if not found than try to find value at ${BOARD_NAME}.menu.cpu.${BOARD_CPU}.${PROPERTY_NAME}
# if not found that show fatal error
#=============================================================================#
function(_get_board_property BOARD_ID PROPERTY_NAME OUTPUT_VAR)
    load_platform_settings()
    string(REPLACE "." ";" BOARD_INFO ${BOARD_ID})
    list(GET BOARD_INFO 0 BOARD_NAME)
    set(VALUE ${${BOARD_NAME}.${PROPERTY_NAME}})
    if (NOT VALUE)
        list(LENGTH BOARD_INFO INFO_PARAMS_COUNT)
        if (${INFO_PARAMS_COUNT} EQUAL 2)
            list(GET BOARD_INFO 1 BOARD_CPU)
            validate_variables_not_empty(VARS BOARD_CPU MSG "cannot find CPU info, must define BOARD_CPU.")
            set(VALUE ${${BOARD_NAME}.menu.cpu.${BOARD_CPU}.${PROPERTY_NAME}})
        endif ()
    endif ()
    if ((NOT VALUE) AND ${PROPERTY_NAME})
        set(VALUE "${${PROPERTY_NAME}}")
    endif ()
    if (NOT VALUE)
        message(FATAL_ERROR "Board info not found: BoardName='${BOARD_NAME}' BoardCPU='${BOARD_CPU}' PropertyName='${PROPERTY_NAME}'")
    endif ()
    _recursively_replace_properties(${BOARD_ID} VALUE)
    set(${OUTPUT_VAR} ${VALUE} PARENT_SCOPE)
endfunction()

#=============================================================================#
# _try_get_board_property
# [PRIVATE/INTERNAL]
#
# _get_board_property_if_exists(BOARD_ID PROPERTY_NAME OUTPUT_VAR)
#
#        BOARD_ID - return value from function "_get_board_id (BOARD_NAME, BOARD_CPU)". It contains BOARD_NAME and BOARD_CPU
#        PROPERTY_NAME - property name for the board, eg.: bootloader.high_fuses
#        OUT_VAR - variable holding value for the property
#
# Similar to _get_board_property, except it returns empty value if value was not found.
#=============================================================================#
function(_try_get_board_property BOARD_ID PROPERTY_NAME OUTPUT_VAR)
    load_platform_settings()
    string(REPLACE "." ";" BOARD_INFO ${BOARD_ID})
    list(GET BOARD_INFO 0 BOARD_NAME)
    set(VALUE ${${BOARD_NAME}.${PROPERTY_NAME}})

    if (NOT VALUE)
        list(LENGTH BOARD_INFO INFO_PARAMS_COUNT)
        if (${INFO_PARAMS_COUNT} EQUAL 2)
            list(GET BOARD_INFO 1 BOARD_CPU)
            validate_variables_not_empty(VARS BOARD_CPU MSG "cannot find CPU info, must define BOARD_CPU.")
            set(VALUE ${${BOARD_NAME}.menu.cpu.${BOARD_CPU}.${PROPERTY_NAME}})
        endif ()
    endif ()

    if ((NOT VALUE) AND ${PROPERTY_NAME})
        set(VALUE "${${PROPERTY_NAME}}")
    endif ()
    _recursively_replace_properties(${BOARD_ID} VALUE)
    set(${OUTPUT_VAR} ${VALUE} PARENT_SCOPE)
endfunction()
