#pragma once
#include "NMEA/NMEA_Helper.h"

// GSA - DOP and active satellites
class NMEA_GSA : public NMEA_SentenceBase {
public:
	NMEA_GSA(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	char OpMode() { return pHelper.Field(1)[0]; }	// M=manual, A=auto 2D/3D
	uint8_t FixType() { return pHelper.FieldAsInt(2); }	// 1=none, 2=2D, 3=3D
	uint8_t SatID(int Index) { if (Index < 0 || Index >= 12) return 0; return pHelper.FieldAsInt(Index + 3); }
	uint8_t SatsUsed() { uint8_t N = 0; for (int i = 0; i < 12; i++) if (SatID(i)) N++; return N; }
	double PDOP() { return pHelper.FieldAsDouble(15); }
	double HDOP() { return pHelper.FieldAsDouble(16); }
	double VDOP() { return pHelper.FieldAsDouble(17); }
	uint8_t SystemID() { return pHelper.FieldAsInt(18); }	// NMEA 4.10: 1=GPS 2=GLONASS 3=Galileo 4=BeiDou

	void Print() {
		ARGPRINT("GSA: Mode: %c, Fix: %d, Sats: %d, PDOP: %4.2f, HDOP: %4.2f, VDOP: %4.2f, Sys: %d\r\n", OpMode(), FixType(), SatsUsed(), PDOP(), HDOP(), VDOP(), SystemID());
	}
};
