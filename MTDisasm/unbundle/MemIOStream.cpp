#include "MemIOStream.h"

#include <cstring>

namespace mtdisasm
{
	MemIOStream::MemIOStream(const void* buf, size_t size)
		: m_buf(buf)
		, m_size(size)
		, m_pos(0)
	{
	}

	size_t MemIOStream::ReadPartial(void* dest, size_t sz)
	{
		size_t available = m_size - m_pos;
		if (available < sz)
			sz = available;

		memcpy(dest, static_cast<const char*>(m_buf) + m_pos, sz);
		m_pos += sz;

		return sz;
	}

	size_t MemIOStream::WritePartial(const void* src, size_t sz)
	{
		return 0;
	}

	bool MemIOStream::SeekSet(int32_t pos)
	{
		if (pos < 0)
			return false;
		if (static_cast<size_t>(pos) > m_size)
			return false;

		m_pos = static_cast<size_t>(pos);

		return true;
	}

	bool MemIOStream::SeekCur(int32_t pos)
	{
		return SeekSet(static_cast<int32_t>(m_pos) + pos);
	}

	bool MemIOStream::SeekEnd(int32_t pos)
	{
		if (pos > 0)
			return false;
		return SeekSet(static_cast<int32_t>(m_size) + pos);
	}

	uint32_t MemIOStream::Tell() const
	{
		return m_pos;
	}

	uint32_t MemIOStream::TellGlobal() const
	{
		return m_pos;
	}
}
