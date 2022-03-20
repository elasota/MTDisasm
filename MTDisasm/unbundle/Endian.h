#pragma once

#include <cstdint>

namespace mtdisasm
{
	namespace endian
	{
		uint32_t SwapU32(uint32_t v);
		int32_t SwapS32(int32_t v);
		uint16_t SwapU16(uint16_t v);
		int16_t SwapS16(int16_t v);
	}
}
