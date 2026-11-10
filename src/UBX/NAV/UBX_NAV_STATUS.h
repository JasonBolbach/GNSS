#pragma once
#include "UBX/UBX_Common.h"

// UBX-NAV-STATUS (0x01 0x03): receiver navigation status, 16-byte payload.
struct UBX_NAV_STATUS {
	static constexpr uint8_t  MSG_CLASS = 0x01;
	static constexpr uint8_t  MSG_ID    = 0x03;
	static constexpr uint16_t CLASSID   = 0x0301;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 16;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint32_t	iTOW;		// (ms) GPS time of week of the navigation epoch
	uint8_t		gpsFix;		// 0 none, 1 dead reckoning, 2 2D, 3 3D, 4 GPS+DR, 5 time only
	uint8_t		flags;		// 0=gpsFixOk 1=diffSoln 2=wknSet 3=towSet
	uint8_t		fixStat;	// 0=diffCorr 1=carrSolnValid 6-7=mapMatching
	uint8_t		flags2;		// 0-1=psmState 3-4=spoofDetState 6-7=carrSoln
	uint32_t	ttff;		// (ms) Time to first fix
	uint32_t	msss;		// (ms) Milliseconds since startup / reset

	bool gpsFixOk() const { return (flags & 0x01) != 0; }
	bool diffSoln() const { return (flags & 0x02) != 0; }
	bool wknSet() const { return (flags & 0x04) != 0; }
	bool towSet() const { return (flags & 0x08) != 0; }
	uint8_t psmState() const { return flags2 & 0x03; }
	uint8_t spoofDetState() const { return (flags2 >> 3) & 0x03; }	// 0 unknown, 1 none, 2 indicated, 3 multiple
	uint8_t carrSoln() const { return flags2 >> 6; }				// 0 none, 1 float, 2 fixed

	const char* s_gpsFix() const {
		switch (gpsFix) {
		case 0: return "None";
		case 1: return "DR";
		case 2: return "2D";
		case 3: return "3D";
		case 4: return "GPS+DR";
		case 5: return "Time";
		default: return "?";
		}
	}
	const char* s_spoofDetState() const {
		switch (spoofDetState()) {
		case 1: return "None";
		case 2: return "Indicated";
		case 3: return "Multiple";
		default: return "Unknown";
		}
	}

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: NAV-STATUS\n");
		ARGPRINT("     iTOW: %" PRIu32 "\n", iTOW);
		ARGPRINT("   gpsFix: %s\n", s_gpsFix());
		ARGPRINT("    flags: (0x%02x) fixOk:%d diff:%d wkn:%d tow:%d\n", (unsigned)flags, gpsFixOk(), diffSoln(), wknSet(), towSet());
		ARGPRINT("  fixStat: (0x%02x)\n", (unsigned)fixStat);
		ARGPRINT("   flags2: (0x%02x) psm:%u spoof:%s carr:%u\n", (unsigned)flags2, (unsigned)psmState(), s_spoofDetState(), (unsigned)carrSoln());
		ARGPRINT("     ttff: %" PRIu32 " ms\n", ttff);
		ARGPRINT("     msss: %" PRIu32 " ms\n", msss);
	}
};
static_assert(sizeof(UBX_NAV_STATUS) == UBX_NAV_STATUS::WIRE_SIZE, "UBX_NAV_STATUS must match the 16-byte wire format");
