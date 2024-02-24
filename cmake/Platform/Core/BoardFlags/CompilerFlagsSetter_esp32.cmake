#_get_board_property(${BOARD_ID} speed_72mhz.build.f_cpu FCPU) # Sane default...
#_get_board_property(${BOARD_ID} menu.cpu.speed_72mhz.build.f_cpu FCPU)
#
#_try_get_board_property(${BOARD_ID} menu.cpu.${ARDUINO_CPU}.build.cpu_flags TRY_CPU_FLAGS)
#IF(TRY_CPU_FLAGS)
    #SET(CPU_FLAGS ${TRY_CPU_FLAGS})
#ELSE(TRY_CPU_FLAGS)
    #_get_board_property(${BOARD_ID} build.cpu_flags CPU_FLAGS)
#ENDIF(TRY_CPU_FLAGS)

#set(COMPILE_FLAGS "-DF_CPU=${FCPU} ${CPU_FLAGS} -DARDUINO=${NORMALIZED_SDK_VERSION} ")
set(COMPILE_FLAGS " -DARDUINO=${NORMALIZED_SDK_VERSION} ")
# This should be derived from the arduino config files
# hardcode them for the moment


#
macro (DBG)
#    MESSAGE(STATUS ${ARGN})
endmacro (DBG)


macro (ADD_IF_DEFINED  key)
dbg ("Checking ${key}")
    if( DEFINED ${BOARD_ID}.${key})
dbg ("Yes ${key}: ${${BOARD_ID}.${key}}")
set (COMPILE_FLAGS  "${COMPILE_FLAGS} ${${BOARD_ID}.${key}}")
    endif( DEFINED ${BOARD_ID}.${key})
endmacro (ADD_IF_DEFINED  key)

macro (SET_IF_DEFINED  key out)
dbg ("Checking ${key}")
    if( DEFINED ${BOARD_ID}.${key})
dbg ("Yes  ${key}=>${${BOARD_ID}.${key}} ")
set (${out}  "${${BOARD_ID}.${key}} ")
    endif( DEFINED ${BOARD_ID}.${key})
endmacro (SET_IF_DEFINED  key out)


macro (ADD_TO_COMPILE_FLAGS key prefix)
dbg ("Checking ${key}")
    if( DEFINED ${BOARD_ID}.${key})
dbg ("Yes ${key} -D${prefix}${${BOARD_ID}.${key}} ")
set (COMPILE_FLAGS "${COMPILE_FLAGS} -D${prefix}${${BOARD_ID}.${key}} ")
    endif( DEFINED ${BOARD_ID}.${key})
endmacro (ADD_TO_COMPILE_FLAGS key prefix)
#
set (ESP32_SYSTEM_ROOT "${PLATFORM_PATH}")
#
#
macro (ADD_RELATIVE_IPATH ipath)
    set(COMPILE_FLAGS "${COMPILE_FLAGS} -I${ESP32_SYSTEM_ROOT}/${ipath}") # Hack, there is a better way to get the system path
endmacro (ADD_RELATIVE_IPATH ipath)
macro (ADD_IPATH path)
add_relative_ipath (tools/sdk/include/${path})
endmacro (ADD_IPATH path)
#
#
add_relative_ipath (libraries/SPI/src/)
add_relative_ipath (libraries/FS/src/)
add_relative_ipath (libraries/SPIFFS/src/)
add_ipath (config)
add_ipath (app_trace)
add_ipath (app_update)
add_ipath (asio)
add_ipath (bootloader_support)
add_ipath (bt)
add_ipath (coap)
add_ipath (console)
add_ipath (driver)
add_ipath (esp-tls)
add_ipath (esp32)
add_ipath (esp_adc_cal)
add_ipath (esp_event)
add_ipath (esp_http_client)
add_ipath (esp_http_server)
add_ipath (esp_https_ota)
add_ipath (esp_ringbuf)
add_ipath (ethernet)
add_ipath (expat)
add_ipath (fatfs)
add_ipath (freemodbus)
add_ipath (freertos)
add_ipath (heap)
add_ipath (idf_test)
add_ipath (jsmn)
add_ipath (json)
add_ipath (libsodium)
add_ipath (log)
add_ipath (lwip)
add_ipath (mbedtls)
add_ipath (mdns)
add_ipath (micro-ecc)
add_ipath (mqtt)
add_ipath (newlib)
add_ipath (nghttp)
add_ipath (nvs_flash)
add_ipath (openssl)
add_ipath (protobuf-c)
add_ipath (protocomm)
add_ipath (pthread)
add_ipath (sdmmc)
add_ipath (smartconfig_ack)
add_ipath (soc)
add_ipath (spi_flash)
add_ipath (spiffs)
add_ipath (tcp_transport)
add_ipath (tcpip_adapter)
add_ipath (ulp)
add_ipath (vfs)
add_ipath (wear_levelling)
add_ipath (wifi_provisioning)
add_ipath (wpa_supplicant)
add_ipath (xtensa-debug-module)
add_ipath (esp-face)
add_ipath (esp32-camera)
add_ipath (esp-face)
add_ipath (freertos)

# VARIANT
set(COMPILE_FLAGS "-I${COMPILE_FLAGS} -I${ARDUINO_VARIANTS_PATH}/${ARDUINO_DEFAULT_BOARD}") # Hack, there is a better way to get the system path + it is wrong

# Arduino
set(COMPILE_FLAGS "-I${COMPILE_FLAGS} -I${ESP32_SYSTEM_ROOT}") # Hack, there is a better way to get the system path + it is wrong

#ADD_TO_COMPILE_FLAGS(build.vect   "")
#ADD_TO_COMPILE_FLAGS(menu.cpu.${ARDUINO_UPLOAD_METHOD}Method.build.vect  "")

# upload flags if any
#ADD_IF_DEFINED(menu.cpu.${ARDUINO_UPLOAD_METHOD}Method.build.upload_flags)
#ADD_IF_DEFINED(menu.cpu.${ARDUINO_CPU}.build.cpu_flags)

#ADD_TO_COMPILE_FLAGS(build.error_led_port  "ERROR_LED_PORT=")
#ADD_TO_COMPILE_FLAGS(build.error_led_pin  "ERROR_LED_PIN=")
#ADD_TO_COMPILE_FLAGS(build.board  "ARDUINO_")
#ADD_TO_COMPILE_FLAGS(build.variant  "BOARD_")


dbg ("Final Compile flags = ${COMPILE_FLAGS}")
#
#set(COMPILE_FLAGS "${COMPILE_FLAGS} -std=gnu11 -MMD -DDEBUG_LEVEL=DEBUG_NONE ")


