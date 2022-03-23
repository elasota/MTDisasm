#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace mtdisasm
{
	class DataReader;

	namespace AssetTypeIDs
	{
		enum AssetTypeID
		{
			kColorTable = 0x02,
			kImage = 0x0e,
			kMToon = 0x10,
			kUnknown1f = 0x1f,	// Appears to be an image?  But always nameless.
			kWaveformSound = 0x54,
			kMovie = 0x55,
			kMIDI = 0x5c,
		};
	}

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
		kUnknown17,
		kUnknown19,
		kProjectLabelMap,
		kProjectInfo,
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
			uint32_t m_isDeleted;
			uint16_t m_nameLength;
			uint16_t m_alwaysZero;
			uint32_t m_unknown1;		// Possibly scene ID
			uint32_t m_filePosition;	// Contains a static value in Obsidian
			uint32_t m_assetType;
			uint32_t m_flags;
			std::vector<char> m_name;
		};

		uint32_t m_marker;
		uint32_t m_totalNameSizePlus22;
		uint8_t m_unknown1[4];
		uint32_t m_numAssets;
		std::vector<AssetInfo> m_assets;
	};

	struct DOUnknown17 final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown1[6];
	};

	struct DOUnknown19 final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown1[2];
	};

	struct DOProjectLabelMap final : public DataObject
	{
		DOProjectLabelMap();
		~DOProjectLabelMap();

		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision) override;

		struct LabelTree
		{
			LabelTree();
			~LabelTree();

			enum
			{
				kExpandedInEditor = 0x80000000,
			};

			uint32_t m_nameLength;
			uint32_t m_isGroup;
			uint32_t m_id;
			uint32_t m_unknown1;
			uint32_t m_flags;

			std::vector<char> m_name;

			uint32_t m_numChildren;
			LabelTree* m_children;
		};

		struct SuperGroup
		{
			SuperGroup();
			~SuperGroup();

			uint32_t m_nameLength;
			uint32_t m_unknown1;
			uint32_t m_unknown2;
			std::vector<char> m_name;

			uint32_t m_numChildren;
			LabelTree* m_tree;
		};

		uint32_t m_marker;
		uint32_t m_unknown1;	// Always 0x16
		uint32_t m_numSuperGroups;
		uint32_t m_nextAvailableID;

		SuperGroup* m_superGroups;

	private:
		static bool LoadSuperGroup(SuperGroup& sg, DataReader& reader, uint16_t revision);
		static bool LoadLabelTree(LabelTree& lt, DataReader& reader, uint16_t revision);
	};

	struct DOProjectInfo final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision) override;

		uint32_t m_unknown1;	// Seems to always be 0x16
		uint32_t m_sizeIncludingTag;
		uint32_t m_unknown2;
		uint32_t m_unknown3;
		uint16_t m_nameLength;

		std::vector<char> m_name;	// Null terminated
	};

	DataObject* CreateObjectFromType(uint32_t objectType);
}
