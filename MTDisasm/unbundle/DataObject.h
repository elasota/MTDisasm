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

	enum class SystemType
	{
		kMac,
		kWindows,
	};

	enum class DataObjectType
	{
		kUnknown,

		kStreamHeader,
		kUnknown3ec,
		kUnknown17,
		kUnknown19,
		kDebris,
		kProjectLabelMap,
		kAssetCatalog,

		kProjectStructuralDef,
		kSectionStructuralDef,
		kSubsectionStructuralDef,
		kSceneStructuralDef,
		kImageStructuralDef,
		kMovieStructuralDef,
		kMToonStructuralDef,

		kBehaviorModifier,
		kPlugInModifier,

		kColorTableAsset,

		kNotYetImplemented,
	};

	struct SerializationProperties
	{
		bool m_isByteSwapped;
		SystemType m_systemType;
	};

	struct DORect
	{
		bool Load(DataReader& reader, const SerializationProperties& sp);

		int16_t m_top;
		int16_t m_left;
		int16_t m_bottom;
		int16_t m_right;
	};

	struct DOPoint
	{
		bool Load(DataReader& reader, const SerializationProperties& sp);

		int16_t m_top;
		int16_t m_left;
	};

	struct DOEvent
	{
		bool Load(DataReader& reader);

		uint32_t m_eventID;
		uint32_t m_eventInfo;
	};

	class DataObject
	{
	public:
		virtual ~DataObject() = 0;

		virtual DataObjectType GetType() const = 0;
		virtual bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) = 0;

		virtual void Delete();
	};

	struct DOStreamHeader final : public DataObject
	{
		DataObjectType GetType() const override;

		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

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
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

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
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		enum
		{
			Flag1_Deleted = 1,
			Flag1_LimitOnePerSegment = 2,
		};

		struct AssetInfo
		{
			uint32_t m_flags1;
			uint16_t m_nameLength;
			uint16_t m_alwaysZero;
			uint32_t m_unknown1;		// Possibly scene ID
			uint32_t m_filePosition;	// Contains a static value in Obsidian
			uint32_t m_assetType;
			uint32_t m_flags2;
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
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown1[6];
	};

	struct DOUnknown19 final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown1[2];
	};

	struct DODebris final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
	};

	struct DOProjectLabelMap final : public DataObject
	{
		DOProjectLabelMap();
		~DOProjectLabelMap();

		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

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

	namespace AnimationFlags
	{
		enum
		{
			kMaintainRate		= 0x02000000,	// mToon
			kPlayEveryFrame		= 0x02000000,	// QuickTime
			kLoop				= 0x08000000,

		};
	}

	namespace StructuralFlags
	{
		enum
		{
			kNotDirectToScreen	= 0x00001000,
			kHidden				= 0x00008000,
			kPaused				= 0x00010000,
			kExpandedInEditor	= 0x00800000,
			kCacheBitmap		= 0x02000000,
			kSelectedInEditor	= 0x10000000,
		};
	}

	struct DOProjectStructuralDef final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		enum
		{
			kExpandedInEditor = 0x800000,
		};

		uint32_t m_unknown1;	// Seems to always be 0x16
		uint32_t m_sizeIncludingTag;
		uint32_t m_unknown2;
		uint32_t m_flags;
		uint16_t m_nameLength;

		std::vector<char> m_name;	// Null terminated
	};

	struct DOColorTableAsset final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown1[4];
		uint32_t m_assetID;
		uint32_t m_unknown2;	// Usually zero-fill but sometimes contains 0xb

		struct ColorDef
		{
			uint16_t m_red;
			uint16_t m_green;
			uint16_t m_blue;
		};

		ColorDef m_colors[256];
	};

	struct DOSectionStructuralDef final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		enum
		{
			kExpandedInEditor = 0x800000,
		};

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_unknown2;
		uint16_t m_lengthOfName;
		uint32_t m_flags;
		uint16_t m_unknown4;
		uint16_t m_sectionID;
		uint32_t m_segmentID;

		std::vector<char> m_name;
	};

	struct DOSubsectionStructuralDef final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		enum
		{
			kExpandedInEditor = 0x800000,
		};

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_unknown2;
		uint16_t m_lengthOfName;
		uint32_t m_flags;
		uint16_t m_sectionID;

		std::vector<char> m_name;
	};

	struct DOSceneStructuralDef final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		enum
		{
			kSceneLocatorStreamIDMask = 0xff,
		};

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_unknown2;
		uint16_t m_lengthOfName;
		uint32_t m_flags;
		uint8_t m_unknown4[2];
		uint16_t m_sectionID;
		DORect m_rect1;
		DORect m_rect2;
		uint32_t m_streamLocator;	// 1-based index, sometimes observed with 0x10000000 flag set, not sure of the meaning
		uint8_t m_unknown11[4];

		std::vector<char> m_name;
	};

	struct DOImageStructuralDef final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_unknown2;
		uint16_t m_lengthOfName;
		uint32_t m_flags;
		uint8_t m_unknown4[2];
		uint16_t m_sectionID;
		DORect m_rect1;
		DORect m_rect2;
		uint32_t m_imageAssetID;
		uint32_t m_streamLocator;
		uint8_t m_unknown7[4];

		std::vector<char> m_name;
	};

	struct DOMovieStructuralDef final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_unknown2;
		uint16_t m_lengthOfName;
		uint32_t m_flags;
		uint8_t m_unknown3[46];
		uint16_t m_sectionID;
		uint8_t m_unknown5[2];
		DORect m_rect1;
		DORect m_rect2;
		uint32_t m_assetID;
		uint32_t m_unknown7;
		uint16_t m_volume;
		uint32_t m_animationFlags;
		uint8_t m_unknown10[4];
		uint8_t m_unknown11[4];
		uint32_t m_streamLocator;
		uint8_t m_unknown13[4];

		std::vector<char> m_name;
	};

	struct DOMToonStructuralDef final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_unknown2;
		uint16_t m_lengthOfName;
		uint32_t m_structuralFlags;
		uint8_t m_unknown3[2];
		uint32_t m_animationFlags;
		uint8_t m_unknown4[4];
		uint16_t m_sectionID;
		DORect m_rect1;
		DORect m_rect2;
		uint32_t m_unknown5;
		uint32_t m_rateTimes10000;
		uint32_t m_streamLocator;
		uint32_t m_unknown6;

		std::vector<char> m_name;
	};

	struct DOBehaviorModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown2[2];
		uint32_t m_unknown3;
		uint32_t m_unknown4;
		uint16_t m_unknown5;
		uint32_t m_unknown6;
		DOPoint m_editorLayoutPosition;
		uint16_t m_lengthOfName;
		uint16_t m_numChildren;
		uint32_t m_flags;
		DOEvent m_enableWhen;
		DOEvent m_disableWhen;
		uint8_t m_unknown7[2];

		std::vector<char> m_name;
	};

	struct DONotYetImplemented final : public DataObject
	{
		explicit DONotYetImplemented(uint32_t actualType, const char* name);

		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown;
		uint32_t m_sizeIncludingTag;
		uint16_t m_revision;

		uint32_t m_actualType;
		const char* m_name;
	};

	struct DOPlugInModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		char m_plugin[17];
		uint32_t m_unknown1;
		uint32_t m_weirdSize;
		uint8_t m_unknown2[20];
		uint16_t m_lengthOfName;

		uint32_t m_privateDataSize;

		std::vector<char> m_name;
	};

	DataObject* CreateObjectFromType(uint32_t objectType);
}
