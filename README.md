# GNSS

Header-only C++ library for reading GNSS receivers: it frames a raw serial byte stream into
checksummed **NMEA** sentences and **UBX** messages, parses the common message types, and builds
u-blox poll / configuration commands. No heap allocation, no dependencies beyond the standard
headers (and Arduino's `Serial` for the default debug output).

## Install

PlatformIO (`platformio.ini`):

```ini
lib_deps = https://github.com/JasonBolbach/GNSS.git
```

For local development use `lib_deps = symlink:///path/to/GNSS`.

## Use

```cpp
#include <GNSS.h>                  // everything; or <GNSS_Filter.h>, <NMEA.h>, <UBX.h> separately

GNSS_Filter Filter;
NMEA_Helper NMEA;
UBX_Helper  UBX;

void loop() {
  while (Serial1.available()) {
    switch (Filter.putch(Serial1.read())) {
      case GNSS_Filter::GNSS_Result::NMEA_Complete:
        NMEA.Parse(Filter.Payload);                    // splits "$GNGGA,...*hh" into fields
        if (!strcmp(NMEA.Header() + 2, "GGA")) NMEA_GGA(NMEA).Print();
        break;
      case GNSS_Filter::GNSS_Result::UBX_Complete:
        UBX.Parse(Filter.Payload);                     // zero-copy view of the frame
        if (UBX.ClassID() == UBX_NAV_PVT::CLASSID)
          ((const UBX_NAV_PVT*)(Filter.Payload + 6))->Print();
        break;
      default: break;
    }
  }
}
```

`Filter.Payload + 6` (the UBX message body) is 4-byte aligned by design, so it can be overlaid on
the `UBX_*` structs on cores that fault on unaligned access.

Talking to a u-blox receiver: every message type has `ToPoll()`, and the CFG messages have `ToSet()`
builders. All of them return the frame size and accept `nullptr` to just report it:

```cpp
uint8_t Cmd[64];
uint16_t n = UBX_CFG_MSG::ToSet(nullptr, UBX_CFG_MSG::CLASS_NMEA, UBX_CFG_MSG::NMEA_GSV, 1);   // size only
UBX_CFG_MSG::ToSet(Cmd, UBX_NAV_PVT::MSG_CLASS, UBX_NAV_PVT::MSG_ID, 1);                        // NAV-PVT every epoch
Serial1.write(Cmd, UBX_NAV_PVT::ToPoll(Cmd));                                                   // poll it once
```

Examples:

- [`examples/BasicMonitor`](examples/BasicMonitor): sets up Serial at 115200, initialises the receiver,
  pumps its serial input through `GNSS_Filter` and prints each NMEA and UBX message as it completes.
- [`examples/SkyPlot`](examples/SkyPlot): draws the satellites on a polar sky plot in a LovyanGFX
  sprite. You supply your own display class in `LGFX_User.h` (a template is included).

### Sky plot

`SatelliteManager` collects the satellites from NMEA GSV and UBX NAV-SAT and gives you plot coordinates;
it does no drawing, so it works with any graphics library:

```cpp
SatelliteManager Sats;
Sats.AddGSV(&GSV, NMEA.Header());        // from a GSV sentence (the header's talker selects the system)
Sats.AddNAVSAT(NavSat);                  // from a UBX-NAV-SAT body (check LengthOK() first)

uint16_t n = Sats.BuildPlot(radius);     // ages out stale satellites, fills Sats.Coords[0..n)
for (uint16_t i = 0; i < n; i++) {
  auto& c = Sats.Coords[i];              // c.X / c.Y from the centre (east right, north up), c.System, c.ID, c.Used, c.Tracked
}
```

Satellites are keyed by constellation and number (`GNSS_System` + per-system ID), so GPS 5 and Galileo 5
are different satellites, and one reported by both NMEA and UBX is merged. Satellites with no position
yet, or below the horizon, are kept but not plotted.

## Layout

```
src/
  GNSS.h              everything
  GNSS_Config.h       debug-print hook (ARGPRINT)
  GNSS_Filter.h       byte-stream framer
  StaticQueue.h       lock-free single-producer / single-consumer ring buffer
  GNSS_System.h       GNSS_System enum (value = u-blox gnssId) and NMEA satellite-number mapping
  SatelliteManager.h  satellite table + sky-plot coordinates, fed from GSV and NAV-SAT
  NMEA.h              all NMEA sentences
  NMEA/
    NMEA_Helper.h     field splitter and sentence base class
    Position/         GGA GLL GNS RMC VTG ZDA
    Satellites/       GSA GSV
    Diagnostics/      GST TXT
  UBX.h               everything UBX
  UBX/
    UBX_Helper.h      frame view, checksum, catalogue lookups
    UBX_Messages.h    148-entry message name / length / description table
    NAV/              POSLLH STATUS DOP PVT VELNED TIMEUTC SAT EOE
    CFG/              PRT MSG RATE NAV5 GNSS CFG RST
    MON/              VER HW GNSS RXBUF TXBUF
    ACK/              ACK / NAK
```

The top-level headers (`<GNSS.h>`, `<NMEA.h>`, `<UBX.h>`, `<GNSS_Filter.h>`, `<StaticQueue.h>`) are all
you need. To compile less, include single messages by path, e.g. `<UBX/NAV/UBX_NAV_PVT.h>`.

## Notes

- **Debug output.** Every `Print()` goes through `ARGPRINT`. On Arduino it defaults to `Serial`; redefine
  `ARGPRINT` (or set `-DGNSS_DEBUG_STREAM=Serial1`) to redirect it.
- **Single context.** `GNSS_Filter` is not thread-safe: call `putch()` from one task. To feed it from a
  network callback or interrupt, put a `StaticQueue` (lock-free SPSC) between the two.
- **Byte order.** The UBX structs overlay the little-endian wire format, which suits ESP32, ARM and RP2040.
- **Message layouts** follow the u-blox M8 interface description. The CFG messages are the legacy
  ones; newer receiver generations still accept them but prefer CFG-VALSET / VALGET.
- **Requires** C++11 and `<atomic>` (for `StaticQueue`). Not for classic AVR.

## License

MIT, see [LICENSE](LICENSE).
