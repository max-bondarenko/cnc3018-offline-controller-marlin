function(create_arduino_bootloader_upload_target TARGET_NAME BOARD_ID PORT AVRDUDE_FLAGS)
    cmake_language(CALL create_arduino_bootloader_upload_target_${CMAKE_SYSTEM_PROCESSOR} TARGET_NAME BOARD_ID PORT AVRDUDE_FLAGS)
endfunction()

#=============================================================================#
# setup_arduino_bootloader_upload for avr/arduino
# [PRIVATE/INTERNAL]
#
# setup_arduino_bootloader_upload(TARGET_NAME BOARD_ID PORT)
#
#      TARGET_NAME - target name
#      BOARD_ID    - board id
#      PORT        - serial port
#      AVRDUDE_FLAGS - avrdude flags (override)
#
# Set up target for upload firmware via the bootloader.
#
# The target for uploading the firmware is ${TARGET_NAME}-upload .
#
#=============================================================================#
function(create_arduino_bootloader_upload_target_avr TARGET_NAME BOARD_ID PORT AVRDUDE_FLAGS)
    set(UPLOAD_TARGET ${TARGET_NAME}-upload)
    set(AVRDUDE_ARGS)

    build_arduino_bootloader_arguments(${BOARD_ID} ${TARGET_NAME} ${PORT} "${AVRDUDE_FLAGS}" AVRDUDE_ARGS)

    if (NOT AVRDUDE_ARGS)
        message("Could not generate default avrdude bootloader args, aborting!")
        return()
    endif ()

    if (NOT EXECUTABLE_OUTPUT_PATH)
        set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
    endif ()
    set(TARGET_PATH ${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME})

    list(APPEND AVRDUDE_ARGS "-Uflash:w:\"${TARGET_PATH}.hex\":i")
    list(APPEND AVRDUDE_ARGS "-Ueeprom:w:\"${TARGET_PATH}.eep\":i")
    add_custom_target(${UPLOAD_TARGET}
            ${ARDUINO_AVRDUDE_PROGRAM}
            ${AVRDUDE_ARGS}
            DEPENDS ${TARGET_NAME})

    # Global upload target
    if (NOT TARGET upload)
        add_custom_target(upload)
    endif ()

    add_dependencies(upload ${UPLOAD_TARGET})
endfunction()


#=============================================================================#
# setup_arduino_bootloader_upload esp32
# [PRIVATE/INTERNAL]
#
# setup_arduino_bootloader_upload(TARGET_NAME BOARD_ID PORT)
#
#      TARGET_NAME - target name
#      BOARD_ID    - board id
#      PORT        - serial port
#      AVRDUDE_FLAGS - avrdude flags (override)
#
# Set up target for upload firmware via the bootloader.
#
# The target for uploading the firmware is ${TARGET_NAME}-upload .
#
#=============================================================================#
function(create_arduino_bootloader_upload_target_esp32 TARGET_NAME BOARD_ID PORT AVRDUDE_FLAGS)
    set(UPLOAD_TARGET ${TARGET_NAME}-upload)
    add_custom_target(${UPLOAD_TARGET}
            ${PLATFORM_PATH}/tools/esptool/esptool.py --chip esp32 --port "/dev/${ARDUINO_DEFAULT_PORT}" --baud ${ESP32_USB_UPLOAD_SPEED} --before default_reset --after hard_reset write_flash -z --flash_mode ${ESP32_FLASH_MODE} --flash_freq ${ESP32_FLASH_SPEED} --flash_size detect 0xe000 "${PLATFORM_PATH}/tools/partitions/boot_app0.bin" 0x1000 "${PLATFORM_PATH}/tools/sdk/bin/bootloader_${ESP32_FLASH_MODE_BOOTLOADER}_${ESP32_FLASH_SPEED}.bin" 0x10000 ${TARGET_NAME}.img.bin 0x8000 "${TARGET_NAME}.partitions.bin"
            DEPENDS ${TARGET_NAME})

    # Global upload target
    if (NOT TARGET upload)
        add_custom_target(upload)
    endif ()

    add_dependencies(upload ${UPLOAD_TARGET})
endfunction()


#=============================================================================#
# setup_arduino_bootloader_upload
# [PRIVATE/INTERNAL]
#
# setup_arduino_bootloader_upload(TARGET_NAME BOARD_ID PORT)
#
#      TARGET_NAME - target name
#      BOARD_ID    - board id
#      PORT        - serial port
#      AVRDUDE_FLAGS - avrdude flags (override)
#
# Set up target for upload firmware via the bootloader.
#
# The target for uploading the firmware is ${TARGET_NAME}-upload .
#
#=============================================================================#
function(create_arduino_bootloader_upload_target_stm32 TARGET_NAME BOARD_ID PORT AVRDUDE_FLAGS)
    set(UPLOAD_TARGET ${TARGET_NAME}-upload)
    set(AVRDUDE_ARGS)

    build_arduino_bootloader_arguments(${BOARD_ID} ${TARGET_NAME} ${PORT} "${AVRDUDE_FLAGS}" AVRDUDE_ARGS)
    if (NOT EXECUTABLE_OUTPUT_PATH)
        set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
    endif ()
    set(TARGET_PATH ${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME})

    if (ARDUINO_USB_PID)
        set(USB_PID "${ARDUINO_USB_PID}")
    else (ARDUINO_USB_PID)
        set(USB_PID "0004") # Default value
    endif (ARDUINO_USB_PID)
    if (NOT DEFINED (MAPLE_UPLOAD))
        set(MAPLE_UPLOAD ${ORIGINAL_PLATFORM_PATH}/tools/linux/maple_upload CACHE INTERNAL "")
    endif (NOT DEFINED (MAPLE_UPLOAD))

    add_custom_target(${UPLOAD_TARGET}
            ${MAPLE_UPLOAD}
            ${PORT} 2 1EAF:${USB_PID}
            ${TARGET_PATH}.bin
            DEPENDS ${TARGET_NAME})

    # Global upload target
    if (NOT TARGET upload)
        add_custom_target(upload)
    endif ()

    add_dependencies(upload ${UPLOAD_TARGET})
endfunction()



