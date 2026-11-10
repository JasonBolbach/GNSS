#pragma once
#include "UBX/UBX_Common.h"

// UBX-CFG-CFG (0x06 0x09): save, load or clear the receiver's stored configuration.
// A command: there is no poll form. After changing settings with the CFG-* set builders,
// send ToSaveAll() to make them survive a power cycle.
struct UBX_CFG_CFG {
	static constexpr uint8_t  MSG_CLASS = 0x06;
	static constexpr uint8_t  MSG_ID    = 0x09;
	static constexpr uint16_t CLASSID   = 0x0906;	// (ID << 8) | Class
	static constexpr uint16_t WIRE_SIZE = 13;		// 12 without deviceMask

	// clearMask / saveMask / loadMask: which sections
	static constexpr uint32_t SEC_IOPORT = 0x0001, SEC_MSGCONF = 0x0002, SEC_INFMSG = 0x0004, SEC_NAVCONF = 0x0008,
		SEC_RXMCONF = 0x0010, SEC_SENCONF = 0x0100, SEC_RINVCONF = 0x0200, SEC_ANTCONF = 0x0400,
		SEC_LOGCONF = 0x0800, SEC_FTSCONF = 0x1000;
	static constexpr uint32_t SEC_ALL = 0x00001F1F;	// every defined section

	// deviceMask: where to save / load
	static constexpr uint8_t DEV_BBR = 0x01, DEV_FLASH = 0x02, DEV_EEPROM = 0x04, DEV_SPIFLASH = 0x10;
	static constexpr uint8_t DEV_ALL = 0x17;		// devices that are absent are ignored

	uint32_t	clearMask;		// Sections to clear (reset to defaults) in the stored config
	uint32_t	saveMask;		// Sections to save: current config -> storage
	uint32_t	loadMask;		// Sections to load: storage -> current config
	uint8_t		deviceMask;		// Devices the above apply to (DEV_*)

	// ---- builders. All return the frame size; Message == nullptr only returns the size. ----

	static uint16_t ToCommand(uint8_t* Message, uint32_t Clear, uint32_t Save, uint32_t Load, uint8_t Devices = DEV_ALL) {
		uint8_t P[WIRE_SIZE];
		memcpy(P + 0, &Clear, 4);	// little-endian targets
		memcpy(P + 4, &Save, 4);
		memcpy(P + 8, &Load, 4);
		P[12] = Devices;
		return UBX_BuildFrame(Message, MSG_CLASS, MSG_ID, P, WIRE_SIZE);
	}
	// Current configuration -> non-volatile storage.
	static uint16_t ToSaveAll(uint8_t* Message, uint8_t Devices = DEV_ALL) { return ToCommand(Message, 0, SEC_ALL, 0, Devices); }
	// Stored configuration -> current configuration.
	static uint16_t ToLoadAll(uint8_t* Message, uint8_t Devices = DEV_ALL) { return ToCommand(Message, 0, 0, SEC_ALL, Devices); }
	// Factory defaults: clear the stored configuration and load the defaults.
	static uint16_t ToRestoreDefaults(uint8_t* Message, uint8_t Devices = DEV_ALL) { return ToCommand(Message, SEC_ALL, 0, SEC_ALL, Devices); }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: CFG-CFG\n");
		ARGPRINT("    clear: 0x%08" PRIx32 "\n", clearMask);
		ARGPRINT("     save: 0x%08" PRIx32 "\n", saveMask);
		ARGPRINT("     load: 0x%08" PRIx32 "\n", loadMask);
		ARGPRINT("   device: 0x%02x\n", (unsigned)deviceMask);
	}
};
// 13 payload bytes; the compiler pads the struct to 16.
static_assert(offsetof(UBX_CFG_CFG, deviceMask) + 1 == UBX_CFG_CFG::WIRE_SIZE, "UBX_CFG_CFG must match the 13-byte wire format");
