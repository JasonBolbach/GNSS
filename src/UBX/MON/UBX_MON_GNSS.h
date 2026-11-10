#pragma once
#include "UBX/UBX_Common.h"

// UBX-MON-GNSS (0x0A 0x28): which GNSS constellations are supported / enabled, 8-byte payload.
struct UBX_MON_GNSS {
	static constexpr uint8_t  MSG_CLASS = 0x0A;
	static constexpr uint8_t  MSG_ID    = 0x28;
	static constexpr uint16_t CLASSID   = 0x280A;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 8;

	// Bits in supported / defaultGnss / enabled
	static constexpr uint8_t GNSS_GPS = 0x01, GNSS_GLONASS = 0x02, GNSS_BEIDOU = 0x04, GNSS_GALILEO = 0x08;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint8_t		version;		// Message version (0x00)
	uint8_t		supported;		// Major GNSS supported by the HW
	uint8_t		defaultGnss;	// Default major GNSS selection
	uint8_t		enabled;		// Current major GNSS selection
	uint8_t		simultaneous;	// Maximum number of concurrent major GNSS
	uint8_t		reserved1[3];

	// Writes e.g. "GPS GLO GAL" into Out (needs 16 bytes).
	static constexpr size_t TEXT_SIZE = 16;
	static const char* s_mask(uint8_t Mask, char (&Out)[TEXT_SIZE]) {
		Out[0] = 0;
		if (Mask & GNSS_GPS) strcat(Out, "GPS ");
		if (Mask & GNSS_GLONASS) strcat(Out, "GLO ");
		if (Mask & GNSS_BEIDOU) strcat(Out, "BDS ");
		if (Mask & GNSS_GALILEO) strcat(Out, "GAL ");
		const size_t N = strlen(Out);
		if (N) Out[N - 1] = 0;	// drop trailing space
		return Out;
	}

	void Print(uint8_t DetailLevel = 0) const {
		char T[TEXT_SIZE];
		ARGPRINT("     Type: MON-GNSS\n");
		ARGPRINT("supported: %s\n", s_mask(supported, T));
		ARGPRINT("  default: %s\n", s_mask(defaultGnss, T));
		ARGPRINT("  enabled: %s\n", s_mask(enabled, T));
		ARGPRINT("  maxConc: %u\n", (unsigned)simultaneous);
	}
};
static_assert(sizeof(UBX_MON_GNSS) == UBX_MON_GNSS::WIRE_SIZE, "UBX_MON_GNSS must match the 8-byte wire format");
