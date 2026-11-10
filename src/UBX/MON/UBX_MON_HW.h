#pragma once
#include "UBX/UBX_Common.h"

// UBX-MON-HW (0x0A 0x09): hardware status (antenna, noise, AGC, jamming), 60-byte payload.
struct UBX_MON_HW {
	static constexpr uint8_t  MSG_CLASS = 0x0A;
	static constexpr uint8_t  MSG_ID    = 0x09;
	static constexpr uint16_t CLASSID   = 0x090A;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 60;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint32_t	pinSel;			// Mask of pins set as peripheral/PIO
	uint32_t	pinBank;		// Mask of pins set as bank A/B
	uint32_t	pinDir;			// Mask of pins set as input/output
	uint32_t	pinVal;			// Mask of pins value low/high
	uint16_t	noisePerMS;		// Noise level as measured by the GPS core
	uint16_t	agcCnt;			// AGC monitor (0..8191 = 0..100 %)
	uint8_t		aStatus;		// Antenna supervisor state: 0 INIT, 1 UNKNOWN, 2 OK, 3 SHORT, 4 OPEN
	uint8_t		aPower;			// Antenna power: 0 OFF, 1 ON, 2 UNKNOWN
	uint8_t		flags;			// 0=rtcCalib 1-2=safeBoot 3-4=jammingState 5=xtalAbsent
	uint8_t		reserved1;
	uint32_t	usedMask;		// Mask of pins used by the virtual pin manager
	uint8_t		VP[17];			// Pin mapping of the 17 virtual pins
	uint8_t		cwSuppression;	// CW interference suppression level (0..255)
	uint8_t		reserved2[2];
	uint32_t	pinIrq;			// Mask of pins value using the PIO Irq
	uint32_t	pullH;			// Mask of pins value using the PIO pull high resistor
	uint32_t	pullL;			// Mask of pins value using the PIO pull low resistor

	double f_agc_percent() const { return agcCnt * (100.0 / 8191.0); }
	bool rtcCalib() const { return (flags & 0x01) != 0; }
	uint8_t jammingState() const { return (flags >> 3) & 0x03; }	// 0 unknown/off, 1 ok, 2 warning, 3 critical
	bool xtalAbsent() const { return (flags & 0x20) != 0; }

	const char* s_aStatus() const {
		switch (aStatus) {
		case 0: return "INIT";
		case 1: return "UNKNOWN";
		case 2: return "OK";
		case 3: return "SHORT";
		case 4: return "OPEN";
		default: return "?";
		}
	}
	const char* s_aPower() const {
		switch (aPower) {
		case 0: return "OFF";
		case 1: return "ON";
		default: return "UNKNOWN";
		}
	}
	const char* s_jammingState() const {
		switch (jammingState()) {
		case 1: return "OK";
		case 2: return "Warning";
		case 3: return "Critical";
		default: return "Unknown";
		}
	}

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: MON-HW\n");
		ARGPRINT("  antenna: %s / power %s\n", s_aStatus(), s_aPower());
		ARGPRINT("    noise: %u\n", (unsigned)noisePerMS);
		ARGPRINT("      AGC: %u (%4.1f %%)\n", (unsigned)agcCnt, f_agc_percent());
		ARGPRINT("  jamming: %s (CW %u)\n", s_jammingState(), (unsigned)cwSuppression);
		ARGPRINT("    flags: 0x%02x rtcCalib:%d xtalAbsent:%d\n", (unsigned)flags, rtcCalib(), xtalAbsent());
	}
};
static_assert(sizeof(UBX_MON_HW) == UBX_MON_HW::WIRE_SIZE, "UBX_MON_HW must match the 60-byte wire format");
