#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mtdisasm
{
	class DataReader;

	struct StreamDesc
	{
		char m_streamType[25];
		uint16_t m_segmentNumber;
		uint32_t m_size;
		uint32_t m_pos;
	};

	struct SegmentDesc
	{
		uint32_t m_segmentID;
		std::string m_label;
		std::string m_exportedPath;
	};

	enum class SystemDesc
	{
		kMac,
		kWindows,

		kUnknown,
	};

	struct CatalogHeader
	{
		uint16_t m_platform;
		uint32_t m_unknown02;	// 0xaa55a5a5
		uint32_t m_unknown06;	// 0
		uint32_t m_unknown0a;	// 0x0e
		uint32_t m_unknown0e;	// 0x03ea
		uint16_t m_unknown12;	// 0
		uint32_t m_unknown14;	// On Win, 0x0100004b, on Mac, 0x0000004a
		uint32_t m_unknown18;	// 0x14
		uint16_t m_unknown1c;	// 0
		uint32_t m_unknown1e;	// 0x22
		uint32_t m_unknown22;	// 0x000003e8
		uint16_t m_unknown26;	// 2 in testing, 3 in Obsidian
		uint32_t m_unknown28;	// On Win, 0x0100004b, on Mac, 0x0000004a
		uint32_t m_sizeOfStreamAndSegmentDescs;
		uint16_t m_numStreams;
		uint16_t m_unknown32;
		uint16_t m_unknown34;	// Probably startup section
		uint16_t m_numSegments;
	};

	class Catalog
	{
	public:
		Catalog();

		bool Load(DataReader& reader);

		size_t NumStreams() const;
		const StreamDesc& GetStream(size_t index) const;

		size_t NumSegments() const;
		const SegmentDesc& GetSegment(size_t index) const;

		const CatalogHeader& GetCatalogHeader() const;
		SystemDesc GetSystem() const;

	private:
		std::vector<StreamDesc> m_streams;
		std::vector<SegmentDesc> m_segments;

		CatalogHeader m_catHeader;
		SystemDesc m_systemDesc;
	};
}
