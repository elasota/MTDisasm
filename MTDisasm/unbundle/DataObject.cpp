#include "DataObject.h"
#include "DataReader.h"

namespace mtdisasm
{
	bool DORect::Load(DataReader& reader, const SerializationProperties& sp)
	{
		if (sp.m_systemType == SystemType::kMac)
			return reader.ReadS16(m_top) && reader.ReadS16(m_left) && reader.ReadS16(m_bottom) && reader.ReadS16(m_right);
		else if (sp.m_systemType == SystemType::kWindows)
			return reader.ReadS16(m_left) && reader.ReadS16(m_top) && reader.ReadS16(m_right) && reader.ReadS16(m_bottom);
		else
			return false;
	}

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
			return new DOProjectStructuralDef();
		case 0x003:
			return new DOSectionStructuralDef();
		case 0x008:
			return new DOSceneStructuralDef();
		case 0x00d:
			return new DOAssetCatalog();
		case 0x017:
			return new DOUnknown17();
		case 0x019:
			return new DOUnknown19();
		case 0x01e:
			return new DOColorTableAsset();
		case 0x021:
			return new DOSubsectionStructuralDef();
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

	bool DOStreamHeader::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
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

	bool DOUnknown3ec::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
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

	bool DOAssetCatalog::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
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
			if (!reader.ReadU32(asset.m_flags1)
				|| !reader.ReadU16(asset.m_nameLength)
				|| !reader.ReadU16(asset.m_alwaysZero)
				|| !reader.ReadU32(asset.m_unknown1)
				|| !reader.ReadU32(asset.m_filePosition)
				|| !reader.ReadU32(asset.m_assetType)
				|| !reader.ReadU32(asset.m_flags2))
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

	bool DOUnknown17::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
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

	bool DOUnknown19::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
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

	bool DOProjectLabelMap::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
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

	DataObjectType DOProjectStructuralDef::GetType() const
	{
		return DataObjectType::kProjectStructuralDef;
	}

	bool DOProjectStructuralDef::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 1 && revision != 2)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_unknown2)
			|| !reader.ReadU32(m_flags)
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
	}

	DataObjectType DOColorTableAsset::GetType() const
	{
		return DataObjectType::kColorTableAsset;
	}

	bool DOColorTableAsset::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadU32(m_sizeIncludingTag))
			return false;

		if (sp.m_systemType == SystemType::kMac)
		{
			if (m_sizeIncludingTag != 0x0836)
				return false;
		}
		else if (sp.m_systemType == SystemType::kWindows)
		{
			if (m_sizeIncludingTag != 0x0428)
				return false;
		}
		else
			return false;

		if (!reader.ReadBytes(m_unknown1, 4)
			|| !reader.ReadU32(m_assetID)
			|| !reader.ReadU32(m_unknown2))
			return false;

		const size_t numColors = 256;
		if (sp.m_systemType == SystemType::kMac)
		{
			if (!reader.Skip(20))
				return false;

			uint8_t cdefBytes[256 * 8];
			if (!reader.ReadBytes(cdefBytes, numColors * 8))
				return false;

			for (size_t i = 0; i < numColors; i++)
			{
				if (cdefBytes[i * 8] != 0)
					return false;	// Bad color index

				ColorDef& cdef = m_colors[cdefBytes[i * 8 + 1]];

				const uint8_t* rgb = cdefBytes + i * 8 + 2;
				cdef.m_red = (rgb[0] << 8) | rgb[1];
				cdef.m_green = (rgb[2] << 8) | rgb[5];
				cdef.m_blue = (rgb[4] << 8) | rgb[6];
			}
		}
		else if (sp.m_systemType == SystemType::kWindows)
		{
			if (!reader.Skip(14))
				return false;

			uint8_t cdefBytes[256 * 4];
			if (!reader.ReadBytes(cdefBytes, numColors * 4))
				return false;

			for (size_t i = 0; i < numColors; i++)
			{
				ColorDef& cdef = m_colors[i];

				cdef.m_red = cdefBytes[i * 4 + 2] * 0x101;
				cdef.m_green = cdefBytes[i * 4 + 1] * 0x101;
				cdef.m_blue = cdefBytes[i * 4 + 0] * 0x101;
			}
		}
		else
			return false;

		return true;
	}

	DataObjectType DOSectionStructuralDef::GetType() const
	{
		return DataObjectType::kSectionStructuralDef;
	}

	bool DOSectionStructuralDef::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 1)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_unknown2)
			|| !reader.ReadU16(m_lengthOfName)
			|| !reader.ReadU32(m_flags)
			|| !reader.ReadU16(m_unknown4)
			|| !reader.ReadU16(m_sectionID)
			|| !reader.ReadU32(m_segmentID))
			return false;

		if (m_lengthOfName > 0)
		{
			m_name.resize(m_lengthOfName);
			if (!reader.ReadBytes(&m_name[0], m_lengthOfName))
				return false;

			if (m_name[m_lengthOfName - 1] != 0)
				return false;
		}

		return true;
	}

	DataObjectType DOSubsectionStructuralDef::GetType() const
	{
		return DataObjectType::kSubsectionStructuralDef;
	}

	bool DOSubsectionStructuralDef::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_unknown2)
			|| !reader.ReadU16(m_lengthOfName)
			|| !reader.ReadU32(m_flags)
			|| !reader.ReadU16(m_sectionID))
			return false;

		if (m_lengthOfName > 0)
		{
			m_name.resize(m_lengthOfName);
			if (!reader.ReadBytes(&m_name[0], m_lengthOfName))
				return false;

			if (m_name[m_lengthOfName - 1] != 0)
				return false;
		}

		return true;
	}

	DataObjectType DOSceneStructuralDef::GetType() const
	{
		return DataObjectType::kSceneStructuralDef;
	}

	bool DOSceneStructuralDef::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 1)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_unknown2)
			|| !reader.ReadU16(m_lengthOfName)
			|| !reader.ReadU32(m_flags)
			|| !reader.ReadBytes(m_unknown4, 2)
			|| !reader.ReadU16(m_sectionID)
			|| !m_rect1.Load(reader, sp)
			|| !m_rect2.Load(reader, sp)
			|| !reader.ReadU32(m_streamLocator)
			|| !reader.ReadBytes(m_unknown11, 4))
			return false;

		if (m_lengthOfName > 0)
		{
			m_name.resize(m_lengthOfName);
			if (!reader.ReadBytes(&m_name[0], m_lengthOfName))
				return false;

			if (m_name[m_lengthOfName - 1] != 0)
				return false;
		}

		return true;
	}
}
