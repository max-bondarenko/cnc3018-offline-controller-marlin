#=============================================================================#
#                          Initialization
#=============================================================================#

if (NOT ARDUINO_FOUND AND ARDUINO_SDK_PATH)

    include(SetupCompilerSettings)
    include(SetupArduinoSettings)

    # get version first (some stuff depends on versions)
    include(DetectVersion)

    include(RegisterHardwarePlatform)
    include(FindPrograms)
#    include(SetDefaults) # TODO
    include(SetupFirmwareSizeScript) # TODO
    include(SetupLibraryBlacklist) # TODO nahua

#    include(TestSetup)  TODO

    set(ARDUINO_FOUND True CACHE INTERNAL "Arduino Found")

endif ()
