#pragma once
#include "UBX/UBX_Common.h"

// UBX-NAV-VELNED (0x01 0x12): velocity in the north/east/down frame, 36-byte payload.
// (Deprecated on protocol 32+, where UBX-NAV-PVT carries the same data; still output by M8 and older.)
struct UBX_NAV_VELNED {
	static constexpr uint8_t  MSG_CLASS = 0x01;
	static constexpr uint8_t  MSG_ID    = 0x12;
	static constexpr uint16_t CLASSID   = 0x1201;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 36;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint32_t	iTOW;		// (ms) GPS time of week of the navigation epoch
	int32_t		velN;		// (cm/s) North velocity
	int32_t		velE;		// (cm/s) East velocity
	int32_t		velD;		// (cm/s) Down velocity
	uint32_t	speed;		// (cm/s) Speed (3-D)
	uint32_t	gSpeed;		// (cm/s) Ground speed (2-D)
	int32_t		heading;	// (1e-5 deg) Heading of motion (2-D)
	uint32_t	sAcc;		// (cm/s) Speed accuracy estimate
	uint32_t	cAcc;		// (1e-5 deg) Course / heading accuracy estimate

	double f_speed_ms() const { return speed * 0.01; }
	double f_gSpeed_ms() const { return gSpeed * 0.01; }
	double f_gSpeed_kph() const { return gSpeed * 0.036; }
	double f_heading() const { return heading * 1e-5; }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: NAV-VELNED\n");
		ARGPRINT("     iTOW: %" PRIu32 "\n", iTOW);
		ARGPRINT("     velN: %" PRId32 " cm/s\n", velN);
		ARGPRINT("     velE: %" PRId32 " cm/s\n", velE);
		ARGPRINT("     velD: %" PRId32 " cm/s\n", velD);
		ARGPRINT("    speed: %" PRIu32 " cm/s\n", speed);
		ARGPRINT("   gSpeed: %" PRIu32 " cm/s (%4.1f km/h)\n", gSpeed, f_gSpeed_kph());
		ARGPRINT("  heading: %f deg\n", f_heading());
		ARGPRINT("     sAcc: %" PRIu32 "\n", sAcc);
		ARGPRINT("     cAcc: %" PRIu32 "\n", cAcc);
	}
};
static_assert(sizeof(UBX_NAV_VELNED) == UBX_NAV_VELNED::WIRE_SIZE, "UBX_NAV_VELNED must match the 36-byte wire format");
