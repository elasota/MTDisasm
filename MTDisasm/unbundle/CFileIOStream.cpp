#include "CFileIOStream.h"

namespace mtdisasm
{
	CFileIOStream::CFileIOStream(FILE* f)
		: m_f(f)
	{
	}

	size_t CFileIOStream::ReadPartial(void* dest, size_t sz)
	{
		return fread(dest, 1, sz, m_f);
	}

	size_t CFileIOStream::WritePartial(const void* src, size_t sz)
	{
		return fwrite(src, 1, sz, m_f);
	}

	bool CFileIOStream::SeekSet(int32_t pos)
	{
		return fseek(m_f, pos, SEEK_SET) == 0;
	}

	bool CFileIOStream::SeekCur(int32_t pos)
	{
		return fseek(m_f, pos, SEEK_CUR) == 0;
	}

	bool CFileIOStream::SeekEnd(int32_t pos)
	{
		return fseek(m_f, pos, SEEK_END) == 0;
	}

	uint32_t CFileIOStream::Tell() const
	{
		return ftell(m_f);
	}

	uint32_t CFileIOStream::TellGlobal() const
	{
		return this->Tell();
	}
}

