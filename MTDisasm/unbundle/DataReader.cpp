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
