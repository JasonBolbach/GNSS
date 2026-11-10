#pragma once
#include "UBX/UBX_Common.h"

// UBX-NAV-SAT (0x01 0x35): 8-byte header followed by numSvs 12-byte blocks.
// Overlay it on a payload whose start is 4-byte aligned (GNSS_Filter guarantees this).
struct UBX_NAV_SAT {
	static constexpr uint8_t  MSG_CLASS = 0x01;
	static constexpr uint8_t  MSG_ID    = 0x35;
	static constexpr uint16_t CLASSID = 0x3501;	// (ID << 8) | Class

	// Writes a poll request to Message and returns its size in bytes (8). Send it to the receiver to get this
	// message once. Message == nullptr writes nothing and just returns the size.
	static uint16_t ToPoll(uint8_t* Message) { return UBX_BuildPoll(Message, MSG_CLASS, MSG_ID); }

	struct SV {
		uint8_t		gnssId;		// GNSS identifier
		uint8_t		svId;		// Satellite identifier
		uint8_t		cno;		// Carrier to noise ratio (dBHz)
		int8_t		elev;		// Elevation (range: +/-90), unknown if out of range
		int16_t		azim;		// Azimuth (range 0-360), unknown if elevation is out of range
		int16_t		prRes;		// Pseudorange residual (0.1 m)
		uint32_t	flags;		// Bitmask, see Flag_* below

		// flags layout
		static constexpr uint32_t Flag_QualityMask   = 0x00000007;	// bits 0-2
		static constexpr uint32_t Flag_SvUsed        = 0x00000008;
		static constexpr uint32_t Flag_HealthShift   = 4;			// bits 4-5
		static constexpr uint32_t Flag_DiffCorr      = 0x00000040;
		static constexpr uint32_t Flag_Smoothed      = 0x00000080;
		static constexpr uint32_t Flag_OrbitShift    = 8;			// bits 8-10
		static constexpr uint32_t Flag_EphAvail      = 0x00000800;
		static constexpr uint32_t Flag_AlmAvail      = 0x00001000;
		static constexpr uint32_t Flag_AnoAvail      = 0x00002000;
		static constexpr uint32_t Flag_AopAvail      = 0x00004000;
		static constexpr uint32_t Flag_SbasCorrUsed  = 0x00010000;
		static constexpr uint32_t Flag_RtcmCorrUsed  = 0x00020000;
		static constexpr uint32_t Flag_SlasCorrUsed  = 0x00040000;
		static constexpr uint32_t Flag_SpartnCorrUsed= 0x00080000;
		static constexpr uint32_t Flag_PrCorrUsed    = 0x00100000;
		static constexpr uint32_t Flag_CrCorrUsed    = 0x00200000;
		static constexpr uint32_t Flag_DoCorrUsed    = 0x00400000;
		static constexpr uint32_t Flag_ClasCorrUsed  = 0x00800000;

		uint8_t qualityInd() const { return flags & Flag_QualityMask; }
		bool    svUsed() const { return (flags & Flag_SvUsed) != 0; }
		uint8_t health() const { return (flags >> Flag_HealthShift) & 0x03; }
		uint8_t orbitSource() const { return (flags >> Flag_OrbitShift) & 0x07; }
		bool    has(uint32_t Flag) const { return (flags & Flag) != 0; }

		static const char* YesNo(bool B) { return B ? "True" : "False"; }

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

		const char* s_flag_QualityInd() const {
			switch (qualityInd()) {
			case 0: return "None";
			case 1: return "Search";
			case 2: return "Acquired";
			case 3: return "Unstable";
			case 4: return "Lock (CT)";
			default: return "Lock (CRT)";	// 5, 6, 7
			}
		}
		const char* s_flag_svUsed() const { return YesNo(svUsed()); }
		const char* s_flag_health() const {
			switch (health()) {
			case 1: return "Healthy";
			case 2: return "Unhealthy";
			default: return "Unknown";	// 0, and reserved value 3
			}
		}
		const char* s_flag_diffCorr() const { return YesNo(has(Flag_DiffCorr)); }
		const char* s_flag_smoothed() const { return YesNo(has(Flag_Smoothed)); }
		const char* s_flag_orbitSource() const {
			switch (orbitSource()) {
			case 0: return "None";
			case 1: return "Ephemeris";
			case 2: return "Almanac";
			case 3: return "AssistNow Offline";
			case 4: return "AssistNow Autonomous";
			default: return "Other";	// 5, 6, 7
			}
		}
		const char* s_flag_ephAvail() const { return YesNo(has(Flag_EphAvail)); }
		const char* s_flag_almAvail() const { return YesNo(has(Flag_AlmAvail)); }
		const char* s_flag_anoAvail() const { return YesNo(has(Flag_AnoAvail)); }
		const char* s_flag_aopAvail() const { return YesNo(has(Flag_AopAvail)); }
		const char* s_flag_sbasCorrUsed() const { return YesNo(has(Flag_SbasCorrUsed)); }
		const char* s_flag_rtcmCorrUsed() const { return YesNo(has(Flag_RtcmCorrUsed)); }
		const char* s_flag_slasCorrUsed() const { return YesNo(has(Flag_SlasCorrUsed)); }
		const char* s_flag_spartnCorrUsed() const { return YesNo(has(Flag_SpartnCorrUsed)); }
		const char* s_flag_prCorrUsed() const { return YesNo(has(Flag_PrCorrUsed)); }
		const char* s_flag_crCorrUsed() const { return YesNo(has(Flag_CrCorrUsed)); }
		const char* s_flag_doCorrUsed() const { return YesNo(has(Flag_DoCorrUsed)); }
		const char* s_flag_clasCorrUsed() const { return YesNo(has(Flag_ClasCorrUsed)); }

