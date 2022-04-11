#include "DataReader.h"

#include "Endian.h"
#include "IOStream.h"

namespace mtdisasm
{
	DataReader::DataReader(IOStream& stream, bool byteSwap)
		: m_stream(stream)
		, m_byteSwap(byteSwap)
	{
	}

	bool DataReader::ReadU64(uint64_t& v)
	{
		uint64_t result = 0;
		if (!m_stream.ReadAll(&result, 8))
			return false;

		if (m_byteSwap)
			result = endian::SwapU64(result);

		v = result;
		return true;
	}

	bool DataReader::ReadS64(int64_t& v)
	{
		int64_t result = 0;
		if (!m_stream.ReadAll(&result, 8))
			return false;

		if (m_byteSwap)
			result = endian::SwapS64(result);

		v = result;
		return true;
	}

	bool DataReader::ReadU32(uint32_t& v)
	{
		uint32_t result = 0;
		if (!m_stream.ReadAll(&result, 4))
			return false;

		if (m_byteSwap)
			result = endian::SwapU32(result);

		v = result;
		return true;
	}

	bool DataReader::ReadS32(int32_t& v)
	{
		int32_t result = 0;
		if (!m_stream.ReadAll(&result, 4))
			return false;

		if (m_byteSwap)
			result = endian::SwapS32(result);

		v = result;
		return true;
	}

	bool DataReader::ReadU16(uint16_t& v)
	{
		uint16_t result = 0;
		if (!m_stream.ReadAll(&result, 2))
			return false;

		if (m_byteSwap)
			result = endian::SwapU16(result);

		v = result;
		return true;
	}

	bool DataReader::ReadS16(int16_t& v)
	{
		int16_t result = 0;
		if (!m_stream.ReadAll(&result, 2))
			return false;

		if (m_byteSwap)
			result = endian::SwapS16(result);

		v = result;
		return true;
	}

	bool DataReader::ReadU8(uint8_t& v)
	{
		return m_stream.ReadAll(&v, 1);
	}

	bool DataReader::ReadS8(int8_t& v)
	{
		return m_stream.ReadAll(&v, 1);
	}

	bool DataReader::ReadF80BE(double& v)
	{
		uint16_t signAndExponent;
		uint64_t mantissa;
		if (!ReadU16(signAndExponent) || !ReadU64(mantissa))
			return false;

		uint8_t sign = (signAndExponent >> 15) & 1;
		int16_t exponent = signAndExponent & 0x7fff;

		// Eliminate implicit 1 and truncate from 63 to 52 bits
		mantissa &= (((static_cast<uint64_t>(1) << 52) - 1u) << 11);
		mantissa >>= 11;

		if (mantissa != 0 || exponent != 0)
		{
			// Adjust exponent
			exponent -= 15360;
			if (exponent > 2046)
			{
				// Too big, set to largest finite magnitude
				exponent = 2046;
				mantissa = (static_cast<uint64_t>(1) << 52) - 1u;
			}
			else if (exponent < 0)
			{
				// Subnormal number
				mantissa |= (static_cast<uint64_t>(1) << 52);
				if (exponent < -52)
				{
					mantissa = 0;
					exponent = 0;
				}
				else
				{
					mantissa >>= (-exponent);
					exponent = 0;
				}
			}
		}

		uint64_t recombined = (static_cast<uint64_t>(sign) << 63) | (static_cast<uint64_t>(exponent) << 52) | static_cast<uint64_t>(mantissa);
		memcpy(&v, &recombined, 8);

		return true;
	}

	bool DataReader::ReadF64(double& v)
	{
		uint64_t u64;
		if (!this->ReadU64(u64))
			return false;

		memcpy(&v, &u64, 8);
		return true;
	}

	bool DataReader::ReadF32(float& v)
	{
		uint32_t u32;
		if (!this->ReadU32(u32))
			return false;

		memcpy(&v, &u32, 4);
		return true;
	}


	bool DataReader::ReadPStr16(std::string& str)
	{
		uint16_t length = 0;
		if (!ReadU16(length) || length == 0)
			return false;

		str.resize(length - 1, 0);
		if (length > 1)
		{
			if (!ReadBytes(&str[0], length - 1))
				return false;
		}

		uint8_t terminator = 0;
		if (!ReadU8(terminator))
			return false;

		if (terminator != 0)
			return false;

		return true;
	}

	bool DataReader::ReadRawU32(uint32_t& v)
	{
		return m_stream.ReadAll(&v, 4);
	}

	bool DataReader::ReadRawS32(int32_t& v)
	{
		return m_stream.ReadAll(&v, 4);
	}

	bool DataReader::ReadRawU16(uint16_t& v)
	{
		return m_stream.ReadAll(&v, 2);
	}

	bool DataReader::ReadRawS16(int16_t& v)
	{
		return m_stream.ReadAll(&v, 2);
	}

	bool DataReader::ReadBytes(void* dest, size_t sz)
	{
		return m_stream.ReadAll(dest, sz);
	}

	bool DataReader::Seek(uint32_t pos)
	{
		if (pos > INT32_MAX)
			return false;
		return m_stream.SeekSet(static_cast<int32_t>(pos));
	}

	bool DataReader::Skip(uint32_t amount)
	{
		return m_stream.SeekCur(amount);
	}

	uint32_t DataReader::Tell() const
	{
		return m_stream.Tell();
	}

	uint32_t DataReader::TellGlobal() const
	{
		return m_stream.TellGlobal();
	}
}
