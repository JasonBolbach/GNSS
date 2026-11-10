#pragma once
#include "UBX/UBX_Common.h"

// UBX-NAV-PVT (0x01 0x07): position, velocity and time, 92-byte payload.
// Pure wire layout: overlay it on a payload whose start is 4-byte aligned
// (GNSS_Filter guarantees this). Text helpers write into a caller-supplied buffer
// so the struct carries no scratch state of its own.
struct UBX_NAV_PVT {
	static constexpr uint8_t  MSG_CLASS = 0x01;
	static constexpr uint8_t  MSG_ID    = 0x07;
	static constexpr uint16_t CLASSID = 0x0701;	// (ID << 8) | Class

	// Writes a poll request to Message and returns its size in bytes (8). Send it to the receiver to get this
	// message once. Message == nullptr writes nothing and just returns the size.
	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint32_t	iTOW;			// GPS time of week of the navigation epoch.
	uint16_t	year;			// Year (UTC)
	uint8_t		month;			// Month, range 1..12 (UTC)
	uint8_t		day;			// Day of month, range 1..31 (UTC)
	uint8_t		hour;			// Hour of day, range 0..23 (UTC)
	uint8_t		min;			// Minute of hour, range 0..59 (UTC)
	uint8_t		sec;			// Seconds of minute, range 0..60 (UTC)
	uint8_t		valid;			// Validity flags: 0=validDate 1=validTime 2=fullyResolved 3=validMag
	uint32_t	tAcc;			// Time accuracy estimate (UTC)
	int32_t		nano;			// Fraction of second, range -1e9 .. 1e9 (UTC)
	uint8_t		fixType;		// GNSSfix Type
	uint8_t		flags;			// 0=gnssFixOK 1=diffSoln 2-4=psmState 5=headVehValid 6-7=carrSoln
	uint8_t		flags2;			// 5=confirmedAvai 6=confirmedDate 7=confirmedTime
	uint8_t		numSV;			// Number of satellites used in Nav Solution
	int32_t		lon;			// Longitude (1e-7 deg)
	int32_t		lat;			// Latitude (1e-7 deg)
	int32_t		height;			// Height above ellipsoid (mm)
	int32_t		hMSL;			// Height above mean sea level (mm)
	uint32_t	hAcc;			// Horizontal accuracy estimate (mm)
	uint32_t	vAcc;			// Vertical accuracy estimate (mm)
	int32_t		velN;			// NED north velocity (mm/s)
	int32_t		velE;			// NED east velocity (mm/s)
	int32_t		velD;			// NED down velocity (mm/s)
	int32_t		gSpeed;			// Ground Speed (2-D) (mm/s)
	int32_t		headMot;		// Heading of motion (2-D) (1e-5 deg)
	uint32_t	sAcc;			// Speed accuracy estimate (mm/s)
	uint32_t	headAcc;		// Heading accuracy estimate (1e-5 deg)
	uint16_t	pDOP;			// Position DOP (0.01)
	uint16_t	flags3;			// 0=invalidLlh 1-4=lastCorrectionAge
	uint32_t	reserved1;		// Reserved
	int32_t		headVeh;		// Heading of vehicle (2-D) (1e-5 deg)
	int16_t		magDec;			// Magnetic declination. Only supported in ADR 4.10 and later.
	uint16_t	magAcc;			// Magnetic declination accuracy. Only supported in ADR 4.10 and later.

	// Size of the buffer each s_*() helper needs.
	static constexpr size_t TEXT_SIZE = 12;

	double f_lon() const { return ((double)lon) * 1e-7; }
	double f_lat() const { return ((double)lat) * 1e-7; }
	double f_nano() const { return ((double)nano) * 1e-9; }

	const char* s_fixType() const {
		switch (fixType) {
		case 0: return "None";
		case 1: return "DR";
		case 2: return "2D";
		case 3: return "3D";
		case 4: return "GNSS+DR";
		case 5: return "Time";
		default: return "?";
		}
	}

	const char* s_valid(char (&Out)[TEXT_SIZE]) const {
		Out[0] = '[';
		Out[1] = (valid & 0x01) ? 'D' : ' ';
		Out[2] = (valid & 0x02) ? 'T' : ' ';
		Out[3] = (valid & 0x04) ? 'R' : ' ';
		Out[4] = (valid & 0x08) ? 'M' : ' ';
		Out[5] = ']';
		Out[6] = 0;
		return Out;
	}

