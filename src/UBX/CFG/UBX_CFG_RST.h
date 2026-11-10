#pragma once
#include "UBX/UBX_Common.h"

// UBX-CFG-RST (0x06 0x04): reset the receiver / clear navigation data, 4-byte payload.
// A command: there is no poll form and the receiver does not acknowledge a hardware reset.
struct UBX_CFG_RST {
	static constexpr uint8_t  MSG_CLASS = 0x06;
	static constexpr uint8_t  MSG_ID    = 0x04;
	static constexpr uint16_t CLASSID   = 0x0406;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 4;

	// navBbrMask: which battery-backed data to clear before restarting
	static constexpr uint16_t BBR_HOT = 0x0000, BBR_WARM = 0x0001, BBR_COLD = 0xFFFF;
	// resetMode
	static constexpr uint8_t MODE_HW_IMMEDIATE = 0x00, MODE_SW = 0x01, MODE_SW_GNSS_ONLY = 0x02,
		MODE_HW_AFTER_SHUTDOWN = 0x04, MODE_GNSS_STOP = 0x08, MODE_GNSS_START = 0x09;

	uint16_t	navBbrMask;		// BBR sections to clear (BBR_*)
	uint8_t		resetMode;		// Reset type (MODE_*)
	uint8_t		reserved1;

	// ---- builders. All return the frame size; Message == nullptr only returns the size. ----

	static uint16_t ToReset(uint8_t* Message, uint16_t NavBbrMask, uint8_t ResetMode = MODE_SW) {
		UBX_CFG_RST R;
		R.navBbrMask = NavBbrMask;
		R.resetMode = ResetMode;
		R.reserved1 = 0;
		return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, (const uint8_t*)&R, WIRE_SIZE);
	}
	// Restart keeping all stored data (ephemeris, almanac, position): fastest re-acquisition.
	static uint16_t ToHotStart(uint8_t* Message) { return ToReset(Message, BBR_HOT); }
	// Restart keeping the almanac but discarding ephemeris.
	static uint16_t ToWarmStart(uint8_t* Message) { return ToReset(Message, BBR_WARM); }
	// Restart discarding everything: a full cold start.
	static uint16_t ToColdStart(uint8_t* Message) { return ToReset(Message, BBR_COLD); }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: CFG-RST\n");
		ARGPRINT("  navBbr: 0x%04x\n", (unsigned)navBbrMask);
		ARGPRINT("   mode: 0x%02x\n", (unsigned)resetMode);
	}
};
static_assert(sizeof(UBX_CFG_RST) == UBX_CFG_RST::WIRE_SIZE, "UBX_CFG_RST must match the 4-byte wire format");
