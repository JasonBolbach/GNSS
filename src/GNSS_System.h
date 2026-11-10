#pragma once
#include <cstdint>

// The satellite navigation system a satellite belongs to. The values are the u-blox
// "gnssId" used by UBX messages (UBX-NAV-SAT, CFG-GNSS, ...), so a UBX gnssId can be
// cast straight to this type.
enum class GNSS_System : uint8_t {
	GPS     = 0,
	SBAS    = 1,
	Galileo = 2,
	BeiDou  = 3,
	IMES    = 4,
	QZSS    = 5,
	GLONASS = 6,
	Unknown = 255
};

inline GNSS_System GNSS_System_FromGnssId(uint8_t GnssId) {
	return (GnssId <= (uint8_t)GNSS_System::GLONASS) ? (GNSS_System)GnssId : GNSS_System::Unknown;
}

inline const char* GNSS_System_ToString(GNSS_System S) {
	switch (S) {
	case GNSS_System::GPS: return "GPS";
	case GNSS_System::SBAS: return "SBAS";
	case GNSS_System::Galileo: return "Galileo";
	case GNSS_System::BeiDou: return "BeiDou";
	case GNSS_System::IMES: return "IMES";
	case GNSS_System::QZSS: return "QZSS";
	case GNSS_System::GLONASS: return "GLONASS";
	default: return "Unknown";
	}
}

// One-letter code as used by RINEX satellite names (G05, R12, E24, C07, J02, S33).
inline char GNSS_System_Letter(GNSS_System S) {
	switch (S) {
	case GNSS_System::GPS: return 'G';
	case GNSS_System::SBAS: return 'S';
	case GNSS_System::Galileo: return 'E';
	case GNSS_System::BeiDou: return 'C';
	case GNSS_System::IMES: return 'I';
	case GNSS_System::QZSS: return 'J';
	case GNSS_System::GLONASS: return 'R';
	default: return '?';
	}
}

// Maps an NMEA satellite number to (system, per-system id), using the numbering that
// the UBX messages use (GPS 1-32, SBAS PRN 120-158, GLONASS 1-24, Galileo 1-36, BeiDou 1-63,
// QZSS 1-10), so GSV and UBX-NAV-SAT report the same satellite under the same key.
//   Header: the sentence header including talker, e.g. "GPGSV" or "GAGSV".
//   NMEA numbers: GPS 1-32, SBAS 33-64 (PRN - 87), GLONASS 65-96, QZSS 193-202,
//                 and for the combined "GN" talker Galileo 301-336, BeiDou 401-437.
inline void GNSS_ResolveNMEA(const char* Header, uint16_t Raw, GNSS_System& System, uint16_t& Id) {
	const char T0 = Header ? Header[0] : 0, T1 = (Header && Header[0]) ? Header[1] : 0;
	System = GNSS_System::Unknown;
	Id = Raw;
	if (T0 != 'G' && !(T0 == 'B' && T1 == 'D')) return;

	if (T1 == 'L') { System = GNSS_System::GLONASS; if (Raw >= 65 && Raw <= 96) Id = Raw - 64; return; }
	if (T1 == 'A') { System = GNSS_System::Galileo; if (Raw >= 301) Id = Raw - 300; return; }
	if (T1 == 'B' || T0 == 'B') { System = GNSS_System::BeiDou; if (Raw >= 401) Id = Raw - 400; return; }
	if (T1 == 'Q') { System = GNSS_System::QZSS; if (Raw >= 193) Id = Raw - 192; return; }

	// "GP" (GPS + SBAS + QZSS + legacy GLONASS) and "GN" (combined): decide by number range.
	if (T1 != 'P' && T1 != 'N') return;
	if (Raw >= 1 && Raw <= 32) System = GNSS_System::GPS;
	else if (Raw >= 33 && Raw <= 64) { System = GNSS_System::SBAS; Id = Raw + 87; }
	else if (Raw >= 65 && Raw <= 96) { System = GNSS_System::GLONASS; Id = Raw - 64; }
	else if (Raw >= 193 && Raw <= 202) { System = GNSS_System::QZSS; Id = Raw - 192; }
	else if (T1 == 'N' && Raw >= 301 && Raw <= 336) { System = GNSS_System::Galileo; Id = Raw - 300; }
	else if (T1 == 'N' && Raw >= 401 && Raw <= 437) { System = GNSS_System::BeiDou; Id = Raw - 400; }
}
