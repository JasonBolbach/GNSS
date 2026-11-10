#pragma once
#include "UBX/UBX_Common.h"

// UBX-NAV-TIMEUTC (0x01 0x21): UTC time solution, 20-byte payload.
struct UBX_NAV_TIMEUTC {
	static constexpr uint8_t  MSG_CLASS = 0x01;
	static constexpr uint8_t  MSG_ID    = 0x21;
	static constexpr uint16_t CLASSID   = 0x2101;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 20;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint32_t	iTOW;		// (ms) GPS time of week of the navigation epoch
	uint32_t	tAcc;		// (ns) Time accuracy estimate (UTC)
	int32_t		nano;		// (ns) Fraction of second, range -1e9 .. 1e9 (UTC)
	uint16_t	year;		// Year (UTC)
	uint8_t		month;		// Month, range 1..12 (UTC)
	uint8_t		day;		// Day of month, range 1..31 (UTC)
	uint8_t		hour;		// Hour of day, range 0..23 (UTC)
	uint8_t		min;		// Minute of hour, range 0..59 (UTC)
	uint8_t		sec;		// Seconds of minute, range 0..60 (UTC)
	uint8_t		valid;		// 0=validTOW 1=validWKN 2=validUTC 4-7=utcStandard

	bool validTOW() const { return (valid & 0x01) != 0; }
	bool validWKN() const { return (valid & 0x02) != 0; }
	bool validUTC() const { return (valid & 0x04) != 0; }
	uint8_t utcStandard() const { return valid >> 4; }
	double f_nano() const { return nano * 1e-9; }

	// "YYYY-MM-DD hh:mm:ss" into Out (needs 20 bytes)
	static constexpr size_t TEXT_SIZE = 20;
	const char* s_datetime(char (&Out)[TEXT_SIZE]) const {
		snprintf(Out, TEXT_SIZE, "%04u-%02u-%02u %02u:%02u:%02u", (unsigned)year, (unsigned)month, (unsigned)day, (unsigned)hour, (unsigned)min, (unsigned)sec);
		return Out;
	}

	void Print(uint8_t DetailLevel = 0) const {
		char T[TEXT_SIZE];
		ARGPRINT("     Type: NAV-TIMEUTC\n");
		ARGPRINT("     iTOW: %" PRIu32 "\n", iTOW);
		ARGPRINT("     UTC: %s (%d%d%d)\n", s_datetime(T), validTOW(), validWKN(), validUTC());
		ARGPRINT("     tAcc: %" PRIu32 " ns\n", tAcc);
		ARGPRINT("     nano: %f\n", f_nano());
		ARGPRINT("  utcStd: %u\n", (unsigned)utcStandard());
	}
};
static_assert(sizeof(UBX_NAV_TIMEUTC) == UBX_NAV_TIMEUTC::WIRE_SIZE, "UBX_NAV_TIMEUTC must match the 20-byte wire format");
