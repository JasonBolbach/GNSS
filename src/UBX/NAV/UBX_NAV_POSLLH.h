#pragma once
#include "UBX/UBX_Common.h"

// UBX-NAV-POSLLH (0x01 0x02): geodetic position, 28-byte payload.
// Overlay it on a payload whose start is 4-byte aligned (GNSS_Filter guarantees this).
struct UBX_NAV_POSLLH {
	static constexpr uint8_t  MSG_CLASS = 0x01;
	static constexpr uint8_t  MSG_ID    = 0x02;
	static constexpr uint16_t WIRE_SIZE = 28;
	static constexpr uint16_t CLASSID = 0x0201;	// (ID << 8) | Class

	// Writes a poll request to Message and returns its size in bytes (8). Send it to the receiver to get this
	// message once. Message == nullptr writes nothing and just returns the size.
	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint32_t	iTOW;		// (ms) GPS time of week of the navigation epoch
	int32_t		lon;		// (1e-7 deg) Longitude
	int32_t		lat;		// (1e-7 deg) Latitude
	int32_t		height;		// (mm) Height above ellipsoid
	int32_t		hMSL;		// (mm) Height above mean sea level
	uint32_t	hAcc;		// (mm) Horizontal accuracy estimate
	uint32_t	vAcc;		// (mm) Vertical accuracy estimate

	double f_lon() const { return ((double)lon) * 1e-7; }
	double f_lat() const { return ((double)lat) * 1e-7; }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: NAV-POSLLH\n");
		ARGPRINT("     iTOW: %" PRIu32 "\n", iTOW);
		ARGPRINT("Longitude: %f (%" PRId32 ")\n", f_lon(), lon);
		ARGPRINT(" Latitude: %f (%" PRId32 ")\n", f_lat(), lat);
		ARGPRINT("   height: %" PRId32 "\n", height);
		ARGPRINT("     hMSL: %" PRId32 "\n", hMSL);
		ARGPRINT("     hAcc: %" PRIu32 "\n", hAcc);
		ARGPRINT("     vAcc: %" PRIu32 "\n", vAcc);
	}
};
static_assert(sizeof(UBX_NAV_POSLLH) == UBX_NAV_POSLLH::WIRE_SIZE, "UBX_NAV_POSLLH must match the 28-byte wire format");
