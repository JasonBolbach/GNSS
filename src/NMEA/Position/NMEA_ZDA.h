#pragma once
#include "NMEA/NMEA_Helper.h"

class NMEA_ZDA : public NMEA_SentenceBase {
public:
	NMEA_ZDA(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	const char* UTC() { return pHelper.Field(1); }
	uint16_t Day() { return pHelper.FieldAsInt(2); }
	uint16_t Month() { return pHelper.FieldAsInt(3); }
	uint16_t Year() { return pHelper.FieldAsInt(4); }
	const char* Zone() { return pHelper.Field(5); }
	const char* ZoneMinutes() { return pHelper.Field(6); }

	void Print() {
		ARGPRINT("ZDA: UTC: %s, Day: %d, Month: %d, Year: %d, Zone: %s, ZoneMinutes: %s\r\n", UTC(), Day(), Month(), Year(), Zone(), ZoneMinutes());
	}
};
