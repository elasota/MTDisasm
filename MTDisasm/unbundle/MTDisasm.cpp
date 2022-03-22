#include "IOStream.h"
#include "CFileIOStream.h"
#include "Catalog.h"
#include "DataObject.h"
#include "DataReader.h"
#include "SliceIOStream.h"

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
	case mtdisasm::DataObjectType::kUnknown3ec:
		return "Unknown3ec";
	case mtdisasm::DataObjectType::kAssetCatalog:
		return "AssetCatalog";
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

void PrintObjectDisassembly(const mtdisasm::DOUnknown3ec& obj, FILE* f)
{
	assert(obj.GetType() == mtdisasm::DataObjectType::kUnknown3ec);

	PrintHex("Marker", obj.m_marker, f);
	PrintVal("Size", obj.m_sizeIncludingTag, f);

	PrintHex("Unknown1", obj.m_unknown1, f);
	PrintHex("Unknown2", obj.m_unknown2, f);
	PrintHex("Unknown3", obj.m_unknown3, f);
	PrintHex("Unknown4", obj.m_unknown4, f);
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
		fprintf(f, "Asset % 4u: Deleted=%u  AlwaysZero=%04x  Unknown1=%08x  FilePosition=%08x  AssetType=%s  Flags=%08x", i, asset.m_isDeleted, asset.m_alwaysZero, asset.m_unknown1, asset.m_filePosition, assetTypeName, asset.m_flags);
		if (asset.m_nameLength > 0)
		{
			fputs("  ", f);
			fwrite(&asset.m_name[0], 1, asset.m_nameLength - 1, f);
		}
		fputs("\n", f);
	}
}

void PrintObjectDisassembly(const mtdisasm::DataObject& obj, FILE* f)
{
	switch (obj.GetType())
	{
	case mtdisasm::DataObjectType::kStreamHeader:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOStreamHeader&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kUnknown3ec:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOUnknown3ec&>(obj), f);
		break;
	case mtdisasm::DataObjectType::kAssetCatalog:
		PrintObjectDisassembly(static_cast<const mtdisasm::DOAssetCatalog&>(obj), f);
		break;

	default:
		fprintf(f, "Unknown contents\n");
	}
}

void DisassembleStream(mtdisasm::IOStream& stream, size_t streamSize, int streamIndex, bool isByteSwapped, FILE* f)
{
	mtdisasm::DataReader reader(stream, isByteSwapped);

	for (;;)
	{
		uint32_t pos = stream.Tell();
		if (pos == streamSize)
			break;

		uint32_t objectType = 0;
		uint16_t revision = 0;
		if (!reader.ReadU32(objectType) || !reader.ReadU16(revision))
		{
			fprintf(stderr, "Stream %i: Couldn't read type at position %x\n", streamIndex, static_cast<int>(pos));
			return;
		}

		mtdisasm::DataObject* dataObject = mtdisasm::CreateObjectFromType(objectType);
		if (!dataObject)
		{
			fprintf(stderr, "Stream %i: Unknown object type %x revision %i at position %x\n", streamIndex, static_cast<int>(objectType), static_cast<int>(revision), static_cast<int>(pos));
			return;
		}

		fprintf(f, "%x %s (%x) rev %i:\n", static_cast<int>(pos), NameObjectType(dataObject->GetType()), static_cast<int>(objectType), static_cast<int>(revision));

		if (dataObject->Load(reader, revision))
		{
			PrintObjectDisassembly(*dataObject, f);
		}
		else
		{
			fprintf(stderr, "Stream %i: Object type %s revision %i at position %x failed to load\n", streamIndex, NameObjectType(dataObject->GetType()), static_cast<int>(revision), static_cast<int>(pos));
			fprintf(f, "FAILED\n");
		}

		dataObject->Delete();

		fprintf(f, "\n");
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

	bool isBigEndian = false;
	if (systemCheck[0] == 0 && systemCheck[1] == 0)
	{
		printf("Detected as Macintosh format\n");
		isBigEndian = true;
	}
	else if (systemCheck[0] == 1 && systemCheck[1] == 0)
	{
		printf("Detected as Windows format\n");
		isBigEndian = false;
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

	bool isByteSwapped = (isSystemBigEndian != isBigEndian);

	if (!seg1Stream.SeekSet(0))
	{
		fprintf(stderr, "Failed to reposition to start\n");
		return -1;
	}

	mtdisasm::DataReader dataReader(seg1Stream, isByteSwapped);

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

		FILE* dumpF = fopen(streamPath.c_str(), "wb");

		if (!stream.SeekSet(streamDesc.m_pos))
		{
			fprintf(stderr, "Failed to load stream\n");
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
			mtdisasm::SliceIOStream slice(stream, streamDesc.m_pos, streamDesc.m_size);
			DisassembleStream(slice, streamDesc.m_size, static_cast<int>(i), isByteSwapped, dumpF);
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
