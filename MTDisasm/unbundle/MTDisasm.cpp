#include "IOStream.h"
#include "CFileIOStream.h"
#include "Catalog.h"
#include "DataObject.h"
#include "DataReader.h"
#include "SliceIOStream.h"
#include "MemIOStream.h"

#include <string>
#include <vector>

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <unordered_map>
#include <unordered_set>

extern "C"
{
#include "stb_image_write.h"
}

struct RGBColor
{
	uint8_t r, g, b;
};

RGBColor g_macStandardPalette[256];

void GenerateMacStandardPalette()
{
	int outColor = 0;
	for (int r = 0; r < 6; r++)
	{
		for (int g = 0; g < 6; g++)
		{
			int maxB = 6;
			if (r == 5 && g == 5)
				maxB = 5;
			for (int b = 0; b < maxB; b++)
			{
				RGBColor& clr = g_macStandardPalette[outColor++];
				clr.r = 255 - 51 * r;
				clr.g = 255 - 51 * g;
				clr.b = 255 - 51 * b;
			}
		}
	}

	for (int v = 0; v < 16; v++)
	{
		if (v % 3 == 0)
			continue;
		RGBColor& clr = g_macStandardPalette[outColor++];
		clr.r = 255 - 17 * v;
		clr.g = 0;
		clr.b = 0;
	}

	for (int v = 0; v < 16; v++)
	{
		if (v % 3 == 0)
			continue;
		RGBColor& clr = g_macStandardPalette[outColor++];
		clr.r = 0;
		clr.g = 255 - 17 * v;
		clr.b = 0;
	}

	for (int v = 0; v < 16; v++)
	{
		if (v % 3 == 0)
			continue;
		RGBColor& clr = g_macStandardPalette[outColor++];
		clr.r = 0;
		clr.g = 0;
		clr.b = 255 - 17 * v;
	}

	for (int v = 0; v < 16; v++)
	{
		if (v % 3 == 0)
			continue;
		RGBColor& clr = g_macStandardPalette[outColor++];
		clr.r = 255 - 17 * v;
		clr.g = clr.r;
		clr.b = clr.g;
	}

	RGBColor& lastClr = g_macStandardPalette[outColor++];
	lastClr.r = lastClr.g = lastClr.b = 0;
}

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
	case mtdisasm::DataObjectType::kMessengerModifier:
		return "MessengerModifier";
	case mtdisasm::DataObjectType::kIfMessengerModifier:
		return "IfMessengerModifier";
	case mtdisasm::DataObjectType::kKeyboardMessengerModifier:
		return "KeyboardMessengerModifier";
	case mtdisasm::DataObjectType::kBooleanVariableModifier:
		return "BooleanVariableModifier";
	case mtdisasm::DataObjectType::kPointVariableModifier:
		return "PointVariableModifier";
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
	PrintHex("GUID", obj.m_guid, f);
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
	PrintHex("GUID", obj.m_guid, f);
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
	PrintHex("GUID", obj.m_guid, f);
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
	PrintHex("GUID", obj.m_guid, f);
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
	PrintHex("GUID", obj.m_guid, f);
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
	PrintHex("GUID", obj.m_guid, f);
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
				if (sp.m_systemType == mtdisasm::SystemType::kMac)
				{
					if (!reader.ReadF80BE(d))
						return false;
				}
				else
				{
					if (!reader.ReadF64(d))
						return false;
				}
				fprintf(f, "double %g", d);
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

struct MiniscriptInstruction
{
	uint16_t m_opcode;
	uint16_t m_flags;
	size_t m_instrNumber;
	std::vector<uint8_t> m_contents;
};

struct MiniscriptBasicBlock
{
	std::vector<size_t> m_predecessors;
	std::vector<size_t> m_dominators;
	std::vector<size_t> m_postDominators;
	std::vector<MiniscriptInstruction> m_instructions;

	bool m_isTerminal;
	bool m_isConditional;
	size_t m_startInstr;
	std::vector<size_t> m_successors;
};

// Control flow islands start at a single block and sink into another block
struct MiniscriptControlFlowIsland
{
	MiniscriptBasicBlock* m_start;
	MiniscriptBasicBlock* m_sinkBB;

	MiniscriptControlFlowIsland* m_sinkIsland;
	std::vector<MiniscriptControlFlowIsland*> m_successorIslands;

	MiniscriptControlFlowIsland();
};

MiniscriptControlFlowIsland::MiniscriptControlFlowIsland()
	: m_start(nullptr)
	, m_sinkBB(nullptr)
	, m_sinkIsland(nullptr)
{
}

bool DecompileJumpOp(const MiniscriptInstruction& instr, const mtdisasm::SerializationProperties& sp, bool& outIsConditional, size_t& outNextInstr)
{
	if (instr.m_opcode != 0x7d3)
		return false;
	if (instr.m_contents.size() != 12)
		return false;

	mtdisasm::MemIOStream stream(&instr.m_contents[0], 12);
	mtdisasm::DataReader reader(stream, sp.m_isByteSwapped);

	uint32_t flags, unknown, offset;
	reader.ReadU32(flags);
	reader.ReadU32(unknown);
	reader.ReadU32(offset);

	if (offset == 0)
		return false;

	outIsConditional = ((flags & 0x2) != 0);
	outNextInstr = offset + instr.m_instrNumber;

	return true;
}

template<class T>
bool InsertUnique(std::vector<T>& vec, const T& item)
{
	for (const T& candidate : vec)
	{
		if (candidate == item)
			return false;
	}

	vec.push_back(item);
	return true;
}

class MiniscriptControlFlowResolver
{
public:
	explicit MiniscriptControlFlowResolver(const std::unordered_map<size_t, MiniscriptBasicBlock*>& bbMap);
	~MiniscriptControlFlowResolver();

	MiniscriptControlFlowIsland* AllocIsland();
	void ResolveAll();

private:
	void ResolveIsland(MiniscriptControlFlowIsland* island);
	void ResolveConditionalTree(MiniscriptControlFlowIsland* island);

	std::vector<MiniscriptControlFlowIsland*> m_islands;
	const std::unordered_map<size_t, MiniscriptBasicBlock*>& m_bbMap;
};

MiniscriptControlFlowResolver::MiniscriptControlFlowResolver(const std::unordered_map<size_t, MiniscriptBasicBlock*>& bbMap)
	: m_bbMap(bbMap)
{
}

MiniscriptControlFlowResolver::~MiniscriptControlFlowResolver()
{
	for (MiniscriptControlFlowIsland* island : m_islands)
		delete island;
}

MiniscriptControlFlowIsland* MiniscriptControlFlowResolver::AllocIsland()
{
	MiniscriptControlFlowIsland* island = new MiniscriptControlFlowIsland();
	m_islands.push_back(island);
	return island;
}

void MiniscriptControlFlowResolver::ResolveAll()
{
	size_t islandIndex = 0;

	while (islandIndex < m_islands.size())
	{
		while (islandIndex < m_islands.size())
			ResolveIsland(m_islands[islandIndex++]);
	}
}

void MiniscriptControlFlowResolver::ResolveConditionalTree(MiniscriptControlFlowIsland* island)
{
	for (size_t successor : island->m_start->m_successors)
	{
		MiniscriptBasicBlock* bb = m_bbMap.find(successor)->second;
		if (bb == island->m_sinkBB)
			island->m_successorIslands.push_back(nullptr);
		else
		{
			MiniscriptControlFlowIsland* newIsland = AllocIsland();
			newIsland->m_start = bb;
			newIsland->m_sinkBB = island->m_sinkBB;

			assert(bb != nullptr);
			island->m_successorIslands.push_back(newIsland);
		}
	}
}

void MiniscriptControlFlowResolver::ResolveIsland(MiniscriptControlFlowIsland* island)
{
	// Determine if this island needs to be split
	MiniscriptBasicBlock* startBB = island->m_start;
	MiniscriptBasicBlock* sinkBB = island->m_sinkBB;
	const std::vector<size_t>& startPostDominators = startBB->m_postDominators;

	if (startPostDominators.size() > 1)
	{
		MiniscriptBasicBlock* searchBB = startBB;
		MiniscriptBasicBlock* newSinkBB = nullptr;
		for (;;)
		{
			if (searchBB->m_successors.size() > 0)
				searchBB = m_bbMap.find(searchBB->m_successors[0])->second;
			else
				break;

			if (std::find(startPostDominators.begin(), startPostDominators.end(), searchBB->m_startInstr) != startPostDominators.end())
			{
				newSinkBB = searchBB;
				break;
			}
		}

		if (newSinkBB != sinkBB)
		{
			MiniscriptControlFlowIsland* newIsland = AllocIsland();
			newIsland->m_sinkBB = sinkBB;
			newIsland->m_start = newSinkBB;

			assert(newSinkBB != nullptr);

			island->m_sinkBB = newSinkBB;
			island->m_sinkIsland = newIsland;
		}
	}

	ResolveConditionalTree(island);
}

struct MiniscriptExpressionTree
{
	const MiniscriptInstruction* m_instr;
	std::vector<MiniscriptExpressionTree*> m_children;

	~MiniscriptExpressionTree();
};

MiniscriptExpressionTree::~MiniscriptExpressionTree()
{
	for (MiniscriptExpressionTree* child : m_children)
		delete child;
}

MiniscriptExpressionTree* PopOne(MiniscriptExpressionTree& stack)
{
	MiniscriptExpressionTree* tree = stack.m_children[stack.m_children.size() - 1];
	stack.m_children.pop_back();
	return tree;
}

void PrintIndent(int indentationLevel, FILE* f)
{
	for (int i = 0; i < indentationLevel; i++)
		fputc('\t', f);
}

enum MiniscriptOperatorPrecedence
{
	kOpPrec_Lowest,

	kOpPrec_Or,
	kOpPrec_And,
	kOpPrec_AbsCmp,
	kOpPrec_RelCmp,
	kOpPrec_Add,
	kOpPrec_Mul,
	kOpPrec_Pow,
	kOpPrec_Unary,
	kOpPrec_Paren,

	kOpPrec_Highest,
};

void ResolveExprFragmentationPrecedence(const MiniscriptExpressionTree* expr, MiniscriptOperatorPrecedence& leftSidePrec, MiniscriptOperatorPrecedence& rightSidePrec);

