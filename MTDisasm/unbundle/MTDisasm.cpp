#include "IOStream.h"
#include "CFileIOStream.h"
#include "Catalog.h"
#include "DataObject.h"
#include "DataReader.h"
#include "SliceIOStream.h"
#include "MemIOStream.h"

#include <string>
#include <vector>

#include <cstring>
#include <cstdio>
#include <cassert>

const char* NameObjectType(mtdisasm::DataObjectType dot)
{
	switch (dot)
	{
	case mtdisasm::DataObjectType::kUnknown:
		return "Unknown";
	case mtdisasm::DataObjectType::kStreamHeader:
		return "StreamHeader";
	case mtdisasm::DataObjectType::kPresentationSettings:
		return "PresentationSettings";
	case mtdisasm::DataObjectType::kGlobalObjectInfo:
		return "GlobalObjectInfo";
	case mtdisasm::DataObjectType::kUnknown19:
		return "Unknown19";
	case mtdisasm::DataObjectType::kDebris:
		return "Debris";
	case mtdisasm::DataObjectType::kProjectLabelMap:
		return "ProjectLabelMap";
	case mtdisasm::DataObjectType::kAssetCatalog:
		return "AssetCatalog";
	case mtdisasm::DataObjectType::kColorTableAsset:
		return "ColorTable";
	case mtdisasm::DataObjectType::kAudioAsset:
		return "AudioAsset";
	case mtdisasm::DataObjectType::kImageAsset:
		return "ImageAsset";
	case mtdisasm::DataObjectType::kMovieAsset:
		return "MovieAsset";
	case mtdisasm::DataObjectType::kMToonAsset:
		return "MToonAsset";
	case mtdisasm::DataObjectType::kProjectStructuralDef:
		return "ProjectStructuralDef";
	case mtdisasm::DataObjectType::kSectionStructuralDef:
		return "SectionStructuralDef";
	case mtdisasm::DataObjectType::kSubsectionStructuralDef:
		return "SubsectionStructuralDef";
	case mtdisasm::DataObjectType::kSceneStructuralDef:
		return "SceneStructuralDef";
	case mtdisasm::DataObjectType::kImageStructuralDef:
		return "ImageStructuralDef";
	case mtdisasm::DataObjectType::kMovieStructuralDef:
		return "MovieStructuralDef";
	case mtdisasm::DataObjectType::kMToonStructuralDef:
		return "MToonStructuralDef";
	case mtdisasm::DataObjectType::kBehaviorModifier:
		return "BehaviorModifier";
	case mtdisasm::DataObjectType::kNotYetImplemented:
		return "NotYetImplemented";
	case mtdisasm::DataObjectType::kPlugInModifier:
		return "PlugInModifier";
	case mtdisasm::DataObjectType::kAssetDataSection:
		return "AssetDataSection";
	case mtdisasm::DataObjectType::kMiniscriptModifier:
		return "MiniscriptModifier";
	default:
		return "BUG_NotNamed";
	}
}

void NameAssetType(char* assetName, uint32_t assetType)
{
	switch (assetType)
	{
	case mtdisasm::AssetTypeIDs::kColorTable:
		memcpy(assetName, "ColorTab", 8);
		break;
	case mtdisasm::AssetTypeIDs::kImage:
		memcpy(assetName, "Image   ", 8);
		break;
	case mtdisasm::AssetTypeIDs::kMToon:
		memcpy(assetName, "mToon   ", 8);
		break;
	case mtdisasm::AssetTypeIDs::kWaveformSound:
		memcpy(assetName, "Sound   ", 8);
		break;
	case mtdisasm::AssetTypeIDs::kMovie:
		memcpy(assetName, "QTMovie ", 8);
		break;
	case mtdisasm::AssetTypeIDs::kMIDI:
		memcpy(assetName, "MIDI    ", 8);
		break;
	case mtdisasm::AssetTypeIDs::kUnknown1f:
		memcpy(assetName, "Unknwn1f", 8);
		break;
	default:
		for (int i = 0; i < 8; i++)
			assetName[i] = ("0123456789abcdef")[(assetType >> (28 - i * 4)) & 0xf];
		break;
	}
}

void PrintSingleVal(uint32_t u32, bool asHex, FILE* f)
{
	if (asHex)
		fprintf(f, "%08x", static_cast<unsigned int>(u32));
	else
		fprintf(f, "%u", static_cast<unsigned int>(u32));
}

void PrintSingleVal(uint16_t u16, bool asHex, FILE* f)
{
	if (asHex)
		fprintf(f, "%04x", static_cast<unsigned int>(u16));
	else
		fprintf(f, "%u", static_cast<unsigned int>(u16));
}

void PrintSingleVal(uint8_t u8, bool asHex, FILE* f)
{
	if (asHex)
		fprintf(f, "%02x", static_cast<unsigned int>(u8));
	else
		fprintf(f, "%u", static_cast<unsigned int>(u8));
}

void PrintSingleVal(int32_t s32, bool asHex, FILE* f)
{
	if (asHex)
		fprintf(f, "%08x", static_cast<unsigned int>(s32 & 0xffffffff));
	else
		fprintf(f, "%i", static_cast<int>(s32));
}

void PrintSingleVal(int16_t s16, bool asHex, FILE* f)
{
	if (asHex)
		fprintf(f, "%04x", static_cast<unsigned int>(s16 & 0xffff));
	else
		fprintf(f, "%i", static_cast<int>(s16));
}

void PrintSingleVal(int8_t s8, bool asHex, FILE* f)
{
	if (asHex)
		fprintf(f, "%02x", static_cast<unsigned int>(s8 & 0xff));
	else
		fprintf(f, "%i", static_cast<int>(s8));
}

