#pragma once
#include "NMEA/NMEA_Helper.h"

class NMEA_VTG : public NMEA_SentenceBase {
public:
	NMEA_VTG(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	double COG_T() { return pHelper.FieldAsDouble(1); }
	double COG_M() { return pHelper.FieldAsDouble(3); }
	double V_Knots() { return pHelper.FieldAsDouble(5); }
	double V_KpH() { return pHelper.FieldAsDouble(7); }
	const char* Mode() { return pHelper.Field(9); }

	void Print() {
		ARGPRINT("VTG: COG_T: %6.2f, COG_M: %6.2f, V_Knots: %6.2f, V_KpH: %6.2f, Mode: %s\r\n", COG_T(), COG_M(), V_Knots(), V_KpH(), Mode());
	}
};
