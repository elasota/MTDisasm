#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace mtdisasm
{
	struct IOStream;

	class DataReader
	{
	public:
		DataReader(IOStream& stream, bool byteSwap);

		bool ReadU64(uint64_t& v);
		bool ReadS64(int64_t& v);
		bool ReadU32(uint32_t& v);
		bool ReadS32(int32_t& v);
		bool ReadU16(uint16_t& v);
		bool ReadS16(int16_t& v);
		bool ReadU8(uint8_t& v);
		bool ReadS8(int8_t& v);
		bool ReadF64(double& v);
		bool ReadF32(float& v);

		bool ReadPStr16(std::string& str);

		bool ReadRawU32(uint32_t& v);
		bool ReadRawS32(int32_t& v);
		bool ReadRawU16(uint16_t& v);
		bool ReadRawS16(int16_t& v);
		bool ReadRawF64(double& v);
		bool ReadRawF32(float& v);

		bool ReadBytes(void* dest, size_t sz);

		bool Skip(uint32_t pos);
		bool Seek(uint32_t pos);
		uint32_t Tell() const;
		uint32_t TellGlobal() const;

	private:
		IOStream& m_stream;
		bool m_byteSwap;
	};
}