void PrintSingleVal(const mtdisasm::DORect& rect, bool asHex, FILE* f)
{
	fputs("(", f);
	PrintSingleVal(rect.m_left, asHex, f);
	fputs(",", f);
	PrintSingleVal(rect.m_top, asHex, f);
	fputs(")-(", f);
	PrintSingleVal(rect.m_right, asHex, f);
	fputs(",", f);
	PrintSingleVal(rect.m_bottom, asHex, f);
	fprintf(f, ") [%i x %i]", static_cast<int>(rect.m_right - rect.m_left), static_cast<int>(rect.m_bottom - rect.m_top));
}

void PrintSingleVal(const mtdisasm::DOPoint& pt, bool asHex, FILE* f)
{
	fputs("(", f);
	PrintSingleVal(pt.m_left, asHex, f);
	fputs(",", f);
	PrintSingleVal(pt.m_top, asHex, f);
	fputs(")", f);
}

void PrintSingleVal(const mtdisasm::DOEvent& pt, bool asHex, FILE* f)
{
	fputs("event(", f);
	PrintSingleVal(pt.m_eventID, asHex, f);
	fputs(",", f);
	PrintSingleVal(pt.m_eventInfo, asHex, f);
	fputs(")", f);
}

template<size_t TSize, class T>
void PrintSingleVal(const T (&arr)[TSize], bool asHex, FILE* f)
{
	for (size_t i = 0; i < TSize; i++)
	{
		if (i != 0)
			fputc(' ', f);
		PrintSingleVal(arr[i], asHex, f);
	}
}

template<class T>
void PrintVal(const char* name, const T& value, FILE* f)
{
	fputs(name, f);
	fputs(": ", f);
	PrintSingleVal(value, false, f);
	fputs("\n", f);
}

template<class T>
void PrintHex(const char* name, const T& value, FILE* f)
{
	fputs(name, f);
	fputs(": ", f);
	PrintSingleVal(value, true, f);
	fputs("\n", f);
}

void PrintStr(const char* name, const char* value, FILE* f)
{
	fputs(name, f);
	fputs(": ", f);
	fputs(value, f);
	fputs("\n", f);
}

void PrintObjectDisassembly(const mtdisasm::DOStreamHeader& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kStreamHeader);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("Size", obj.m_sizeIncludingTag, f);
	PrintStr("Name", obj.m_name, f);
	PrintHex("ProjectID", obj.m_projectID, f);
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
}

void PrintObjectDisassembly(const mtdisasm::DOPresentationSettings& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kPresentationSettings);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("Size", obj.m_sizeIncludingTag, f);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("Dimensions", obj.m_dimensions, f);
	PrintVal("BitsPerPixel", obj.m_bitsPerPixel, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
}

void PrintObjectDisassembly(const mtdisasm::DOGlobalObjectInfo& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kGlobalObjectInfo);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("Size", obj.m_sizeIncludingTag, f);
	PrintVal("NumGlobalModifiers", obj.m_numGlobalModifiers, f);
	PrintHex("Unknown1", obj.m_unknown1, f);
}

void PrintObjectDisassembly(const mtdisasm::DOUnknown19& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kUnknown19);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("Size", obj.m_sizeIncludingTag, f);

	PrintHex("Unknown1", obj.m_unknown1, f);
}

void PrintObjectDisassembly(const mtdisasm::DODebris& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kDebris);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("Size", obj.m_sizeIncludingTag, f);
}

void PrintLabelTree(const mtdisasm::DOProjectLabelMap::LabelTree& obj, FILE* f, int indentLevel)
{
	for (int i = 0; i < indentLevel; i++)
		fputs("    ", f);


	fprintf(f, "Item '");
	if (obj.m_nameLength > 0)
		fwrite(&obj.m_name[0], 1, obj.m_nameLength, f);
	fprintf(f, "'  IsGroup=%u  ID=%x  Unknown2=%x  Flags=%x\n", obj.m_isGroup, obj.m_id, obj.m_unknown1, obj.m_flags);

	for (size_t i = 0; i < obj.m_numChildren; i++)
		PrintLabelTree(obj.m_children[i], f, indentLevel + 1);
}

void PrintObjectDisassembly(const mtdisasm::DOProjectLabelMap& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kProjectLabelMap);

	PrintHex("Marker", obj.m_marker, f);
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("NumSuperGroups", obj.m_numSuperGroups, f);
	PrintVal("NextAvailableID", obj.m_nextAvailableID, f);

	for (size_t i = 0; i < obj.m_numSuperGroups; i++)
	{
		const mtdisasm::DOProjectLabelMap::SuperGroup& sg = obj.m_superGroups[i];
		fprintf(f, "SuperGroup '");
		if (sg.m_nameLength > 0)
			fwrite(&sg.m_name[0], 1, sg.m_nameLength, f);

		fprintf(f, "'  NumChildren=%u  Unknown1=%x  Unknown2=%x\n", sg.m_numChildren, sg.m_unknown1, sg.m_unknown2);

		for (size_t j = 0; j < sg.m_numChildren; j++)
			PrintLabelTree(sg.m_tree[j], f, 1);
	}
}

void PrintObjectDisassembly(const mtdisasm::DOProjectStructuralDef& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kProjectStructuralDef);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Flags", obj.m_flags, f);
	fputs("Name: '", f);
	if (obj.m_nameLength >= 1)
		fwrite(&obj.m_name[0], 1, obj.m_nameLength - 1, f);
	fputs("'\n", f);
}

