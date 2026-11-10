#pragma once
#include "UBX/UBX_Common.h"

// UBX-ACK-ACK (0x05 0x01) / UBX-ACK-NAK (0x05 0x00): the receiver's reply to a CFG command.
// Both carry the class and id of the message being answered, 2-byte payload.
struct UBX_ACK {
	static constexpr uint8_t  MSG_CLASS = 0x05;
	static constexpr uint16_t ACK_CLASSID = 0x0105;	// (ID << 8) | Class
	static constexpr uint16_t NAK_CLASSID = 0x0005;
	static constexpr uint16_t WIRE_SIZE = 2;

	uint8_t		clsID;		// Class of the acknowledged message
	uint8_t		msgID;		// ID of the acknowledged message

	void Print(bool Acknowledged) const {
		ARGPRINT("%s: %s (class 0x%02x, id 0x%02x)\n", Acknowledged ? "ACK" : "NAK", UBX_ClassName(clsID), (unsigned)clsID, (unsigned)msgID);
	}
private:
	static const char* UBX_ClassName(uint8_t C) { return (C == 0x06) ? "CFG" : (C == 0x01) ? "NAV" : (C == 0x0A) ? "MON" : "UBX"; }
};
static_assert(sizeof(UBX_ACK) == UBX_ACK::WIRE_SIZE, "UBX_ACK must match the 2-byte wire format");
