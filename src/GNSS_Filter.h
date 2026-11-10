#pragma once
#include <cstdint>

// Byte-at-a-time NMEA / UBX framer. Single-context object: call putch() from one
// task only (feed it from the consumer side of the SPSC queue).
class GNSS_Filter {
public:
	enum class GNSS_Result : uint8_t { Processing = 0, UBX_Complete = 1, NMEA_Complete = 2, ChecksumMismatch = 3, OutOfSync = 4, Overflow = 5 };

	// Largest frame we can hold. NMEA needs ~82 + CRLF + NUL; UBX needs 8 + payload length.
	static const uint16_t PAYLOAD_MAX = 1024;

private:
	enum class GNSS_State : uint8_t { Idle = 0, NMEA = 1, UBX = 2 };
	enum class NMEA_State : uint8_t { Header = 0, Payload = 1, ChecksumA = 2, ChecksumB = 3 };
	enum class UBX_State  : uint8_t { WaitB = 0, Class = 1, ID = 2, LenL = 3, LenH = 4, Payload = 5, ChecksumA = 6, ChecksumB = 7 };

	// Storage is offset by 2 so that Payload + 6 (the start of a UBX message body)
	// lands on a 4-byte boundary. This makes casting the body to a UBX_* struct
	// safe on cores that fault on unaligned loads (Xtensa, Cortex-M0, ...).
	alignas(4) uint8_t PayloadStore[PAYLOAD_MAX + 4] = { 0 };

	GNSS_State sGNSS = GNSS_State::Idle;
	NMEA_State sNMEA = NMEA_State::Header;
	UBX_State  sUBX  = UBX_State::WaitB;
	uint8_t NMEA_CheckSum = 0;
	uint8_t NMEA_MessageCheckSum = 0;
	uint8_t UBX_Class = 0;
	uint8_t UBX_ID = 0;
	uint8_t UBX_ChecksumA = 0;
	uint8_t UBX_ChecksumB = 0;
	uint16_t UBX_Len = 0;
	uint16_t UBX_Payload_Counter = 0;