void PrintObjectDisassembly(const mtdisasm::DOAssetCatalog& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kAssetCatalog);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("TotalNameSizePlus22", obj.m_totalNameSizePlus22, f);
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("NumAssets", obj.m_numAssets, f);

	for (uint32_t i = 0; i < obj.m_numAssets; i++)
	{
		const mtdisasm::DOAssetCatalog::AssetInfo& asset = obj.m_assets[i];

		char assetTypeName[9];
		NameAssetType(assetTypeName, asset.m_assetType);
		assetTypeName[8] = 0;
		fprintf(f, "Asset % 4u: Flags1=%08x  AlwaysZero=%04x  Unknown1=%08x  FilePosition=%08x  AssetType=%s  Flags2=%08x", static_cast<unsigned int>(i + 1), asset.m_flags1, asset.m_alwaysZero, asset.m_unknown1, asset.m_filePosition, assetTypeName, asset.m_flags2);
		if (asset.m_nameLength > 0)
		{
			fputs("  ", f);
			fwrite(&asset.m_name[0], 1, asset.m_nameLength - 1, f);
		}
		fputs("\n", f);
	}
}

void PrintObjectDisassembly(const mtdisasm::DOColorTableAsset& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kColorTableAsset);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("AssetID", obj.m_assetID, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	fprintf(f, "Colors:");

	for (uint32_t i = 0; i < 256; i++)
	{
		if (i % 16 == 0)
			fputs("\n   ", f);
		fputc(' ', f);

		const mtdisasm::DOColorTableAsset::ColorDef& cdef = obj.m_colors[i];
		fprintf(f, "%02x%02x%02x", (cdef.m_red / 0x101), (cdef.m_green / 0x101), (cdef.m_blue / 0x101));
	}
	fputc('\n', f);
}

void PrintObjectDisassembly(const mtdisasm::DOSectionStructuralDef& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kSectionStructuralDef);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintVal("SegmentID", obj.m_segmentID, f);
	PrintVal("SectionID", obj.m_sectionID, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Flags", obj.m_flags, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);
}

void PrintObjectDisassembly(const mtdisasm::DOSubsectionStructuralDef& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kSubsectionStructuralDef);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Flags", obj.m_flags, f);
	PrintVal("SectionID", obj.m_sectionID, f);
	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);
}

void PrintObjectDisassembly(const mtdisasm::DOSceneStructuralDef& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kSceneStructuralDef);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("GUID", obj.m_guid, f);
	PrintHex("Flags", obj.m_flags, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintVal("SectionID", obj.m_sectionID, f);
	PrintVal("Rect1", obj.m_rect1, f);
	PrintVal("Rect2", obj.m_rect2, f);
	PrintHex("StreamLocator", obj.m_streamLocator, f);
	fprintf(f, "    Stream ID: %i\n", static_cast<int>(obj.m_streamLocator & mtdisasm::DOSceneStructuralDef::kSceneLocatorStreamIDMask));

	PrintHex("Unknown11", obj.m_unknown11, f);
	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);
}

void PrintObjectDisassembly(const mtdisasm::DOImageStructuralDef& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kImageStructuralDef);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Flags", obj.m_flags, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintVal("SectionID", obj.m_sectionID, f);
	PrintVal("Rect1", obj.m_rect1, f);
	PrintVal("Rect2", obj.m_rect2, f);
	PrintVal("ImageAssetID", obj.m_imageAssetID, f);
	PrintVal("StreamLocator", obj.m_streamLocator, f);
	fprintf(f, "    Stream ID: %i\n", static_cast<int>(obj.m_streamLocator & mtdisasm::DOSceneStructuralDef::kSceneLocatorStreamIDMask));
	PrintHex("Unknown7", obj.m_unknown7, f);
	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);
}

void PrintObjectDisassembly(const mtdisasm::DOMovieStructuralDef& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kMovieStructuralDef);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Flags", obj.m_flags, f);
	PrintHex("Layer", obj.m_layer, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintVal("SectionID", obj.m_sectionID, f);
	PrintHex("Unknown5", obj.m_unknown5, f);
	PrintVal("Rect1", obj.m_rect1, f);
	PrintVal("Rect2", obj.m_rect2, f);
	PrintVal("AssetID", obj.m_assetID, f);
	PrintVal("Unknown7", obj.m_unknown7, f);
	PrintVal("Volume", obj.m_volume, f);
	PrintHex("AnimationFlags", obj.m_animationFlags, f);
	PrintHex("Unknown10", obj.m_unknown10, f);
	PrintHex("Unknown11", obj.m_unknown11, f);
	PrintHex("StreamLocator", obj.m_streamLocator, f);
	fprintf(f, "    Stream ID: %i\n", static_cast<int>(obj.m_streamLocator & mtdisasm::DOSceneStructuralDef::kSceneLocatorStreamIDMask));
	PrintHex("Unknown13", obj.m_unknown13, f);
	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);
}

void PrintObjectDisassembly(const mtdisasm::DOMToonStructuralDef& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kMToonStructuralDef);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintVal("LengthOfName", obj.m_lengthOfName, f);
	PrintHex("StructuralFlags", obj.m_structuralFlags, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintHex("AnimationFlags", obj.m_animationFlags, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintVal("SectionID", obj.m_sectionID, f);
	PrintVal("Rect1", obj.m_rect1, f);
	PrintVal("Rect2", obj.m_rect2, f);
	PrintHex("Unknown5", obj.m_unknown5, f);
	PrintVal("RateTimes10000", obj.m_rateTimes10000, f);
	PrintHex("StreamLocator", obj.m_streamLocator, f);
	PrintHex("Unknown6", obj.m_unknown6, f);

	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);
}

