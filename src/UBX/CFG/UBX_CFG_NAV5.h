#pragma once
#include "UBX/UBX_Common.h"

// UBX-CFG-NAV5 (0x06 0x24): navigation engine settings, 36-byte payload.
struct UBX_CFG_NAV5 {
	static constexpr uint8_t  MSG_CLASS = 0x06;
	static constexpr uint8_t  MSG_ID    = 0x24;
	static constexpr uint16_t CLASSID   = 0x2406;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 36;

	// mask bits: which fields a SET applies
	static constexpr uint16_t MASK_DYN = 0x0001, MASK_MINEL = 0x0002, MASK_POSFIXMODE = 0x0004, MASK_DRLIM = 0x0008,
		MASK_POSMASK = 0x0010, MASK_TIMEMASK = 0x0020, MASK_STATICHOLDMASK = 0x0040, MASK_DGPSMASK = 0x0080,
		MASK_CNOTHRESHOLD = 0x0100, MASK_UTC = 0x0400;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint16_t	mask;				// Parameters bitmask (MASK_*)
	uint8_t		dynModel;			// Dynamic platform model: 0 portable, 2 stationary, 3 pedestrian, 4 automotive, 5 sea, 6-8 airborne, 9 wrist, 10 bike
	uint8_t		fixMode;			// 1 = 2D only, 2 = 3D only, 3 = auto 2D/3D
	int32_t		fixedAlt;			// (0.01 m) Fixed altitude for 2D mode
	uint32_t	fixedAltVar;		// (0.0001 m^2) Fixed altitude variance for 2D mode
	int8_t		minElev;			// (deg) Minimum elevation for a GNSS satellite to be used
	uint8_t		drLimit;			// (s) Reserved
	uint16_t	pDop;				// (0.1) Position DOP mask
	uint16_t	tDop;				// (0.1) Time DOP mask
	uint16_t	pAcc;				// (m) Position accuracy mask
	uint16_t	tAcc;				// (m) Time accuracy mask
	uint8_t		staticHoldThresh;	// (cm/s) Static hold threshold
	uint8_t		dgnssTimeout;		// (s) DGNSS timeout
	uint8_t		cnoThreshNumSVs;	// Number of satellites required above cnoThresh
	uint8_t		cnoThresh;			// (dBHz) C/N0 threshold for the above
	uint16_t	reserved1;
	uint16_t	staticHoldMaxDist;	// (m) Static hold distance threshold
	uint8_t		utcStandard;		// 0 auto, 3 USNO (GPS), 5 Europe (Galileo), 6 SU (GLONASS), 7 NTSC (BeiDou)
	uint8_t		reserved2[5];

	// ---- set builders. All return the frame size; Message == nullptr only returns the size. ----

	// Only the fields selected in `mask` are applied by the receiver, so these touch one setting each.
	//   UBX_CFG_NAV5::ToSetDynModel(Buf, 4);	// automotive
	static uint16_t ToSetDynModel(uint8_t* Message, uint8_t DynModel) {
		UBX_CFG_NAV5 N;
		memset(&N, 0, sizeof(N));
		N.mask = MASK_DYN;
		N.dynModel = DynModel;
		return N.ToSet(Message);
	}
	// FixMode: 1 = 2D only, 2 = 3D only, 3 = auto 2D/3D.
	static uint16_t ToSetFixMode(uint8_t* Message, uint8_t FixMode) {
		UBX_CFG_NAV5 N;
		memset(&N, 0, sizeof(N));
		N.mask = MASK_POSFIXMODE;
		N.fixMode = FixMode;
		return N.ToSet(Message);
	}
	// Serialises this struct. Set `mask` to the MASK_* bits of the fields you want applied.
	uint16_t ToSet(uint8_t* Message) const { return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, (const uint8_t*)this, WIRE_SIZE); }

	const char* s_dynModel() const {
		switch (dynModel) {
		case 0: return "Portable";
		case 2: return "Stationary";
		case 3: return "Pedestrian";
		case 4: return "Automotive";
		case 5: return "Sea";
		case 6: return "Airborne <1g";
		case 7: return "Airborne <2g";
		case 8: return "Airborne <4g";
		case 9: return "Wrist";
		case 10: return "Bike";
		default: return "?";
		}
	}
	const char* s_fixMode() const {
		switch (fixMode) {
		case 1: return "2D only";
		case 2: return "3D only";
		case 3: return "Auto 2D/3D";
		default: return "?";
		}
	}

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: CFG-NAV5\n");
		ARGPRINT("     mask: 0x%04x\n", (unsigned)mask);
		ARGPRINT(" dynModel: %s (%u)\n", s_dynModel(), (unsigned)dynModel);
		ARGPRINT("  fixMode: %s\n", s_fixMode());
		ARGPRINT(" fixedAlt: %" PRId32 " (0.01 m)\n", fixedAlt);
		ARGPRINT(" minElev: %d deg\n", (int)minElev);
		ARGPRINT("     pDop: %u  tDop: %u (0.1)\n", (unsigned)pDop, (unsigned)tDop);
		ARGPRINT("     pAcc: %u  tAcc: %u (m)\n", (unsigned)pAcc, (unsigned)tAcc);
		ARGPRINT("  cnoThr: %u SVs > %u dBHz\n", (unsigned)cnoThreshNumSVs, (unsigned)cnoThresh);
		ARGPRINT("   utcStd: %u\n", (unsigned)utcStandard);
	}
};
static_assert(sizeof(UBX_CFG_NAV5) == UBX_CFG_NAV5::WIRE_SIZE, "UBX_CFG_NAV5 must match the 36-byte wire format");
