_get_board_property(${BOARD_ID} build.mcu MCU)
set(LINK_FLAGS "-mmcu=${MCU}")
set(${LINKER_FLAGS} "${LINK_FLAGS}" PARENT_SCOPE)
