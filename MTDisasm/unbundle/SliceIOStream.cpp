#include "SliceIOStream.h"

namespace mtdisasm
{
	SliceIOStream::SliceIOStream(IOStream& parent, size_t offset, size_t length)
		: m_parent(parent)
		, m_offset(offset)
		, m_length(length)
	{
	}

	size_t SliceIOStream::ReadPartial(void* dest, size_t sz)
	{
		size_t currentPos = m_parent.Tell();
		if (currentPos < m_offset)
		{
			if (!m_parent.SeekSet(static_cast<int32_t>(m_offset)))
				return 0;
			currentPos = m_offset;
		}

		size_t available = m_length - (currentPos - m_offset);

		if (sz > available)
			sz = available;

		if (sz == 0)
			return 0;

		return m_parent.ReadPartial(dest, sz);
	}

	size_t SliceIOStream::WritePartial(const void* src, size_t sz)
	{
		size_t currentPos = m_parent.Tell();
		if (currentPos < m_offset)
		{
			if (!m_parent.SeekSet(static_cast<int32_t>(m_offset)))
				return 0;
			currentPos = m_offset;
		}

		size_t available = m_length - (currentPos - m_offset);

		if (sz > available)
			sz = available;

		if (sz == 0)
			return 0;

		return m_parent.WritePartial(src, sz);
	}

	bool SliceIOStream::SeekSet(int32_t pos)
	{
		if (pos < 0)
			pos = 0;

		return m_parent.SeekSet(pos + static_cast<int32_t>(m_offset));
	}

	bool SliceIOStream::SeekCur(int32_t pos)
	{
		int32_t currentPos = static_cast<int32_t>(Tell());
		int32_t adjustedPos = currentPos + pos;
		if (adjustedPos < 0)
			return false;
		if (adjustedPos > static_cast<int32_t>(m_length))
			return false;

		return m_parent.SeekSet(adjustedPos + static_cast<int32_t>(m_offset));
	}

	bool SliceIOStream::SeekEnd(int32_t pos)
	{
		int32_t adjustedPos = static_cast<int32_t>(m_length) + pos;
		if (adjustedPos < 0)
			return false;
		if (adjustedPos > static_cast<int32_t>(m_length))
			return false;

		return m_parent.SeekSet(adjustedPos + static_cast<int32_t>(m_offset));
	}

	uint32_t SliceIOStream::Tell() const
	{
		int32_t pos = static_cast<int32_t>(m_parent.Tell()) - static_cast<int32_t>(m_offset);
		if (pos < 0)
			return 0;
		if (static_cast<size_t>(pos) > m_length)
			return static_cast<uint32_t>(m_length);

		return static_cast<uint32_t>(pos);
	}
}
