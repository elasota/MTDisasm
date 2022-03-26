#pragma once

#include "IOStream.h"

#include <stdio.h>

namespace mtdisasm
{
	class CFileIOStream final : public IOStream
	{
	public:
		explicit CFileIOStream(FILE* f);

		size_t ReadPartial(void* dest, size_t sz) override;
		size_t WritePartial(const void* src, size_t sz) override;

		bool SeekSet(int32_t pos) override;
		bool SeekCur(int32_t pos) override;
		bool SeekEnd(int32_t pos) override;

		uint32_t Tell() const override;
		uint32_t TellGlobal() const override;

	private:
		FILE* m_f;
	};
}
