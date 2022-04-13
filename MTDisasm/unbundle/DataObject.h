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
		kMIDIModf,
		kMediaCue,
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
		kTimerMessengerModifier,
		kBoundaryDetectionMessengerModifier,
		kCollisionDetectionMessengerModifier,
		kSetModifier,
		kKeyboardMessengerModifier,
		kBooleanVariableModifier,
		kIntegerVariableModifier,
		kIntegerRangeVariableModifier,
		kStringVariableModifier,
		kFloatVariableModifier,
		kCompoundVariableModifier,
		kVectorVariableModifier,
		kPointVariableModifier,
		kGraphicModifier,
		kTextStyleModifier,
		kDragMotionModifier,
		kVectorMotionModifier,

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

	struct DOColor
	{
		bool Load(DataReader& reader, const SerializationProperties& sp);

		uint16_t m_red;
		uint16_t m_green;
		uint16_t m_blue;
	};

	struct DOFloat
	{
		bool Load(DataReader& reader, const SerializationProperties& sp);

		double m_value;
	};

	struct DOVector
	{
		bool Load(DataReader& reader, const SerializationProperties& sp);

		DOFloat m_angleRadians;
		DOFloat m_magnitude;
	};

	struct DOTypicalModifierHeader
	{
		uint32_t m_modifierFlags;
		uint32_t m_sizeIncludingTag;
		uint32_t m_guid;
		uint8_t m_unknown2[6];	// 0
		uint32_t m_unknown3;
		uint8_t m_unknown4[4];	// ff
		uint16_t m_lengthOfName;

		std::vector<char> m_name;

		bool Load(DataReader& reader);
	};

	struct DOMessageDataSpec
	{
		enum
		{
			kNull = 0x0,
			kInteger = 0x1,
			kString = 0x0d,
			kPoint = 0x10,
			kIntRange = 0x11,
			kFloat = 0x15,
			kBool = 0x1a,
			kIncomingData = 0x1b,
			kReference = 0x1c,
			kLabel = 0x1d,
		};

		struct Label
		{
			uint32_t m_superGroupID;
			uint32_t m_id;
		};

		struct VariableRef
		{
			uint32_t m_unknown1;
			uint32_t m_guid;
		};

		struct IntRange
		{
			int32_t m_min;
			int32_t m_max;
		};

		union ValueUnion
		{
			uint8_t m_unknown[44];
			Label m_label;
			VariableRef m_varRef;
			IntRange m_intRange;
			uint8_t m_bool;
			DOFloat m_float;
			int32_t m_integer;
		};

		uint16_t m_typeCode;
		ValueUnion m_value;

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
			uint32_t m_id;
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
			kHasChildren		= 0x00000004,
			kLastChild			= 0x00000008,
		};
	}

	namespace ModifierFlags
	{
		enum
		{
			kNoMoreChildren		= 0x00000002,
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
			kSceneLocatorStreamIDMask = 0xfffffff,
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
		DOMessageDataSpec m_with;
		uint8_t m_withSourceLength;
		uint8_t m_withStringLength;

		std::vector<char> m_name;
		std::vector<char> m_withSource;
		std::vector<char> m_withString;
	};

	struct DOSetModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;

		uint8_t m_unknown1[4];
		DOEvent m_when;
		DOMessageDataSpec m_source;
		DOMessageDataSpec m_target;
		uint8_t m_unknown3;
		uint8_t m_sourceNameLength;
		uint8_t m_targetNameLength;
		uint8_t m_sourceStrLength;
		uint8_t m_targetStrLength;
		uint8_t m_unknown4;

		std::vector<char> m_sourceName;
		std::vector<char> m_targetName;
		std::vector<char> m_sourceStr;
		std::vector<char> m_targetStr;
	};

	struct DOIfMessengerModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
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

		std::vector<char> m_withSource;
	};

	struct DOBoundaryDetectionMessengerModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		enum HighFlags
		{
			kDetectTopEdge    = 0x1000,
			kDetectBottomEdge = 0x0800,
			kDetectLeftEdge   = 0x0400,
			kDetectRightEdge  = 0x0200,
			kDetectExiting    = 0x0100,	// Off = once exited
			kWhileDetected    = 0x0080,	// Off = on first detected
		};

		DOTypicalModifierHeader m_modHeader;
		uint16_t m_messageFlagsHigh;
		DOEvent m_enableWhen;
		DOEvent m_disableWhen;
		DOEvent m_send;
		uint16_t m_unknown2;
		uint32_t m_destination;
		uint8_t m_unknown3[10];
		DOMessageDataSpec m_with;
		uint8_t m_withSourceLength;
		uint8_t m_unknown4;

		std::vector<char> m_withSource;
	};

	struct DOCollisionDetectionMessengerModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		enum ModifierFlags
		{
			kDetectLayerInFront              = 0x10000000,
			kDetectLayerBehind               = 0x08000000,
			kSendToCollidingElement          = 0x02000000,
			kSendToOnlyFirstCollidingElement = 0x00200000,

			kDetectionModeMask               = 0x01c00000,
			kDetectionModeFirstContact       = 0x01400000,
			kDetectionModeWhileInContact     = 0x01000000,
			kDetectionModeExiting            = 0x00800000,

			kNoCollideWithParent             = 0x00100000,
		};

		DOTypicalModifierHeader m_modHeader;

		uint32_t m_messageAndModifierFlags;
		DOEvent m_enableWhen;
		DOEvent m_disableWhen;
		DOEvent m_send;
		uint16_t m_unknown2;
		uint32_t m_destination;
		uint8_t m_unknown3[10];
		DOMessageDataSpec m_with;
		uint8_t m_withSourceLength;
		uint8_t m_unknown4;

		std::vector<char> m_withSource;
	};

	struct DOTimerMessengerModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
		uint32_t m_messageAndTimerFlags;
		DOEvent m_executeWhen;
		DOEvent m_send;
		DOEvent m_terminateWhen;
		uint16_t m_unknown2;
		uint32_t m_destination;
		uint8_t m_unknown4[10];
		DOMessageDataSpec m_with;
		uint8_t m_unknown5;
		uint8_t m_minutes;
		uint8_t m_seconds;
		uint8_t m_hundredthsOfSeconds;
		uint32_t m_unknown6;
		uint32_t m_unknown7;
		uint8_t m_unknown8[10];
		uint8_t m_withSourceLength;
		uint8_t m_unknown9;

		std::vector<char> m_withSource;
	};

	struct DOKeyboardMessengerModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		enum MessageFlagEmbeds
		{
			kOnDown = 0x10000000,
			kOnUp = 0x4000000,
			kOnRepeat = 0x8000000,

			kKeyStateMask = (kOnDown | kOnUp | kOnRepeat),
		};

		enum KeyModifiers
		{
			kControl = 0x1000,
			kCommand = 0x0100,
			kOption = 0x0800,
		};

		DOTypicalModifierHeader m_modHeader;
		uint32_t m_messageFlagsAndKeyStates;
		uint16_t m_unknown2;
		uint16_t m_keyModifiers;
		uint8_t m_keycode;
		uint8_t m_unknown4[7];
		DOEvent m_message;
		uint16_t m_unknown7;
		uint32_t m_destination;
		uint8_t m_unknown9[10];
		DOMessageDataSpec m_with;
		uint8_t m_withSourceLength;
		uint8_t m_unknown14;

		std::vector<char> m_withSource;
	};

	struct DOBehaviorModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_modifierFlags;
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

		DOTypicalModifierHeader m_modHeader;
		uint8_t m_value;
		uint8_t m_unknown5;
	};

	struct DOIntegerVariableModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
		uint8_t m_unknown1[4];
		int32_t m_value;
	};

	struct DOStringVariableModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
		uint32_t m_lengthOfString;
		uint8_t m_unknown1[4];
		std::vector<char> m_string;
	};

	struct DOFloatVariableModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
		uint8_t m_unknown1[4];
		DOFloat m_value;
	};

	struct DOCompoundVariableModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		uint32_t m_modifierFlags;
		uint32_t m_sizeIncludingTag;
		uint8_t m_unknown1[2];
		uint32_t m_guid;
		uint8_t m_unknown4[6];
		uint32_t m_unknown5;
		DOPoint m_editorLayoutPosition;
		uint16_t m_lengthOfName;
		uint8_t m_unknown6[2];
		std::vector<char> m_name;
		uint8_t m_unknown7[4];
	};

	struct DOVectorVariableModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
		uint8_t m_unknown1[4];
		DOVector m_value;
	};

	struct DOIntegerRangeVariableModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
		uint8_t m_unknown1[4];
		int32_t m_min;
		int32_t m_max;
	};

	struct DOPointVariableModifier final : public DataObject
	{
		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
		uint8_t m_unknown5[4];
		DOPoint m_value;
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

	struct PlugInTypeTaggedValue
	{
		enum Type
		{
			kNull = 0x00,
			kInteger = 0x01,
			kBoolean = 0x14,
			kLabel = 0x64,
			kIncomingData = 0x6e,
			kVariableRef = 0x73,
		};

		struct Label
		{
			uint32_t m_id;
			uint32_t m_superGroup;
		};

		struct VariableRef
		{
			uint32_t m_guid;
			uint32_t m_extraDataSize;
			std::vector<uint8_t>* m_extraData;
		};

		union ValueUnion
		{
			Label m_lbl;
			VariableRef m_var;
			int32_t m_int;
			uint16_t m_bool;
		};

		bool Load(DataReader& reader, const SerializationProperties& sp);

		uint16_t m_type;
		ValueUnion m_value;

		PlugInTypeTaggedValue();
		~PlugInTypeTaggedValue();

		PlugInTypeTaggedValue(const PlugInTypeTaggedValue& other);
		PlugInTypeTaggedValue& operator=(const PlugInTypeTaggedValue& other);

	private:
		void Reset();
		void CopyFrom(const PlugInTypeTaggedValue& other);
	};

	struct POMediaCueModifier final : public PlugInObject
	{
		PlugInObjectType GetType() const override;
		bool Load(const DOPlugInModifier& base, DataReader& reader, const SerializationProperties& sp) override;

		enum TriggerTiming
		{
			kTriggerTimingStart = 0,
			kTriggerTimingDuring = 1,
			kTriggerTimingEnd = 2,
		};

		enum MessageFlags
		{
			kMessageFlagImmediate = 0x1,
			kMessageFlagCascade   = 0x2,
			kMessageFlagRelay     = 0x4,
		};

		uint16_t m_enableWhenTypeTag;
		DOEvent m_enableWhen;
		uint16_t m_disableWhenTypeTag;
		DOEvent m_disableWhen;
		uint16_t m_sendEventTypeTag;
		DOEvent m_sendEvent;
		uint16_t m_unknown1;
		uint32_t m_nonStandardMessageFlags;
		uint16_t m_unknown3;
		uint32_t m_destination;
		uint32_t m_unknown4;

		PlugInTypeTaggedValue m_with;
		PlugInTypeTaggedValue m_range;

		uint16_t m_unknown10;
		uint32_t m_triggerTiming;
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

	struct POMidiModifier final : public PlugInObject
	{
		PlugInObjectType GetType() const override;
		bool Load(const DOPlugInModifier& base, DataReader& reader, const SerializationProperties& sp) override;

		struct EmbeddedPart
		{
			uint8_t m_hasFile;
			uint8_t m_bigEndianLength[4];
			uint8_t m_loop;
			uint8_t m_overrideTempo;
			uint8_t m_volume;
			uint16_t m_tempoTypeTag;
			DOFloat m_tempo;
			uint16_t m_fadeInTypeTag;
			DOFloat m_fadeIn;
			uint16_t m_fadeOutTypeTag;
			DOFloat m_fadeOut;
		};

		struct SingleNotePart
		{
			uint8_t m_channel;
			uint8_t m_note;
			uint8_t m_velocity;
			uint8_t m_program;
			uint16_t m_durationTypeTag;
			DOFloat m_duration;
		};

		union TypeDependentPart
		{
			EmbeddedPart m_embedded;
			SingleNotePart m_singleNote;
		};

		uint16_t m_unknown1;
		DOEvent m_executeWhen;
		uint16_t m_unknown2;
		DOEvent m_terminateWhen;
		uint8_t m_embeddedFlag;

		TypeDependentPart m_typeDependent;

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

	struct DOGraphicModifier final : public DataObject {

		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
		uint16_t m_unknown1;
		DOEvent m_applyWhen;
		DOEvent m_removeWhen;
		uint8_t m_unknown2[2];
		uint16_t m_inkMode;
		uint16_t m_shape;

		struct MacPart
		{
			uint8_t m_unknown4_1[6];
			uint8_t m_unknown4_2[26];
		};

		struct WinPart
		{
			uint8_t m_unknown5_1[4];
			uint8_t m_unknown5_2[22];
		};

		union PlatformPart
		{
			MacPart m_mac;
			WinPart m_win;
		};

		bool m_haveMacPart;
		bool m_haveWinPart;
		PlatformPart m_platform;

		DOColor m_foreColor;
		DOColor m_backColor;
		uint16_t m_borderSize;
		DOColor m_borderColor;
		uint16_t m_shadowSize;
		DOColor m_shadowColor;

		uint16_t m_numPolygonPoints;
		uint8_t m_unknown6[8];

		std::vector<DOPoint> m_polyPoints;
	};

	struct DOTextStyleModifier final : public DataObject {

		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;
		uint8_t m_unknown1[4];
		uint16_t m_macFontID;
		uint8_t m_flags;
		uint8_t m_unknown2;
		uint16_t m_size;
		DOColor m_textColor;
		DOColor m_backgroundColor;
		uint16_t m_alignment;
		uint16_t m_unknown3;
		DOEvent m_applyWhen;
		DOEvent m_removeWhen;
		uint16_t m_lengthOfFontName;

		std::vector<char> m_fontName;
	};

	struct DODragMotionModifier final : public DataObject {

		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;

		DOEvent m_enableWhen;
		DOEvent m_disableWhen;

		struct WinPart
		{
			uint8_t m_unknown2;
			uint8_t m_constrainHorizontal;
			uint8_t m_constrainVertical;
			uint8_t m_constrainToParent;
		};

		struct MacPart
		{
			uint8_t m_flags;
			uint8_t m_unknown3;

			enum Flags
			{
				kConstrainToParent = 0x10,
				kConstrainVertical = 0x20,
				kConstrainHorizontal = 0x40,
			};
		};

		union PlatformPart
		{
			WinPart m_win;
			MacPart m_mac;
		};

		PlatformPart m_platform;

		bool m_haveMacPart;
		bool m_haveWinPart;
		DORect m_constraintMargin;
		uint16_t m_unknown1;
	};

	struct DOVectorMotionModifier final : public DataObject {

		DataObjectType GetType() const override;
		bool Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp) override;

		DOTypicalModifierHeader m_modHeader;

		DOEvent m_enableWhen;
		DOEvent m_disableWhen;
		DOMessageDataSpec m_varSource;
		uint16_t m_unknown1;
		uint8_t m_varSourceNameLength;
		uint8_t m_varStringLength;

		std::vector<char> m_varSourceName;
		std::vector<char> m_varString;
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
