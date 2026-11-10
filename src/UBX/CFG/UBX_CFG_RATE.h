#pragma once
#include "UBX/UBX_Common.h"

// UBX-CFG-RATE (0x06 0x08): navigation / measurement rate, 6-byte payload.
struct UBX_CFG_RATE {
	static constexpr uint8_t  MSG_CLASS = 0x06;
	static constexpr uint8_t  MSG_ID    = 0x08;
	static constexpr uint16_t CLASSID   = 0x0806;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 6;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint16_t	measRate;		// (ms) Time between measurements (e.g. 1000 = 1 Hz, 100 = 10 Hz)
	uint16_t	navRate;		// Navigation solutions per measurement (normally 1)
	uint16_t	timeRef;		// 0 = UTC, 1 = GPS, 2 = GLONASS, 3 = BeiDou, 4 = Galileo

	// ---- set builders. All return the frame size; Message == nullptr only returns the size. ----

	// Convenience: update every MeasRate ms, one solution per measurement, GPS time.
	//   UBX_CFG_RATE::ToSet(Buf, 100);	// 10 Hz
	static constexpr uint16_t TIMEREF_UTC = 0, TIMEREF_GPS = 1, TIMEREF_GLONASS = 2, TIMEREF_BEIDOU = 3, TIMEREF_GALILEO = 4;
	static uint16_t ToSet(uint8_t* Message, uint16_t MeasRate_ms, uint16_t NavRate = 1, uint16_t TimeRef = TIMEREF_GPS) {
		UBX_CFG_RATE R;
		R.measRate = MeasRate_ms;
		R.navRate = NavRate;
		R.timeRef = TimeRef;
		return R.ToSet(Message);
	}
	// Serialises this struct.
	uint16_t ToSet(uint8_t* Message) const { return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, (const uint8_t*)this, WIRE_SIZE); }

	double f_Hz() const { return (measRate && navRate) ? 1000.0 / ((double)measRate * navRate) : 0.0; }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: CFG-RATE\n");
		ARGPRINT(" measRate: %u ms\n", (unsigned)measRate);
		ARGPRINT("  navRate: %u\n", (unsigned)navRate);
		ARGPRINT("  timeRef: %u\n", (unsigned)timeRef);
		ARGPRINT("     rate: %4.2f Hz\n", f_Hz());
	}
};
static_assert(sizeof(UBX_CFG_RATE) == UBX_CFG_RATE::WIRE_SIZE, "UBX_CFG_RATE must match the 6-byte wire format");
