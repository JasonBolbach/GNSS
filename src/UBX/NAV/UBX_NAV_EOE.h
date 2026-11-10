#pragma once
#include "UBX/UBX_Common.h"

// UBX-NAV-EOE (0x01 0x61): end of epoch, 4-byte payload. The receiver sends it after all
// periodic NAV messages of an epoch, so it's a good "frame is complete, redraw now" marker.
struct UBX_NAV_EOE {
	static constexpr uint8_t  MSG_CLASS = 0x01;
	static constexpr uint8_t  MSG_ID    = 0x61;
	static constexpr uint16_t CLASSID   = 0x6101;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 4;

	// EOE is periodic-only on most firmware, but the poll is harmless and kept for symmetry.
	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint32_t	iTOW;		// (ms) GPS time of week of the navigation epoch

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: NAV-EOE\n");
		ARGPRINT("     iTOW: %" PRIu32 "\n", iTOW);
	}
};
static_assert(sizeof(UBX_NAV_EOE) == UBX_NAV_EOE::WIRE_SIZE, "UBX_NAV_EOE must match the 4-byte wire format");
