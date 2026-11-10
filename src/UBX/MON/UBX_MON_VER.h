#pragma once
#include "UBX/UBX_Common.h"

// UBX-MON-VER (0x0A 0x04): receiver and software version. 40-byte header followed by
// zero or more 30-byte extension strings (protocol version, supported GNSS, ...).
// All strings are NUL-padded fixed-size fields, not necessarily terminated when full.
struct UBX_MON_VER {
	static constexpr uint8_t  MSG_CLASS = 0x0A;
	static constexpr uint8_t  MSG_ID    = 0x04;
	static constexpr uint16_t CLASSID   = 0x040A;	// (ID << 8) | Class
	static constexpr uint16_t HEADER_SIZE = 40;
	static constexpr uint16_t EXTENSION_SIZE = 30;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	char		swVersion[30];		// Software version string
	char		hwVersion[10];		// Hardware version string
	char		extension[30];		// First extension string; more follow (see Extension())

	// Number of extension strings present in a payload of the given length.
	static uint8_t ExtensionCount(uint16_t PayloadLength) {
		return (PayloadLength < HEADER_SIZE) ? 0 : (uint8_t)((PayloadLength - HEADER_SIZE) / EXTENSION_SIZE);
	}
	// Extension i of a payload of the given length, or nullptr. Copy it out with CopyField().
	const char* Extension(uint8_t i, uint16_t PayloadLength) const {
		return (i < ExtensionCount(PayloadLength)) ? extension + (size_t)i * EXTENSION_SIZE : nullptr;
	}

	// Copies a fixed-size, possibly unterminated field into Out as a C string.
	static const char* CopyField(char* Out, size_t OutSize, const char* Field, size_t FieldSize) {
		size_t N = 0;
		while (N < FieldSize && N + 1 < OutSize && Field[N]) { Out[N] = Field[N]; N++; }
		Out[N] = 0;
		return Out;
	}

	void Print(uint8_t DetailLevel = 0) const { Print(DetailLevel, HEADER_SIZE); }
	void Print(uint8_t DetailLevel, uint16_t PayloadLength) const {
		char T[EXTENSION_SIZE + 1];
		ARGPRINT("     Type: MON-VER\n");
		ARGPRINT("       SW: %s\n", CopyField(T, sizeof(T), swVersion, sizeof(swVersion)));
		ARGPRINT("       HW: %s\n", CopyField(T, sizeof(T), hwVersion, sizeof(hwVersion)));
		if (DetailLevel > 0) {
			for (uint8_t i = 0; i < ExtensionCount(PayloadLength); i++)
				ARGPRINT("      ext: %s\n", CopyField(T, sizeof(T), Extension(i, PayloadLength), EXTENSION_SIZE));
		}
	}
};
static_assert(sizeof(UBX_MON_VER) == UBX_MON_VER::HEADER_SIZE + UBX_MON_VER::EXTENSION_SIZE, "UBX_MON_VER header must be 40 bytes");
