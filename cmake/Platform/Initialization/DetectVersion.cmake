#=============================================================================#
# Detects the Arduino SDK Version based on the revisions.txt file.
# The following variables will be generated:
#
#    ${OUTPUT_VAR_NAME}         -> the full version (major.minor.patch)
#    ${OUTPUT_VAR_NAME}_MAJOR   -> the major version
#    ${OUTPUT_VAR_NAME}_MINOR   -> the minor version
#    ${OUTPUT_VAR_NAME}_PATCH   -> the patch version
#
#=============================================================================#

# _append_suffix_zero_to_version_if_required
# [PRIVATE/INTERNAL]
#
# _append_suffix_zero_to_version_if_required(VERSION_PART VERSION_LIMIT OUTPUT_VAR)
#
#       VERSION_PART - Version to check and possibly append to.
#                 Must be a version part - Major, Minor or Patch.
#       VERSION_LIMIT - Append limit. For a version greater than this number
#                       a zero will NOT be appended.
#       OUTPUT_VAR - Returned variable storing the normalized version.
#
# Appends a suffic zero to the given version part if it's below than the given limit.
# Otherwise, the version part is returned as it is.
#
#=============================================================================#
macro(_append_suffix_zero_to_version_if_required VERSION_PART VERSION_LIMIT OUTPUT_VAR)
    if (${VERSION_PART} LESS ${VERSION_LIMIT})
        set(${OUTPUT_VAR} "${VERSION_PART}0")
    else ()
        set(${OUTPUT_VAR} "${VERSION_PART}")
    endif ()
endmacro()

if (NOT DEFINED ARDUINO_SDK_VERSION)
    find_file(ARDUINO_VERSION_PATH
            NAMES lib/version.txt
            PATHS ${PLATFORM_PATH}
            DOC "Path to Arduino version file.")

    find_file(ARDUINO_REVISION_PATH
            NAMES revisions.txt
            PATHS ${PLATFORM_PATH}
            DOC "Path to Arduino's revision-tracking file.")

    if (ARDUINO_VERSION_PATH)
        file(READ ${ARDUINO_VERSION_PATH} RAW_VERSION)
        if ("${RAW_VERSION}" MATCHES " *[0]+([0-9]+)")
            set(PARSED_VERSION 0.${CMAKE_MATCH_1}.0)
        elseif ("${RAW_VERSION}" MATCHES "[ ]*([0-9]+[.][0-9]+[.][0-9]+)")
            set(PARSED_VERSION ${CMAKE_MATCH_1})
        elseif ("${RAW_VERSION}" MATCHES "[ ]*([0-9]+[.][0-9]+)")
            set(PARSED_VERSION ${CMAKE_MATCH_1}.0)
        endif ()
    elseif (NOT ARDUINO_REVISION_PATH)
        message(WARNING "Couldn't find SDK's revisions file, defaulting to 0.")
    else ()
        file(READ ${ARDUINO_REVISION_PATH} RAW_REVISION 0 30)
        if ("${RAW_REVISION}" MATCHES ".*${PARSED_VERSION}.*[-].*")
            string(REGEX MATCH "[-][ ]?([0-9]+[.][0-9]+[.][0-9]+)"
                    TMP_REV ${RAW_REVISION})
            set(PARSED_REVISION ${CMAKE_MATCH_1})
        else ()
            set(PARSED_REVISION 0)
        endif ()
    endif ()

    set(ARDUINO_SDK_VERSION ${PARSED_VERSION})
endif ()

string(REPLACE "." ";" SPLIT_VERSION ${ARDUINO_SDK_VERSION})
list(GET SPLIT_VERSION 0 SPLIT_VERSION_MAJOR)
list(GET SPLIT_VERSION 1 SPLIT_VERSION_MINOR)
list(GET SPLIT_VERSION 2 SPLIT_VERSION_PATCH)

set(ARDUINO_SDK_VERSION "${PARSED_VERSION}" CACHE STRING "Arduino SDK Version")
string(CONCAT FULL_SDK_VERSION "${PARSED_VERSION}" "-" "${PARSED_REVISION}")

set(ARUDINO_SDK_FULL_VERSION "${FULL_SDK_VERSION}")
set(ARDUINO_SDK_VERSION_MAJOR ${SPLIT_VERSION_MAJOR})
set(ARDUINO_SDK_VERSION_MINOR ${SPLIT_VERSION_MINOR})
set(ARDUINO_SDK_VERSION_PATCH ${SPLIT_VERSION_PATCH})
set(ARDUINO_SDK_VERSION_REVISION ${PARSED_REVISION})

if (ARDUINO_SDK_VERSION VERSION_LESS 0.19)
    message(FATAL_ERROR "Unsupported Arduino SDK (requires version 0.19 or higher)")
endif ()
if (${ARDUINO_SDK_VERSION} VERSION_GREATER 1.5.8)
    # -DARDUINO format has changed since 1.6.0 by appending zeros when required,
    # e.g for 1.6.5 version -DARDUINO=10605
    _append_suffix_zero_to_version_if_required(${ARDUINO_SDK_VERSION_MAJOR} 10 MAJOR_VERSION)
    _append_suffix_zero_to_version_if_required(${ARDUINO_SDK_VERSION_MINOR} 10 MINOR_VERSION)
    set(NORMALIZED_VERSION
            "${MAJOR_VERSION}${MINOR_VERSION}${ARDUINO_SDK_VERSION_PATCH}")
else ()
    # -DARDUINO format before 1.0.0 uses only minor version,
    # e.g. for 0020 version -DARDUINO=20
    if (${ARDUINO_SDK_VERSION} VERSION_LESS 1.0.0)
        set(NORMALIZED_VERSION "${ARDUINO_SDK_VERSION_MINOR}")
    else ()
        # -DARDUINO format after 1.0.0 combines all 3 version parts together,
        # e.g. for 1.5.3 version -DARDUINO=153
        set(NORMALIZED_VERSION
                "${ARDUINO_SDK_VERSION_MAJOR}${ARDUINO_SDK_VERSION_MINOR}${ARDUINO_SDK_VERSION_PATCH}" PARENT_SCOPE)
    endif ()
endif ()
set(NORMALIZED_SDK_VERSION ${NORMALIZED_VERSION} CACHE STRING "SDK version for board setup")
message(STATUS "Arduino SDK version ${ARDUINO_SDK_VERSION} [${NORMALIZED_SDK_VERSION}]")


