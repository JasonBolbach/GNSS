#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "GNSS_Config.h"

class NMEA_Helper {
	static const uint8_t MAX_FIELDS = 32;
	char pSentence[128] = "";
	uint8_t pFieldIndex[MAX_FIELDS] = { 0 };
	uint8_t pFieldCount = 0;
public:
	NMEA_Helper() {}
	NMEA_Helper(char* Sentence) {
		strncpy(pSentence, Sentence, sizeof(pSentence) - 1);
		pSentence[sizeof(pSentence) - 1] = '\0';
	}

	uint8_t Parse(uint8_t * Sentence) {
		uint8_t StartIndex = 1;
		pFieldCount = 0;
		for (uint8_t Index = StartIndex; Sentence[Index] != '\0' && Index < 128 && pFieldCount < MAX_FIELDS;Index++) {
			if (Sentence[Index] == ',' || Sentence[Index] == '*') {
				Sentence[Index] = '\0';
				pFieldIndex[pFieldCount++] = StartIndex;
				StartIndex = Index + 1;
			}
			if (Sentence[Index] == '*') break;
			pSentence[Index] = Sentence[Index];
		}
		return pFieldCount;
	}

	const char* Field(uint8_t Index) {
		if (Index >= pFieldCount) return "";
		return (const char*)pSentence + pFieldIndex[Index];
	}
	void PrintFields() {
		for (uint8_t i = 0; i < pFieldCount; i++) {
			ARGPRINT("\tField %d: %s\r\n", i, Field(i));
		}
	}
	bool HasField(uint8_t Index) { return Index < pFieldCount && Field(Index)[0] != '\0'; }

	// Parses a ddmm.mmmm / dddmm.mmmm field plus its N/S/E/W hemisphere field (Index + 1)
	// into signed decimal degrees. Returns false (Degrees untouched) if either field is
	// missing or empty, which is what a receiver without a fix sends.
	bool FieldAsLatLon(uint8_t Index, double& Degrees) {
		if (!HasField(Index) || !HasField(Index + 1)) return false;
		const double Raw = atof(Field(Index));
		const double Whole = (double)((int32_t)(Raw / 100.0));	// degrees
		const double Value = Whole + (Raw - Whole * 100.0) / 60.0;
		const char H = Field(Index + 1)[0];
		Degrees = (H == 'S' || H == 'W') ? -Value : Value;
		return true;
	}
	// Convenience form: 0.0 when there is no position. Prefer the bool overload (or a
	// sentence's Valid()) when 0,0 must not be mistaken for a real fix.
	double FieldAsLatLon(uint8_t Index) {
		double Degrees = 0.0;
		FieldAsLatLon(Index, Degrees);
		return Degrees;
	}
	double FieldAsDouble(uint8_t Index) {
		if (Index >= pFieldCount) return 0.0f;
		return atof(Field(Index));
	}
	int16_t FieldAsInt(uint8_t Index) {
		if (Index >= pFieldCount) return 0;
		return (int16_t)atoi(Field(Index));
	}
	void Print() { ARGPRINT("%s %d\r\n", Header(),pFieldCount); }
	const char* Header() { return Field(0); }
};

// Non-owning typed view over a parsed NMEA_Helper (one pointer wide, no vtable).
// The helper must outlive the view and must not be re-Parse()d while the view is in use.
class NMEA_SentenceBase {
protected:
	NMEA_Helper& pHelper;
	explicit NMEA_SentenceBase(NMEA_Helper& H) : pHelper(H) {}
};
