#pragma once
#include "UBX/UBX_Common.h"

// UBX-MON-RXBUF (0x0A 0x07): receive buffer status per port, 24-byte payload.
// Port order for every array: DDC, UART1, UART2, USB, SPI, reserved.
struct UBX_MON_RXBUF {
	static constexpr uint8_t  MSG_CLASS = 0x0A;
	static constexpr uint8_t  MSG_ID    = 0x07;
	static constexpr uint16_t CLASSID   = 0x070A;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 24;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint16_t	pending[6];		// (bytes) Number of bytes pending in receiver buffer
	uint8_t		usage[6];		// (%) Maximum usage of receiver buffer during the last sysmon period
	uint8_t		peakUsage[6];	// (%) Maximum usage of receiver buffer

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: MON-RXBUF\n");
		ARGPRINT("  pending: DDC:%u UART1:%u UART2:%u USB:%u SPI:%u\n", (unsigned)pending[0], (unsigned)pending[1], (unsigned)pending[2], (unsigned)pending[3], (unsigned)pending[4]);
		ARGPRINT("    usage: DDC:%u%% UART1:%u%% UART2:%u%% USB:%u%% SPI:%u%%\n", (unsigned)usage[0], (unsigned)usage[1], (unsigned)usage[2], (unsigned)usage[3], (unsigned)usage[4]);
		ARGPRINT("     peak: DDC:%u%% UART1:%u%% UART2:%u%% USB:%u%% SPI:%u%%\n", (unsigned)peakUsage[0], (unsigned)peakUsage[1], (unsigned)peakUsage[2], (unsigned)peakUsage[3], (unsigned)peakUsage[4]);
	}
};
static_assert(sizeof(UBX_MON_RXBUF) == UBX_MON_RXBUF::WIRE_SIZE, "UBX_MON_RXBUF must match the 24-byte wire format");
