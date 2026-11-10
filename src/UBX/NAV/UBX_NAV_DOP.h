#pragma once
#include "UBX/UBX_Common.h"

// UBX-NAV-DOP (0x01 0x04): dilution of precision, 18-byte payload. All DOPs are scaled by 0.01.
struct UBX_NAV_DOP {
	static constexpr uint8_t  MSG_CLASS = 0x01;
	static constexpr uint8_t  MSG_ID    = 0x04;
	static constexpr uint16_t CLASSID   = 0x0401;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 18;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint32_t	iTOW;		// (ms) GPS time of week of the navigation epoch
	uint16_t	gDOP;		// Geometric DOP
	uint16_t	pDOP;		// Position DOP
	uint16_t	tDOP;		// Time DOP
	uint16_t	vDOP;		// Vertical DOP
	uint16_t	hDOP;		// Horizontal DOP
	uint16_t	nDOP;		// Northing DOP
	uint16_t	eDOP;		// Easting DOP

	double f_gDOP() const { return gDOP * 0.01; }
	double f_pDOP() const { return pDOP * 0.01; }
	double f_tDOP() const { return tDOP * 0.01; }
	double f_vDOP() const { return vDOP * 0.01; }
	double f_hDOP() const { return hDOP * 0.01; }
	double f_nDOP() const { return nDOP * 0.01; }
	double f_eDOP() const { return eDOP * 0.01; }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: NAV-DOP\n");
		ARGPRINT("     iTOW: %" PRIu32 "\n", iTOW);
		ARGPRINT("     gDOP: %4.2f\n", f_gDOP());
		ARGPRINT("     pDOP: %4.2f\n", f_pDOP());
		ARGPRINT("     tDOP: %4.2f\n", f_tDOP());
		ARGPRINT("     vDOP: %4.2f\n", f_vDOP());
		ARGPRINT("     hDOP: %4.2f\n", f_hDOP());
		ARGPRINT("     nDOP: %4.2f\n", f_nDOP());
		ARGPRINT("     eDOP: %4.2f\n", f_eDOP());
	}
};
// 18 bytes of payload; the compiler pads the struct to 20, so check the last field's end instead.
static_assert(offsetof(UBX_NAV_DOP, eDOP) + sizeof(uint16_t) == UBX_NAV_DOP::WIRE_SIZE, "UBX_NAV_DOP must match the 18-byte wire format");
