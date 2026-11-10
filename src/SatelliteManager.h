#pragma once
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cinttypes>
#include "GNSS_Config.h"
#include "GNSS_System.h"
#include "NMEA/Satellites/NMEA_GSV.h"
#include "UBX/NAV/UBX_NAV_SAT.h"

// Tracks the satellites reported by NMEA GSV and UBX NAV-SAT and turns them into sky-plot
// coordinates. It does no drawing: feed it messages, call BuildPlot(radius), and draw Coords[]
// however you like (see examples/SkyPlot).
//
// A satellite is identified by (System, ID) where ID is the per-system number (GPS 1-32,
// Galileo 1-36, ...), so equal numbers in different constellations stay separate and the same
// satellite reported by both NMEA and UBX is merged into one entry.
//
// Not thread-safe: call everything from one context.
class SatelliteManager {
	static const uint32_t SAT_TIMEOUT = 10000;	// ms without a report before a satellite is dropped
public:
	static const int MAX_SATS = 64;

	// Position of one satellite on the plot, relative to the centre of the sky circle.
	struct DisplayCoord {
		GNSS_System System = GNSS_System::Unknown;
		uint16_t ID = 0;
		uint16_t Elev = 0;		// 0..90 degrees
		uint16_t Azim = 0;		// 0..360 degrees, 0 = north
		int16_t X = 0, Y = 0;	// pixels, +X east (right), +Y north (up)
		bool Used = false;		// used in the position solution (UBX only)
		bool Tracked = false;	// signal strength > 0

		void Build(GNSS_System Sys, uint16_t I, uint16_t E, uint16_t A, uint16_t Radius) {
			System = Sys; ID = I; Elev = (E > 90) ? 90 : E; Azim = A;
			const float Range = (Radius / 90.0f) * (90 - Elev);	// zenith at the centre, horizon on the rim
			const float Angle = Azim * 0.017453292519943295f;	// degrees -> radians
			X = (int16_t)(Range * sinf(Angle));
			Y = (int16_t)(Range * cosf(Angle));
		}

		// "G05" style label (system letter + number); Cap >= 8.
		const char* Label(char* Out, size_t Cap) const {
			snprintf(Out, Cap, "%c%02u", GNSS_System_Letter(System), (unsigned)ID);
			return Out;
		}
		const char* ToString(char* Out, size_t Cap) const {
			snprintf(Out, Cap, "%s ID: %3u Elv: %3u Azi: %3u X: %4d Y: %4d", GNSS_System_ToString(System), (unsigned)ID, (unsigned)Elev, (unsigned)Azim, (int)X, (int)Y);
			return Out;
		}
	};

	struct Satellite {
		uint32_t LastUpdate = 0;	// GNSS_MILLIS() of the last report
		uint16_t ID = 0;			// number within its system
		int16_t Elevation = 0;		// degrees; negative = below the horizon
		uint16_t Azimuth = 0;		// degrees, 0 = north
		uint8_t SNR = 0;			// C/N0 in dBHz, 0 = not tracked
		GNSS_System System = GNSS_System::Unknown;	// value is the u-blox gnssId
		bool Active = false;
		bool PosKnown = false;		// elevation / azimuth are valid
		bool Used = false;			// used in the position solution (UBX only)

		// Worth drawing: reported recently, position known, above the horizon.
		bool Plottable() const { return Active && PosKnown && Elevation >= 0 && Elevation <= 90 && Azimuth <= 360; }

		const char* ToString(char* Out, size_t Cap) const {
			snprintf(Out, Cap, "%c%02u %-7s Elv: %3d Azi: %3u SNR: %3u Used: %c Active: %c Upd: %010" PRIu32,
				GNSS_System_Letter(System), (unsigned)ID, GNSS_System_ToString(System), (int)Elevation, (unsigned)Azimuth, (unsigned)SNR,
				Used ? 'T' : 'F', Active ? 'T' : 'F', LastUpdate);
			return Out;
		}
	};

	Satellite Satellites[MAX_SATS];
	DisplayCoord Coords[MAX_SATS];		// filled by BuildPlot()

