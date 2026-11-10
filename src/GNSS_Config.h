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

// ---------------------------------------------------------------------------
// Millisecond clock used for ageing out satellites (SatelliteManager).
// Default: Arduino millis(), else a steady clock. Define GNSS_MILLIS() yourself to override.
// ---------------------------------------------------------------------------
#ifndef GNSS_MILLIS
  #include <cstdint>
  #if defined(ARDUINO)
    #include <Arduino.h>
    inline uint32_t GNSS_DefaultMillis() { return (uint32_t)millis(); }
  #else
    #include <chrono>
    inline uint32_t GNSS_DefaultMillis() {
      return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
  #endif
  #define GNSS_MILLIS() GNSS_DefaultMillis()
#endif
