#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cinttypes>	// PRIu32 / PRId32 / PRIx32: uint32_t is 'long' on some cores, 'int' on others

#include "GNSS_Config.h"	// ARGPRINT debug-print hook

// ---------------------------------------------------------------------------
// Frame building shared by every message type's ToPoll() / ToSet().
//   B5 62 | class | id | lenL lenH | payload | ckA ckB
//
// Every builder follows one convention: pass Out == nullptr to learn the frame size
// without writing anything (size the buffer, then call again).
// ---------------------------------------------------------------------------
static constexpr uint16_t UBX_FRAME_OVERHEAD = 8;
static constexpr uint16_t UBX_POLL_FRAME_SIZE = UBX_FRAME_OVERHEAD;	// a poll with no payload

// Completes a frame whose payload is already at Out + 6: writes sync, class, id and
// length in front of it and the checksum after it. Returns the frame size (Length + 8).
inline uint16_t UBX_FinishFrame(uint8_t* Out, uint8_t Class, uint8_t ID, uint16_t Length) {
	Out[0] = 0xb5;
	Out[1] = 0x62;
	Out[2] = Class;
	Out[3] = ID;
	Out[4] = Length & 0xff;	// little-endian
	Out[5] = Length >> 8;
	uint8_t A = 0, B = 0;
	for (uint16_t i = 2; i < (uint16_t)(Length + 6); i++) { A += Out[i]; B += A; }
	Out[Length + 6] = A;
	Out[Length + 7] = B;
	return Length + UBX_FRAME_OVERHEAD;
}

// Writes a complete frame (including checksum) to Out and returns its size
// (Length + 8). Out must have room for that many bytes; Out == nullptr only returns the size.
// Payload may be nullptr when Length is 0.
inline uint16_t UBX_BuildFrame(uint8_t* Out, uint8_t Class, uint8_t ID, const uint8_t* Payload, uint16_t Length) {
	if (Out == nullptr) return Length + UBX_FRAME_OVERHEAD;
	if (Length) memmove(Out + 6, Payload, Length);
	return UBX_FinishFrame(Out, Class, ID, Length);
}

// Poll request with an empty payload: asks the receiver to send message (Class, ID) once.
inline uint16_t UBX_BuildPoll(uint8_t* Out, uint8_t Class, uint8_t ID) { return UBX_BuildFrame(Out, Class, ID, nullptr, 0); }

// Appends one frame to a batch buffer. `Build` is a callable taking the output pointer and
// returning the frame size (any ToPoll / ToSet*); it is called once with nullptr to measure,
// then again to write, so nothing is written if the frame doesn't fit.
//   size_t N = 0;
//   UBX_Append(Buf, sizeof Buf, N, [](uint8_t* M) { return UBX_MON_VER::ToPoll(M); });
// Returns false (and leaves Used untouched) if the frame would overflow Capacity.
template <typename Builder>
inline bool UBX_Append(uint8_t* Buf, size_t Capacity, size_t& Used, Builder Build) {
	const size_t Size = Build((uint8_t*)nullptr);
	if (Used + Size > Capacity) return false;
	Build(Buf + Used);
	Used += Size;
	return true;
}
