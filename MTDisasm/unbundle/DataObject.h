#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace mtdisasm
{
	class DataReader;

	enum class AssetTypeID
	{
		kColorTable		= 0x02,
		kImage			= 0x0e,
		kMToon			= 0x10,
		kWaveformSound	= 0x54,
		kMovie			= 0x55,
		kMIDI			= 0x5c,

		kDeleted		= 0x1f,
	};

	namespace AssetFlags
	{
		enum
		{
			kExternal = 0x4000,
		};
	}

	enum class DataObjectType
	{
		kUnknown,

		kStreamHeader,
		kUnknown3ec,
		kAssetCatalog,
	};

	class DataObject
	{
	public:
		virtual ~DataObject() = 0;

		virtual DataObjectType GetType() const = 0;
		virtual bool Load(DataReader& reader, uint16_t revision) = 0;

		virtual void Delete();
	};

	struct DOStreamHeader final : public DataObject
	{
		DataObjectType GetType() const override;

		bool Load(DataReader& reader, uint16_t revision) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
		char m_name[17];
		uint8_t m_projectID[2];
		uint8_t m_unknown1[4];	// Seems to be consistent across builds
		uint16_t m_unknown2;	// 0
	};

	struct DOUnknown3ec final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown1[2];
		uint32_t m_unknown2;
		uint16_t m_unknown3;
		uint16_t m_unknown4;
	};

	struct DOAssetCatalog final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision) override;

		struct AssetInfo
		{
			uint32_t m_unknown1;
			uint16_t m_nameLength;
			uint16_t m_unknown2;
			uint32_t m_unknown3;
			uint32_t m_unknown4;
			uint32_t m_assetType;
			uint32_t m_unknown6;
			std::vector<char> m_name;
		};

		uint32_t m_marker;
		uint32_t m_totalNameSizePlus22;
		uint8_t m_unknown1[4];
		uint32_t m_numAssets;
		std::vector<AssetInfo> m_assets;
	};

	DataObject* CreateObjectFromType(uint32_t objectType);
}