	static bool IsNum(uint8_t c) { return (c >= '0' && c <= '9'); }
	static bool IsAlpha(uint8_t c) { return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')); }
	static bool IsAlphaNum(uint8_t c) { return (IsAlpha(c) || IsNum(c)); }
	static bool IsNMEA(uint8_t c) {
		if (c == ',' || c == '-' || c == '.' || c == '*' || c == ' ') return true;
		return IsAlphaNum(c);
	}
	static uint8_t H2D(uint8_t c) {
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		return 0;
	}
	void UBX_UpdateChecksum(uint8_t c) { UBX_ChecksumA += c; UBX_ChecksumB += UBX_ChecksumA; }

	GNSS_Result putch_UBX(uint8_t c);
	GNSS_Result putch_NMEA(uint8_t c);

public:
	GNSS_Filter() : Payload(PayloadStore + 2) {}
	GNSS_Filter(const GNSS_Filter&) = delete;
	GNSS_Filter& operator=(const GNSS_Filter&) = delete;

	// Completed frame. NMEA: "$....*hh\r\n" + NUL terminator. UBX: B5 62 cls id lenL lenH body ckA ckB.
	// Valid after NMEA_Complete / UBX_Complete until the next frame starts.
	uint8_t* const Payload;
	uint16_t Payload_Size = 0;

	void Reset() {
		sGNSS = GNSS_State::Idle;
		Payload_Size = 0;

		sNMEA = NMEA_State::Header;
		NMEA_CheckSum = 0;
		NMEA_MessageCheckSum = 0;

		sUBX = UBX_State::WaitB;
		UBX_Payload_Counter = 0;
		UBX_Class = 0;
		UBX_ID = 0;
		UBX_Len = 0;
		UBX_ChecksumA = 0;
		UBX_ChecksumB = 0;
	}

	static const char* GNSS_Result_ToString(GNSS_Result Result) {
		switch (Result) {
		case GNSS_Result::Processing: return "Processing";
		case GNSS_Result::UBX_Complete: return "UBX Complete";
		case GNSS_Result::NMEA_Complete: return "NMEA Complete";
		case GNSS_Result::ChecksumMismatch: return "Checksum Mismatch";
		case GNSS_Result::OutOfSync: return "Out of Sync";
		case GNSS_Result::Overflow: return "Overflow";
		default: return "Unknown Result";
		}
	}

	GNSS_Result putch(uint8_t c);
};

inline GNSS_Filter::GNSS_Result GNSS_Filter::putch(uint8_t c) {
	switch (sGNSS) {
	case GNSS_State::Idle:
		if (c == '$') {
			Reset();
			Payload[Payload_Size++] = c;
			sGNSS = GNSS_State::NMEA;
		}
		else if (c == 0xb5) {
			Reset();
			Payload[Payload_Size++] = c;
			sGNSS = GNSS_State::UBX;
		}
		return GNSS_Result::Processing;

	case GNSS_State::NMEA:
		if (c == '$') {	// new sentence start mid-frame: resync on it instead of dropping it
			Reset();
			Payload[Payload_Size++] = c;
			sGNSS = GNSS_State::NMEA;
			return GNSS_Result::OutOfSync;
		}
		if (!IsNMEA(c)) {
			Reset();
			return GNSS_Result::OutOfSync;
		}
		// Need room for this char + CR + LF + NUL.
		if (Payload_Size + 4 > PAYLOAD_MAX) {
			Reset();
			return GNSS_Result::Overflow;
		}
		Payload[Payload_Size++] = c;
		if (putch_NMEA(c) == GNSS_Result::NMEA_Complete) {
			Payload[Payload_Size++] = 0x0d;
			Payload[Payload_Size++] = 0x0a;
			Payload[Payload_Size] = 0x00;	// terminator, not counted in Payload_Size
			return GNSS_Result::NMEA_Complete;
		}
		return GNSS_Result::Processing;

	case GNSS_State::UBX:
		if (Payload_Size >= PAYLOAD_MAX) {	// belt and braces; UBX_Len is validated below
			Reset();
			return GNSS_Result::Overflow;
		}
		Payload[Payload_Size++] = c;
		return putch_UBX(c);
	}
	Reset();
	return GNSS_Result::OutOfSync;
}

inline GNSS_Filter::GNSS_Result GNSS_Filter::putch_UBX(uint8_t c) {
	switch (sUBX) {
	case UBX_State::WaitB:
		if (c == 0x62) {
			sUBX = UBX_State::Class;
			return GNSS_Result::Processing;
		}
		Reset();
		return GNSS_Result::OutOfSync;

	case UBX_State::Class:
		UBX_Class = c;
		UBX_UpdateChecksum(c);
		sUBX = UBX_State::ID;
		return GNSS_Result::Processing;

	case UBX_State::ID:
		UBX_ID = c;
		UBX_UpdateChecksum(c);
		sUBX = UBX_State::LenL;
		return GNSS_Result::Processing;

	case UBX_State::LenL:
		UBX_Len = c;
		UBX_UpdateChecksum(c);
		sUBX = UBX_State::LenH;
		return GNSS_Result::Processing;

	case UBX_State::LenH:
		UBX_Len |= ((uint16_t)c) << 8;
		UBX_UpdateChecksum(c);
		// Frame = 6 header bytes + body + 2 checksum bytes.
		if (UBX_Len > PAYLOAD_MAX - 8) {
			Reset();
			return GNSS_Result::Overflow;
		}
		sUBX = (UBX_Len == 0) ? UBX_State::ChecksumA : UBX_State::Payload;
		return GNSS_Result::Processing;

	case UBX_State::Payload:
		UBX_Payload_Counter++;
		UBX_UpdateChecksum(c);
		if (UBX_Payload_Counter >= UBX_Len) sUBX = UBX_State::ChecksumA;
		return GNSS_Result::Processing;

	case UBX_State::ChecksumA:
		if (c == UBX_ChecksumA) {
			sUBX = UBX_State::ChecksumB;
			return GNSS_Result::Processing;
		}
		Reset();
		return GNSS_Result::ChecksumMismatch;

	case UBX_State::ChecksumB:
		if (c == UBX_ChecksumB) {
			sGNSS = GNSS_State::Idle;
			return GNSS_Result::UBX_Complete;
		}
		Reset();
		return GNSS_Result::ChecksumMismatch;
	}
	Reset();
	return GNSS_Result::OutOfSync;
}

inline GNSS_Filter::GNSS_Result GNSS_Filter::putch_NMEA(uint8_t c) {
	switch (sNMEA) {
	case NMEA_State::Header:
		NMEA_CheckSum ^= c;
		if (c == ',') sNMEA = NMEA_State::Payload;
		break;

	case NMEA_State::Payload:
		if (c == '*') sNMEA = NMEA_State::ChecksumA;
		else NMEA_CheckSum ^= c;
		break;

	case NMEA_State::ChecksumA:
		NMEA_MessageCheckSum = H2D(c) << 4;
		sNMEA = NMEA_State::ChecksumB;
		break;

	case NMEA_State::ChecksumB:
		NMEA_MessageCheckSum |= H2D(c) & 0x0f;
		if (NMEA_CheckSum == NMEA_MessageCheckSum) {
			sGNSS = GNSS_State::Idle;
			return GNSS_Result::NMEA_Complete;
		}
		Reset();
		return GNSS_Result::ChecksumMismatch;
	}
	return GNSS_Result::Processing;
}