// This resolves the fragmentation precedence of the leftmost and rightmost sides of an expression.
// Basically, if an operator of the specified precedence is placed to the left or right of the expression,
// then it will be higher-priority than the actual expression there and fragment the expression.
void ResolveBinaryOpExprFragmentationPrecedence(const MiniscriptExpressionTree* expr, MiniscriptOperatorPrecedence& leftSidePrec, MiniscriptOperatorPrecedence& rightSidePrec, bool& outLeftNeedsParen, bool& outRightNeedsParen)
{
	MiniscriptOperatorPrecedence leftLeft;
	MiniscriptOperatorPrecedence leftRight;
	MiniscriptOperatorPrecedence rightLeft;
	MiniscriptOperatorPrecedence rightRight;

	ResolveExprFragmentationPrecedence(expr->m_children[0], leftLeft, leftRight);
	ResolveExprFragmentationPrecedence(expr->m_children[1], rightLeft, rightRight);

	MiniscriptOperatorPrecedence thisPrec = kOpPrec_Lowest;
	switch (expr->m_instr->m_opcode)
	{
	case 0xc9:
	case 0xca:
	case 0xdb:	// String concat - not actually sure of this precedence
		thisPrec = kOpPrec_Add;
		break;
	case 0xcb:
	case 0xcc:
	case 0xd9:
	case 0xda:
		thisPrec = kOpPrec_Mul;
		break;
	case 0xcd:
		thisPrec = kOpPrec_Pow;
		break;
	case 0xce:
		thisPrec = kOpPrec_And;
		break;
	case 0xcf:
		thisPrec = kOpPrec_Or;
		break;
	case 0xd2:
	case 0xd3:
		thisPrec = kOpPrec_AbsCmp;
		break;
	case 0xd4:
	case 0xd5:
	case 0xd6:
	case 0xd7:
		thisPrec = kOpPrec_RelCmp;
		break;
	default:
		break;
	};

	bool leftNeedsParen = false;
	bool rightNeedsParen = false;
	if (rightLeft <= thisPrec)
	{
		// Right side would be fragmented by this operator
		rightNeedsParen = true;
		rightLeft = rightRight = kOpPrec_Paren;
	}
	if (leftRight < thisPrec)
	{
		// Left side would be fragmented by this operator
		leftNeedsParen = true;
		leftLeft = leftRight = kOpPrec_Paren;
	}

	leftSidePrec = static_cast<MiniscriptOperatorPrecedence>(std::min<int>(leftLeft, thisPrec));
	rightSidePrec = static_cast<MiniscriptOperatorPrecedence>(std::min<int>(rightRight, thisPrec + 1));

	outLeftNeedsParen = leftNeedsParen;
	outRightNeedsParen = rightNeedsParen;
}

void ResolveUnaryOpExprFragmentationPrecedence(const MiniscriptExpressionTree* expr, MiniscriptOperatorPrecedence& leftSidePrec, MiniscriptOperatorPrecedence& rightSidePrec, bool& outNeedsParen)
{
	MiniscriptOperatorPrecedence chLeft;
	MiniscriptOperatorPrecedence chRight;

	ResolveExprFragmentationPrecedence(expr->m_children[0], chLeft, chRight);

	MiniscriptOperatorPrecedence thisPrec = kOpPrec_Lowest;
	switch (expr->m_instr->m_opcode)
	{
	case 0xd0:
	case 0xd1:
	case 0x135:
		thisPrec = kOpPrec_Unary;
		break;
	default:
		break;
	};

	bool needsParen = false;
	if (chLeft < thisPrec)
	{
		// Right side would be fragmented by this operator
		needsParen = true;
		chLeft = chRight = kOpPrec_Paren;
	}

	leftSidePrec = static_cast<MiniscriptOperatorPrecedence>(std::min<int>(chLeft, thisPrec));
	rightSidePrec = static_cast<MiniscriptOperatorPrecedence>(std::min<int>(chRight, thisPrec + 1));

	outNeedsParen = needsParen;
}

void ResolveExprFragmentationPrecedence(const MiniscriptExpressionTree* expr, MiniscriptOperatorPrecedence& leftSidePrec, MiniscriptOperatorPrecedence& rightSidePrec)
{
	switch (expr->m_instr->m_opcode)
	{
	case 0xc9:
	case 0xca:
	case 0xdb:
	case 0xcb:
	case 0xcc:
	case 0xd9:
	case 0xda:
	case 0xcd:
	case 0xce:
	case 0xcf:
	case 0xd2:
	case 0xd3:
	case 0xd4:
	case 0xd5:
	case 0xd6:
	case 0xd7:
		{
			bool leftNeedsParen, rightNeedsParen;
			ResolveBinaryOpExprFragmentationPrecedence(expr, leftSidePrec, rightSidePrec, leftNeedsParen, rightNeedsParen);
		}
		break;
	case 0xd0:
	case 0xd1:
	case 0x135:
		{
			bool needsParen;
			ResolveUnaryOpExprFragmentationPrecedence(expr, leftSidePrec, rightSidePrec, needsParen);
		}
		break;
	default:
		leftSidePrec = rightSidePrec = kOpPrec_Highest;
		break;
	}
}

void PrintExpression(const MiniscriptExpressionTree* expr, const mtdisasm::DOMiniscriptProgram& obj, FILE* f);

bool GetBuiltinFunctionProperties(uint32_t builtinId, uint32_t& outNumParams, const char*& outName)
{
	switch (builtinId)
	{
	case 0x01: outName = "sin"; outNumParams = 1; return true;
	case 0x02: outName = "cos"; outNumParams = 1; return true;
	case 0x03: outName = "random"; outNumParams = 1; return true;
	case 0x04: outName = "sqrt"; outNumParams = 1; return true;
	case 0x05: outName = "tan"; outNumParams = 1; return true;
	case 0x06: outName = "abs"; outNumParams = 1; return true;
	case 0x07: outName = "sgn"; outNumParams = 1; return true;
	case 0x08: outName = "arctangent"; outNumParams = 1; return true;
	case 0x09: outName = "exp"; outNumParams = 1; return true;
	case 0x0a: outName = "ln"; outNumParams = 1; return true;
	case 0x0b: outName = "log"; outNumParams = 1; return true;
	case 0x0c: outName = "cosh"; outNumParams = 1; return true;
	case 0x0d: outName = "sinh"; outNumParams = 1; return true;
	case 0x0e: outName = "tanh"; outNumParams = 1; return true;
	case 0x0f: outName = "rect2polar"; outNumParams = 1; return true;
	case 0x10: outName = "polar2rect"; outNumParams = 1; return true;
	case 0x11: outName = "trunc"; outNumParams = 1; return true;
	case 0x12: outName = "round"; outNumParams = 1; return true;
	case 0x13: outName = "num2str"; outNumParams = 1; return true;
	case 0x14: outName = "str2num"; outNumParams = 1; return true;
	default:
		outName = "unknown";
		outNumParams = 0;
		return false;
	}
}

void PrintBinaryExpression(const MiniscriptExpressionTree* expr, const mtdisasm::DOMiniscriptProgram& obj, FILE* f)
{
	const char* op = "???";

	switch (expr->m_instr->m_opcode)
	{
	case 0xc9: op = "+"; break;
	case 0xca: op = "-"; break;
	case 0xcb: op = "*"; break;
	case 0xcc: op = "/"; break;
	case 0xcd: op = "^"; break;
	case 0xce: op = "and"; break;
	case 0xcf: op = "or"; break;
	case 0xd2: op = "="; break;
	case 0xd3: op = "<>"; break;
	case 0xd4: op = "<="; break;
	case 0xd5: op = "<"; break;
	case 0xd6: op = ">="; break;
	case 0xd7: op = ">"; break;
	case 0xd9: op = "div"; break;
	case 0xda: op = "mod"; break;
	case 0xdb: op = "&"; break;
	default:
		break;
	}

	MiniscriptOperatorPrecedence leftPrec;
	MiniscriptOperatorPrecedence rightPrec;

	bool leftNeedsParen;
	bool rightNeedsParen;
	ResolveBinaryOpExprFragmentationPrecedence(expr, leftPrec, rightPrec, leftNeedsParen, rightNeedsParen);

	if (leftNeedsParen)
		fputc('(', f);
	PrintExpression(expr->m_children[0], obj, f);
	if (leftNeedsParen)
		fputc(')', f);
	fputc(' ', f);
	fputs(op, f);
	fputc(' ', f);
	if (rightNeedsParen)
		fputc('(', f);
	PrintExpression(expr->m_children[1], obj, f);
	if (rightNeedsParen)
		fputc(')', f);
}

void PrintUnaryExpression(const MiniscriptExpressionTree* expr, const mtdisasm::DOMiniscriptProgram& obj, FILE* f)
{
	const char* op = "???";

	switch (expr->m_instr->m_opcode)
	{
	case 0xd0: op = "-"; break;
	case 0xd1: op = "not "; break;
	default:
		break;
	}

	MiniscriptOperatorPrecedence leftPrec;
	MiniscriptOperatorPrecedence rightPrec;

	bool needsParen;
	ResolveUnaryOpExprFragmentationPrecedence(expr, leftPrec, rightPrec, needsParen);

	fputs(op, f);
	if (needsParen)
		fputc('(', f);
	PrintExpression(expr->m_children[0], obj, f);
	if (needsParen)
		fputc(')', f);
}

