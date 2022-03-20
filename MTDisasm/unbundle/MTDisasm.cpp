#include "IOStream.h"
#include "CFileIOStream.h"
#include "Catalog.h"
#include "DataReader.h"

#include <string>
#include <vector>

#include <cstring>
#include <cstdio>

int main(int argc, const char** argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: unbundle <segment 1 path> <output dir>\n");
		return -1;
	}

	std::string seg1Path = argv[1];
	std::string outputDir = argv[2];

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

		mtdisasm::CFileIOStream stream(segments[streamDesc.m_segmentNumber - 1]);
		if (!stream.SeekSet(streamDesc.m_pos))
		{
			fprintf(stderr, "Failed to load stream\n");
			return -1;
		}

		std::string streamPath = outputDir + "/stream-" + std::to_string(i) + "-" + std::to_string(streamDesc.m_segmentNumber) + ".";
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

		fclose(dumpF);
	}

	for (FILE* f : segments)
	{
		fclose(f);
	}

	return 0;
}
