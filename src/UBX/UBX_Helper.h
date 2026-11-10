#pragma once
#include "UBX/UBX_Common.h"
#include "UBX/UBX_Messages.h"

// Selects what UBX_Helper::UBX_ID_ToString returns.
enum UBX_Info : uint8_t { UBX_Info_Name = 0, UBX_Info_Length = 1, UBX_Info_Description = 2 };

// Zero-copy view of one UBX frame:  B5 62 | class | id | lenL lenH | payload | ckA ckB
// Parse() does not copy the payload: Payload points into the buffer you handed in, so
// that buffer must stay unchanged while the helper is in use.
class UBX_Helper {
	static const uint8_t UBX_HEADER_A = 0xb5;
	static const uint8_t UBX_HEADER_B = 0x62;
public:
	uint8_t        Class = 0;
	uint8_t        ID = 0;
	uint16_t       Length = 0;
	const uint8_t* Payload = nullptr;
	uint8_t        CK_A = 0;
	uint8_t        CK_B = 0;

	UBX_Helper() {}
	// Frame view with explicit parts (e.g. to build a message to send; checksums are
	// recomputed by ToBytes()).
	UBX_Helper(uint8_t Class, uint8_t ID, uint16_t Length, const uint8_t* FramePayload, uint8_t Checksum_A = 0, uint8_t Checksum_B = 0)
		: Class(Class), ID(ID), Length(Length), Payload(FramePayload), CK_A(Checksum_A), CK_B(Checksum_B) {}
	explicit UBX_Helper(const uint8_t* FrameBuffer) { Parse(FrameBuffer); }

	static const uint16_t FRAME_OVERHEAD = 8;	// 2 sync + class + id + 2 length + 2 checksum
	uint16_t TotalSize() const { return Length + FRAME_OVERHEAD; }

	// Fills this view from a complete, already-framed message (as produced by GNSS_Filter).
	// Returns false if the sync bytes are wrong.
	bool Parse(const uint8_t* FrameBuffer) {
		if (FrameBuffer == nullptr || FrameBuffer[0] != UBX_HEADER_A || FrameBuffer[1] != UBX_HEADER_B) return false;
		Class = FrameBuffer[2];
		ID = FrameBuffer[3];
		Length = (uint16_t)(FrameBuffer[4] | ((uint16_t)FrameBuffer[5] << 8));	// little-endian, alignment-safe
		Payload = FrameBuffer + 6;
		CK_A = FrameBuffer[Length + 6];
		CK_B = FrameBuffer[Length + 7];
		return true;
	}

	// 8-bit Fletcher checksum over class, id, length and payload. Zero-length messages
	// (poll requests) are valid and checksum normally.
	uint16_t CalcCheckSum() const {
		uint8_t A = 0, B = 0;
		A += Class;          B += A;
		A += ID;             B += A;
		A += Length & 0xff;  B += A;
		A += Length >> 8;    B += A;
		for (uint16_t i = 0; i < Length; i++) { A += Payload[i]; B += A; }
		return ((uint16_t)B << 8) | A;
	}
	void UpdateCheckSum() { const uint16_t C = CalcCheckSum(); CK_A = C & 0xff; CK_B = C >> 8; }
	uint16_t CheckSum() const { return ((uint16_t)CK_B << 8) | CK_A; }
	bool ChecksumOK() const { return CheckSum() == CalcCheckSum(); }

	// Serialises the frame (recomputing the checksum). Returns false if Capacity is too small.
	bool ToBytes(uint8_t* Buffer, uint16_t Capacity) {
		if (Buffer == nullptr || Capacity < TotalSize() || (Length != 0 && Payload == nullptr)) return false;
		UpdateCheckSum();
		Buffer[0] = UBX_HEADER_A;
		Buffer[1] = UBX_HEADER_B;
		Buffer[2] = Class;
		Buffer[3] = ID;
		Buffer[4] = Length & 0xff;	// little-endian
		Buffer[5] = Length >> 8;
		if (Length) memmove(Buffer + 6, Payload, Length);
		Buffer[Length + 6] = CK_A;
		Buffer[Length + 7] = CK_B;
		return true;
	}

	// Matches the UBX_NAV_*::CLASSID constants: (ID << 8) | Class.
	uint16_t ClassID() const { return ((uint16_t)ID << 8) | Class; }

	static const char* UBX_Class_ToString(uint8_t Class);
	static const char* UBX_ID_ToString(uint8_t Class, uint8_t ID, uint8_t Info = UBX_Info_Name) {
		const UBX_MsgInfo* M = UBX_FindMessage(Class, ID);
		if (M == nullptr) return "UNKNOWN ID";
		switch (Info) {
		case UBX_Info_Length: return M->length;
		case UBX_Info_Description: return M->description;
		default: return M->name;
		}
	}

	void Print(uint8_t DetailLevel = 0) const {
		ARGPRINT("UBX: %s-%s (%u) (%s)\r\n", UBX_Class_ToString(Class), UBX_ID_ToString(Class, ID), (unsigned)Length, ChecksumOK() ? "OK" : "FAIL");
	}
};

inline const char* UBX_Helper::UBX_Class_ToString(uint8_t Class) {
	switch (Class) {
	case 0x01: return "NAV";
	case 0x02: return "RXM";
	case 0x04: return "INF";
	case 0x05: return "ACK";
	case 0x06: return "CFG";
	case 0x09: return "UPD";
	case 0x0A: return "MON";
	case 0x0B: return "AID";
	case 0x0D: return "TIM";
	case 0x10: return "ESF";
	case 0x13: return "MGA";
	case 0x21: return "LOG";
	case 0x27: return "SEC";
	case 0x28: return "HNR";
	default: return "???";
	}
}
