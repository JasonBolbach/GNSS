#pragma once
#include "UBX/UBX_Common.h"

// UBX-CFG-MSG (0x06 0x01): output rate of one message on each port, 8-byte payload.
// (A 3-byte form with a single rate for the current port also exists on input.)
struct UBX_CFG_MSG {
	static constexpr uint8_t  MSG_CLASS = 0x06;
	static constexpr uint8_t  MSG_ID    = 0x01;
	static constexpr uint16_t CLASSID   = 0x0106;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 8;

	// Index into rate[]
	static constexpr uint8_t RATE_DDC = 0, RATE_UART1 = 1, RATE_UART2 = 2, RATE_USB = 3, RATE_SPI = 4;

	// Polls the current output rates of one message, e.g. ToPoll(Buf, 0x01, 0x07) for NAV-PVT
	// or ToPoll(Buf, 0xF0, 0x03) for NMEA GSV. A poll needs the message identifiers (2-byte payload).
	static uint16_t ToPoll(uint8_t* Message, uint8_t TargetClass, uint8_t TargetID) {
		const uint8_t Payload[2] = { TargetClass, TargetID };
		return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, Payload, 2);
	}

	uint8_t		msgClass;		// Message class (0xF0 = standard NMEA)
	uint8_t		msgID;			// Message identifier
	uint8_t		rate[6];		// Send rate per port: 0 = off, n = every n-th navigation epoch

	// Well-known NMEA message ids (msgClass 0xF0)
	static constexpr uint8_t CLASS_NMEA = 0xF0;
	static constexpr uint8_t NMEA_GGA = 0x00, NMEA_GLL = 0x01, NMEA_GSA = 0x02, NMEA_GSV = 0x03, NMEA_RMC = 0x04,
		NMEA_VTG = 0x05, NMEA_GRS = 0x06, NMEA_GST = 0x07, NMEA_ZDA = 0x08, NMEA_GBS = 0x09, NMEA_DTM = 0x0A, NMEA_GNS = 0x0D;

	// ---- set builders. All return the frame size; Message == nullptr only returns the size. ----

	// Sets the output rate of one message on the port the command is received on (3-byte form).
	// Rate 0 = off, 1 = every epoch, n = every n-th epoch.
	//   UBX_CFG_MSG::ToSet(Buf, UBX_CFG_MSG::CLASS_NMEA, UBX_CFG_MSG::NMEA_GSV, 1);	// GSV on
	//   UBX_CFG_MSG::ToSet(Buf, UBX_NAV_PVT::MSG_CLASS, UBX_NAV_PVT::MSG_ID, 1);	// NAV-PVT on
	static uint16_t ToSet(uint8_t* Message, uint8_t TargetClass, uint8_t TargetID, uint8_t Rate) {
		const uint8_t Payload[3] = { TargetClass, TargetID, Rate };
		return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, Payload, 3);
	}
	// Sets a separate rate for every port (8-byte form); Rates is indexed by RATE_*.
	static uint16_t ToSet(uint8_t* Message, uint8_t TargetClass, uint8_t TargetID, const uint8_t (&Rates)[6]) {
		uint8_t Payload[WIRE_SIZE] = { TargetClass, TargetID };
		memcpy(Payload + 2, Rates, 6);
		return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, Payload, WIRE_SIZE);
	}
	// Serialises this struct (fill it in, or copy a polled one and edit it).
	uint16_t ToSet(uint8_t* Message) const { return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, (const uint8_t*)this, WIRE_SIZE); }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: CFG-MSG\n");
		ARGPRINT("  message: 0x%02x 0x%02x\n", (unsigned)msgClass, (unsigned)msgID);
		ARGPRINT("     rate: DDC:%u UART1:%u UART2:%u USB:%u SPI:%u\n", (unsigned)rate[0], (unsigned)rate[1], (unsigned)rate[2], (unsigned)rate[3], (unsigned)rate[4]);
	}
};
static_assert(sizeof(UBX_CFG_MSG) == UBX_CFG_MSG::WIRE_SIZE, "UBX_CFG_MSG must match the 8-byte wire format");