		void Print(uint8_t DetailLevel = 0) const {
			ARGPRINT("\tgnssId:%7s svId:%03d cno:%02d elev:%4d azim:%03d prRes:%5d flags:0x%08" PRIx32 "\n", s_gnssId(), svId, cno, elev, azim, prRes, flags);
			if (DetailLevel > 1) PrintFlags();
		}

		void PrintFlags() const {
			ARGPRINT("\t\t    QualityInd:%s\n", s_flag_QualityInd());
			ARGPRINT("\t\t        svUsed:%s\n", s_flag_svUsed());
			ARGPRINT("\t\t        health:%s\n", s_flag_health());
			ARGPRINT("\t\t      diffCorr:%s\n", s_flag_diffCorr());
			ARGPRINT("\t\t      smoothed:%s\n", s_flag_smoothed());
			ARGPRINT("\t\t   orbitSource:%s\n", s_flag_orbitSource());
			ARGPRINT("\t\t      ephAvail:%s\n", s_flag_ephAvail());
			ARGPRINT("\t\t      almAvail:%s\n", s_flag_almAvail());
			ARGPRINT("\t\t      anoAvail:%s\n", s_flag_anoAvail());
			ARGPRINT("\t\t      aopAvail:%s\n", s_flag_aopAvail());
			ARGPRINT("\t\t  sbasCorrUsed:%s\n", s_flag_sbasCorrUsed());
			ARGPRINT("\t\t  rtcmCorrUsed:%s\n", s_flag_rtcmCorrUsed());
			ARGPRINT("\t\t  slasCorrUsed:%s\n", s_flag_slasCorrUsed());
			ARGPRINT("\t\tspartnCorrUsed:%s\n", s_flag_spartnCorrUsed());
			ARGPRINT("\t\t    prCorrUsed:%s\n", s_flag_prCorrUsed());
			ARGPRINT("\t\t    crCorrUsed:%s\n", s_flag_crCorrUsed());
			ARGPRINT("\t\t    doCorrUsed:%s\n", s_flag_doCorrUsed());
			ARGPRINT("\t\t  clasCorrUsed:%s\n", s_flag_clasCorrUsed());
		}
	};

	static constexpr uint16_t HEADER_SIZE = 8;
	static constexpr uint16_t SV_SIZE = 12;

	uint32_t	iTOW;			// GPS time of week of the navigation epoch.
	uint8_t		version;		// Message version
	uint8_t		numSvs;			// Number of satellites
	uint16_t	reserved;		// Reserved
	SV			Svs;			// Start of repeated block (numSvs times); use Sv(i)

	// Payload length this header promises. Compare with the received length before
	// touching the SV blocks.
	uint16_t ExpectedLength() const { return HEADER_SIZE + SV_SIZE * (uint16_t)numSvs; }
	bool LengthOK(uint16_t PayloadLength) const { return PayloadLength >= HEADER_SIZE && PayloadLength == ExpectedLength(); }

	// Bounds-checked access to the repeated block; nullptr when i >= numSvs.
	const SV* Sv(uint8_t i) const { return (i < numSvs) ? (&Svs) + i : nullptr; }

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("    Type: NAV-SAT\n");
		ARGPRINT("    iTOW: %" PRIu32 "\n", iTOW);
		ARGPRINT(" Version: %d\n", version);
		ARGPRINT("  numSvs: %d\n", numSvs);
		ARGPRINT("reserved: %04x\n", reserved);

		if (DetailLevel > 0) {
			ARGPRINT("     SVs:\n");
			for (uint8_t i = 0; i < numSvs; i++) Sv(i)->Print(DetailLevel);
		}
	}
};
static_assert(sizeof(UBX_NAV_SAT::SV) == UBX_NAV_SAT::SV_SIZE, "UBX_NAV_SAT::SV must match the 12-byte wire format");
static_assert(sizeof(UBX_NAV_SAT) == UBX_NAV_SAT::HEADER_SIZE + UBX_NAV_SAT::SV_SIZE, "UBX_NAV_SAT header must be 8 bytes");
