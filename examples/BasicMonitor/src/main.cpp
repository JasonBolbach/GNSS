// BasicMonitor: talk to a u-blox receiver, feed its serial stream to GNSS_Filter and
// print every NMEA sentence and UBX message as it completes.
//
//   Serial  (115200) : debug / monitor output on the USB port
//   GNSS_PORT (115200): the receiver (Serial1 on the pins below)
//
// If your receiver is on the main UART instead, build with -DGNSS_PORT=Serial and
// send the debug output elsewhere (e.g. -DGNSS_DEBUG_STREAM=Serial1).
#include <Arduino.h>
#include <GNSS.h>

#ifndef GNSS_PORT
#define GNSS_PORT Serial1
#endif
#ifndef GNSS_RX_PIN
#define GNSS_RX_PIN 18		// receiver TX -> this pin
#define GNSS_TX_PIN 17		// receiver RX <- this pin
#endif
static const uint32_t GNSS_BAUD = 115200;

static GNSS_Filter Filter;		// frames the byte stream (~1 KB, no heap)
static NMEA_Helper NMEA;		// splits a completed NMEA sentence into fields
static UBX_Helper  UBX;			// zero-copy view of a completed UBX frame

// UBX messages to switch on, as "message every Rate-th navigation epoch".
struct MsgRate { uint8_t Class, ID, Rate; };
static const MsgRate StartupRates[] = {
  { UBX_NAV_PVT::MSG_CLASS, UBX_NAV_PVT::MSG_ID, 1 },	// position, fix type, satellites used
  { UBX_NAV_DOP::MSG_CLASS, UBX_NAV_DOP::MSG_ID, 1 },	// dilution of precision
  { UBX_NAV_SAT::MSG_CLASS, UBX_NAV_SAT::MSG_ID, 1 },	// satellites in view
};

// Builds one batch of commands and sends it: message rates, then polls for what the receiver is doing.
// Nothing is saved to the receiver's flash, so call this on every start.
static void InitReceiver() {
  uint8_t Cmd[256];
  size_t N = 0;
  bool Ok = true;

  for (const MsgRate& R : StartupRates)
    Ok &= UBX_Append(Cmd, sizeof(Cmd), N, [&R](uint8_t* M) { return UBX_CFG_MSG::ToSet(M, R.Class, R.ID, R.Rate); });

  Ok &= UBX_Append(Cmd, sizeof(Cmd), N, [](uint8_t* M) { return UBX_MON_VER::ToPoll(M); });
  Ok &= UBX_Append(Cmd, sizeof(Cmd), N, [](uint8_t* M) { return UBX_MON_GNSS::ToPoll(M); });
  Ok &= UBX_Append(Cmd, sizeof(Cmd), N, [](uint8_t* M) { return UBX_CFG_RATE::ToPoll(M); });
  Ok &= UBX_Append(Cmd, sizeof(Cmd), N, [](uint8_t* M) { return UBX_CFG_NAV5::ToPoll(M); });

  if (!Ok) { Serial.println("Receiver config does not fit its buffer!"); return; }
  GNSS_PORT.write(Cmd, N);
  Serial.printf("Sent %u bytes of receiver config\n", (unsigned)N);
}

static void OnNMEA() {
  Serial.print((const char*)Filter.Payload);	// raw sentence, NUL-terminated with CRLF
  NMEA.Parse(Filter.Payload);				// note: splits the buffer in place
  const char* Type = NMEA.Header() + 2;		// skip the talker id (GP, GN, GL, ...)

  if (!strcmp(Type, "GGA")) NMEA_GGA(NMEA).Print();
  else if (!strcmp(Type, "RMC")) NMEA_RMC(NMEA).Print();
  else if (!strcmp(Type, "GSA")) NMEA_GSA(NMEA).Print();
  else if (!strcmp(Type, "GSV")) NMEA_GSV(NMEA).Print();
  else if (!strcmp(Type, "VTG")) NMEA_VTG(NMEA).Print();
  else if (!strcmp(Type, "GLL")) NMEA_GLL(NMEA).Print();
  else if (!strcmp(Type, "TXT")) NMEA_TXT(NMEA).Print();
  else NMEA.Print();						// anything else: header and field count
}

static void OnUBX() {
  if (!UBX.Parse(Filter.Payload)) return;
  UBX.Print();							// "UBX: NAV-PVT (92) (OK)"

  // The message body starts at Payload + 6 and is 4-byte aligned, so it can be overlaid on the structs.
  const uint8_t* Body = Filter.Payload + 6;
  const uint16_t Len = UBX.Length;
  switch (UBX.ClassID()) {
    case UBX_NAV_PVT::CLASSID:    if (Len >= sizeof(UBX_NAV_PVT)) ((const UBX_NAV_PVT*)Body)->Print(); break;
    case UBX_NAV_DOP::CLASSID:    if (Len == UBX_NAV_DOP::WIRE_SIZE) ((const UBX_NAV_DOP*)Body)->Print(); break;
    case UBX_NAV_SAT::CLASSID:
      if (Len >= UBX_NAV_SAT::HEADER_SIZE && ((const UBX_NAV_SAT*)Body)->LengthOK(Len)) ((const UBX_NAV_SAT*)Body)->Print(1);
      break;
    case UBX_ACK::ACK_CLASSID:
    case UBX_ACK::NAK_CLASSID:    if (Len == UBX_ACK::WIRE_SIZE) ((const UBX_ACK*)Body)->Print(UBX.ClassID() == UBX_ACK::ACK_CLASSID); break;
    case UBX_MON_VER::CLASSID:    if (Len >= UBX_MON_VER::HEADER_SIZE) ((const UBX_MON_VER*)Body)->Print(1, Len); break;
    case UBX_MON_GNSS::CLASSID:   if (Len == UBX_MON_GNSS::WIRE_SIZE) ((const UBX_MON_GNSS*)Body)->Print(); break;
    case UBX_CFG_RATE::CLASSID:   if (Len == UBX_CFG_RATE::WIRE_SIZE) ((const UBX_CFG_RATE*)Body)->Print(); break;
    case UBX_CFG_NAV5::CLASSID:   if (Len == UBX_CFG_NAV5::WIRE_SIZE) ((const UBX_CFG_NAV5*)Body)->Print(); break;
    default: break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("GNSS BasicMonitor");

#if defined(ESP32)
  GNSS_PORT.begin(GNSS_BAUD, SERIAL_8N1, GNSS_RX_PIN, GNSS_TX_PIN);
#else
  GNSS_PORT.begin(GNSS_BAUD);
#endif
  delay(250);
  InitReceiver();
}

void loop() {
  // Pump the receiver's bytes through the filter; it reports each time a message completes.
  while (GNSS_PORT.available()) {
    switch (Filter.putch((uint8_t)GNSS_PORT.read())) {
      case GNSS_Filter::GNSS_Result::NMEA_Complete:     OnNMEA(); break;
      case GNSS_Filter::GNSS_Result::UBX_Complete:      OnUBX(); break;
      case GNSS_Filter::GNSS_Result::ChecksumMismatch:
      case GNSS_Filter::GNSS_Result::OutOfSync:
      case GNSS_Filter::GNSS_Result::Overflow:
        break;	// a corrupt or oversized frame was dropped; the filter has already resynchronised
      case GNSS_Filter::GNSS_Result::Processing:
      default:  break;
    }
  }
}