bool PrintMiniscriptInstructionDisassembly(FILE* f, mtdisasm::DataReader& reader, const mtdisasm::SerializationProperties& sp, int opcode, int sizeOfInstrData)
{
	switch (opcode)
	{
	case 0x898:
		{
			mtdisasm::DOEvent evt;
			if (!evt.Load(reader))
				return false;
			PrintSingleVal(evt, false, f);
		}
		break;
	case 0xd8:	// Builtin function
	case 0x135:	// Get attribute
		{
			uint32_t funcID;
			if (!reader.ReadU32(funcID))
				return false;
			PrintSingleVal(funcID, true, f);
		}
		break;
	case 0x192: // Push global
		{
			uint32_t globID;
			if (!reader.ReadU32(globID))
				return false;
			if (globID == 1)
				fputs("default", f);
			else if (globID == 2)
				fputs("subsection", f);
			else if (globID == 3)
				fputs("source", f);
			else if (globID == 4)
				fputs("incoming", f);
			else if (globID == 5)
				fputs("mouse", f);
			else if (globID == 6)
				fputs("ticks", f);
			else if (globID == 7)
				fputs("scene", f);
			else if (globID == 8)
				fputs("sharedScene", f);
			else if (globID == 9)
				fputs("section", f);
			else if (globID == 10)
				fputs("project", f);
			else if (globID == 11)
				fputs("sharedScene", f);
			else
				PrintSingleVal(globID, true, f);
		}
		break;
	case 0x7d3: // Jump
		{
			uint32_t flags, unknown, instrOffset;
			if (!reader.ReadU32(flags) || !reader.ReadU32(unknown) || !reader.ReadU32(instrOffset))
				return false;
			if (flags == 2)
				fputs("conditional ", f);
			else if (flags == 1)
			{
			}
			else
				fprintf(f, "unknown_flags(%08x)", flags);

			fprintf(f, "  unknown=%x  skip=%u", unknown, instrOffset);

		}
		break;
	case 0x191:
		{
			uint16_t dataType;
			if (!reader.ReadU16(dataType))
				return false;

			if (dataType == 0)
				fputs("null", f);
			else if (dataType == 0x15)
			{
				double d;
				if (!reader.ReadF64(d))
					return false;
				fprintf(f, "double %f", d);
			}
			else if (dataType == 0x1a)
			{
				uint8_t b;
				if (!reader.ReadU8(b))
					return false;
				fprintf(f, "bool %s", ((b == 0) ? "false" : "true"));
			}
			else if (dataType == 0x1f9)
			{
				uint32_t u;
				if (!reader.ReadU32(u))
					return false;
				fprintf(f, "local %u", u);
			}
			else if (dataType == 0x1fa)
			{
				uint32_t u;
				if (!reader.ReadU32(u))
					return false;
				fprintf(f, "global %08x", u);
			}
			else
				fprintf(f, "unknown_type %x", static_cast<int>(dataType));
		}
		break;
	case 0x193:
		{
			uint16_t strLength;
			if (!reader.ReadU16(strLength))
				return false;

			std::vector<char> str;
			if (strLength > 0)
			{
				str.resize(strLength + 1);
				if (!reader.ReadBytes(&str[0], strLength + 1) || str[strLength] != 0)
					return false;
				fprintf(f, "str '%s'", &str[0]);
			}
			else
				fputs("str ''", f);
		}
		break;
	default:
		for (int i = 0; i < sizeOfInstrData; i++)
		{
			uint8_t byte;
			if (!reader.ReadU8(byte))
				return false;

			if (i != 0)
				fputc(' ', f);
			fprintf(f, "%02x", static_cast<int>(byte));
		}
		break;
	}

	return true;
}

void PrintObjectDisassembly(const mtdisasm::DOMiniscriptModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kMiniscriptModifier);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintHex("LengthOfName", obj.m_lengthOfName, f);
	PrintHex("EnableWhen", obj.m_enableWhen, f);
	PrintHex("Unknown6", obj.m_unknown6, f);
	PrintHex("Unknown7", obj.m_unknown7, f);
	PrintHex("Unknown8", obj.m_unknown8, f);
	PrintHex("SizeOfInstructions", obj.m_sizeOfInstructions, f);
	PrintHex("NumOfInstructions", obj.m_numOfInstructions, f);
	PrintHex("NumLocalRefs", obj.m_numLocalRefs, f);
	PrintHex("NumAttributes", obj.m_numAttributes, f);

	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);

	fputs("Attributes:\n", f);
	for (size_t i = 0; i < obj.m_numAttributes; i++)
	{
		const mtdisasm::DOMiniscriptModifier::Attribute& attrib = obj.m_attributes[i];
		fprintf(f, "    % 5i: %02x '", static_cast<int>(i), static_cast<int>(attrib.m_unknown11));
		if (attrib.m_name.size() > 1)
			fwrite(&attrib.m_name[0], 1, attrib.m_name.size() - 1, f);
		fputs("'\n", f);
	}

	fputs("Local references:\n", f);
	for (size_t i = 0; i < obj.m_numLocalRefs; i++)
	{
		const mtdisasm::DOMiniscriptModifier::LocalRef& ref = obj.m_localRefs[i];
		fprintf(f, "    % 5i: %08x %02x '", static_cast<int>(i), static_cast<int>(ref.m_unknown9), static_cast<int>(ref.m_unknown10));
		if (ref.m_name.size() > 1)
			fwrite(&ref.m_name[0], 1, ref.m_name.size() - 1, f);
		fputs("'\n", f);
	}

	fputs("Instructions:\n", f);
	bool readFailure = false;
	if (obj.m_sizeOfInstructions > 0)
	{
		mtdisasm::MemIOStream memStream(&obj.m_bytecode[0], obj.m_bytecode.size());
		mtdisasm::DataReader reader(memStream, obj.m_sp.m_isByteSwapped);

		size_t numInstrsDecoded = 0;
		for (size_t i = 0; i < obj.m_numOfInstructions; i++)
		{
			const char* opName = "???";

			uint32_t instrStartPos = memStream.Tell();

			uint16_t opcode, unknownField, sizeOfInstruction;
			if (!reader.ReadU16(opcode) || !reader.ReadU16(unknownField), !reader.ReadU16(sizeOfInstruction))
				break;

			bool isUnknownOp = false;
			switch (opcode)
			{
			case 0x834: opName = "set"; break;
			case 0x898: opName = "send"; break;
			case 0xc9: opName = "add"; break;
			case 0xca: opName = "sub"; break;
			case 0xcb: opName = "mul"; break;
			case 0xcc: opName = "div"; break;
			case 0xcd: opName = "pow"; break;
			case 0xce: opName = "and"; break;
			case 0xcf: opName = "or"; break;
			case 0xd0: opName = "neg"; break;
			case 0xd1: opName = "not"; break;
			case 0xd2: opName = "cmp_eq"; break;
			case 0xd3: opName = "cmp_neq"; break;
			case 0xd4: opName = "cmp_le"; break;
			case 0xd5: opName = "cmp_lt"; break;
			case 0xd6: opName = "cmp_le"; break;
			case 0xd7: opName = "cmp_gt"; break;
			case 0xd8: opName = "builtin_func"; break;
			case 0xda: opName = "mod"; break;
			case 0xdb: opName = "str_concat"; break;
			case 0x12f: opName = "point_create"; break;
			case 0x130: opName = "make_range"; break;
			case 0x131: opName = "make_vector"; break;
			case 0x135: opName = "get_attr"; break;
			case 0x136: opName = "list_append"; break;
			case 0x137: opName = "list_create"; break;
			case 0x191: opName = "push_value"; break;
			case 0x192: opName = "push_global"; break;
			case 0x193: opName = "push_str"; break;
			case 0x7d3: opName = "jump"; break;
			default:
				isUnknownOp = true;
				break;
			}

			if (!isUnknownOp)
			{
				if (unknownField == 0)
					fprintf(f, "    %s  ", opName);
				else
					fprintf(f, "    %s,args=%i  ", opName, static_cast<int>(unknownField));
			}
			else
			{

				fprintf(f, "    %s(0x%x),%i  ", opName, static_cast<int>(opcode), static_cast<int>(unknownField));
			}


			if (sizeOfInstruction < 6)
				break;

			bool decodedOK = PrintMiniscriptInstructionDisassembly(f, reader, obj.m_sp, opcode, sizeOfInstruction - 6);
			fputs("\n", f);

			if (!decodedOK)
				break;

			memStream.SeekSet(instrStartPos + sizeOfInstruction);

			numInstrsDecoded++;
		}

		if (numInstrsDecoded != obj.m_numOfInstructions)
			fprintf(f, "    <An error occurred during disassembly>\n");
	}


}

