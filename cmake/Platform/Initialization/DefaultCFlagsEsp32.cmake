# -fno-rtti 
# -fno-exceptions
# -x c++
if (NOT DEFINED ARDUINO_UPLOAD_METHOD)
set (ARDUINO_UPLOAD_METHOD "DFUUpload")
endif (NOT DEFINED ARDUINO_UPLOAD_METHOD)

if (NOT DEFINED ARDUINO_CFLAGS_SET)
set (DEFINED ARDUINO_CFLAGS_SET 1)
set (COMMON_FLAGS "-fstack-protector -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib -Wpointer-arith  -Wno-maybe-uninitialized -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-variable -Wno-deprecated-declarations -Wno-unused-parameter -Wno-sign-compare ")
set (COMMON_FLAGS " ${COMMON_FLAGS}  -DF_CPU=240000000L  -DARDUINO_ESP32_DEV -DARDUINO_ARCH_ESP32 -DARDUINO_BOARD='ESP32_DEV' -DARDUINO_VARIANT='esp32' -DESP32 -DCORE_DEBUG_LEVEL=0 ")

set (ARDUINO_DEFAULT_CFLAGS " -std=gnu99 -Os -g3  -Wno-old-style-declaration ${COMMON_FLAGS}")
set (ARDUINO_DEFAULT_CXXFLAGS "-std=gnu++11 -Os -g3  -fexceptions  -fno-rtti  ${COMMON_FLAGS}")

endif (NOT DEFINED ARDUINO_CFLAGS_SET)
