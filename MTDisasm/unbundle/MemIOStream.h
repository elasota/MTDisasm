#pragma once

#include "IOStream.h"

namespace mtdisasm
{
	class MemIOStream final : public IOStream
	{
	public:
		explicit MemIOStream(const void* buf, size_t size);

		size_t ReadPartial(void* dest, size_t sz) override;
		size_t WritePartial(const void* src, size_t sz) override;

		bool SeekSet(int32_t pos) override;
		bool SeekCur(int32_t pos) override;
		bool SeekEnd(int32_t pos) override;

		uint32_t Tell() const override;
		uint32_t TellGlobal() const override;

	private:
		const void* m_buf;
		size_t m_size;
		uint32_t m_pos;
	};
}
