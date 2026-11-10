#pragma once
#include "NMEA/NMEA_Helper.h"

class NMEA_GLL : public NMEA_SentenceBase {
public:
	NMEA_GLL(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	double Latitude() { return pHelper.FieldAsLatLon(1); }
	double Longitude() { return pHelper.FieldAsLatLon(3); }
	bool Valid() { return (pHelper.Field(6)[0] == 'A'); }
	double UTC_Time() { return pHelper.FieldAsDouble(5); }

	void Print() {
		ARGPRINT("GLL: Lat: %f, Lon: %f, Valid: %s, UTC: %6.2f\r\n", Latitude(), Longitude(), Valid() ? "Yes" : "No", UTC_Time());
	}
};
