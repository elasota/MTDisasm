#include "Catalog.h"

#include "DataReader.h"

namespace mtdisasm
{
	Catalog::Catalog()
		: m_systemDesc(SystemDesc::kUnknown)
	{
	}

	bool Catalog::Load(DataReader& reader)
	{
		if (!reader.ReadU16(m_catHeader.m_platform)
			|| !reader.ReadU32(m_catHeader.m_unknown02)
			|| !reader.ReadU32(m_catHeader.m_unknown06)
			|| !reader.ReadU32(m_catHeader.m_unknown0a)
			|| !reader.ReadU32(m_catHeader.m_unknown0e)
			|| !reader.ReadU16(m_catHeader.m_unknown12)
			|| !reader.ReadU32(m_catHeader.m_unknown14)
			|| !reader.ReadU32(m_catHeader.m_unknown18)
			|| !reader.ReadU16(m_catHeader.m_unknown1c)
			|| !reader.ReadU32(m_catHeader.m_unknown1e)
			|| !reader.ReadU32(m_catHeader.m_unknown22)
			|| !reader.ReadU16(m_catHeader.m_unknown26)
			|| !reader.ReadU32(m_catHeader.m_unknown28)
			|| !reader.ReadU32(m_catHeader.m_sizeOfStreamAndSegmentDescs)
			|| !reader.ReadU16(m_catHeader.m_numStreams)
			|| !reader.ReadU16(m_catHeader.m_unknown32)
			|| !reader.ReadU16(m_catHeader.m_unknown34)
			|| !reader.ReadU16(m_catHeader.m_numSegments))
		{
			return false;
		}

		// 0x800 = Make movies external flag
		const uint32_t allowFlagsMask = 0xfffff7ff;
		if (m_catHeader.m_platform == 1)
		{
			m_systemDesc = SystemDesc::kWindows;
			if ((m_catHeader.m_unknown14 & allowFlagsMask) != 0x0100004b || (m_catHeader.m_unknown28 & allowFlagsMask) != 0x0100004b)
				return false;
		}
		else if (m_catHeader.m_platform == 0)
		{
			m_systemDesc = SystemDesc::kMac;
			if (m_catHeader.m_unknown14 != 0x0000004a || m_catHeader.m_unknown28 != 0x0000004a)
				return false;
		}
		else
			return false;

		if (m_catHeader.m_unknown02 != 0xaa55a5a5
			|| m_catHeader.m_unknown06 != 0
			|| m_catHeader.m_unknown0a != 0x0e
			|| m_catHeader.m_unknown0e != 0x03ea
			|| m_catHeader.m_unknown12 != 0
			|| m_catHeader.m_unknown18 != 0x14
			|| m_catHeader.m_unknown1c != 0
			|| m_catHeader.m_unknown1e != 0x22
			|| m_catHeader.m_unknown22 != 0x3e8
			|| m_catHeader.m_unknown32 != 0
			|| m_catHeader.m_unknown34 != 1
			|| m_catHeader.m_numSegments < 1)
			return false;

		if (m_catHeader.m_unknown26 != 2 && m_catHeader.m_unknown26 != 3)
			return false;

		bool hasStreamDescPrePadding = (m_catHeader.m_unknown26 == 3 && m_systemDesc == SystemDesc::kWindows);
		bool hasStreamDescPostPadding = (m_catHeader.m_unknown26 == 3 && m_systemDesc == SystemDesc::kMac);

		m_streams.resize(m_catHeader.m_numStreams);

		for (size_t i = 0; i < m_catHeader.m_numStreams; i++)
		{
			StreamDesc& desc = m_streams[i];
			desc.m_streamType[24] = 0;
			if (!reader.ReadBytes(desc.m_streamType, 24)
				|| !reader.ReadU16(desc.m_segmentNumber))
				return false;

			if (hasStreamDescPrePadding)
			{
				uint32_t padding[2];
				if (!reader.ReadBytes(&padding, 8))
					return false;

				if (padding[0] != 0 || padding[1] != 0)
					return false;
			}

			if (!reader.ReadU32(desc.m_pos)
				|| !reader.ReadU32(desc.m_size))
				return false;

			if (hasStreamDescPostPadding)
			{
				uint32_t padding[2];
				if (!reader.ReadBytes(&padding, 8))
					return false;

				if (padding[0] != 0 || padding[1] != 0)
					return false;
			}

			if (desc.m_segmentNumber == 0 || desc.m_segmentNumber > m_catHeader.m_numSegments)
				return false;
		}

		uint32_t unknownSegmentPrefix = 0;
		if (!reader.ReadU32(unknownSegmentPrefix) || unknownSegmentPrefix != 1)
			return false;

		m_segments.resize(m_catHeader.m_numSegments);
		for (size_t i = 0; i < m_catHeader.m_numSegments; i++)
		{
			SegmentDesc& segDesc = m_segments[i];
			if (!reader.ReadU32(segDesc.m_segmentID)
				|| !reader.ReadPStr16(segDesc.m_label)
				|| !reader.ReadPStr16(segDesc.m_exportedPath))
				return false;
		}

		return true;
	}

	size_t Catalog::NumStreams() const
	{
		return m_streams.size();
	}

	const StreamDesc& Catalog::GetStream(size_t index) const
	{
		return m_streams[index];
	}

	size_t Catalog::NumSegments() const
	{
		return m_segments.size();
	}

	const SegmentDesc& Catalog::GetSegment(size_t index) const
	{
		return m_segments[index];
	}

	const CatalogHeader& Catalog::GetCatalogHeader() const
	{
		return m_catHeader;
	}

	SystemDesc Catalog::GetSystem() const
	{
		return m_systemDesc;
	}
}
