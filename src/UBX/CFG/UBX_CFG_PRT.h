#pragma once
#include "UBX/UBX_Common.h"

// UBX-CFG-PRT (0x06 0x00): port configuration, UART/USB/SPI/DDC layout, 20-byte payload.
struct UBX_CFG_PRT {
	static constexpr uint8_t  MSG_CLASS = 0x06;
	static constexpr uint8_t  MSG_ID    = 0x00;
	static constexpr uint16_t CLASSID   = 0x0006;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 20;

	// Port identifiers
	static constexpr uint8_t PORT_DDC = 0, PORT_UART1 = 1, PORT_UART2 = 2, PORT_USB = 3, PORT_SPI = 4;
	// inProtoMask / outProtoMask bits
	static constexpr uint16_t PROTO_UBX = 0x0001, PROTO_NMEA = 0x0002, PROTO_RTCM2 = 0x0004, PROTO_RTCM3 = 0x0020;

	// Polls the port the request arrives on (no payload).
	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }
	// Polls a specific port (1-byte payload).
	static uint16_t ToPoll(uint8_t* Message, uint8_t PortID) { return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, &PortID, 1); }

	uint8_t		portID;			// Port identifier (see PORT_*)
	uint8_t		reserved1;
	uint16_t	txReady;		// TX ready pin configuration
	uint32_t	mode;			// UART: bits 6-7 charLen (3 = 8 bit), 9-11 parity (4 = none), 12-13 nStopBits (0 = 1)
	uint32_t	baudRate;		// (bits/s) UART baud rate
	uint16_t	inProtoMask;	// Protocols accepted on input (PROTO_*)
	uint16_t	outProtoMask;	// Protocols produced on output (PROTO_*)
	uint16_t	flags;			// bit 1 = extendedTxTimeout
	uint8_t		reserved2[2];

	// ---- set builders. All return the frame size; Message == nullptr only returns the size. ----

	// UART mode word for 8 data bits, no parity, 1 stop bit.
	static constexpr uint32_t MODE_8N1 = 0x000008D0;

	// Configures a UART: 8N1 at the given baud, with the chosen input / output protocols.
	//   UBX_CFG_PRT::ToSetUART(Buf, UBX_CFG_PRT::PORT_UART1, 115200, PROTO_UBX | PROTO_NMEA, PROTO_UBX | PROTO_NMEA);
	// Note: the port changes speed as soon as it processes this, so re-open your side at the new baud.
	static uint16_t ToSetUART(uint8_t* Message, uint8_t PortID, uint32_t Baud, uint16_t InProtoMask, uint16_t OutProtoMask) {
		UBX_CFG_PRT P;
		memset(&P, 0, sizeof(P));
		P.portID = PortID;
		P.mode = MODE_8N1;
		P.baudRate = Baud;
		P.inProtoMask = InProtoMask;
		P.outProtoMask = OutProtoMask;
		return P.ToSet(Message);
	}
	// Serialises this struct (fill it in, or copy a polled one and edit it).
	uint16_t ToSet(uint8_t* Message) const { return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, (const uint8_t*)this, WIRE_SIZE); }

	uint8_t charLen() const { return (mode >> 6) & 0x03; }	// 0 = 5 bit ... 3 = 8 bit
	uint8_t parity() const { return (mode >> 9) & 0x07; }	// 4 = none, 0 = even, 1 = odd
	uint8_t stopBits() const { return (mode >> 12) & 0x03; }	// 0 = 1 stop bit

	const char* s_portID() const {
		switch (portID) {
		case PORT_DDC: return "DDC";
		case PORT_UART1: return "UART1";
		case PORT_UART2: return "UART2";
		case PORT_USB: return "USB";
		case PORT_SPI: return "SPI";
		default: return "?";
		}
	}

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: CFG-PRT\n");
		ARGPRINT("   portID: %s (%u)\n", s_portID(), (unsigned)portID);
		ARGPRINT("     mode: 0x%08" PRIx32 " (%u-%s-%u)\n", mode, (unsigned)charLen() + 5, parity() == 4 ? "N" : (parity() == 1 ? "O" : "E"), (unsigned)stopBits() + 1);
		ARGPRINT(" baudRate: %" PRIu32 "\n", baudRate);
		ARGPRINT("  inProto: 0x%04x\n", (unsigned)inProtoMask);
		ARGPRINT(" outProto: 0x%04x\n", (unsigned)outProtoMask);
	}
};
static_assert(sizeof(UBX_CFG_PRT) == UBX_CFG_PRT::WIRE_SIZE, "UBX_CFG_PRT must match the 20-byte wire format");