void PrintObjectDisassembly(const mtdisasm::DONotYetImplemented& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kNotYetImplemented);

	fprintf(f, "    Unimplemented '%s'  DataSize=%u\n", obj.m_name, obj.m_sizeIncludingTag - 14);
}

void PrintObjectDisassembly(const mtdisasm::DOPlugInModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kPlugInModifier);

	fprintf(f, "Plug-in modifier '%s'  PrivateDataSize=%u\n", obj.m_plugin, obj.m_privateDataSize);
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("WeirdSize", obj.m_weirdSize, f);
	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);
}

void PrintObjectDisassembly(const mtdisasm::DOAudioAsset& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kAudioAsset);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("AssetAndDataCombinedSize", obj.m_assetAndDataCombinedSize, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintVal("AssetID", obj.m_assetID, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintVal("SampleRate1", obj.m_sampleRate1, f);
	PrintVal("BitsPerSample", obj.m_bitsPerSample, f);
	PrintVal("Encoding1", obj.m_encoding1, f);
	PrintVal("Channels", obj.m_channels, f);
	PrintVal("CodedDuration", obj.m_codedDuration, f);
	PrintVal("SampleRate2", obj.m_sampleRate2, f);
	PrintVal("ExtraDataSize", obj.m_extraDataSize, f);
	PrintHex("Unknown13", obj.m_unknown13, f);
	PrintHex("Unknown14", obj.m_unknown14, f);
	PrintHex("FilePosition", obj.m_filePosition, f);
	PrintVal("Size", obj.m_size, f);

	if (obj.m_haveMacPart)
	{
		PrintHex("Unknown4", obj.m_macPart.m_unknown4, f);
		PrintHex("Unknown5", obj.m_macPart.m_unknown5, f);
		PrintHex("Unknown6", obj.m_macPart.m_unknown6, f);
		PrintHex("Unknown8", obj.m_macPart.m_unknown8, f);
	}

	if (obj.m_haveWinPart)
	{

		PrintHex("Unknown9", obj.m_winPart.m_unknown9, f);
		PrintHex("Unknown10", obj.m_winPart.m_unknown10, f);
		PrintHex("Unknown11", obj.m_winPart.m_unknown11, f);
		PrintHex("Unknown12_1", obj.m_winPart.m_unknown12_1, f);
	}
}

void PrintObjectDisassembly(const mtdisasm::DOImageAsset& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kImageAsset);

	PrintHex("Marker", obj.m_marker, f);
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintVal("AssetID", obj.m_assetID, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintVal("Rect1", obj.m_rect1, f);
	PrintHex("HDPI", obj.m_hdpiFixed, f);
	PrintHex("VDPI", obj.m_vdpiFixed, f);
	PrintVal("BitsPerPixel", obj.m_bitsPerPixel, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintHex("Unknown5", obj.m_unknown5, f);
	PrintHex("Unknown6", obj.m_unknown6, f);
	PrintVal("Rect2", obj.m_rect2, f);
	PrintHex("FilePosition", obj.m_filePosition, f);
	PrintVal("Size", obj.m_size, f);

	if (obj.m_haveMacPart)
		PrintHex("Unknown7", obj.m_platform.m_mac.m_unknown7, f);
	if (obj.m_haveWinPart)
		PrintHex("Unknown8", obj.m_platform.m_win.m_unknown8, f);
}

void PrintObjectDisassembly(const mtdisasm::DOMovieAsset& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kMovieAsset);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("AssetAndDataCombinedSize", obj.m_marker, f);
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("AssetID", obj.m_assetID, f);
	PrintHex("Unknown1_1", obj.m_unknown1_1, f);
	PrintHex("ExtFileNameLength", obj.m_extFileNameLength, f);
	PrintHex("MovieDataPos", obj.m_movieDataPos, f);
	PrintVal("MovieDataSize", obj.m_movieDataSize, f);
	PrintHex("MoovAtomPos", obj.m_moovAtomPos, f);

	if (obj.m_haveMacPart)
	{
		PrintHex("Unknown5_1", obj.m_macPart.m_unknown5_1, f);
		PrintHex("Unknown6", obj.m_macPart.m_unknown6, f);
	}

	if (obj.m_haveWinPart)
	{
		PrintHex("Unknown3_1", obj.m_winPart.m_unknown3_1, f);
		PrintHex("Unknown4", obj.m_winPart.m_unknown4, f);
		PrintHex("Unknown7", obj.m_winPart.m_unknown7, f);
	}

	if (obj.m_extFileNameLength > 1)
	{
		fputs("ExternalFileName: '", f);
		fwrite(&obj.m_extFileName[0], 1, obj.m_extFileNameLength - 1, f);
		fputs("'\n", f);
	}
}


void PrintObjectDisassembly(const mtdisasm::DOMToonAsset& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kMToonAsset);

	PrintHex("Marker", obj.m_marker, f);
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintVal("AssetID", obj.m_assetID, f);
	PrintHex("SizeOfFrameData", obj.m_sizeOfFrameData, f);

	PrintHex("MToonHeader", obj.m_mtoonHeader, f);
	PrintVal("Version", obj.m_version, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("EncodingFlags", obj.m_encodingFlags, f);
	PrintHex("Rect", obj.m_rect, f);

	PrintVal("NumFrames", obj.m_numFrames, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintVal("BitsPerPixel", obj.m_bitsPerPixel, f);
	PrintHex("CodecID", obj.m_codecID, f);
	PrintHex("Unknown4_1", obj.m_unknown4_1, f);
	PrintHex("CodecDataSize", obj.m_codecDataSize, f);
	PrintHex("Unknown4_2", obj.m_unknown4_2, f);

	for (size_t i = 0; i < obj.m_numFrames; i++)
	{
		fprintf(f, "Frame %i:\n", static_cast<int>(i));
		const mtdisasm::DOMToonAsset::FrameDef& frame = obj.m_frames[i];

		if (obj.m_haveMacPart)
			PrintHex("Unknown12", frame.m_platform.m_mac.m_unknown12, f);
		if (obj.m_haveWinPart)
			PrintHex("Unknown13", frame.m_platform.m_win.m_unknown13, f);
	}

	fprintf(f, "CodecData:");
	for (size_t i = 0; i < obj.m_codecData.size(); i++)
		fprintf(f, " %02x", static_cast<int>(obj.m_codecData[i]));
	fprintf(f, "\n");

	if (obj.m_encodingFlags & mtdisasm::DOMToonAsset::kEncodingFlag_HasRanges)
	{
		PrintHex("FrameRangesTag", obj.m_frameRangesPart.m_tag, f);
		PrintHex("FrameRangesSizeIncludingTag", obj.m_frameRangesPart.m_sizeIncludingTag, f);
		PrintHex("NumFrameRanges", obj.m_frameRangesPart.m_numFrameRanges, f);
		PrintHex("Unknown4_2", obj.m_unknown4_2, f);

		for (size_t i = 0; i < obj.m_frameRangesPart.m_numFrameRanges; i++)
		{
			fprintf(f, "FrameRange %i:\n", static_cast<int>(i));
			const mtdisasm::DOMToonAsset::FrameRangeDef& frameRange = obj.m_frameRangesPart.m_frameRanges[i];

			PrintVal("StartFrame", frameRange.m_startFrame, f);
			PrintVal("EndFrame", frameRange.m_startFrame, f);
			fputs("Name: '", f);
			if (frameRange.m_name.size() > 1)
				fwrite(&frameRange.m_name[0], 1, frameRange.m_name.size() - 1, f);
			fputs("'\n", f);
			PrintHex("Unknown14", frameRange.m_unknown14, f);
		}
	}
}

void PrintObjectDisassembly(const mtdisasm::DOAssetDataSection& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kAssetDataSection);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("SizeIncludingTag", obj.m_sizeIncludingTag, f);
}

void PrintObjectDisassembly(const mtdisasm::DOBehaviorModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kBehaviorModifier);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintHex("Unknown5", obj.m_unknown5, f);
	PrintHex("Unknown6", obj.m_unknown6, f);
	PrintVal("EditorLayoutPosition", obj.m_editorLayoutPosition, f);
	PrintVal("LengthOfName", obj.m_lengthOfName, f);
	PrintVal("NumChildren", obj.m_numChildren, f);
	PrintHex("Flags", obj.m_flags, f);
	PrintVal("EnableWhen", obj.m_enableWhen, f);
	PrintVal("DisableWhen", obj.m_disableWhen, f);
	PrintHex("Unknown7", obj.m_unknown7, f);

	if (obj.m_name.size() > 1)
	{
		fputs("Name: '", f);
		fwrite(&obj.m_name[0], 1, obj.m_name.size() - 1u, f);
		fputs("'\n", f);
	}
}

