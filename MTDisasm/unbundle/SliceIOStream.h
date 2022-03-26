#pragma once

#include "IOStream.h"

namespace mtdisasm
{
	class SliceIOStream final : public IOStream
	{
	public:
		SliceIOStream(IOStream& parent, size_t offset, size_t length);

		size_t ReadPartial(void* dest, size_t sz) override;
		size_t WritePartial(const void* src, size_t sz) override;

		bool SeekSet(int32_t pos) override;
		bool SeekCur(int32_t pos) override;
		bool SeekEnd(int32_t pos) override;

		uint32_t Tell() const override;
		uint32_t TellGlobal() const override;

	private:
		IOStream& m_parent;
		size_t m_offset;
		size_t m_length;
	};
}
