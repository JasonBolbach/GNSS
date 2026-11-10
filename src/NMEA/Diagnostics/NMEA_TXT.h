#pragma once
#include "NMEA/NMEA_Helper.h"

// TXT - Receiver text messages (startup banner, warnings, errors)
class NMEA_TXT : public NMEA_SentenceBase {
public:
	NMEA_TXT(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	uint8_t TotalMessages() { return pHelper.FieldAsInt(1); }
	uint8_t MessageNumber() { return pHelper.FieldAsInt(2); }
	uint8_t Type() { return pHelper.FieldAsInt(3); }	// 0=error, 1=warning, 2=notice, 7=user
	const char* Text() { return pHelper.Field(4); }

	void Print() {
		ARGPRINT("TXT: %d/%d Type: %d, %s\r\n", MessageNumber(), TotalMessages(), Type(), Text());
	}
};