	const char* s_flags(char (&Out)[TEXT_SIZE]) const {
		Out[0] = '[';
		Out[1] = (flags & 0x01) ? 'F' : ' ';	// gnssFixOK
		Out[2] = (flags & 0x02) ? 'D' : ' ';	// diffSoln
		Out[3] = (flags & 0x20) ? 'V' : ' ';	// headVehValid
		Out[4] = (char)((flags >> 6) + '0');	// carrSoln 0..2
		Out[5] = ']';
		Out[6] = 0;
		return Out;
	}

	const char* s_flags2(char (&Out)[TEXT_SIZE]) const {
		Out[0] = '[';
		Out[1] = (flags2 & 0x20) ? 'A' : ' ';	// confirmedAvai
		Out[2] = (flags2 & 0x40) ? 'D' : ' ';	// confirmedDate
		Out[3] = (flags2 & 0x80) ? 'T' : ' ';	// confirmedTime
		Out[4] = ']';
		Out[5] = 0;
		return Out;
	}

	// Age of differential corrections in seconds: 254 = not available, 255 = 120 s or more.
	// Values are the lower bound of each reported bucket.
	uint8_t age_flags3() const {
		static const uint8_t Age[12] = { 254, 1, 2, 5, 10, 15, 20, 30, 45, 60, 90, 120 };
		const uint8_t Code = (flags3 >> 1) & 0x0f;
		return (Code < 12) ? Age[Code] : 255;
	}

	const char* s_flags3(char (&Out)[TEXT_SIZE]) const {
		snprintf(Out, TEXT_SIZE, "[%c:%03u]", (flags3 & 0x01) ? 'I' : 'V', (unsigned)age_flags3());
		return Out;
	}

	void Print(uint8_t DetailLevel = 0) const {
		char T[TEXT_SIZE];
		ARGPRINT("     Type: NAV-PVT\n");
		ARGPRINT("     iTOW: %" PRIu32 "\n", iTOW);
		ARGPRINT("     year: %u\n", (unsigned)year);
		ARGPRINT("    month: %u\n", (unsigned)month);
		ARGPRINT("      day: %u\n", (unsigned)day);
		ARGPRINT("     hour: %u\n", (unsigned)hour);
		ARGPRINT("      min: %u\n", (unsigned)min);
		ARGPRINT("      sec: %u\n", (unsigned)sec);
		ARGPRINT("    valid: %s\n", s_valid(T));
		ARGPRINT("     tAcc: %" PRIu32 "\n", tAcc);
		ARGPRINT("     nano: %f\n", f_nano());
		ARGPRINT("  fixType: %s\n", s_fixType());
		ARGPRINT("    flags: (0x%02x)%s\n", (unsigned)flags, s_flags(T));
		ARGPRINT("   flags2: (0x%02x)%s\n", (unsigned)flags2, s_flags2(T));
		ARGPRINT("    numSV: %u\n", (unsigned)numSV);
		ARGPRINT("      lon: %f\n", f_lon());
		ARGPRINT("      lat: %f\n", f_lat());
		ARGPRINT("   height: %" PRId32 "\n", height);
		ARGPRINT("     hMSL: %" PRId32 "\n", hMSL);
		ARGPRINT("     hAcc: %" PRIu32 "\n", hAcc);
		ARGPRINT("     vAcc: %" PRIu32 "\n", vAcc);
		ARGPRINT("     velN: %" PRId32 "\n", velN);
		ARGPRINT("     velE: %" PRId32 "\n", velE);
		ARGPRINT("     velD: %" PRId32 "\n", velD);
		ARGPRINT("   gSpeed: %" PRId32 "\n", gSpeed);
		ARGPRINT("  headMot: %" PRId32 "\n", headMot);
		ARGPRINT("     sAcc: %" PRIu32 "\n", sAcc);
		ARGPRINT("  headAcc: %" PRIu32 "\n", headAcc);
		ARGPRINT("     pDOP: %u\n", (unsigned)pDOP);
		ARGPRINT("   flags3: (0x%04x)%s\n", (unsigned)flags3, s_flags3(T));
		ARGPRINT("reserved1: %08" PRIx32 "\n", reserved1);
		ARGPRINT("  headVeh: %" PRId32 "\n", headVeh);
		ARGPRINT("   magDec: %d\n", (int)magDec);
		ARGPRINT("   magAcc: %u\n", (unsigned)magAcc);
	}
};
static_assert(sizeof(UBX_NAV_PVT) == 92, "UBX_NAV_PVT must match the 92-byte wire format");