void PrintObjectDisassembly(const mtdisasm::DataObject& obj, FILE* f)
{
	switch (obj.GetType())
	{
	case mtdisasm::DataObjectType::kStreamHeader:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOStreamHeader&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kPresentationSettings:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOPresentationSettings&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kAssetCatalog:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOAssetCatalog&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kGlobalObjectInfo:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOGlobalObjectInfo&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kUnknown19:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOUnknown19&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kProjectLabelMap:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOProjectLabelMap&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kProjectStructuralDef:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOProjectStructuralDef&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kColorTableAsset:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOColorTableAsset&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kAudioAsset:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOAudioAsset&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kImageAsset:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOImageAsset&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kMovieAsset:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOMovieAsset&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kMToonAsset:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOMToonAsset&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kSectionStructuralDef:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOSectionStructuralDef&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kSubsectionStructuralDef:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOSubsectionStructuralDef&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kSceneStructuralDef:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOSceneStructuralDef&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kImageStructuralDef:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOImageStructuralDef&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kMovieStructuralDef:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOMovieStructuralDef&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kMToonStructuralDef:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOMToonStructuralDef&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kNotYetImplemented:
		PrintObjectDisassembly(static_cast<const mtdisasm::DONotYetImplemented&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kPlugInModifier:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOPlugInModifier&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kAssetDataSection:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOAssetDataSection&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kBehaviorModifier:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOBehaviorModifier&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kMiniscriptModifier:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOMiniscriptModifier&>(obj), f);
		break;

	default:
		fprintf(f, "Unknown contents\n");
	}
}

