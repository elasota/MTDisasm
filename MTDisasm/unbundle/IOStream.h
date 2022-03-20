#pragma once

#include <cstddef>
#include <cstdint>

namespace mtdisasm
{
	struct IOStream
	{
		virtual size_t ReadPartial(void* dest, size_t sz) = 0;
		virtual size_t WritePartial(const void* src, size_t sz) = 0;

		virtual bool SeekSet(int32_t pos) = 0;
		virtual bool SeekCur(int32_t pos) = 0;
		virtual bool SeekEnd(int32_t pos) = 0;

		virtual uint32_t Tell() const = 0;

		bool ReadAll(void* dest, size_t sz);
		bool WriteAll(const void* src, size_t sz);
	};

	inline bool IOStream::ReadAll(void* dest, size_t sz)
	{
		return this->ReadPartial(dest, sz) == sz;
	}

	inline bool IOStream::WriteAll(const void* src, size_t sz)
	{
		return this->WritePartial(src, sz) == sz;
	}
}
