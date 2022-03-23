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
		case 0x002:
			return new DOProjectInfo();
		case 0x00d:
			return new DOAssetCatalog();
		case 0x017:
			return new DOUnknown17();
		case 0x019:
			return new DOUnknown19();
		case 0x022:
			return new DOProjectLabelMap();
		case 0x3e9:
			return new DOStreamHeader();
		case 0x3ec:
			return new DOUnknown3ec();
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
			if (!reader.ReadU32(asset.m_isDeleted)
				|| !reader.ReadU16(asset.m_nameLength)
				|| !reader.ReadU16(asset.m_alwaysZero)
				|| !reader.ReadU32(asset.m_unknown1)
				|| !reader.ReadU32(asset.m_filePosition)
				|| !reader.ReadU32(asset.m_assetType)
				|| !reader.ReadU32(asset.m_flags))
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

	DataObjectType DOUnknown17::GetType() const
	{
		return DataObjectType::kUnknown17;
	}

	bool DOUnknown17::Load(DataReader& reader, uint16_t revision)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadBytes(m_unknown1, 6))
			return false;

		if (m_sizeIncludingTag != 0x14)
			return false;

		return true;
	}

	DataObjectType DOUnknown19::GetType() const
	{
		return DataObjectType::kUnknown19;
	}

	bool DOUnknown19::Load(DataReader& reader, uint16_t revision)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadBytes(m_unknown1, 2))
			return false;

		if (m_sizeIncludingTag != 0x10)
			return false;

		return true;
	}

	DOProjectLabelMap::DOProjectLabelMap()
		: m_superGroups(nullptr)
	{
	}

	DOProjectLabelMap::~DOProjectLabelMap()
	{
		if (m_superGroups)
			delete[] m_superGroups;
	}

	DataObjectType DOProjectLabelMap::GetType() const
	{
		return DataObjectType::kProjectLabelMap;
	}

	bool DOProjectLabelMap::Load(DataReader& reader, uint16_t revision)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_numSuperGroups)
			|| !reader.ReadU32(m_nextAvailableID))
			return false;

		if (m_unknown1 != 0x16)
			return false;

		m_superGroups = new SuperGroup[m_numSuperGroups];
		for (size_t i = 0; i < m_numSuperGroups; i++)
		{
			if (!LoadSuperGroup(m_superGroups[i], reader, revision))
				return false;
		}

		return true;
	}


	DOProjectLabelMap::LabelTree::LabelTree()
		: m_children(nullptr)
	{
	}

	DOProjectLabelMap::LabelTree::~LabelTree()
	{
		if (m_children)
			delete[] m_children;
	}

	DOProjectLabelMap::SuperGroup::SuperGroup()
		: m_tree(nullptr)
	{
	}

	DOProjectLabelMap::SuperGroup::~SuperGroup()
	{
		if (m_tree)
			delete[] m_tree;
	}

	bool DOProjectLabelMap::LoadSuperGroup(SuperGroup& sg, DataReader& reader, uint16_t revision)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(sg.m_nameLength)
			|| !reader.ReadU32(sg.m_unknown1)
			|| !reader.ReadU32(sg.m_unknown2))
			return false;

		sg.m_name.resize(sg.m_nameLength);
		if (sg.m_nameLength > 0)
		{
			if (!reader.ReadBytes(&sg.m_name[0], sg.m_nameLength))
				return false;
		}

		if (!reader.ReadU32(sg.m_numChildren))
			return false;

		if (sg.m_numChildren)
		{
			sg.m_tree = new LabelTree[sg.m_numChildren];
			for (size_t i = 0; i < sg.m_numChildren; i++)
			{
				if (!LoadLabelTree(sg.m_tree[i], reader, revision))
					return false;
			}
		}

		return true;
	}

	bool DOProjectLabelMap::LoadLabelTree(LabelTree& lt, DataReader& reader, uint16_t revision)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(lt.m_nameLength)
			|| !reader.ReadU32(lt.m_isGroup)
			|| !reader.ReadU32(lt.m_id)
			|| !reader.ReadU32(lt.m_unknown1)
			|| !reader.ReadU32(lt.m_flags))
			return false;

		lt.m_name.resize(lt.m_nameLength);
		if (lt.m_nameLength > 0)
		{
			if (!reader.ReadBytes(&lt.m_name[0], lt.m_nameLength))
				return false;
		}

		if (lt.m_isGroup)
		{
			if (!reader.ReadU32(lt.m_numChildren))
				return false;

			if (lt.m_numChildren)
			{
				lt.m_children = new LabelTree[lt.m_numChildren];
				for (size_t i = 0; i < lt.m_numChildren; i++)
				{
					if (!LoadLabelTree(lt.m_children[i], reader, revision))
						return false;
				}
			}
		}
		else
			lt.m_numChildren = 0;

		return true;
	}

	DataObjectType DOProjectInfo::GetType() const
	{
		return DataObjectType::kProjectInfo;
	}

	bool DOProjectInfo::Load(DataReader& reader, uint16_t revision)
	{
		if (revision != 1 && revision != 2)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_unknown2)
			|| !reader.ReadU32(m_unknown3)
			|| !reader.ReadU16(m_nameLength))
			return false;

		if (m_nameLength > 0)
		{
			m_name.resize(m_nameLength);
			if (!reader.ReadBytes(&m_name[0], m_nameLength))
				return false;

			if (m_name[m_nameLength - 1] != 0)
				return false;
		}

		return true;
	};
}
