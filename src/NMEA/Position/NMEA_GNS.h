#pragma once
#include "NMEA/NMEA_Helper.h"

// GNS - Multi-constellation fix data
class NMEA_GNS : public NMEA_SentenceBase {
public:
	NMEA_GNS(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	const char* UTC() { return pHelper.Field(1); }
	double Latitude() { return pHelper.FieldAsLatLon(2); }
	double Longitude() { return pHelper.FieldAsLatLon(4); }
	const char* Modes() { return pHelper.Field(6); }	// one char per constellation; N = no fix
	bool Valid() { const char* M = Modes(); for (; *M; M++) if (*M != 'N') return true; return false; }
	uint8_t SatsUsed() { return pHelper.FieldAsInt(7); }
	double HDOP() { return pHelper.FieldAsDouble(8); }
	double Altitude_M() { return pHelper.FieldAsDouble(9); }

	void Print() {
		ARGPRINT("GNS: UTC: %s, Lat: %f, Lon: %f, Modes: %s, Sats: %d, HDOP: %4.2f, Alt: %6.1f\r\n", UTC(), Latitude(), Longitude(), Modes(), SatsUsed(), HDOP(), Altitude_M());
	}
};
