#pragma once

//send all data from pc to device
#ifdef FEATURE_USB_TO_PC
    #define USB_TO_PC(...)  do{SerialUSB.println(__VA_ARGS__);}while(0)
#else
    #define USB_TO_PC(...) //noop
#endif
