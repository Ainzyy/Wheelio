#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

// Debug mode flag
#define DEBUG_MODE 1

#if DEBUG_MODE
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
#endif

#endif // DEBUG_CONFIG_H