void DisassembleStream(mtdisasm::IOStream& stream, size_t streamSize, int streamIndex, uint32_t streamPos, const mtdisasm::SerializationProperties& sp, FILE* f)
{
	mtdisasm::DataReader reader(stream, sp.m_isByteSwapped);

	for (;;)
	{
		uint32_t pos = stream.Tell();
		if (pos == streamSize)
			break;

		uint32_t objectType = 0;
		uint16_t revision = 0;
		if (!reader.ReadU32(objectType) || !reader.ReadU16(revision))
		{
			fprintf(stderr, "Stream %i: Couldn't read type at stream position %x (global position %x)\n", streamIndex, static_cast<int>(pos), static_cast<int>(pos + streamPos));
			return;
		}

		mtdisasm::DataObject* dataObject = mtdisasm::CreateObjectFromType(objectType);
		if (!dataObject)
		{
			fprintf(stderr, "Stream %i: Unknown object type %x revision %i at position %x (global position %x)\n", streamIndex, static_cast<int>(objectType), static_cast<int>(revision), static_cast<int>(pos), static_cast<int>(pos + streamPos));
			return;
		}

		fprintf(f, "Pos=%x AbsPos=%x  %s (%x) rev %i:\n", static_cast<int>(pos), static_cast<int>(pos + streamPos), NameObjectType(dataObject->GetType()), static_cast<int>(objectType), static_cast<int>(revision));

		const bool succeeded = dataObject->Load(reader, revision, sp);
		if (succeeded)
		{
			PrintObjectDisassembly(*dataObject, f);
		}
		else
		{
			fprintf(stderr, "Stream %i: Object type %s revision %i at position %x (global position %x) failed to load\n", streamIndex, NameObjectType(dataObject->GetType()), static_cast<int>(revision), static_cast<int>(pos), static_cast<int>(pos + streamPos));
			fprintf(f, "FAILED\n");
		}

		dataObject->Delete();

		fprintf(f, "\n");

		if (!succeeded)
			break;
	}
}

void PrintCatalogDisassembly(const mtdisasm::Catalog& cat, FILE* f)
{
	fprintf(f, "System desc: %i\n", static_cast<int>(cat.GetSystem()));

	const mtdisasm::CatalogHeader& catHeader = cat.GetCatalogHeader();
	PrintHex("Platform", catHeader.m_platform, f);
	PrintHex("Unknown34", catHeader.m_unknown34, f);

	fprintf(f, "Streams:\n");

	size_t numStreams = cat.NumStreams();
	for (size_t i = 0; i < numStreams; i++)
	{
		const mtdisasm::StreamDesc& stream = cat.GetStream(i);
		fprintf(f, "    Stream % 4i   Pos: %i  Size: %i  Segment: %i  Type: '%s'\n", static_cast<int>(i + 1), static_cast<int>(stream.m_pos), static_cast<int>(stream.m_size), static_cast<int>(stream.m_segmentNumber), stream.m_streamType);
	}

	size_t numSegments = cat.NumSegments();

	fprintf(f, "\nSegments:\n");
	for (size_t i = 0; i < numSegments; i++)
	{
		const mtdisasm::SegmentDesc& seg = cat.GetSegment(i);
		fprintf(f, "    Path: '%s'  Label: '%s'  SegmentID: %i\n", seg.m_exportedPath.c_str(), seg.m_label.c_str(), seg.m_segmentID);
	}
}