	// Adds or refreshes one satellite. Used < 0 leaves the "used in solution" flag unchanged
	// (NMEA GSV doesn't carry it). Returns the table index, or -1 if the id is invalid.
	int16_t AddSatellite(GNSS_System System, uint16_t ID, int16_t Elevation, uint16_t Azimuth, uint8_t SNR, bool PosKnown = true, int8_t Used = -1) {
		if (ID == 0 || ID >= 255) return -1;
		AgeOutSatellites();

		int16_t Index = -1;
		for (int i = 0; i < MAX_SATS; i++) {
			if (Satellites[i].Active && Satellites[i].ID == ID && Satellites[i].System == System) { Index = i; break; }
		}
		if (Index < 0) {	// new satellite: take a free slot, or replace the stalest one if the table is full
			uint32_t Oldest = 0;
			const uint32_t Now = GNSS_MILLIS();
			for (int i = 0; i < MAX_SATS; i++) {
				if (!Satellites[i].Active) { Index = i; break; }
				const uint32_t Age = Now - Satellites[i].LastUpdate;
				if (Index < 0 || Age > Oldest) { Index = i; Oldest = Age; }
			}
		}

		Satellite& S = Satellites[Index];
		const bool IsNew = !(S.Active && S.ID == ID && S.System == System);
		S.System = System;
		S.ID = ID;
		S.Elevation = Elevation;
		S.Azimuth = Azimuth;
		S.SNR = SNR;
		S.PosKnown = PosKnown;
		if (Used >= 0) S.Used = (Used != 0); else if (IsNew) S.Used = false;
		S.Active = true;
		S.LastUpdate = GNSS_MILLIS();
		return Index;
	}

	void AgeOutSatellites() {
		const uint32_t Now = GNSS_MILLIS();
		for (int i = 0; i < MAX_SATS; i++) {
			if (Satellites[i].Active && (Now - Satellites[i].LastUpdate) > SAT_TIMEOUT) Satellites[i].Active = false;
		}
	}

	// Number of satellites currently reported (plottable or not).
	uint16_t NumSats() const {
		uint16_t Count = 0;
		for (int i = 0; i < MAX_SATS; i++) if (Satellites[i].Active) Count++;
		return Count;
	}

	// One NMEA GSV sentence. Header is the sentence header including its talker ("GPGSV",
	// "GAGSV", ...), i.e. NMEA_Helper::Header(). Satellites with empty elevation / azimuth are
	// recorded but not plotted until the receiver reports where they are.
	void AddGSV(NMEA_GSV* GSV, const char* Header) {
		for (int i = 0; i < GSV->SatsInMessage(); i++) {
			GNSS_System System; uint16_t ID;
			GNSS_ResolveNMEA(Header, GSV->PRN(i), System, ID);
			AddSatellite(System, ID, GSV->Elevation(i), GSV->Azimuth(i), (uint8_t)GSV->SNR(i), GSV->PositionKnown(i));
		}
	}

	// One UBX NAV-SAT message (check LengthOK() first).
	void AddNAVSAT(const UBX_NAV_SAT* NS) {
		if (NS == nullptr) return;
		for (uint8_t i = 0; i < NS->numSvs; i++) {
			const UBX_NAV_SAT::SV* SV = NS->Sv(i);
			// elev is a signed byte, and unknown when outside +/-90; azim is unknown when elev is
			const bool PosKnown = (SV->elev >= -90 && SV->elev <= 90 && SV->azim >= 0 && SV->azim <= 360);
			AddSatellite(GNSS_System_FromGnssId(SV->gnssId), SV->svId, SV->elev, (uint16_t)SV->azim, SV->cno, PosKnown, SV->svUsed() ? 1 : 0);
		}
	}

	// Ages out stale satellites, then fills Coords[] with the ones worth drawing for a sky circle
	// of the given pixel radius. Returns how many entries of Coords[] are valid.
	uint16_t BuildPlot(uint16_t Radius) {
		AgeOutSatellites();
		uint16_t Count = 0;
		for (int i = 0; i < MAX_SATS; i++) {
			const Satellite& S = Satellites[i];
			if (!S.Plottable()) continue;
			Coords[Count].Build(S.System, S.ID, (uint16_t)S.Elevation, S.Azimuth, Radius);
			Coords[Count].Used = S.Used;
			Coords[Count].Tracked = (S.SNR > 0);
			Count++;
		}
		return Count;
	}

	// One line per active satellite into Out; truncates at Capacity. Returns Out.
	const char* SatellitesToString(char* Out, size_t Capacity) const {
		size_t Used = 0;
		if (Capacity) Out[0] = 0;
		char Line[128];
		for (int i = 0; i < MAX_SATS; i++) {
			if (!Satellites[i].Active) continue;
			Satellites[i].ToString(Line, sizeof(Line));
			const int N = snprintf(Out + Used, Capacity - Used, "%s\n", Line);
			if (N < 0 || (size_t)N >= Capacity - Used) break;
			Used += N;
		}
		return Out;
	}
};
