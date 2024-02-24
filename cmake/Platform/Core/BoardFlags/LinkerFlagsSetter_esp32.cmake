set(LINK_FLAGS "${LINK_FLAGS}")
if (ARDUINO_USE_NEWLIB) # smaller
    set(LINK_FLAGS "${LINK_FLAGS} ")
endif (ARDUINO_USE_NEWLIB) # smaller
#-Wl,--warn-unresolved-symbols -lstdc++")
set(${LINKER_FLAGS} "${LINK_FLAGS}" PARENT_SCOPE)
