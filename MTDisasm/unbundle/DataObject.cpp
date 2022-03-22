#include "DataObject.h"
#include "DataReader.h"

namespace mtdisasm
{
	DataObject::~DataObject()
	{
	}

	void DataObject::Delete()
	{
		delete this;
	}

	DataObject* CreateObjectFromType(uint32_t objectType)
	{
		switch (objectType)
		{
		case 0x3e9:
			return new DOStreamHeader();
		case 0x3ec:
			return new DOUnknown3ec();
		case 0xd:
			return new DOAssetCatalog();
		default:
			return nullptr;
		};
	}

	DataObjectType DOStreamHeader::GetType() const
	{
		return DataObjectType::kStreamHeader;
	}

	bool DOStreamHeader::Load(DataReader& reader, uint16_t revision)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(m_marker) ||
			!reader.ReadU32(m_sizeIncludingTag) ||
			!reader.ReadBytes(m_name, 16) ||
			!reader.ReadBytes(m_projectID, 2) ||
			!reader.ReadBytes(m_unknown1, 4) ||
			!reader.ReadU16(m_unknown2))
			return false;

		if (m_sizeIncludingTag != 38)
			return false;

		m_name[16] = 0;

		return true;
	}

	DataObjectType DOUnknown3ec::GetType() const
	{
		return DataObjectType::kUnknown3ec;
	}

	bool DOUnknown3ec::Load(DataReader& reader, uint16_t revision)
	{
		if (revision != 2)
			return false;

		if (!reader.ReadU32(m_marker) ||
			!reader.ReadU32(m_sizeIncludingTag) ||
			!reader.ReadBytes(m_unknown1, 2) ||
			!reader.ReadU32(m_unknown2) ||
			!reader.ReadU16(m_unknown3) ||
			!reader.ReadU16(m_unknown4))
			return false;

		if (m_sizeIncludingTag != 24)
			return false;

		return true;
	}

	DataObjectType DOAssetCatalog::GetType() const
	{
		return DataObjectType::kAssetCatalog;
	}

	bool DOAssetCatalog::Load(DataReader& reader, uint16_t revision)
	{
		if (revision != 4)
			return false;

		if (!reader.ReadU32(m_marker) ||
			!reader.ReadU32(m_totalNameSizePlus22) ||
			!reader.ReadBytes(m_unknown1, 4) ||
			!reader.ReadU32(m_numAssets))
			return false;

		m_assets.resize(m_numAssets);

		for (size_t i = 0; i < m_numAssets; i++)
		{
			AssetInfo& asset = m_assets[i];
			if (!reader.ReadU32(asset.m_unknown1)
				|| !reader.ReadU16(asset.m_nameLength)
				|| !reader.ReadU16(asset.m_unknown2)
				|| !reader.ReadU32(asset.m_unknown3)
				|| !reader.ReadU32(asset.m_unknown4)
				|| !reader.ReadU32(asset.m_unknown5)
				|| !reader.ReadU32(asset.m_unknown6))
				return false;

			if (asset.m_nameLength > 0)
			{
				asset.m_name.resize(asset.m_nameLength);

				if (!reader.ReadBytes(&asset.m_name[0], asset.m_nameLength))
					return false;
			}
		}

		return true;
	}
}
