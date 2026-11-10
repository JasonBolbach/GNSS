#pragma once
#include "NMEA/NMEA_Helper.h"

// RMC - Recommended minimum (position, speed, course, date)
class NMEA_RMC : public NMEA_SentenceBase {
public:
	NMEA_RMC(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	const char* UTC() { return pHelper.Field(1); }
	bool Valid() { return (pHelper.Field(2)[0] == 'A'); }
	double Latitude() { return pHelper.FieldAsLatLon(3); }
	double Longitude() { return pHelper.FieldAsLatLon(5); }
	double V_Knots() { return pHelper.FieldAsDouble(7); }
	double COG_T() { return pHelper.FieldAsDouble(8); }
	const char* Date() { return pHelper.Field(9); }	// ddmmyy
	char Mode() { return pHelper.Field(12)[0]; }	// A=auto, D=diff, E=est, N=invalid

	void Print() {
		ARGPRINT("RMC: UTC: %s, Valid: %s, Lat: %f, Lon: %f, Knots: %5.2f, COG: %6.2f, Date: %s, Mode: %c\r\n", UTC(), Valid() ? "Yes" : "No", Latitude(), Longitude(), V_Knots(), COG_T(), Date(), Mode());
	}
};
