#pragma once
#include "UBX/UBX_Common.h"

// UBX-MON-TXBUF (0x0A 0x08): transmit buffer status per port, 28-byte payload.
// Port order for every array: DDC, UART1, UART2, USB, SPI, reserved.
// A non-zero "limit" error means the receiver wanted to send more than the port's baud rate / buffer allowed.
struct UBX_MON_TXBUF {
	static constexpr uint8_t  MSG_CLASS = 0x0A;
	static constexpr uint8_t  MSG_ID    = 0x08;
	static constexpr uint16_t CLASSID   = 0x080A;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 28;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	uint16_t	pending[6];		// (bytes) Number of bytes pending in transmitter buffer
	uint8_t		usage[6];		// (%) Maximum usage of transmitter buffer during the last sysmon period
	uint8_t		peakUsage[6];	// (%) Maximum usage of transmitter buffer
	uint8_t		tUsage;			// (%) Maximum usage of transmitter buffer during the last sysmon period for all targets
	uint8_t		tPeakusage;		// (%) Maximum usage of transmitter buffer for all targets
	uint8_t		errors;			// 0-5=limit (per target) 6=mem 7=alloc
	uint8_t		reserved1;

	uint8_t limitErrors() const { return errors & 0x3f; }	// bit mask, one bit per port
	bool memError() const { return (errors & 0x40) != 0; }
	bool allocError() const { return (errors & 0x80) != 0; }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: MON-TXBUF\n");
		ARGPRINT("  pending: DDC:%u UART1:%u UART2:%u USB:%u SPI:%u\n", (unsigned)pending[0], (unsigned)pending[1], (unsigned)pending[2], (unsigned)pending[3], (unsigned)pending[4]);
		ARGPRINT("    usage: DDC:%u%% UART1:%u%% UART2:%u%% USB:%u%% SPI:%u%%\n", (unsigned)usage[0], (unsigned)usage[1], (unsigned)usage[2], (unsigned)usage[3], (unsigned)usage[4]);
		ARGPRINT("     peak: all:%u%% (usage %u%%)\n", (unsigned)tPeakusage, (unsigned)tUsage);
		ARGPRINT("   errors: limit:0x%02x mem:%d alloc:%d\n", (unsigned)limitErrors(), memError(), allocError());
	}
};
static_assert(sizeof(UBX_MON_TXBUF) == UBX_MON_TXBUF::WIRE_SIZE, "UBX_MON_TXBUF must match the 28-byte wire format");