void EmitStr(const std::vector<char>& str, FILE* f)
{
	size_t len = str.size();
	if (len == 0)
	{
		fputs("\"\"", f);
		return;
	}

	if (str[len - 1] == 0)
		len--;

	bool needsQuotes = false;
	for (size_t i = 0; i < len; i++)
	{
		bool isAlpha = false;
		bool isNumeric = false;

		char c = str[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
			isAlpha = true;
		else if (c >= '0' && c <= '9')
			isNumeric = true;

		if ((isNumeric && i == 0) || (!isNumeric && !isAlpha))
		{
			needsQuotes = true;
			break;
		}
	}

	if (needsQuotes)
		fputc('\"', f);
	fwrite(&str[0], 1, len, f);
	if (needsQuotes)
		fputc('\"', f);
}

void EmitPushValue(const MiniscriptInstruction& instr, const mtdisasm::DOMiniscriptProgram& obj, FILE* f)
{
	if (instr.m_contents.size() != 0)
	{
		mtdisasm::MemIOStream stream(&instr.m_contents[0], instr.m_contents.size());
		mtdisasm::DataReader reader(stream, obj.m_sp.m_isByteSwapped);

		uint16_t type;
		if (reader.ReadU16(type))
		{
			switch (type)
			{
			case 0x00:
				fputs("NULL", f);
				return;
			case 0x15:
				{
					double d;
					bool readOK = false;
					if (obj.m_sp.m_systemType == mtdisasm::SystemType::kMac)
						readOK = reader.ReadF80BE(d);
					else
						readOK = reader.ReadF64(d);

					if (readOK)
					{
						fprintf(f, "%g", d);
						return;
					}
				}
				break;
			case 0x1a:
				{
					uint8_t b;
					if (reader.ReadU8(b))
					{
						fprintf(f, "%s", ((b == 0) ? "false" : "true"));
						return;
					}
				}
				break;;
			case 0x1f9:
				{
					uint32_t u32;
					if (reader.ReadU32(u32))
					{
						if (u32 < obj.m_numLocalRefs)
						{
							fputs("local:", f);
							EmitStr(obj.m_localRefs[u32].m_name, f);
							return;
						}
					}
				}
				return;
			case 0x1fa:
				{
					uint32_t u32;
					if (reader.ReadU32(u32))
					{
						fprintf(f, "global:%08x", static_cast<int>(u32));
						return;
					}
				}
				return;
			}
		}
	}

	fputs("<BAD VALUE>", f);
}

void EmitPushGlobal(const MiniscriptInstruction& instr, const mtdisasm::DOMiniscriptProgram& obj, FILE* f)
{
	if (instr.m_contents.size() < 4)
		return;

	mtdisasm::MemIOStream stream(&instr.m_contents[0], instr.m_contents.size());
	mtdisasm::DataReader reader(stream, obj.m_sp.m_isByteSwapped);

	uint32_t globID;
	reader.ReadU32(globID);

	const char* name = "???";
	switch (globID)
	{
	case 1: name = "element"; break;
	case 2: name = "subsection"; break;
	case 3: name = "source"; break;
	case 4: name = "incoming"; break;
	case 5: name = "mouse"; break;
	case 6: name = "ticks"; break;
	case 7: name = "scene"; break;
	case 8: name = "sharedScene"; break;
	case 9: name = "section"; break;
	case 10: name = "project"; break;
	case 11: name = "sharedScene"; break;
	default:
		fprintf(f, "unknown_env_%08x", static_cast<int>(globID));
		return;
	}

	fputs(name, f);
}

void EmitPushStr(const MiniscriptInstruction& instr, const mtdisasm::DOMiniscriptProgram& obj, FILE* f)
{
	fputc('\"', f);

	if (instr.m_contents.size() >= 2)
	{
		mtdisasm::MemIOStream stream(&instr.m_contents[0], instr.m_contents.size());
		mtdisasm::DataReader reader(stream, obj.m_sp.m_isByteSwapped);

		uint16_t strLength = 0;
		if (reader.ReadU16(strLength) && instr.m_contents.size() >= (3 + strLength))
			fwrite(&instr.m_contents[2], 1, strLength, f);
	}

	fputc('\"', f);
}

void EmitGetChild(const MiniscriptExpressionTree* expr, const mtdisasm::DOMiniscriptProgram& obj, FILE* f)
{
	if (expr->m_instr->m_contents.size() < 4)
		return;

	mtdisasm::MemIOStream stream(&expr->m_instr->m_contents[0], expr->m_instr->m_contents.size());
	mtdisasm::DataReader reader(stream, obj.m_sp.m_isByteSwapped);

	uint32_t attribID;
	reader.ReadU32(attribID);

	PrintExpression(expr->m_children[0], obj, f);
	fputc('.', f);

	if (attribID < obj.m_attributes.size())
		EmitStr(obj.m_attributes[attribID].m_name, f);
	else
		fprintf(f, "unknown_attrib_%08x", static_cast<int>(attribID));

	if (expr->m_instr->m_flags & 0x20)
	{
		fputc('[', f);
		PrintExpression(expr->m_children[1], obj, f);
		fputc(']', f);
	}
}

void PrintExpression(const MiniscriptExpressionTree* expr, const mtdisasm::DOMiniscriptProgram& obj, FILE* f)
{
	switch (expr->m_instr->m_opcode)
	{
	case 0xc9:
	case 0xca:
	case 0xcb:
	case 0xcc:
	case 0xcd:
	case 0xce:
	case 0xcf:
	case 0xd2:
	case 0xd3:
	case 0xd4:
	case 0xd5:
	case 0xd6:
	case 0xd7:
	case 0xd9:
	case 0xda:
	case 0xdb:
		PrintBinaryExpression(expr, obj, f);
		break;

	case 0xd0:
	case 0xd1:
		PrintUnaryExpression(expr, obj, f);
		break;

	case 0xd8:
		{
			if (expr->m_instr->m_contents.size() < 4)
				return;

			mtdisasm::MemIOStream stream(&expr->m_instr->m_contents[0], expr->m_instr->m_contents.size());
			mtdisasm::DataReader reader(stream, obj.m_sp.m_isByteSwapped);

			uint32_t funcID;
			if (!reader.ReadU32(funcID))
				return;

			uint32_t numArgs = 0;
			const char* name = nullptr;
			if (GetBuiltinFunctionProperties(funcID, numArgs, name))
			{
				fputs(name, f);
				fputc('(', f);
				for (size_t i = 0; i < numArgs; i++)
				{
					PrintExpression(expr->m_children[i], obj, f);
					if (i != numArgs - 1)
						fputs(", ", f);
				}
				fputc(')', f);
			}
		}
		break;
	
	case 0x12f:
		{
			fputs("(", f);
			PrintExpression(expr->m_children[0], obj, f);
			fputs(", ", f);
			PrintExpression(expr->m_children[1], obj, f);
			fputs(")", f);

		}
		break;
	case 0x130:
		{
			fputs("(", f);
			PrintExpression(expr->m_children[0], obj, f);
			fputs(" thru ", f);
			PrintExpression(expr->m_children[1], obj, f);
			fputs(")", f);

		}
		break;
	case 0x131:
		{
			fputs("(", f);
			PrintExpression(expr->m_children[0], obj, f);
			fputs(" deg ", f);
			PrintExpression(expr->m_children[1], obj, f);
			fputs(" mag)", f);
		}
		break;
	case 0x135:
		EmitGetChild(expr, obj, f);
		break;
	case 0x136:
		{
			std::vector<const MiniscriptExpressionTree*> rsExprs;
			while (expr->m_instr->m_opcode == 0x136)
			{
				rsExprs.push_back(expr->m_children[1]);
				expr = expr->m_children[0];
			}
			fputs("{ ", f);
			PrintExpression(expr, obj, f);
			for (size_t i = 0; i < rsExprs.size(); i++)
			{
				fputs(", ", f);
				PrintExpression(rsExprs[rsExprs.size() - 1 - i], obj, f);
			}
			fputs(" }", f);
		}
		break;
	case 0x137:
		{
			PrintExpression(expr->m_children[0], obj, f);
			fputs(", ", f);
			PrintExpression(expr->m_children[1], obj, f);
		}
		break;
	case 0x191:
		EmitPushValue(*expr->m_instr, obj, f);
		break;
	case 0x192:
		EmitPushGlobal(*expr->m_instr, obj, f);
		break;
	case 0x193:
		EmitPushStr(*expr->m_instr, obj, f);
		break;
	}
}

void PrintEvent(const mtdisasm::DOEvent& evt, FILE* f)
{
	PrintSingleVal(evt, false, f);
}

bool CombineExpr(MiniscriptExpressionTree& stack, const MiniscriptInstruction* instr, size_t count)
{
	if (stack.m_children.size() < count)
		return false;

	MiniscriptExpressionTree* newTree = new MiniscriptExpressionTree();
	for (size_t i = 0; i < count; i++)
		newTree->m_children.push_back(stack.m_children[stack.m_children.size() - count + i]);

	for (size_t i = 0; i < count; i++)
		stack.m_children.pop_back();

	newTree->m_instr = instr;
	stack.m_children.push_back(newTree);

	return true;
}


bool EmitMiniscriptIsland(int indentationLevel, const MiniscriptControlFlowIsland* island, const mtdisasm::DOMiniscriptProgram& obj, bool forceExpression, FILE* f)
{
	MiniscriptExpressionTree stack;

	while (island != nullptr)
	{
		for (const MiniscriptInstruction& instr : island->m_start->m_instructions)
		{
			switch (instr.m_opcode)
			{
				case 0x834:
				{
					if (stack.m_children.size() < 2)
						return false;
					MiniscriptExpressionTree* value = PopOne(stack);
					MiniscriptExpressionTree* dest = PopOne(stack);
					PrintIndent(indentationLevel, f);
					fputs("set ", f);
					PrintExpression(dest, obj, f);
					fputs(" to ", f);
					PrintExpression(value, obj, f);
					fputs("\n", f);

					delete value;
					delete dest;
				}
				break;

			case 0x898:
				{
					if (instr.m_contents.size() < 8)
						return false;

					mtdisasm::MemIOStream stream(&instr.m_contents[0], instr.m_contents.size());
					mtdisasm::DataReader reader(stream, obj.m_sp.m_isByteSwapped);

					if (stack.m_children.size() < 2)
						return false;

					mtdisasm::DOEvent evt;
					if (!evt.Load(reader))
						return false;

					MiniscriptExpressionTree* dest = PopOne(stack);
					MiniscriptExpressionTree* addl = PopOne(stack);
					PrintIndent(indentationLevel, f);
					fputs("send ", f);
					PrintEvent(evt, f);
					fputs(" to ", f);
					PrintExpression(dest, obj, f);
					fputs(" with ", f);
					PrintExpression(addl, obj, f);
					if ((instr.m_flags & 0x1c) == 0x1c)
						fputs(" options none", f);
					else if ((instr.m_flags & 0x1c) != 0)
					{
						fputs(" options", f);
						if ((instr.m_flags & 0x04) == 0)
							fputs(" immediate", f);
						if ((instr.m_flags & 0x08) == 0)
							fputs(" cascade", f);
						if ((instr.m_flags & 0x10) == 0)
							fputs(" relay", f);
					}
					fputs("\n", f);

					delete dest;
					delete addl;
				}
				break;
			case 0xc9:
			case 0xca:
			case 0xcb:
			case 0xcc:
			case 0xcd:
			case 0xce:
			case 0xcf:
			case 0xd2:
			case 0xd3:
			case 0xd4:
			case 0xd5:
			case 0xd6:
			case 0xd7:
			case 0xd9:
			case 0xda:
			case 0xdb:
			case 0x12f:
			case 0x130:
			case 0x131:
			case 0x136:
			case 0x137:
				// Binary expression ops
				if (!CombineExpr(stack, &instr, 2))
					return false;
				break;
			case 0xd0:
			case 0xd1:
				// Unary ops
				if (!CombineExpr(stack, &instr, 1))
					return false;
				break;
			case 0x135:
				if (instr.m_flags & 0x20)
				{
					if (!CombineExpr(stack, &instr, 2))
						return false;
				}
				else
				{
					if (!CombineExpr(stack, &instr, 1))
						return false;
				}
				break;
			case 0xd8:
				// Builtin function
				{
					mtdisasm::MemIOStream stream(&instr.m_contents[0], instr.m_contents.size());
					mtdisasm::DataReader reader(stream, obj.m_sp.m_isByteSwapped);

					uint32_t funcID;
					if (!reader.ReadU32(funcID))
						return false;

					uint32_t numParams = 0;
					const char* name = nullptr;
					if (!GetBuiltinFunctionProperties(funcID, numParams, name))
						return false;


					if (stack.m_children.size() < numParams)
						return false;

					if (!CombineExpr(stack, &instr, numParams))
						return false;
				}
				break;
			case 0x191:
			case 0x192:
			case 0x193:
				// Push ops
				{
					MiniscriptExpressionTree* tree = new MiniscriptExpressionTree();
					tree->m_instr = &instr;
					stack.m_children.push_back(tree);
				}
				break;
			default:
				// Unknown opcode
				return false;
			}
		}

		if (island->m_start->m_isConditional)
		{
			if (stack.m_children.size() != 1)
				return false;

			MiniscriptExpressionTree* condition = PopOne(stack);

			PrintIndent(indentationLevel, f);
			fputs("if ", f);
			PrintExpression(condition, obj, f);
			fputs(" then\n", f);

			const MiniscriptControlFlowIsland* trueIsland = island->m_successorIslands[0];
			const MiniscriptControlFlowIsland* falseIsland = island->m_successorIslands[1];

			if (trueIsland && !EmitMiniscriptIsland(indentationLevel + 1, trueIsland, obj, false, f))
				return false;

			if (falseIsland)
			{
				PrintIndent(indentationLevel, f);
				fputs("else\n", f);
				if (falseIsland && !EmitMiniscriptIsland(indentationLevel + 1, falseIsland, obj, false, f))
					return false;
			}

			PrintIndent(indentationLevel, f);
			fputs("end if\n", f);

			delete condition;
		}

		island = island->m_sinkIsland;
	}

	if (stack.m_children.size() > 0)
	{
		if (forceExpression)
		{
			MiniscriptExpressionTree* expr = PopOne(stack);
			PrintIndent(indentationLevel, f);
			PrintExpression(expr, obj, f);
			fputs("\n", f);
			delete expr;
		}
		else
			fputs("Program ended with trailing stack values!\n", f);
	}

	return true;
}

bool DecompileMiniscript(const mtdisasm::DOMiniscriptProgram& obj, const mtdisasm::SerializationProperties& sp, bool isExpression, FILE* f)
{
	if (obj.m_numOfInstructions == 0)
		return true;

	if (obj.m_bytecode.size() == 0)
		return false;

	mtdisasm::MemIOStream stream(&obj.m_bytecode[0], obj.m_bytecode.size());
	mtdisasm::DataReader reader(stream, sp.m_isByteSwapped);

	std::vector<MiniscriptInstruction> instructions;

	// Read all ops
	for (size_t i = 0; i < obj.m_numOfInstructions; i++)
	{
		MiniscriptInstruction instr;
		uint16_t size;
		if (!reader.ReadU16(instr.m_opcode) || !reader.ReadU16(instr.m_flags) || !reader.ReadU16(size))
			return false;
		if (size < 6)
			return false;
		if (size > 6)
		{
			instr.m_contents.resize(size - 6);
			if (!reader.ReadBytes(&instr.m_contents[0], size - 6))
				return false;
		}

		instr.m_instrNumber = i;
		instructions.push_back(instr);
	}

	// Locate basic blocks
	std::vector<size_t> bbStarts;
	bbStarts.push_back(0);
	InsertUnique(bbStarts, instructions.size());

	for (const MiniscriptInstruction& instr : instructions)
	{
		bool isConditional;
		size_t nextInstr;
		if (instr.m_opcode == 0x7d3)
		{
			if (DecompileJumpOp(instr, sp, isConditional, nextInstr))
			{
				if (nextInstr > instructions.size())
					return false;

				if (isConditional)
					InsertUnique(bbStarts, instr.m_instrNumber + 1);
				InsertUnique(bbStarts, nextInstr);
			}
			else
				return false;
		}
	}

	std::sort(bbStarts.begin(), bbStarts.end());

	std::vector<MiniscriptBasicBlock> basicBlocks;
	basicBlocks.resize(bbStarts.size());

	for (size_t i = 0; i < bbStarts.size(); i++)
	{
		size_t bbStart = bbStarts[i];
		size_t bbEnd = instructions.size();

		if (i != bbStarts.size() - 1)
			bbEnd = bbStarts[i + 1];

		MiniscriptBasicBlock& bb = basicBlocks[i];
		bb.m_isTerminal = (bbStart == instructions.size());
		bb.m_isConditional = false;
		bb.m_startInstr = bbStart;

		bb.m_instructions.reserve(bbEnd - bbStart);
		for (size_t j = bbStart; j < bbEnd; j++)
			bb.m_instructions.push_back(instructions[j]);

		if (!bb.m_isTerminal)
		{
			if (bb.m_instructions.size() >= 1)
			{
				bool isConditional;
				size_t nextInstr;
				const MiniscriptInstruction& lastInstr = bb.m_instructions[bb.m_instructions.size() - 1];
				if (lastInstr.m_opcode == 0x7d3 && DecompileJumpOp(lastInstr, sp, isConditional, nextInstr))
				{
					if (isConditional)
					{
						bb.m_successors.push_back(lastInstr.m_instrNumber + 1);
						bb.m_successors.push_back(nextInstr);
						bb.m_isConditional = true;
					}
					else
						bb.m_successors.push_back(nextInstr);

					bb.m_instructions.pop_back();
				}
				else
				{
					// Fallthrough
					bb.m_successors.push_back(lastInstr.m_instrNumber + 1);
				}
			}
		}
	}

	std::unordered_map<size_t, MiniscriptBasicBlock*> bbMap;
	for (MiniscriptBasicBlock& bb : basicBlocks)
	{
		bbMap[bb.m_startInstr] = &bb;
	}

	// Identify predecessors
	for (const MiniscriptBasicBlock& bb : basicBlocks)
	{
		if (bb.m_isTerminal)
			continue;

		for (size_t successorStart : bb.m_successors)
		{
			MiniscriptBasicBlock* successor = bbMap.find(successorStart)->second;
			InsertUnique(successor->m_predecessors, bb.m_startInstr);
		}
	}

	// Resolve dominators
	for (MiniscriptBasicBlock& bb : basicBlocks)
	{
		if (bb.m_startInstr == 0)
		{
			bb.m_dominators.resize(1);
			bb.m_dominators[0] = 0;
		}
		else
			bb.m_dominators = bbStarts;
	}

	bool dominatorsDirty = true;
	while (dominatorsDirty)
	{
		dominatorsDirty = false;

		for (MiniscriptBasicBlock& bb : basicBlocks)
		{
			if (bb.m_startInstr == 0)
				continue;

			bool blockDirty = false;
			for (size_t pdi = 0; pdi < bb.m_dominators.size(); )
			{
				bool isValid = true;
				size_t possibleDominator = bb.m_dominators[pdi];
				if (possibleDominator != bb.m_startInstr)
				{
					// Not the block itself, so this must be a dominator of all predecessors
					for (size_t pred : bb.m_predecessors)
					{
						MiniscriptBasicBlock* predBB = bbMap.find(pred)->second;
						if (std::find(predBB->m_dominators.begin(), predBB->m_dominators.end(), possibleDominator) == predBB->m_dominators.end())
						{
							isValid = false;
							break;
						}
					}
				}

				if (isValid)
					pdi++;
				else
				{
					bb.m_dominators[pdi] = bb.m_dominators[bb.m_dominators.size() - 1];
					bb.m_dominators.pop_back();
					blockDirty = true;
				}
			}

			if (blockDirty)
			{
				std::sort(bb.m_dominators.begin(), bb.m_dominators.end());
				dominatorsDirty = true;
			}
		}
	}

	// Resolve postdominators
	for (MiniscriptBasicBlock& bb : basicBlocks)
	{
		if (bb.m_isTerminal)
		{
			bb.m_postDominators.resize(1);
			bb.m_postDominators[0] = bb.m_startInstr;
		}
		else
			bb.m_postDominators = bbStarts;
	}

	bool postDominatorsDirty = true;
	while (postDominatorsDirty)
	{
		postDominatorsDirty = false;

		for (MiniscriptBasicBlock& bb : basicBlocks)
		{
			if (bb.m_isTerminal)
				continue;

			bool blockDirty = false;
			for (size_t pdi = 0; pdi < bb.m_postDominators.size(); )
			{
				bool isValid = true;
				size_t possiblePostDominator = bb.m_postDominators[pdi];
				if (possiblePostDominator != bb.m_startInstr)
				{
					// Not the block itself, so this must be a post-dominator of all successors
					for (size_t succ : bb.m_successors)
					{
						MiniscriptBasicBlock* succBB = bbMap.find(succ)->second;
						if (std::find(succBB->m_postDominators.begin(), succBB->m_postDominators.end(), possiblePostDominator) == succBB->m_postDominators.end())
						{
							isValid = false;
							break;
						}
					}
				}

				if (isValid)
					pdi++;
				else
				{
					bb.m_postDominators[pdi] = bb.m_postDominators[bb.m_postDominators.size() - 1];
					bb.m_postDominators.pop_back();
					blockDirty = true;
				}
			}

			if (blockDirty)
			{
				std::sort(bb.m_postDominators.begin(), bb.m_postDominators.end());
				postDominatorsDirty = true;
			}
		}
	}

	MiniscriptControlFlowResolver cfResolver(bbMap);
	MiniscriptControlFlowIsland* initialIsland = cfResolver.AllocIsland();
	initialIsland->m_start = &basicBlocks[0];
	initialIsland->m_sinkBB = &basicBlocks[basicBlocks.size() - 1];

	assert(initialIsland->m_start != nullptr);

	cfResolver.ResolveAll();

	if (!EmitMiniscriptIsland(1, initialIsland, obj, isExpression, f))
		return false;

	return true;
}

void PrintObjectDisassembly(const mtdisasm::DOMiniscriptProgram& obj, FILE* f, bool isExpression)
{
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("SizeOfInstructions", obj.m_sizeOfInstructions, f);
	PrintHex("NumOfInstructions", obj.m_numOfInstructions, f);
	PrintHex("NumLocalRefs", obj.m_numLocalRefs, f);
	PrintHex("NumAttributes", obj.m_numAttributes, f);

	fputs("Attributes:\n", f);
	for (size_t i = 0; i < obj.m_numAttributes; i++)
	{
		const mtdisasm::DOMiniscriptProgram::Attribute& attrib = obj.m_attributes[i];
		fprintf(f, "    % 5i: %02x '", static_cast<int>(i), static_cast<int>(attrib.m_unknown11));
		if (attrib.m_name.size() > 1)
			fwrite(&attrib.m_name[0], 1, attrib.m_name.size() - 1, f);
		fputs("'\n", f);
	}

	fputs("Local references:\n", f);
	for (size_t i = 0; i < obj.m_numLocalRefs; i++)
	{
		const mtdisasm::DOMiniscriptProgram::LocalRef& ref = obj.m_localRefs[i];
		fprintf(f, "    % 5i: %08x %02x '", static_cast<int>(i), static_cast<int>(ref.m_guid), static_cast<int>(ref.m_unknown10));
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
			case 0xd6: opName = "cmp_ge"; break;
			case 0xd7: opName = "cmp_gt"; break;
			case 0xd8: opName = "builtin_func"; break;
			case 0xd9: opName = "div_int"; break;
			case 0xda: opName = "mod"; break;
			case 0xdb: opName = "str_concat"; break;
			case 0x12f: opName = "point_create"; break;
			case 0x130: opName = "range_create"; break;
			case 0x131: opName = "vector_create"; break;
			case 0x135: opName = "get_child"; break;
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
					fprintf(f, "    % 5i: %s  ", static_cast<int>(i), opName);
				else
					fprintf(f, "    % 5i: %s,args=%i  ", static_cast<int>(i), opName, static_cast<int>(unknownField));
			}
			else
			{

				fprintf(f, "    % 5i: %s(0x%x),%i  ", static_cast<int>(i), opName, static_cast<int>(opcode), static_cast<int>(unknownField));
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

	fputs("Decompiled:\n", f);
	if (!DecompileMiniscript(obj, obj.m_sp, isExpression, f))
	{
		fputs("Decompile failed\n", f);
	}
}

void PrintObjectDisassembly(const mtdisasm::DOMiniscriptModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kMiniscriptModifier);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("GUID", obj.m_guid, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintHex("LengthOfName", obj.m_lengthOfName, f);
	PrintHex("EnableWhen", obj.m_enableWhen, f);
	PrintHex("Unknown6", obj.m_unknown6, f);
	PrintHex("Unknown7", obj.m_unknown7, f);
	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);

	fputs("Program:\n", f);
	PrintObjectDisassembly(obj.m_program, f, false);
}

void PrintObjectDisassembly(const mtdisasm::DONotYetImplemented& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kNotYetImplemented);

	fprintf(f, "    Unimplemented '%s'  DataSize=%u\n", obj.m_name, obj.m_sizeIncludingTag - 14);
}

