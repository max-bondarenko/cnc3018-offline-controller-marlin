# ToDo: Comment
function(set_board_linker_flags LINKER_FLAGS BOARD_ID IS_MANUAL)
    if (NOT DEFINED ARDUINO_MCU_FLAGS)
        SET(ARDUINO_MCU_FLAGS "-mcpu=cortex-m3 -mthumb")
    endif ()
    set(LINK_FLAGS "${LINK_FLAGS} -Os  -Wl,--gc-sections ${ARDUINO_MCU_FLAGS}  -Xlinker -print-memory-usage ")
    set(LINK_FLAGS "${LINK_FLAGS} -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -lstdc++")
    if (ARDUINO_USE_NEWLIB) # smaller
        set(LINK_FLAGS "${LINK_FLAGS} --specs=nano.specs") # -u _printf_float")
        if (ARDUINO_USE_FLOAT_PRINTF)
            set(LINK_FLAGS "${LINK_FLAGS}  -u _printf_float")
        endif (ARDUINO_USE_FLOAT_PRINTF)
    endif (ARDUINO_USE_NEWLIB) # smaller

    #-Wl,--warn-unresolved-symbols -lstdc++")
    set(${LINKER_FLAGS} "${LINK_FLAGS}" PARENT_SCOPE)
endfunction()
