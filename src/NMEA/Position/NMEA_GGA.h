#pragma once
#include "NMEA/NMEA_Helper.h"

// GGA - Fix data (quality, satellites used, HDOP, altitude)
class NMEA_GGA : public NMEA_SentenceBase {
public:
	NMEA_GGA(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	const char* UTC() { return pHelper.Field(1); }
	double Latitude() { return pHelper.FieldAsLatLon(2); }
	double Longitude() { return pHelper.FieldAsLatLon(4); }
	// 0=none, 1=GPS, 2=DGPS, 4=RTK fixed, 5=RTK float, 6=dead reckoning
	uint8_t Quality() { return pHelper.FieldAsInt(6); }
	bool Valid() { return Quality() != 0; }
	uint8_t SatsUsed() { return pHelper.FieldAsInt(7); }
	double HDOP() { return pHelper.FieldAsDouble(8); }
	double Altitude_M() { return pHelper.FieldAsDouble(9); }
	double GeoidSep_M() { return pHelper.FieldAsDouble(11); }
	double DiffAge_S() { return pHelper.FieldAsDouble(13); }

	void Print() {
		ARGPRINT("GGA: UTC: %s, Lat: %f, Lon: %f, Q: %d, Sats: %d, HDOP: %4.2f, Alt: %6.1f, Sep: %6.1f\r\n", UTC(), Latitude(), Longitude(), Quality(), SatsUsed(), HDOP(), Altitude_M(), GeoidSep_M());
	}
};
