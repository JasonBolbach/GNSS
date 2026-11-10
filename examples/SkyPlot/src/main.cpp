// SkyPlot: draw the satellites a GNSS receiver reports on a polar "sky plot" in a LovyanGFX sprite.
//
//   receiver (GNSS_PORT) -> GNSS_Filter -> SatelliteManager -> BuildPlot() -> drawn into `canvas` -> display
//
// The library does the data work; everything about the picture is in DrawSkyPlot() below, so
// change it freely. You must describe your display in LGFX_User.h first.
//
//   Plot: centre = straight overhead (90 deg elevation), rim = horizon, north up, east right.
//   Dot colour = constellation. Filled = used in the position fix, ring = in view but not used,
//   dim grey = no signal. The label is the constellation letter and number (G05, E24, R12, C07, ...).
#include <Arduino.h>
#include <GNSS.h>
#include "LGFX_User.h"			// <<< you implement this: the display class

#ifndef GNSS_PORT
#define GNSS_PORT Serial1		// the receiver
#endif
#ifndef GNSS_RX_PIN
#define GNSS_RX_PIN 18			// receiver TX -> this pin
#define GNSS_TX_PIN 17			// receiver RX <- this pin
#endif
static const uint32_t GNSS_BAUD = 115200;
static const uint32_t REDRAW_MS = 500;

static LGFX_User          lcd;			// your display
static lgfx::LGFX_Sprite  canvas(&lcd);	// off-screen buffer; drawn in full, then pushed in one go (no flicker)

static GNSS_Filter      Filter;
static NMEA_Helper      NMEA;
static UBX_Helper       UBX;
static SatelliteManager Sats;

// ---- receiver setup -------------------------------------------------------------------------
// Ask for the satellite messages every epoch: UBX NAV-SAT (position, signal and "used in fix"
// flags) and NMEA GSV (works on any receiver, so the plot still fills in without NAV-SAT).
// Both feed the same table; a satellite seen in both is merged.
static void InitReceiver() {
  uint8_t Cmd[64];
  size_t N = 0;
  UBX_Append(Cmd, sizeof(Cmd), N, [](uint8_t* M) { return UBX_CFG_MSG::ToSet(M, UBX_NAV_SAT::MSG_CLASS, UBX_NAV_SAT::MSG_ID, 1); });
  UBX_Append(Cmd, sizeof(Cmd), N, [](uint8_t* M) { return UBX_CFG_MSG::ToSet(M, UBX_CFG_MSG::CLASS_NMEA, UBX_CFG_MSG::NMEA_GSV, 1); });
  GNSS_PORT.write(Cmd, N);
}

// ---- drawing --------------------------------------------------------------------------------
static uint16_t SystemColour(GNSS_System S) {
  switch (S) {
    case GNSS_System::GPS:     return TFT_GREEN;
    case GNSS_System::GLONASS: return TFT_RED;
    case GNSS_System::Galileo: return TFT_CYAN;
    case GNSS_System::BeiDou:  return TFT_YELLOW;
    case GNSS_System::QZSS:    return TFT_MAGENTA;
    case GNSS_System::SBAS:    return TFT_ORANGE;
    default:                   return TFT_WHITE;
  }
}

