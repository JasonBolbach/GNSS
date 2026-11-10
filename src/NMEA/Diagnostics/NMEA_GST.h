#pragma once
#include "NMEA/NMEA_Helper.h"

// GST - Pseudorange error statistics (accuracy estimates)
class NMEA_GST : public NMEA_SentenceBase {
public:
	NMEA_GST(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	const char* UTC() { return pHelper.Field(1); }
	double RangeRMS() { return pHelper.FieldAsDouble(2); }
	double StdLat_M() { return pHelper.FieldAsDouble(6); }
	double StdLon_M() { return pHelper.FieldAsDouble(7); }
	double StdAlt_M() { return pHelper.FieldAsDouble(8); }

	void Print() {
		ARGPRINT("GST: UTC: %s, RMS: %5.2f, StdLat: %5.2f, StdLon: %5.2f, StdAlt: %5.2f\r\n", UTC(), RangeRMS(), StdLat_M(), StdLon_M(), StdAlt_M());
	}
};
