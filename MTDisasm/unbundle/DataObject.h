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

	namespace AudioEncodings
	{
		enum
		{
			kUncompressed	= 0x00,
			kMace6			= 0x04,
			kMace3			= 0x03,
		};
	}

	enum class SystemType
	{
		kMac,
		kWindows,
	};

	enum class PlugInObjectType
	{
		kUnknown,

		kCursorMod,
	};

	enum class DataObjectType
	{
		kUnknown,

		kStreamHeader,
		kPresentationSettings,
		kGlobalObjectInfo,
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
		kMacOnlyCursorModifier,	// Obsolete
		kMiniscriptModifier,
		kMessengerModifier,
		kIfMessengerModifier,
		kBooleanVariableModifier,
		kPointVariableModifier,

		kColorTableAsset,
		kAudioAsset,
		kImageAsset,
		kMovieAsset,
		kMToonAsset,

		kAssetDataSection,
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

	struct DOTypicalModifierHeader
	{
		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_guid;
		uint8_t m_unknown2[6];	// 0
		uint32_t m_unknown3;
		uint8_t m_unknown4[4];	// ff
		uint16_t m_lengthOfName;

		std::vector<char> m_name;

		bool Load(DataReader& reader);
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

	struct DOPresentationSettings final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown1[2];
		DOPoint m_dimensions;
		uint16_t m_bitsPerPixel;
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

	struct DOGlobalObjectInfo final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_marker;
		uint32_t m_sizeIncludingTag;
		uint16_t m_numGlobalModifiers;
		uint8_t m_unknown1[4];
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
		uint32_t m_guid;
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
		uint32_t m_guid;
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
		uint32_t m_guid;
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
		uint32_t m_guid;
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
		uint32_t m_guid;
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
		uint32_t m_guid;
		uint16_t m_lengthOfName;
		uint32_t m_flags;
		uint16_t m_layer;
		uint8_t m_unknown3[44];
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
		uint32_t m_guid;
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

	struct DOMiniscriptProgram
	{
		bool Load(DataReader& reader, const SerializationProperties& sp);

		struct LocalRef
		{
			uint32_t m_guid;
			uint8_t m_lengthOfName;
			uint8_t m_unknown10;

			std::vector<char> m_name;
		};

		struct Attribute
		{
			uint8_t m_lengthOfName;
			uint8_t m_unknown11;

			std::vector<char> m_name;
		};

		uint32_t m_unknown1;

		uint32_t m_sizeOfInstructions;
		uint32_t m_numOfInstructions;
		uint32_t m_numLocalRefs;
		uint32_t m_numAttributes;

		std::vector<uint8_t> m_bytecode;
		std::vector<LocalRef> m_localRefs;
		std::vector<Attribute> m_attributes;

		SerializationProperties m_sp;
	};

	enum MessageFlags
	{
		kMessageFlagNoRelay = 0x20000000,
		kMessageFlagNoCascade = 0x40000000,
		kMessageFlagNoImmediate = 0x80000000,
	};

	struct DOMessengerModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_guid;
		uint8_t m_unknown3[6];
		uint32_t m_unknown4;
		uint8_t m_unknown5[4];
		uint16_t m_lengthOfName;
		uint32_t m_messageFlags;
		DOEvent m_send;
		DOEvent m_when;
		uint16_t m_unknown14;
		uint32_t m_destination;
		uint8_t m_unknown11[10];
		uint16_t m_with;
		uint8_t m_unknown15[4];
		uint32_t m_withSourceGUID;
		uint8_t m_unknown12[36];
		uint8_t m_withSourceLength;
		uint8_t m_unknown13;

		std::vector<char> m_name;
		std::vector<char> m_withSource;
	};

	struct DOIfMessengerModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_guid;
		uint8_t m_unknown3[6];
		uint32_t m_unknown4;
		uint8_t m_unknown5[4];
		uint16_t m_lengthOfName;
		uint32_t m_messageFlags;
		DOEvent m_send;
		DOEvent m_when;
		uint16_t m_unknown6;
		uint32_t m_destination;
		uint8_t m_unknown7[10];
		uint16_t m_with;
		uint8_t m_unknown8[4];
		uint32_t m_withSourceGUID;
		uint8_t m_unknown9[46];
		uint8_t m_withSourceLength;
		uint8_t m_unknown10;
		DOMiniscriptProgram m_program;

		std::vector<char> m_name;
		std::vector<char> m_withSource;
	};

	struct DOBehaviorModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown2[2];
		uint32_t m_guid;
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

	struct DOBooleanVariableModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_guid;
		uint8_t m_unknown2[6];
		uint32_t m_unknown3;
		uint8_t m_unknown4[4];
		uint16_t m_lengthOfName;
		uint8_t m_value;
		uint8_t m_unknown5;

		std::vector<char> m_name;
	};

	struct DOPointVariableModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_guid;
		uint8_t m_unknown2[6];
		uint32_t m_unknown3;
		uint8_t m_unknown4[4];
		uint16_t m_lengthOfName;
		uint8_t m_unknown5[4];
		DOPoint m_value;

		std::vector<char> m_name;
	};

	struct DOMiniscriptModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_guid;
		uint8_t m_unknown3[6];
		uint32_t m_unknown4;
		uint8_t m_unknown5[4];
		uint16_t m_lengthOfName;
		DOEvent m_enableWhen;
		uint8_t m_unknown6[11];
		uint8_t m_unknown7;
		DOMiniscriptProgram m_program;

		std::vector<char> m_name;

		SerializationProperties m_sp;
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

	struct DOPlugInModifier;

	struct PlugInObject
	{
		virtual ~PlugInObject();
		virtual PlugInObjectType GetType() const = 0;
		virtual bool Load(const DOPlugInModifier& base, DataReader& reader, const SerializationProperties& sp) = 0;
	};

	struct POCursorMod final : public PlugInObject
	{
		PlugInObjectType GetType() const override;
		bool Load(const DOPlugInModifier& base, DataReader& reader, const SerializationProperties& sp) override;

		uint16_t m_unknown1;
		DOEvent m_applyWhen;
		uint16_t m_unknown2;
		DOEvent m_removeWhen;
		uint16_t m_unknown3;

		uint32_t m_cursorID;
		uint8_t m_unknown4[4];
	};

	struct POSTransCt final : public PlugInObject
	{
		PlugInObjectType GetType() const override;
		bool Load(const DOPlugInModifier& base, DataReader& reader, const SerializationProperties& sp) override;

		uint16_t m_unknown1;
		DOEvent m_applyWhen;
		uint16_t m_unknown2;
		DOEvent m_removeWhen;
		uint16_t m_unknown3;

		uint32_t m_cursorID;
		uint8_t m_unknown4[4];
	};

	struct POUnknown final : public PlugInObject
	{
		PlugInObjectType GetType() const override;
		bool Load(const DOPlugInModifier& base, DataReader& reader, const SerializationProperties& sp) override;

		std::vector<uint8_t> m_data;
	};

	struct DOPlugInModifier final : public DataObject
	{
		DOPlugInModifier();
		~DOPlugInModifier();

		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		char m_plugin[17];
		uint32_t m_unknown1;
		uint32_t m_weirdSize;
		uint32_t m_guid;
		uint8_t m_unknown2[6];
		uint16_t m_plugInRevision;
		uint32_t m_unknown4;
		uint8_t m_unknown5[4];
		uint16_t m_lengthOfName;

		uint32_t m_privateDataSize;

		std::vector<char> m_name;

		PlugInObject* m_plugInData;
	};

	struct DOMacOnlyCursorModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		struct MacOnlyPart
		{
			enum
			{
				kCursor_Inactive,
				kCursor_Interact,
				kCursor_HandGrabBW,
				kCursor_HandOpenBW,
				kCursor_HandPointUp,
				kCursor_HandPointRight,
				kCursor_HandPointLeft,
				kCursor_HandPointDown,
				kCursor_HandGrabColor,
				kCursor_HandOpenColor,
				kCursor_Arrow,
				kCursor_Pencil,
				kCursor_Smiley,
				kCursor_Wait,
				kCursor_Hidden,
			};

			DOEvent m_applyWhen;
			uint32_t m_unknown1;
			uint16_t m_unknown2;
			uint32_t m_cursorIndex;
		};

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
		uint32_t m_unknown2;
		uint32_t m_unknown3;
		uint16_t m_unknown4;
		uint32_t m_unknown5;
		uint8_t m_unknown6[4];
		uint16_t m_lengthOfName;
		std::vector<char> m_name;

		bool m_hasMacOnlyPart;
		MacOnlyPart m_macOnlyPart;
	};

	struct DOAudioAsset final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		struct MacPart
		{
			uint8_t m_unknown4[4];
			uint8_t m_unknown5[5];
			uint8_t m_unknown6[3];
			uint8_t m_unknown8[20];
		};

		struct WinPart
		{
			uint8_t m_unknown9[3];
			uint8_t m_unknown10[3];
			uint8_t m_unknown11[15];
			uint8_t m_unknown12_1[2];
		};

		struct CuePoint
		{
			uint8_t m_unknown13[2];
			uint32_t m_unknown14;
			uint32_t m_position;
			uint32_t m_cuePointID;
		};

		uint32_t m_marker;
		uint32_t m_assetAndDataCombinedSize;
		uint8_t m_unknown2[4];
		uint32_t m_assetID;
		uint8_t m_unknown3[20];
		uint16_t m_sampleRate1;
		uint8_t m_bitsPerSample;
		uint8_t m_encoding1;
		uint8_t m_channels;
		uint8_t m_codedDuration[4];
		uint16_t m_sampleRate2;
		uint32_t m_cuePointDataSize;
		uint16_t m_numCuePoints;
		uint8_t m_unknown14[4];
		uint32_t m_filePosition;
		uint32_t m_size;

		std::vector<CuePoint> m_cuePoints;

		bool m_haveMacPart;
		MacPart m_macPart;

		bool m_haveWinPart;
		WinPart m_winPart;
	};

	struct DOImageAsset final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		struct MacPart
		{
			uint8_t m_unknown7[44];
		};

		struct WinPart
		{
			uint8_t m_unknown8[10];
		};

		union PlatformPart
		{
			WinPart m_win;
			MacPart m_mac;
		};

		uint32_t m_marker;
		uint32_t m_unknown1;
		uint8_t m_unknown2[4];
		uint32_t m_assetID;
		uint32_t m_unknown3;

		DORect m_rect1;
		uint32_t m_hdpiFixed;
		uint32_t m_vdpiFixed;
		uint16_t m_bitsPerPixel;
		uint8_t m_unknown4[2];
		uint8_t m_unknown5[4];
		uint8_t m_unknown6[8];
		DORect m_rect2;
		uint32_t m_filePosition;
		uint32_t m_size;

		bool m_haveMacPart;
		bool m_haveWinPart;

		PlatformPart m_platform;
	};

	struct DOMovieAsset final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		struct MacPart
		{
			uint8_t m_unknown5_1[66];
			uint8_t m_unknown6[12];
		};

		struct WinPart
		{
			uint8_t m_unknown3_1[32];
			uint8_t m_unknown4[12];
			uint8_t m_unknown7[12];
		};

		uint32_t m_marker;
		uint32_t m_assetAndDataCombinedSize;
		uint8_t m_unknown1[4];
		uint32_t m_assetID;
		uint8_t m_unknown1_1[4];
		uint16_t m_extFileNameLength;

		uint32_t m_movieDataPos;
		uint32_t m_moovAtomPos;
		uint32_t m_movieDataSize;

		bool m_haveMacPart;
		MacPart m_macPart;

		bool m_haveWinPart;
		WinPart m_winPart;

		std::vector<char> m_extFileName;
	};

	struct DOMToonAsset final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		struct MacPart
		{
			uint8_t m_unknown10[88];
		};

		struct WinPart
		{
			uint8_t m_unknown11[54];
		};

		struct FrameDef
		{
			struct MacPart
			{
				uint8_t m_unknown17[4];
			};

			struct WinPart
			{
				uint8_t m_unknown18[2];
			};

			union PlatformUnion
			{
				MacPart m_mac;
				WinPart m_win;
			} m_platform;

			uint8_t m_unknown12[4];
			DORect m_rect1;
			uint32_t m_dataOffset;
			uint8_t m_unknown13[2];
			uint32_t m_compressedSize;
			uint8_t m_unknown14;
			uint8_t m_keyframeFlag;
			uint8_t m_platformBit;
			uint8_t m_unknown15;
			DORect m_rect2;
			uint32_t m_hdpiFixed;
			uint32_t m_vdpiFixed;
			uint16_t m_bitsPerPixel;
			uint32_t m_unknown16;
			uint16_t m_decompressedBytesPerRow;
			uint32_t m_decompressedSize;
		};

		struct FrameRangeDef
		{
			uint32_t m_startFrame;
			uint32_t m_endFrame;
			uint8_t m_lengthOfName;
			uint8_t m_unknown14;

			std::vector<char> m_name;	// Null terminated
		};

		enum
		{
			kEncodingFlag_TemporalCompression	= 0x80,
			kEncodingFlag_HasRanges				= 0x20000000,
			kEncodingFlag_Trimming				= 0x08,
		};

		uint32_t m_marker;
		uint8_t m_unknown1[8];
		uint32_t m_assetID;

		bool m_haveMacPart;
		bool m_haveWinPart;

		union PlatformUnion
		{
			MacPart m_mac;
			WinPart m_win;
		} m_platform;

		uint32_t m_frameDataPosition;
		uint32_t m_sizeOfFrameData;

		// mToon data
		uint32_t m_mtoonHeader[2];
		uint16_t m_version;
		uint8_t m_unknown2[4];
		uint32_t m_encodingFlags;
		DORect m_rect;

		uint16_t m_numFrames;
		uint8_t m_unknown3[14];
		uint16_t m_bitsPerPixel;
		uint32_t m_codecID;
		uint8_t m_unknown4_1[8];
		uint32_t m_codecDataSize;
		uint8_t m_unknown4_2[4];

		std::vector<FrameDef> m_frames;

		std::vector<uint8_t> m_codecData;

		struct FrameRangePart
		{
			uint32_t m_tag;
			uint32_t m_sizeIncludingTag;

			uint32_t m_numFrameRanges;
			std::vector<FrameRangeDef> m_frameRanges;
		} m_frameRangesPart;
	};

	struct DOAssetDataSection final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_unknown1;
		uint32_t m_sizeIncludingTag;
	};

	DataObject* CreateObjectFromType(uint32_t objectType);
}
