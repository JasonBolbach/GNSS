#pragma once

// ---------------------------------------------------------------------------
// Debug output hook used by every Print() method in the library.
//
// Default: Arduino -> GNSS_Printf() writing to GNSS_DEBUG_STREAM (Serial);
//          elsewhere -> printf().
// Redirect it by defining ARGPRINT yourself before the first GNSS include, or with a
// build flag (-DARGPRINT=my_printf). It must accept a printf-style format and arguments.
// Select another stream with -DGNSS_DEBUG_STREAM=Serial1.
// ---------------------------------------------------------------------------
#ifndef ARGPRINT
  #if defined(ARDUINO)
    #include <Arduino.h>
    #include <cstdarg>
    #include <cstdio>
    #ifndef GNSS_DEBUG_STREAM
      #define GNSS_DEBUG_STREAM Serial
    #endif
    inline void GNSS_Printf(const char* Format, ...) __attribute__((format(printf, 1, 2)));
    inline void GNSS_Printf(const char* Format, ...) {
      char Buffer[256];
      va_list Args;
      va_start(Args, Format);
      vsnprintf(Buffer, sizeof(Buffer), Format, Args);
      va_end(Args);
      GNSS_DEBUG_STREAM.print(Buffer);
    }
    #define ARGPRINT GNSS_Printf
  #else
    #include <cstdio>
    #define ARGPRINT printf
  #endif
#endif
