#pragma once

#include <cstdint>

namespace mtdisasm
{
	namespace endian
	{
		uint64_t SwapU64(uint64_t v);
		int64_t SwapS64(int64_t v);
		uint32_t SwapU32(uint32_t v);
		int32_t SwapS32(int32_t v);
		uint16_t SwapU16(uint16_t v);
		int16_t SwapS16(int16_t v);
	}
}
