#pragma once
#include "NMEA/NMEA_Helper.h"

class NMEA_GSV : public NMEA_SentenceBase {
public:
	NMEA_GSV(NMEA_Helper& H) : NMEA_SentenceBase(H) {}

	uint8_t TotalMessages() { return pHelper.FieldAsInt(1); }
	uint8_t MessageNumber() { return pHelper.FieldAsInt(2); }
	uint8_t TotalSatsInView() { return pHelper.FieldAsInt(3); }
	uint8_t SatsInMessage() {
		// Sats left to report = total - 4 per earlier message; clamp to this message's 0..4.
		// (The old "total % 4" returned 0 for a final message that held exactly 4 sats.)
		int Remaining = (int)TotalSatsInView() - ((int)MessageNumber() - 1) * 4;
		return (Remaining < 0) ? 0 : (Remaining > 4 ? 4 : Remaining);
	}
	uint16_t PRN(int Index) { if (Index >= SatsInMessage() || Index < 0) return 0; return pHelper.FieldAsInt((Index * 4) + 4); }
	uint16_t Elevation(int Index) { if (Index >= SatsInMessage() || Index < 0) return 0; return pHelper.FieldAsInt((Index * 4) + 5); }
	uint16_t Azimuth(int Index) { if (Index >= SatsInMessage() || Index < 0) return 0; return pHelper.FieldAsInt((Index * 4) + 6); }
	uint16_t SNR(int Index) { if (Index >= SatsInMessage() || Index < 0) return 0; return pHelper.FieldAsInt((Index * 4) + 7); }

	void Print() {
		ARGPRINT("GSV: TotalMessages: %d, MessageNumber: %d, TotalSatsInView: %d, SatsInMessage: %d\r\n", TotalMessages(), MessageNumber(), TotalSatsInView(), SatsInMessage());
		for (int i = 0; i < SatsInMessage(); i++) {
			ARGPRINT("\tSat %d: PRN: %d, Elevation: %d, Azimuth: %d, SNR: %d\r\n", i + 1, PRN(i), Elevation(i), Azimuth(i), SNR(i));
		}
	}
};
