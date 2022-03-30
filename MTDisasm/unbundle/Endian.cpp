#include "Endian.h"

#ifdef _MSC_VER
#include <stdlib.h>

namespace mtdisasm
{
	uint64_t endian::SwapU64(uint64_t v)
	{
		return _byteswap_uint64(v);
	}

	int64_t endian::SwapS64(int64_t v)
	{
		return _byteswap_uint64(v);
	}

	uint32_t endian::SwapU32(uint32_t v)
	{
		return _byteswap_ulong(v);
	}

	int32_t endian::SwapS32(int32_t v)
	{
		return _byteswap_ulong(v);
	}

	uint16_t endian::SwapU16(uint16_t v)
	{
		return _byteswap_ushort(v);
	}

	int16_t endian::SwapS16(int16_t v)
	{
		return _byteswap_ushort(v);
	}
}
#else

namespace mtdisasm
{
	uint32_t endian::SwapU32(uint64_t v)
	{
		return __builtin_bswap64(v);
	}

	int64_t endian::SwapS64(int64_t v)
	{
		return __builtin_bswap64(v);
	}

	uint32_t endian::SwapU32(uint32_t v)
	{
		return __builtin_bswap32(v);
	}

	int32_t endian::SwapS32(int32_t v)
	{
		return __builtin_bswap32(v);
	}

	uint16_t endian::SwapU16(uint16_t v)
	{
		return __builtin_bswap16(v);
	}

	int16_t endian::SwapS16(int16_t v)
	{
		return __builtin_bswap16(v);
	}
}
#endif