int main(int argc, const char** argv)
{
	if (argc != 4)
	{
		fprintf(stderr, "Usage: unbundle <mode> <segment 1 path> <output dir>\n");
		return -1;
	}

	std::string mode = argv[1];
	std::string seg1Path = argv[2];
	std::string outputDir = argv[3];

	if (mode != "bin" && mode != "text")
	{
		fprintf(stderr, "Supported disassembly modes: bin, text\n");
		return -1;
	}

	if (seg1Path.size() < 5)
	{
		fprintf(stderr, "Segment 1 path needs to end in .MPL");
		return -1;
	}

	printf("Loading catalog...\n");
	FILE* catFile = fopen(seg1Path.c_str(), "rb");

	if (!catFile)
	{
		fprintf(stderr, "Failed to open catalog file\n");
		return -1;
	}

	mtdisasm::CFileIOStream seg1Stream(catFile);

	uint8_t systemCheck[2];
	if (!seg1Stream.ReadAll(systemCheck, 2))
	{
		fprintf(stderr, "Failed to read system ID\n");
		return -1;
	}


	mtdisasm::SerializationProperties sp;

	bool isBigEndian = false;
	if (systemCheck[0] == 0 && systemCheck[1] == 0)
	{
		printf("Detected as Macintosh format\n");
		isBigEndian = true;
		sp.m_systemType = mtdisasm::SystemType::kMac;
	}
	else if (systemCheck[0] == 1 && systemCheck[1] == 0)
	{
		printf("Detected as Windows format\n");
		isBigEndian = false;
		sp.m_systemType = mtdisasm::SystemType::kWindows;
	}
	else
	{
		fprintf(stderr, "Unknown system value\n");
		return -1;
	}

	uint32_t byteOrderCheckInt;
	uint8_t byteOrderCheckBytes[4] = { 0, 0, 0, 1 };

	memcpy(&byteOrderCheckInt, byteOrderCheckBytes, 4);

	bool isSystemBigEndian = false;
	if (byteOrderCheckInt == 1)
		isSystemBigEndian = true;
	else if (byteOrderCheckInt == 0x01000000)
		isSystemBigEndian = false;
	else
	{
		fprintf(stderr, "Couldn't detect system endian\n");
		return -1;
	}

	sp.m_isByteSwapped = (isSystemBigEndian != isBigEndian);

	if (!seg1Stream.SeekSet(0))
	{
		fprintf(stderr, "Failed to reposition to start\n");
		return -1;
	}

	mtdisasm::DataReader dataReader(seg1Stream, sp.m_isByteSwapped);

	dataReader.Seek(0);

	mtdisasm::Catalog catalog;
	if (!catalog.Load(dataReader))
	{
		fprintf(stderr, "Failed to load catalog\n");
		return -1;
	}

	std::vector<FILE*> segments;
	size_t numSegments = catalog.NumSegments();

	segments.resize(numSegments);
	segments[0] = catFile;

	bool isWinConvention = false;
	if (numSegments > 1)
	{
		if (seg1Path[seg1Path.size() - 1] != '1')
		{
			isWinConvention = true;
			if (seg1Path.size() < 5 || seg1Path[seg1Path.size() - 5] != '1' || seg1Path[seg1Path.size() - 4] != '.')
			{
				fprintf(stderr, "Couldn't figure out segment naming convention");
				return -1;
			}
		}
	}

	for (size_t i = 1; i < numSegments; i++)
	{
		std::string mpxPath;
		if (isWinConvention)
			mpxPath = seg1Path.substr(0, seg1Path.size() - 5) + std::to_string(i + 1) + ".MPX";
		else
			mpxPath = seg1Path.substr(0, seg1Path.size() - 1) + std::to_string(i + 1);

		segments[i] = fopen(mpxPath.c_str(), "rb");
		if (!segments[i])
		{
			fprintf(stderr, "Attempted to open %s but couldn't find it\n", mpxPath.c_str());
			return -1;
		}
	}

	if (mode == "text")
	{
		std::string catPath = outputDir + "/catalog.txt";

		FILE* dumpF = fopen(catPath.c_str(), "wb");
		if (!dumpF)
		{
			fprintf(stderr, "Failed to open catalog path '%s'", catPath.c_str());
			return -1;
		}

		PrintCatalogDisassembly(catalog, dumpF);

		fclose(dumpF);
	}

	const size_t numStreams = catalog.NumStreams();

	printf("Unbundling %i streams...\n", static_cast<int>(numStreams));

	for (size_t i = 0; i < numStreams; i++)
	{
		const mtdisasm::StreamDesc& streamDesc = catalog.GetStream(i);

		std::string streamPath = outputDir + "/stream-" + std::to_string(i) + "-" + std::to_string(streamDesc.m_segmentNumber) + ".";
		mtdisasm::CFileIOStream stream(segments[streamDesc.m_segmentNumber - 1]);

		if (!strcmp(streamDesc.m_streamType, "assetStream"))
			streamPath += "asset";
		else if (!strcmp(streamDesc.m_streamType, "bootStream"))
			streamPath += "boot";
		else if (!strcmp(streamDesc.m_streamType, "sceneStream"))
			streamPath += "scene";
		else
		{
			fprintf(stderr, "Unknown stream type '%s'\n", streamDesc.m_streamType);
			return -1;
		}

		if (!stream.SeekSet(streamDesc.m_pos))
		{
			fprintf(stderr, "Failed to load stream\n");
			return -1;
		}

		FILE* dumpF = fopen(streamPath.c_str(), "wb");
		if (!dumpF)
		{
			fprintf(stderr, "Failed to open output path '%s'", streamPath.c_str());
			return -1;
		}

		if (mode == "bin")
		{
			mtdisasm::CFileIOStream outStream(dumpF);

			uint8_t copyBuffer[2048];
			size_t bytesRemaining = streamDesc.m_size;
			while (bytesRemaining > 0)
			{
				size_t chunkSize = bytesRemaining;
				if (chunkSize > sizeof(copyBuffer))
					chunkSize = sizeof(copyBuffer);

				bytesRemaining -= chunkSize;

				if (!stream.ReadAll(copyBuffer, chunkSize) || !outStream.WriteAll(copyBuffer, chunkSize))
				{
					fprintf(stderr, "Failed to dump stream data at position %i\n", static_cast<int>(stream.Tell()));
					return -1;
				}
			}
		}
		else if (mode == "text")
		{
			fprintf(dumpF, "Stream %i   Segment: %i   Position in file: %x\n\n", static_cast<int>(i), static_cast<int>(streamDesc.m_segmentNumber), static_cast<int>(streamDesc.m_pos));

			mtdisasm::SliceIOStream slice(stream, streamDesc.m_pos, streamDesc.m_size);
			DisassembleStream(slice, streamDesc.m_size, static_cast<int>(i), streamDesc.m_pos, sp, dumpF);
		}
		else
		{
			fprintf(stderr, "Unknown mode %s\n", mode.c_str());
			return -1;
		}

		fclose(dumpF);
	}

	for (FILE* f : segments)
	{
		fclose(f);
	}

	return 0;
}
