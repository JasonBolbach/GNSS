#pragma once
// GNSS: NMEA / UBX framing and message parsing for GNSS receivers.
//
//   #include <GNSS.h>        // everything
//   #include <GNSS_Filter.h> // just the byte-stream framer
//   #include <NMEA.h>        // NMEA sentence parsers
//   #include <UBX.h>         // UBX framing, message catalogue and message types
//   #include <SatelliteManager.h> // satellite table + sky-plot coordinates
//
// Individual messages can be included on their own, e.g.
//   #include <UBX/NAV/UBX_NAV_PVT.h>
//   #include <NMEA/Position/NMEA_GGA.h>
#include "GNSS_Config.h"
#include "StaticQueue.h"
#include "GNSS_Filter.h"
#include "NMEA.h"
#include "UBX.h"
#include "GNSS_System.h"
#include "SatelliteManager.h"