static void DrawSkyPlot() {
  const int W = canvas.width(), H = canvas.height();
  const int Radius = min(W, H) / 2 - 14;		// leave room for the N/E/S/W letters
  const int Cx = W / 2;
  const int Cy = (H > W) ? W / 2 : H / 2;		// portrait: plot on top, status text underneath

  canvas.fillScreen(TFT_BLACK);

  // Frame: horizon, 30 and 60 degree elevation rings, cross hairs, compass letters.
  canvas.drawCircle(Cx, Cy, Radius, TFT_DARKGREY);
  canvas.drawCircle(Cx, Cy, Radius * 2 / 3, TFT_DARKGREY);
  canvas.drawCircle(Cx, Cy, Radius / 3, TFT_DARKGREY);
  canvas.drawFastHLine(Cx - Radius, Cy, 2 * Radius + 1, TFT_DARKGREY);
  canvas.drawFastVLine(Cx, Cy - Radius, 2 * Radius + 1, TFT_DARKGREY);
  const int Half = canvas.fontHeight() / 2;
  canvas.setTextColor(TFT_WHITE);
  canvas.drawCenterString("N", Cx, Cy - Radius - Half - 6);
  canvas.drawCenterString("S", Cx, Cy + Radius + 6);
  canvas.drawCenterString("E", Cx + Radius + 7, Cy - Half);
  canvas.drawCenterString("W", Cx - Radius - 7, Cy - Half);

  // The satellites. BuildPlot() ages out stale ones and returns only those worth drawing.
  const uint16_t Count = Sats.BuildPlot(Radius);
  uint16_t Used = 0;
  for (uint16_t i = 0; i < Count; i++) {
    const SatelliteManager::DisplayCoord& C = Sats.Coords[i];
    const int X = Cx + C.X;
    const int Y = Cy - C.Y;					// screen Y grows downwards, plot Y grows north
    const uint16_t Colour = C.Tracked ? SystemColour(C.System) : (uint16_t)TFT_DARKGREY;
    if (C.Used) { canvas.fillCircle(X, Y, 4, Colour); Used++; }
    else        { canvas.drawCircle(X, Y, 4, Colour); }
    char Label[8];
    canvas.setTextColor(Colour);
    canvas.drawCenterString(C.Label(Label, sizeof(Label)), X, Y - 6 - Half);
  }

  // Status text.
  canvas.setTextColor(TFT_WHITE);
  canvas.setCursor(2, 2);
  canvas.printf("In view %u\nUsed    %u", (unsigned)Count, (unsigned)Used);
}

// ---- message handling -----------------------------------------------------------------------
static void OnNMEA() {
  NMEA.Parse(Filter.Payload);
  if (!strcmp(NMEA.Header() + 2, "GSV")) {		// the part after the talker id (GP, GL, GA, GB, GN)
    NMEA_GSV GSV(NMEA);
    Sats.AddGSV(&GSV, NMEA.Header());			// the full header carries the talker, which selects the system
  }
}

static void OnUBX() {
  if (!UBX.Parse(Filter.Payload)) return;
  if (UBX.ClassID() == UBX_NAV_SAT::CLASSID && UBX.Length >= UBX_NAV_SAT::HEADER_SIZE) {
    const UBX_NAV_SAT* Sat = (const UBX_NAV_SAT*)(Filter.Payload + 6);	// body is 4-byte aligned
    if (Sat->LengthOK(UBX.Length)) Sats.AddNAVSAT(Sat);
  }
}

// ---- Arduino --------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.setRotation(0);						// try 1-3 for landscape
  canvas.setColorDepth(16);
#ifdef BOARD_HAS_PSRAM
  canvas.setPsram(true);					// a full-screen 16-bit sprite is large; keep it out of internal RAM
#endif
  if (!canvas.createSprite(lcd.width(), lcd.height())) {
    Serial.println("Not enough memory for the sprite: use a smaller display area or enable PSRAM");
    while (true) delay(1000);
  }

#if defined(ESP32)
  GNSS_PORT.begin(GNSS_BAUD, SERIAL_8N1, GNSS_RX_PIN, GNSS_TX_PIN);
#else
  GNSS_PORT.begin(GNSS_BAUD);
#endif
  delay(250);
  InitReceiver();
}

void loop() {
  while (GNSS_PORT.available()) {
    switch (Filter.putch((uint8_t)GNSS_PORT.read())) {
      case GNSS_Filter::GNSS_Result::NMEA_Complete: OnNMEA(); break;
      case GNSS_Filter::GNSS_Result::UBX_Complete:  OnUBX(); break;
      default: break;
    }
  }

  static uint32_t LastDraw = 0;
  if (millis() - LastDraw >= REDRAW_MS) {
    LastDraw = millis();
    DrawSkyPlot();
    canvas.pushSprite(0, 0);					// copy the finished picture to the display
  }
}