void PrintObjectDisassembly(const mtdisasm::POUnknown& obj, FILE* f)
{
	for (size_t i = 0; i < obj.m_data.size(); i++)
	{
		if (i % 32 == 0)
		{
			if (i != 0)
				fputs("\n", f);
			fputs("   ", f);
		}
		fprintf(f, " %02x", obj.m_data[i]);
	}

	fputs("\n", f);
}

void PrintObjectDisassembly(const mtdisasm::POCursorMod& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::PlugInObjectType::kCursorMod);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintVal("ApplyWhen", obj.m_applyWhen, f);
	PrintVal("RemoveWhen", obj.m_removeWhen, f);
	PrintVal("CursorID", obj.m_cursorID, f);
}

void PrintObjectDisassembly(const mtdisasm::DOPlugInModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kPlugInModifier);

	fprintf(f, "Plug-in modifier '%s'  PrivateDataSize=%u\n", obj.m_plugin, obj.m_privateDataSize);
	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("PlugInRevision", obj.m_plugInRevision, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintHex("Unknown5", obj.m_unknown5, f);
	PrintHex("GUID", obj.m_guid, f);
	PrintVal("WeirdSize", obj.m_weirdSize, f);

	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);

	switch (obj.m_plugInData->GetType())
	{
	case mtdisasm::PlugInObjectType::kUnknown:
		PrintObjectDisassembly(static_cast<const mtdisasm::POUnknown&>(*obj.m_plugInData), f);
		break;
	case mtdisasm::PlugInObjectType::kCursorMod:
		PrintObjectDisassembly(static_cast<const mtdisasm::POCursorMod&>(*obj.m_plugInData), f);
		break;
	default:
		break;
	}
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
	PrintVal("CuePointDataSize", obj.m_cuePointDataSize, f);
	PrintVal("NumCuePoints", obj.m_numCuePoints, f);
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

	for (size_t i = 0; i < obj.m_numCuePoints; i++)
	{
		const mtdisasm::DOAudioAsset::CuePoint& cuePoint = obj.m_cuePoints[i];
		fprintf(f, "Cue point %i:\n", static_cast<int>(i));
		PrintHex("    Unknown13", cuePoint.m_unknown13, f);
		PrintHex("    Unknown14", cuePoint.m_unknown14, f);
		PrintVal("    Position", cuePoint.m_position, f);
		PrintVal("    CuePointID", cuePoint.m_cuePointID, f);
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
	PrintHex("FrameDataPosition", obj.m_frameDataPosition, f);
	PrintVal("SizeOfFrameData", obj.m_sizeOfFrameData, f);

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

	if (obj.m_haveMacPart)
		PrintHex("Unknown10", obj.m_platform.m_mac.m_unknown10, f);
	if (obj.m_haveWinPart)
		PrintHex("Unknown11", obj.m_platform.m_win.m_unknown11, f);

	for (size_t i = 0; i < obj.m_numFrames; i++)
	{
		fprintf(f, "Frame %i:\n", static_cast<int>(i));
		const mtdisasm::DOMToonAsset::FrameDef& frame = obj.m_frames[i];

		PrintHex("    Unknown12", frame.m_unknown12, f);
		PrintVal("    Rect1", frame.m_rect1, f);
		PrintHex("    DataOffset", frame.m_dataOffset, f);
		PrintHex("    Unknown13", frame.m_unknown13, f);
		PrintVal("    CompressedSize", frame.m_compressedSize, f);
		PrintHex("    Unknown14", frame.m_unknown14, f);
		PrintHex("    KeyframeFlag", frame.m_keyframeFlag, f);
		PrintHex("    PlatformBit", frame.m_platformBit, f);
		PrintHex("    Unknown15", frame.m_unknown15, f);
		PrintVal("    Rect2", frame.m_rect2, f);
		PrintHex("    HdpiFixed", frame.m_hdpiFixed, f);
		PrintHex("    VdpiFixed", frame.m_vdpiFixed, f);
		PrintVal("    BitsPerPixel", frame.m_bitsPerPixel, f);
		PrintHex("    Unknown16", frame.m_unknown16, f);
		PrintVal("    DecompressedBytesPerRow", frame.m_decompressedBytesPerRow, f);
		PrintVal("    DecompressedSize", frame.m_decompressedSize, f);

		if (obj.m_haveMacPart)
			PrintHex("    Unknown12", frame.m_platform.m_mac.m_unknown17, f);
		if (obj.m_haveWinPart)
			PrintHex("    Unknown13", frame.m_platform.m_win.m_unknown18, f);
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
	PrintHex("GUID", obj.m_guid, f);
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

void PrintObjectDisassembly(const mtdisasm::DOMessengerModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kMessengerModifier);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintHex("Unknown5", obj.m_unknown5, f);
	PrintHex("MessageFlags", obj.m_messageFlags, f);
	PrintHex("Unknown11", obj.m_unknown11, f);
	PrintHex("Unknown12", obj.m_unknown12, f);
	PrintHex("Unknown13", obj.m_unknown13, f);
	PrintHex("Unknown14", obj.m_unknown14, f);
	PrintHex("Unknown15", obj.m_unknown15, f);
	PrintVal("Send", obj.m_send, f);
	PrintVal("When", obj.m_when, f);
	PrintHex("GUID", obj.m_guid, f);
	PrintHex("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintVal("LengthOfName", obj.m_lengthOfName, f);
	PrintHex("Destination", obj.m_destination, f);
	PrintHex("With", obj.m_with, f);
	PrintHex("WithSourceGUID", obj.m_withSourceGUID, f);

	if (obj.m_withSource.size() > 1)
	{
		fputs("WithSource: '", f);
		fwrite(&obj.m_withSource[0], 1, obj.m_withSource.size() - 1u, f);
		fputs("'\n", f);
	}

	if (obj.m_name.size() > 1)
	{
		fputs("Name: '", f);
		fwrite(&obj.m_name[0], 1, obj.m_name.size() - 1u, f);
		fputs("'\n", f);
	}
}
void PrintObjectDisassembly(const mtdisasm::DOTypicalModifierHeader& obj, FILE* f)
{
	PrintHex("ModHeader_Unknown1", obj.m_unknown1, f);
	PrintHex("ModHeader_Unknown3", obj.m_unknown3, f);
	PrintHex("ModHeader_Unknown4", obj.m_unknown4, f);
	PrintHex("GUID", obj.m_guid, f);
	PrintHex("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintVal("LengthOfName", obj.m_lengthOfName, f);

	if (obj.m_name.size() > 1)
	{
		fputs("Name: '", f);
		fwrite(&obj.m_name[0], 1, obj.m_name.size() - 1u, f);
		fputs("'\n", f);
	}
}

void PrintObjectDisassembly(const mtdisasm::DOIfMessengerModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kIfMessengerModifier);

	PrintObjectDisassembly(obj.m_modHeader, f);
	PrintHex("Unknown6", obj.m_unknown6, f);
	PrintHex("Unknown7", obj.m_unknown7, f);
	PrintHex("Unknown8", obj.m_unknown8, f);
	PrintHex("Unknown9", obj.m_unknown9, f);
	PrintHex("Unknown10", obj.m_unknown10, f);
	PrintHex("MessageFlags", obj.m_messageFlags, f);
	PrintVal("Send", obj.m_send, f);
	PrintVal("When", obj.m_when, f);
	PrintHex("Destination", obj.m_destination, f);
	PrintHex("With", obj.m_with, f);
	PrintHex("WithSourceGUID", obj.m_withSourceGUID, f);

	if (obj.m_withSource.size() > 1)
	{
		fputs("WithSource: '", f);
		fwrite(&obj.m_withSource[0], 1, obj.m_withSource.size() - 1u, f);
		fputs("'\n", f);
	}

	fputs("Program:\n", f);
	PrintObjectDisassembly(obj.m_program, f, true);
}

void PrintObjectDisassembly(const mtdisasm::DOKeyboardMessengerModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kKeyboardMessengerModifier);

	PrintObjectDisassembly(obj.m_modHeader, f);
	PrintHex("MessageFlagsAndKeyStates", obj.m_messageFlagsAndKeyStates, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("KeyModifiers", obj.m_keyModifiers, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintHex("Message", obj.m_message, f);
	PrintHex("Unknown7", obj.m_unknown7, f);
	PrintHex("Destination", obj.m_destination, f);
	PrintHex("Unknown9", obj.m_unknown9, f);
	PrintHex("With", obj.m_with, f);
	PrintHex("Unknown11", obj.m_unknown11, f);
	PrintHex("WithSourceGUID", obj.m_withSourceGUID, f);
	PrintHex("Unknown13", obj.m_unknown13, f);
	PrintHex("Unknown14", obj.m_unknown14, f);

	PrintHex("KeyCode", obj.m_keycode, f);
	PrintVal("WithSourceLength", obj.m_withSourceLength, f);

	if (obj.m_withSource.size() > 1)
	{
		fputs("WithSource: '", f);
		fwrite(&obj.m_withSource[0], 1, obj.m_withSource.size() - 1u, f);
		fputs("'\n", f);
	}
}

void PrintObjectDisassembly(const mtdisasm::DOBooleanVariableModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kBooleanVariableModifier);

	PrintObjectDisassembly(obj.m_modHeader, f);
	PrintHex("Unknown5", obj.m_unknown5, f);

	PrintVal("Value", obj.m_value, f);
}

void PrintObjectDisassembly(const mtdisasm::DOPointVariableModifier& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kPointVariableModifier);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("SizeIncludingTag", obj.m_sizeIncludingTag, f);
	PrintHex("GUID", obj.m_guid, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintHex("LengthOfName", obj.m_lengthOfName, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
	PrintHex("Unknown5", obj.m_unknown5, f);

	fputs("Name: '", f);
	if (obj.m_lengthOfName > 0)
		fwrite(&obj.m_name[0], 1, obj.m_lengthOfName - 1, f);
	fputs("'\n", f);

	PrintVal("Value", obj.m_value, f);
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
	case mtdisasm::DataObjectType::kMessengerModifier:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOMessengerModifier&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kIfMessengerModifier:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOIfMessengerModifier&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kKeyboardMessengerModifier:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOKeyboardMessengerModifier&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kMiniscriptModifier:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOMiniscriptModifier&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kBooleanVariableModifier:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOBooleanVariableModifier&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kPointVariableModifier:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOPointVariableModifier&>(obj), f);
		break;

	default:
		fprintf(f, "Unknown contents\n");
	}
}

#define ATOM(a,b,c,d) static_cast<uint32_t>((a << 24) + (b << 16) + (c << 8) + d)

bool PullData(void* dest, const std::vector<uint8_t>& data, size_t pos, size_t size)
{
	if (size == 0)
		return true;

	if (size > data.size())
		return false;

	if (data.size() - size < pos)
		return false;

	memcpy(dest, &data[pos], size);
	return true;
}

bool RecursiveFixupSTCOChunkFromAtomStart(std::vector<uint8_t>& data, uint32_t atomStartPos, uint32_t& outAtomEndPos, uint32_t basePosition, uint32_t atomSize)
{
	static const uint32_t descendableAtoms[] = {
		ATOM('m', 'o', 'o', 'v'),
		ATOM('t', 'r', 'a', 'k'),
		ATOM('m', 'd', 'i', 'a'),
		ATOM('m', 'i', 'n', 'f'),
		ATOM('s', 't', 'b', 'l')
	};

	uint8_t atomHeader[8];
	if (!PullData(atomHeader, data, atomStartPos, 8))
		return false;

	uint32_t atomCodedSize = (atomHeader[0] << 24) + (atomHeader[1] << 16) + (atomHeader[2] << 8) + atomHeader[3];
	uint32_t atomID = (atomHeader[4] << 24) + (atomHeader[5] << 16) + (atomHeader[6] << 8) + atomHeader[7];

	if (atomCodedSize == 0)
		atomCodedSize = atomSize;

	if (atomCodedSize < 8)
		return false;

	uint32_t atomEndPos = atomCodedSize + atomStartPos;

	bool isDescendable = false;
	for (uint32_t descendable : descendableAtoms)
	{
		if (descendable == atomID)
		{
			isDescendable = true;
			break;
		}
	}

	if (isDescendable)
	{
		// Descendable atom
		uint32_t remainingSize = atomCodedSize - 8;
		uint32_t childStartPos = atomStartPos + 8;
		while (remainingSize > 0)
		{
			uint32_t childEndPos = 0;
			if (!RecursiveFixupSTCOChunkFromAtomStart(data, childStartPos, childEndPos, basePosition, remainingSize))
				return false;

			remainingSize = atomEndPos - childEndPos;
			childStartPos = childEndPos;
		}
	}
	else if (atomID == ATOM('s', 't', 'c', 'o'))
	{
		uint8_t stcoData[8];
		if (!PullData(stcoData, data, atomStartPos + 8, 8))
			return false;

		uint8_t version = stcoData[0];
		uint32_t flags = (stcoData[1] << 16) + (stcoData[2] << 8) + stcoData[3];
		uint32_t numEntries = (stcoData[4] << 24) + (stcoData[5] << 16) + (stcoData[6] << 8) + stcoData[7];

		for (uint32_t i = 0; i < numEntries; i++)
		{
			uint8_t chunkOffsetData[4];

			uint32_t fileOffsetPos = atomStartPos + 16 + i * 4;
			if (!PullData(chunkOffsetData, data, fileOffsetPos, 4))
				return false;

			uint32_t offset = (chunkOffsetData[0] << 24) + (chunkOffsetData[1] << 16) + (chunkOffsetData[2] << 8) + chunkOffsetData[3];
			offset -= basePosition;
			data[fileOffsetPos + 0] = (offset >> 24) & 0xff;
			data[fileOffsetPos + 1] = (offset >> 16) & 0xff;
			data[fileOffsetPos + 2] = (offset >> 8) & 0xff;
			data[fileOffsetPos + 3] = offset & 0xff;
		}
	}

	outAtomEndPos = atomEndPos;

	return true;
}

void FixupQuickTimeFileOffsets(std::vector<uint8_t>& data, uint32_t basePosition, uint32_t moovAtomPositionAbsolute, uint32_t moovDataSize)
{
	uint32_t scratch = 0;
	RecursiveFixupSTCOChunkFromAtomStart(data, moovAtomPositionAbsolute - basePosition, scratch, basePosition, moovDataSize);
}

void ExtractMovieAsset(std::unordered_set<uint32_t>& assetIDs, const mtdisasm::DOMovieAsset& asset, mtdisasm::IOStream& stream, const mtdisasm::SerializationProperties& sp, const std::string& basePath)
{
	if (assetIDs.find(asset.m_assetID) != assetIDs.end())
		return;

	assetIDs.insert(asset.m_assetID);

	stream.SeekSet(asset.m_movieDataPos);

	std::vector<uint8_t> movieData;
	movieData.resize(asset.m_movieDataSize);

	if (asset.m_movieDataSize == 0)
		return;

	if (!stream.ReadAll(&movieData[0], asset.m_movieDataSize))
		return;

	FixupQuickTimeFileOffsets(movieData, asset.m_movieDataPos, asset.m_moovAtomPos, asset.m_movieDataSize);

	std::string outPath = basePath + "/asset_" + std::to_string(asset.m_assetID) + ".mov";

	FILE* outF = fopen(outPath.c_str(), "w+b");

	if (!outF)
		return;

	fwrite(&movieData[0], 1, asset.m_movieDataSize, outF);

	fclose(outF);
}

void ExtractImageAsset(std::unordered_set<uint32_t>& assetIDs, const mtdisasm::DOImageAsset& asset, mtdisasm::IOStream& stream, const mtdisasm::SerializationProperties& sp, const std::string& basePath)
{
	if (assetIDs.find(asset.m_assetID) != assetIDs.end())
		return;

	assetIDs.insert(asset.m_assetID);

	std::string outPath = basePath + "/asset_" + std::to_string(asset.m_assetID) + ".png";

	size_t width = asset.m_rect1.m_right - asset.m_rect1.m_left;
	size_t height = asset.m_rect1.m_bottom - asset.m_rect1.m_top;
	size_t bytesPerRow = (width * asset.m_bitsPerPixel + 7) / 8;
	bytesPerRow = (bytesPerRow + 3) / 4 * 4;

	if (width < 0)
		return;

	size_t expectedSize = bytesPerRow * height;

	if (expectedSize != asset.m_size)
	{
		fprintf(stderr, "Asset %u unexpected data size: Expected %u but was %u\n", static_cast<unsigned int>(asset.m_assetID), static_cast<unsigned int>(expectedSize), static_cast<unsigned int>(asset.m_size));
		return;
	}
	if (asset.m_bitsPerPixel != 1 && asset.m_bitsPerPixel != 2 && asset.m_bitsPerPixel != 4 && asset.m_bitsPerPixel != 16 && asset.m_bitsPerPixel != 32)
	{
		fprintf(stderr, "Asset %u unsupported bit depth: %u\n", static_cast<unsigned int>(asset.m_assetID), static_cast<unsigned int>(asset.m_bitsPerPixel));
		return;
	}

	std::vector<uint8_t> rowData;
	rowData.resize(bytesPerRow);

	uint8_t* rowBytes = &rowData[0];

	size_t outBytesPerRow = width * 3;

	std::vector<uint8_t> decoded;
	decoded.resize(height * outBytesPerRow);

	stream.SeekSet(asset.m_filePosition);
	for (size_t row = 0; row < height; row++)
	{
		stream.ReadAll(rowBytes, bytesPerRow);
		uint8_t* outRowBytes = nullptr;

		if (sp.m_systemType == mtdisasm::SystemType::kWindows)
			outRowBytes = &decoded[(height - 1 - row) * outBytesPerRow];
		else if (sp.m_systemType == mtdisasm::SystemType::kMac)
			outRowBytes = &decoded[row * outBytesPerRow];
		else
			return;

		if (asset.m_bitsPerPixel == 32)
		{
			if (sp.m_systemType == mtdisasm::SystemType::kWindows)
			{
				for (size_t x = 0; x < width; x++)
				{
					outRowBytes[x * 3 + 0] = rowBytes[x * 4 + 3];
					outRowBytes[x * 3 + 1] = rowBytes[x * 4 + 2];
					outRowBytes[x * 3 + 2] = rowBytes[x * 4 + 1];
				}
			}
			else if (sp.m_systemType == mtdisasm::SystemType::kMac)
			{
				for (size_t x = 0; x < width; x++)
				{
					outRowBytes[x * 3 + 0] = rowBytes[x * 4 + 0];
					outRowBytes[x * 3 + 1] = rowBytes[x * 4 + 1];
					outRowBytes[x * 3 + 2] = rowBytes[x * 4 + 2];
				}
			}
		}
		else if (asset.m_bitsPerPixel == 16)
		{
			if (sp.m_systemType == mtdisasm::SystemType::kWindows)
			{
				for (size_t x = 0; x < width; x++)
				{
					uint16_t packedPixel = rowBytes[x * 2 + 0] + (rowBytes[x * 2 + 1] << 8);
					outRowBytes[x * 3 + 2] = ((packedPixel & 0x1f) * 33) >> 2;
					outRowBytes[x * 3 + 1] = (((packedPixel >> 5) & 0x1f) * 33) >> 2;
					outRowBytes[x * 3 + 0] = (((packedPixel >> 10) & 0x1f) * 33) >> 2;
				}
			}
			else if (sp.m_systemType == mtdisasm::SystemType::kMac)
			{
				for (size_t x = 0; x < width; x++)
				{
					uint16_t packedPixel = rowBytes[x * 2 + 1] + (rowBytes[x * 2 + 0] << 8);
					outRowBytes[x * 3 + 2] = ((packedPixel & 0x1f) * 33) >> 2;
					outRowBytes[x * 3 + 1] = (((packedPixel >> 5) & 0x1f) * 33) >> 2;
					outRowBytes[x * 3 + 0] = (((packedPixel >> 10) & 0x1f) * 33) >> 2;
				}
			}
		}
		else if (asset.m_bitsPerPixel == 8)
		{
			for (size_t x = 0; x < width; x++)
			{
				uint8_t byte = rowBytes[x];

				outRowBytes[x * 3 + 0] = byte;
				outRowBytes[x * 3 + 1] = byte;
				outRowBytes[x * 3 + 2] = byte;
			}
		}
		else if (asset.m_bitsPerPixel == 4)
		{
			for (size_t x = 0; x < width; x++)
			{
				int bit = (rowBytes[x / 2] >> (1 - (x % 2))) & 15;
				uint8_t byte = bit * 17;

				outRowBytes[x * 3 + 2] = byte;
				outRowBytes[x * 3 + 1] = byte;
				outRowBytes[x * 3 + 0] = byte;
			}
		}
		else if (asset.m_bitsPerPixel == 2)
		{
			for (size_t x = 0; x < width; x++)
			{
				int bit = (rowBytes[x / 4] >> (3 - (x % 4))) & 3;
				uint8_t byte = bit * 85;

				outRowBytes[x * 3 + 2] = byte;
				outRowBytes[x * 3 + 1] = byte;
				outRowBytes[x * 3 + 0] = byte;
			}
		}
		else if (asset.m_bitsPerPixel == 1)
		{
			for (size_t x = 0; x < width; x++)
			{
				int bit = (rowBytes[x / 8] >> (7 - (x % 8))) & 1;
				uint8_t byte = bit * 255;

				outRowBytes[x * 3 + 2] = byte;
				outRowBytes[x * 3 + 1] = byte;
				outRowBytes[x * 3 + 0] = byte;
			}
		}
	}

	stbi_write_png(outPath.c_str(), width, height, 3, &decoded[0], outBytesPerRow);
}

void DecodeRGB15(uint16_t v, RGBColor& color)
{
	int b = v & 0x1f;
	int g = (v >> 5) & 0x1f;
	int r = (v >> 10) & 0x1f;

	color.r = (r * 33) >> 2;
	color.g = (g * 33) >> 2;
	color.b = (b * 33) >> 2;
}

void ExtractMToonAsset(std::unordered_set<uint32_t>& assetIDs, const mtdisasm::DOMToonAsset& asset, mtdisasm::IOStream& stream, const mtdisasm::SerializationProperties& sp, const std::string& basePath)
{
	if (assetIDs.find(asset.m_assetID) != assetIDs.end())
		return;

	assetIDs.insert(asset.m_assetID);

	bool isMToonRLE = (asset.m_codecID == 0x2e524c45);
	bool isUncompressed = (asset.m_codecID == 0);

	if (!isMToonRLE && !isUncompressed)
	{
		fprintf(stderr, "Not yet supported mToon compression type\n");
		return;
	}

	std::vector<uint8_t> frameData;
	stream.SeekSet(asset.m_frameDataPosition);
	frameData.resize(asset.m_sizeOfFrameData);

	stream.ReadAll(&frameData[0], asset.m_sizeOfFrameData);

	for (size_t i = 0; i < asset.m_numFrames; i++)
	{
		const mtdisasm::DOMToonAsset::FrameDef& frameDef = asset.m_frames[i];

		size_t numRows = frameDef.m_rect1.m_bottom - frameDef.m_rect1.m_top;
		size_t numCols = frameDef.m_rect1.m_right - frameDef.m_rect1.m_left;
		size_t dataOffset = frameDef.m_dataOffset;
		bool isKeyframe = (frameDef.m_keyframeFlag != 0);

		bool isBottomUp = (sp.m_systemType == mtdisasm::SystemType::kWindows);

		if (isMToonRLE)
		{
			uint8_t rleHeaderBytes[20];
			for (size_t b = 0; b < 20; b++)
				rleHeaderBytes[b] = frameData[dataOffset++];

			uint32_t rleHeaderInts[5];
			for (size_t i = 0; i < 5; i++)
				rleHeaderInts[i] = (rleHeaderBytes[i * 4 + 0] << 24) + (rleHeaderBytes[i * 4 + 1] << 16) + (rleHeaderBytes[i * 4 + 2] << 8) + rleHeaderBytes[i * 4 + 3];

			const uint32_t expectedHeader = isKeyframe ? 0x524c4520 : 1;
			if (isKeyframe && rleHeaderInts[0] == 0x524c4520)
			{
				fprintf(stderr, "Keyframe header in non-keyframe mToon frame for some reason?\n");
			}

			if (rleHeaderInts[1] == 0x01000001 && asset.m_bitsPerPixel == 8)
			{
				size_t rleCols = rleHeaderInts[2];
				size_t rleRows = rleHeaderInts[3];
				size_t rleSize = rleHeaderInts[4];

				if (rleSize < 20)
				{
					fprintf(stderr, "RLE data size is too small\n");
					break;
				}

				//rleSize -= 20;
				rleSize = frameDef.m_compressedSize - 20;

				std::vector<uint8_t> imageData;
				imageData.resize(rleCols * rleRows * 4);

				std::vector<uint8_t> compressedData;
				compressedData.resize(rleSize);

				for (size_t j = 0; j < rleSize; j++)
					compressedData[j] = frameData[dataOffset++];

				size_t rleDataOffset = 0;
				for (size_t row = 0; row < rleRows; row++)
				{
					size_t colDataStart = row * rleCols * 4;

					if (isBottomUp)
						colDataStart = (rleRows - 1 - row) * rleCols * 4;

					for (size_t col = 0; col < rleCols; )
					{
						size_t numDecompressed = 0;
						if (rleDataOffset == compressedData.size())
							break;

						uint8_t rleCode = compressedData[rleDataOffset++];
						if (rleCode == 0)
						{
							uint8_t numTransparent = compressedData[rleDataOffset++];
							for (size_t tr = 0; tr < numTransparent; tr++)
							{
								if (col == rleCols)
									break;	// Last row transparent run sometimes overruns the end of the buffer...

								imageData[colDataStart + col * 4 + 0] = 0;
								imageData[colDataStart + col * 4 + 1] = 0;
								imageData[colDataStart + col * 4 + 2] = 0;
								imageData[colDataStart + col * 4 + 3] = 0;
								col++;
							}
						}
						else if (rleCode & 0x80)
						{
							uint8_t numLiterals = rleCode & 0x7f;
							for (size_t lit = 0; lit < numLiterals; lit++)
							{
								uint8_t litByte = compressedData[rleDataOffset++];
								const RGBColor& color = g_macStandardPalette[litByte];
								imageData[colDataStart + col * 4 + 0] = color.r;
								imageData[colDataStart + col * 4 + 1] = color.g;
								imageData[colDataStart + col * 4 + 2] = color.b;
								imageData[colDataStart + col * 4 + 3] = 255;
								col++;
							}
						}
						else
						{
							uint8_t repeatedByte = compressedData[rleDataOffset++];
							uint8_t numRepeats = rleCode;
							const RGBColor& color = g_macStandardPalette[repeatedByte];
							for (size_t rep = 0; rep < numRepeats; rep++)
							{
								imageData[colDataStart + col * 4 + 0] = color.r;
								imageData[colDataStart + col * 4 + 1] = color.g;
								imageData[colDataStart + col * 4 + 2] = color.b;
								imageData[colDataStart + col * 4 + 3] = 255;
								col++;
							}
						}
					}
				}

				std::string outPath = basePath + "/asset_" + std::to_string(asset.m_assetID) + "_frame_" + std::to_string(i) + ".png";
				stbi_write_png(outPath.c_str(), rleCols, rleRows, 4, &imageData[0], rleCols * 4);
			}
			else if (rleHeaderInts[1] == 0x01000002 && asset.m_bitsPerPixel == 16)
			{
				size_t rleCols = rleHeaderInts[2];
				size_t rleRows = rleHeaderInts[3];
				size_t rleSize = rleHeaderInts[4];

				// In this version rleSize appears to NOT include the header

				//rleSize -= 20;
				rleSize = frameDef.m_compressedSize - 20;

				std::vector<uint8_t> imageData;
				imageData.resize(rleCols * rleRows * 4);

				std::vector<uint8_t> compressedDataBytes;
				compressedDataBytes.resize(rleSize);

				for (size_t j = 0; j < rleSize; j++)
					compressedDataBytes[j] = frameData[dataOffset++];

				std::vector<uint16_t> compressedData;
				compressedData.resize(rleSize / 2);

				for (size_t j = 0; j < rleSize / 2; j++)
					compressedData[j] = (compressedDataBytes[j * 2 + 1] << 8) + compressedDataBytes[j * 2];

				compressedDataBytes.clear();

				size_t rleDataOffset = 0;

				for (size_t row = 0; row < rleRows; row++)
				{
					size_t colDataStart = row * rleCols * 4;
					for (size_t col = 0; col < rleCols; )
					{
						size_t numDecompressed = 0;
						if (rleDataOffset == compressedData.size())
							break;

						uint16_t rleCode = compressedData[rleDataOffset++];
						if (rleCode == 0)
						{
							uint16_t numTransparent = compressedData[rleDataOffset++];
							if (numTransparent & 0x8000)
							{
								// Appears to be vertical displacement...?
								row += (numTransparent & 0x7fff) - 1;
								break;
							}
							else
							{
								for (size_t tr = 0; tr < numTransparent; tr++)
								{
									if (col == rleCols)
										break;	// Last row transparent run sometimes overruns the end of the buffer...

									imageData[colDataStart + col * 4 + 0] = 0;
									imageData[colDataStart + col * 4 + 1] = 0;
									imageData[colDataStart + col * 4 + 2] = 0;
									imageData[colDataStart + col * 4 + 3] = 0;
									col++;
								}
							}
						}
						else if (rleCode & 0x8000)
						{
							uint8_t numLiterals = rleCode & 0x7fff;
							for (size_t lit = 0; lit < numLiterals; lit++)
							{
								uint16_t litWord = compressedData[rleDataOffset++];
								RGBColor color;
								DecodeRGB15(litWord, color);
								imageData[colDataStart + col * 4 + 0] = color.r;
								imageData[colDataStart + col * 4 + 1] = color.g;
								imageData[colDataStart + col * 4 + 2] = color.b;
								imageData[colDataStart + col * 4 + 3] = 255;
								col++;
							}
						}
						else
						{
							if (rleDataOffset == compressedData.size())
							{
								while (col < rleCols)
								{
									imageData[colDataStart + col * 4 + 0] = 255;
									imageData[colDataStart + col * 4 + 1] = 0;
									imageData[colDataStart + col * 4 + 2] = 255;
									imageData[colDataStart + col * 4 + 3] = 255;
									col++;
								}
								break;
							}

							uint16_t repeatedWord = compressedData[rleDataOffset++];
							uint16_t numRepeats = rleCode;
							RGBColor color;
							DecodeRGB15(repeatedWord, color);
							for (size_t rep = 0; rep < numRepeats; rep++)
							{
								if (col == rleCols)
									break;
								imageData[colDataStart + col * 4 + 0] = color.r;
								imageData[colDataStart + col * 4 + 1] = color.g;
								imageData[colDataStart + col * 4 + 2] = color.b;
								imageData[colDataStart + col * 4 + 3] = 255;
								col++;
							}
						}
					}
				}

				std::string outPath = basePath + "/asset_" + std::to_string(asset.m_assetID) + "_frame_" + std::to_string(i) + ".png";
				stbi_write_png(outPath.c_str(), rleCols, rleRows, 4, &imageData[0], rleCols * 4);
			}
		}
		else if (isUncompressed)
		{
			std::vector<uint8_t> imageData;
			size_t numPixels = numCols * numRows;
			imageData.resize(numPixels * 4);

			size_t bytesPerRow = frameDef.m_decompressedBytesPerRow;

			if (asset.m_bitsPerPixel == 8)
			{
				for (size_t row = 0; row < numRows; row++)
				{
					size_t rowOffset = dataOffset + row * bytesPerRow;
					if (isBottomUp)
						rowOffset = dataOffset + (numRows - 1 - row) * bytesPerRow;

					for (size_t col = 0; col < numCols; col++)
					{
						uint8_t pixel8 = frameData[rowOffset + col];
						const RGBColor& color = g_macStandardPalette[pixel8];

						const size_t px = col + row * numCols;
						imageData[px * 4 + 0] = color.r;
						imageData[px * 4 + 1] = color.g;
						imageData[px * 4 + 2] = color.b;
						imageData[px * 4 + 3] = 255;
					}
				}
			}
			else if (asset.m_bitsPerPixel == 16)
			{
				for (size_t row = 0; row < numRows; row++)
				{
					size_t rowOffset = dataOffset + row * bytesPerRow;
					if (isBottomUp)
						rowOffset = dataOffset + (numRows - 1 - row) * bytesPerRow;

					for (size_t col = 0; col < numCols; col++)
					{
						const size_t px = col + row * numCols;

						uint16_t pixel16 = frameData[rowOffset + col * 2 + 0] + (frameData[rowOffset + col * 2 + 1] << 8);
						RGBColor color;
						DecodeRGB15(pixel16, color);
						imageData[px * 4 + 0] = color.r;
						imageData[px * 4 + 1] = color.g;
						imageData[px * 4 + 2] = color.b;
						imageData[px * 4 + 3] = 255;
					}
				}
			}
			else if (asset.m_bitsPerPixel == 32)
			{
				for (size_t row = 0; row < numRows; row++)
				{
					size_t rowOffset = dataOffset + row * bytesPerRow;
					if (isBottomUp)
						rowOffset = dataOffset + (numRows - 1 - row) * bytesPerRow;

					for (size_t col = 0; col < numCols; col++)
					{
						const size_t px = col + row * numCols;

						const uint8_t* pixel32 = &frameData[rowOffset + col * 4];

						imageData[px * 4 + 0] = pixel32[0];
						imageData[px * 4 + 1] = pixel32[1];
						imageData[px * 4 + 2] = pixel32[2];
						imageData[px * 4 + 3] = 255;
					}
				}
			}
			else
			{
				fprintf(stderr, "Unsupported uncompressed bit count\n");
				return;
			}

			std::string outPath = basePath + "/asset_" + std::to_string(asset.m_assetID) + "_frame_" + std::to_string(i) + ".png";
			stbi_write_png(outPath.c_str(), numCols, numRows, 4, &imageData[0], numCols * 4);
		}
	}
}


void ExtractAsset(std::unordered_set<uint32_t>& assetIDs, const mtdisasm::DataObject& dataObject, mtdisasm::IOStream& stream, const mtdisasm::SerializationProperties& sp, const std::string& basePath)
{
	switch (dataObject.GetType())
	{
	case mtdisasm::DataObjectType::kImageAsset:
		//ExtractImageAsset(assetIDs, static_cast<const mtdisasm::DOImageAsset&>(dataObject), stream, sp, basePath);
		break;
	case mtdisasm::DataObjectType::kMovieAsset:
		//ExtractMovieAsset(assetIDs, static_cast<const mtdisasm::DOMovieAsset&>(dataObject), stream, sp, basePath);
		break;
	case mtdisasm::DataObjectType::kMToonAsset:
		ExtractMToonAsset(assetIDs, static_cast<const mtdisasm::DOMToonAsset&>(dataObject), stream, sp, basePath);
		break;
	case mtdisasm::DataObjectType::kAudioAsset:
	default:
		break;
	}
}

void ExtractAssetsFromStream(std::unordered_set<uint32_t>& assetIDs, mtdisasm::IOStream& globalStream, mtdisasm::IOStream& stream, size_t streamSize, int streamIndex, uint32_t streamPos, const mtdisasm::SerializationProperties& sp, const std::string& basePath)
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

		const bool succeeded = dataObject->Load(reader, revision, sp);
		if (succeeded)
		{
			uint32_t prevPos = stream.Tell();
			ExtractAsset(assetIDs, *dataObject, globalStream, sp, basePath);
			if (!stream.SeekSet(prevPos))
			{
				fprintf(stderr, "Failed to reset stream position\n");
				return;
			}
		}
		else
			fprintf(stderr, "Stream %i: Object type %s revision %i at position %x (global position %x) failed to load\n", streamIndex, NameObjectType(dataObject->GetType()), static_cast<int>(revision), static_cast<int>(pos), static_cast<int>(pos + streamPos));

		dataObject->Delete();

		if (!succeeded)
			break;
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

	GenerateMacStandardPalette();

	std::string mode = argv[1];
	std::string seg1Path = argv[2];
	std::string outputDir = argv[3];

	if (mode != "bin" && mode != "text" && mode != "assets")
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

	std::unordered_set<uint32_t> extractedAssets;

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
		else if (mode == "assets")
		{
			fprintf(dumpF, "Stream %i   Segment: %i   Position in file: %x\n\n", static_cast<int>(i), static_cast<int>(streamDesc.m_segmentNumber), static_cast<int>(streamDesc.m_pos));

			mtdisasm::SliceIOStream slice(stream, streamDesc.m_pos, streamDesc.m_size);
			ExtractAssetsFromStream(extractedAssets, stream, slice, streamDesc.m_size, static_cast<int>(i), streamDesc.m_pos, sp, outputDir);
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
