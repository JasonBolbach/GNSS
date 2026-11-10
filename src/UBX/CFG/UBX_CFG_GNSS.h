#pragma once
#include "UBX/UBX_Common.h"

// UBX-CFG-GNSS (0x06 0x3E): GNSS system configuration. 4-byte header followed by
// numConfigBlocks 8-byte blocks. Overlay it on a 4-byte aligned payload.
struct UBX_CFG_GNSS {
	static constexpr uint8_t  MSG_CLASS = 0x06;
	static constexpr uint8_t  MSG_ID    = 0x3E;
	static constexpr uint16_t CLASSID   = 0x3E06;	// (ID << 8) | Class
	static constexpr uint16_t HEADER_SIZE = 4;
	static constexpr uint16_t BLOCK_SIZE = 8;

	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	struct Block {
		uint8_t		gnssId;			// 0 GPS, 1 SBAS, 2 Galileo, 3 BeiDou, 4 IMES, 5 QZSS, 6 GLONASS
		uint8_t		resTrkCh;		// Number of reserved (minimum) tracking channels
		uint8_t		maxTrkCh;		// Maximum number of tracking channels
		uint8_t		reserved1;
		uint32_t	flags;			// bit 0 = enable, bits 16-23 = sigCfgMask

		bool enabled() const { return (flags & 0x00000001) != 0; }
		uint8_t sigCfgMask() const { return (flags >> 16) & 0xff; }

		const char* s_gnssId() const {
			switch (gnssId) {
			case 0: return "GPS";
			case 1: return "SBAS";
			case 2: return "Galileo";
			case 3: return "BeiDou";
			case 4: return "IMES";
			case 5: return "QZSS";
			case 6: return "GLONASS";
			default: return "UNKNOWN";
			}
		}

		void Print(uint8_t DetailLevel = 0) const {
			ARGPRINT("\t%7s enabled:%d resTrkCh:%02u maxTrkCh:%02u flags:0x%08" PRIx32 "\n", s_gnssId(), enabled(), (unsigned)resTrkCh, (unsigned)maxTrkCh, flags);
		}
	};

	uint8_t		msgVer;				// Message version (0)
	uint8_t		numTrkChHw;			// Number of tracking channels in hardware
	uint8_t		numTrkChUse;		// Number of tracking channels to use
	uint8_t		numConfigBlocks;	// Number of configuration blocks following
	Block		Blocks;				// Start of repeated block (numConfigBlocks times); use Cfg(i)

	// ---- set builder. Returns the frame size; Message == nullptr only returns the size. ----

	// Writes a set for `Count` constellation blocks (4 + 8*Count payload bytes). Build the blocks
	// yourself, or edit copies of the ones the receiver reported. NumTrkChUse 0xFF = use all hardware channels.
	// Message must hold ToSet(nullptr, ...) bytes. A receiver restarts its GNSS subsystem on accepting this.
	static uint16_t ToSet(uint8_t* Message, const Block* Blocks, uint8_t Count, uint8_t NumTrkChUse = 0xFF) {
		const uint16_t Length = HEADER_SIZE + BLOCK_SIZE * (uint16_t)Count;
		if (Message == nullptr) return Length + UBX_FRAME_OVERHEAD;
		uint8_t* P = Message + 6;	// build the payload in place, then add the frame around it
		P[0] = 0;				// msgVer
		P[1] = 0;				// numTrkChHw (read-only)
		P[2] = NumTrkChUse;
		P[3] = Count;
		if (Count) memcpy(P + HEADER_SIZE, Blocks, BLOCK_SIZE * (size_t)Count);
		return UBX_FinishFrame(Message, MSG_CLASS, MSG_ID, Length);
	}
	// Makes a block: gnssId (0 GPS, 1 SBAS, 2 Galileo, 3 BeiDou, 5 QZSS, 6 GLONASS), channels, enable.
	// SigCfgMask 0x01 selects the primary signal (L1) for GPS/Galileo/BeiDou-style defaults.
	static Block MakeBlock(uint8_t GnssId, uint8_t ResTrkCh, uint8_t MaxTrkCh, bool Enable, uint8_t SigCfgMask = 0x01) {
		Block B;
		memset(&B, 0, sizeof(B));
		B.gnssId = GnssId;
		B.resTrkCh = ResTrkCh;
		B.maxTrkCh = MaxTrkCh;
		B.flags = (Enable ? 1u : 0u) | ((uint32_t)SigCfgMask << 16);
		return B;
	}

	uint16_t ExpectedLength() const { return HEADER_SIZE + BLOCK_SIZE * (uint16_t)numConfigBlocks; }
	bool LengthOK(uint16_t PayloadLength) const { return PayloadLength >= HEADER_SIZE && PayloadLength == ExpectedLength(); }
	const Block* Cfg(uint8_t i) const { return (i < numConfigBlocks) ? (&Blocks) + i : nullptr; }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("     Type: CFG-GNSS\n");
		ARGPRINT("   msgVer: %u\n", (unsigned)msgVer);
		ARGPRINT("  trkChHw: %u (use %u)\n", (unsigned)numTrkChHw, (unsigned)numTrkChUse);
		ARGPRINT("   blocks: %u\n", (unsigned)numConfigBlocks);
		if (DetailLevel > 0) for (uint8_t i = 0; i < numConfigBlocks; i++) Cfg(i)->Print(DetailLevel);
	}
};
static_assert(sizeof(UBX_CFG_GNSS::Block) == UBX_CFG_GNSS::BLOCK_SIZE, "UBX_CFG_GNSS::Block must match the 8-byte wire format");
static_assert(sizeof(UBX_CFG_GNSS) == UBX_CFG_GNSS::HEADER_SIZE + UBX_CFG_GNSS::BLOCK_SIZE, "UBX_CFG_GNSS header must be 4 bytes");
