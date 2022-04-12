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

	bool DOPoint::Load(DataReader& reader, const SerializationProperties& sp)
	{
		if (sp.m_systemType == SystemType::kMac)
			return reader.ReadS16(m_top) && reader.ReadS16(m_left);
		else if (sp.m_systemType == SystemType::kWindows)
			return reader.ReadS16(m_left) && reader.ReadS16(m_top);
		else
			return false;
	}

	bool DOEvent::Load(DataReader& reader)
	{
		return reader.ReadU32(m_eventID) && reader.ReadU32(m_eventInfo);
	}


	bool DOColor::Load(DataReader& reader, const SerializationProperties& sp)
	{
		uint32_t colorPos = reader.TellGlobal();
		if (sp.m_systemType == mtdisasm::SystemType::kMac)
			return reader.ReadU16(m_red) && reader.ReadU16(m_green) && reader.ReadU16(m_blue);

		if (sp.m_systemType == mtdisasm::SystemType::kWindows)
		{
			uint32_t absPos = reader.TellGlobal();
			uint8_t bgra[4];
			if (!reader.ReadBytes(bgra, 4))
				return false;

			m_blue = bgra[0] * 0x101;
			m_green = bgra[1] * 0x101;
			m_red = bgra[2] * 0x101;

			return true;
		}

		return false;
	}

	bool DOFloat::Load(DataReader& reader, const SerializationProperties& sp)
	{
		if (sp.m_systemType == mtdisasm::SystemType::kMac)
			return reader.ReadF80BE(m_value);

		if (sp.m_systemType == mtdisasm::SystemType::kWindows)
			return reader.ReadF64(m_value);

		return false;
	}

	bool DOVector::Load(DataReader& reader, const SerializationProperties& sp)
	{
		return m_angleRadians.Load(reader, sp) && m_magnitude.Load(reader, sp);
	}

	bool DOTypicalModifierHeader::Load(DataReader& reader)
	{
		if (!reader.ReadU32(m_modifierFlags)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_guid)
			|| !reader.ReadBytes(m_unknown2, 6)
			|| !reader.ReadU32(m_unknown3)
			|| !reader.ReadBytes(m_unknown4, 4)
			|| !reader.ReadU16(m_lengthOfName))
			return false;

		if (m_lengthOfName > 0)
		{
			m_name.resize(m_lengthOfName);

			if (!reader.ReadBytes(&m_name[0], m_lengthOfName))
				return false;
		}

		return true;
	}

	bool DOMessageDataLocator::Load(DataReader& reader)
	{
		if (!reader.ReadU16(m_withCode) || !reader.ReadBytes(m_unknown1, 4)
			|| !reader.ReadU32(m_guid) || !reader.ReadBytes(m_unknown2, 36))
			return false;

		return true;
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
		case 0x005:
			return new DOMovieStructuralDef();
		case 0x006:
			return new DOMToonStructuralDef();
		case 0x007:
			return new DOImageStructuralDef();
		case 0x008:
			return new DOSceneStructuralDef();
		case 0x0015:
			return new DONotYetImplemented(objectType, "Text label object");
		case 0x001f:
			return new DONotYetImplemented(objectType, "Unknown '0x1f' type asset");
		case 0xa:
			return new DONotYetImplemented(objectType, "Sound structural def");
		case 0x00d:
			return new DOAssetCatalog();
		case 0x017:
			return new DOGlobalObjectInfo();
		case 0x019:
			return new DOUnknown19();
		case 0xfffffffe:
			return new DODebris();
		case 0x00e:
			return new DOImageAsset();
		case 0x00f:
			return new DOMToonAsset();
		case 0x010:
			return new DOMovieAsset();
		case 0x011:
			return new DOAudioAsset();
		case 0x01e:
			return new DOColorTableAsset();
		case 0x021:
			return new DOSubsectionStructuralDef();
		case 0x022:
			return new DOProjectLabelMap();
		case 0x3e9:
			return new DOStreamHeader();
		case 0x3ec:
			return new DOPresentationSettings();
		case 0x2c6:
			return new DOBehaviorModifier();
		case 0x3c0:
			return new DOMiniscriptModifier();
		case 0x2da:
			return new DOMessengerModifier();
		case 0x2bc:
			return new DOIfMessengerModifier();
		case 0x2e4:
			return new DOTimerMessengerModifier();
		case 0x2f8:
			return new DONotYetImplemented(objectType, "Boundary Detection Messenger modifier");
		case 0x2ee:
			return new DOCollisionDetectionMessengerModifier();
		case 0x302:
			return new DOKeyboardMessengerModifier();
		case 0x321:
			return new DOBooleanVariableModifier();
		case 0x2c7:
			return new DONotYetImplemented(objectType, "Compound Variable modifier");
		case 0x322:
			return new DOIntegerVariableModifier();
		case 0x329:
			return new DOStringVariableModifier();
		case 0x324:
			return new DOIntegerRangeVariableModifier();
		case 0x328:
			return new DOFloatVariableModifier();
		case 0x327:
			return new DOVectorVariableModifier();
		case 0x326:
			return new DOPointVariableModifier();
		case 0x136:
			return new DONotYetImplemented(objectType, "Change Scene modifier");
		case 0x140:
			return new DONotYetImplemented(objectType, "Return modifier");
		case 0x29a:
			return new DONotYetImplemented(objectType, "Shared Scene modifier");
		case 0x2df:
			return new DOSetModifier();
		case 0x4d8:
			return new DONotYetImplemented(objectType, "Save and Restore modifier");
		case 0x26c:
			return new DONotYetImplemented(objectType, "Scene Transition modifier");
		case 0x276:
			return new DONotYetImplemented(objectType, "Element Transition modifier");
		case 0x1fe:
			return new DONotYetImplemented(objectType, "Simple Motion modifier");
		case 0x21b:
			return new DONotYetImplemented(objectType, "Path Motion modifier");
		case 0x21c:
			return new DONotYetImplemented(objectType, "Path Motion modifier - obsolete version");
		case 0x208:
			return new DODragMotionModifier();
		case 0x226:
			return new DOVectorMotionModifier();
		case 0x1a4:
			return new DONotYetImplemented(objectType, "Sound Effect modifier");
		case 0x32a:
			return new DOTextStyleModifier();
		case 0x334:
			return new DOGraphicModifier();
		case 0x4c4:
			return new DONotYetImplemented(objectType, "Color Table modifier");
		case 0x4b0:
			return new DONotYetImplemented(objectType, "Gradient modifier");
		case 0x384:
			return new DONotYetImplemented(objectType, "Image Effect modifier");
		case 0x27:
			return new DONotYetImplemented(objectType, "Alias");
		case 0xffffffff:
			return new DOPlugInModifier();
		case 0x3ca:
			return new DOMacOnlyCursorModifier();
		case 0xffff:
			return new DOAssetDataSection();
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

	DataObjectType DOPresentationSettings::GetType() const
	{
		return DataObjectType::kPresentationSettings;
	}

	bool DOPresentationSettings::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 2)
			return false;

		if (!reader.ReadU32(m_marker) ||
			!reader.ReadU32(m_sizeIncludingTag) ||
			!reader.ReadBytes(m_unknown1, 2) ||
			!m_dimensions.Load(reader, sp) ||
			!reader.ReadU16(m_bitsPerPixel) ||
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

	DataObjectType DOGlobalObjectInfo::GetType() const
	{
		return DataObjectType::kGlobalObjectInfo;
	}

	bool DOGlobalObjectInfo::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU16(m_numGlobalModifiers)
			|| !reader.ReadBytes(m_unknown1, 4))
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

	DataObjectType DODebris::GetType() const
	{
		return DataObjectType::kDebris;
	}

	bool DODebris::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadU32(m_sizeIncludingTag))
			return false;

		if (m_sizeIncludingTag != 0xe)
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
			|| !reader.ReadU32(sg.m_id)
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
			|| !reader.ReadU32(m_guid)
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

		size_t numColors = 256;
		if (sp.m_systemType == SystemType::kMac)
		{
			if (!reader.Skip(20))
				return false;

			uint8_t clutHeader[8];
			if (!reader.ReadBytes(clutHeader, 8))
				return false;

			uint8_t cdefBytes[256 * 8];
			if (!reader.ReadBytes(cdefBytes, numColors * 8))
				return false;

			for (size_t i = 0; i < numColors; i++)
			{
				ColorDef& cdef = m_colors[i];

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
			|| !reader.ReadU32(m_guid)
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
			|| !reader.ReadU32(m_guid)
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
			|| !reader.ReadU32(m_guid)
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

	DataObjectType DOImageStructuralDef::GetType() const
	{
		return DataObjectType::kImageStructuralDef;
	}

	bool DOImageStructuralDef::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 2)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_guid)
			|| !reader.ReadU16(m_lengthOfName)
			|| !reader.ReadU32(m_flags)
			|| !reader.ReadBytes(m_unknown4, 2)
			|| !reader.ReadU16(m_sectionID)
			|| !m_rect1.Load(reader, sp)
			|| !m_rect2.Load(reader, sp)
			|| !reader.ReadU32(m_imageAssetID)
			|| !reader.ReadU32(m_streamLocator)
			|| !reader.ReadBytes(m_unknown7, 4))
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

	DataObjectType DOMovieStructuralDef::GetType() const
	{
		return DataObjectType::kMovieStructuralDef;
	}

	bool DOMovieStructuralDef::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 2)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_guid)
			|| !reader.ReadU16(m_lengthOfName)
			|| !reader.ReadU32(m_flags)
			|| !reader.ReadU16(m_layer)
			|| !reader.ReadBytes(m_unknown3, 44)
			|| !reader.ReadU16(m_sectionID)
			|| !reader.ReadBytes(m_unknown5, 2)
			|| !m_rect1.Load(reader, sp)
			|| !m_rect2.Load(reader, sp)
			|| !reader.ReadU32(m_assetID)
			|| !reader.ReadU32(m_unknown7)
			|| !reader.ReadU16(m_volume)
			|| !reader.ReadU32(m_animationFlags)
			|| !reader.ReadBytes(m_unknown10, 4)
			|| !reader.ReadBytes(m_unknown11, 4)
			|| !reader.ReadU32(m_streamLocator)
			|| !reader.ReadBytes(m_unknown13, 4))
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

	DataObjectType DOMToonStructuralDef::GetType() const
	{
		return DataObjectType::kMToonStructuralDef;
	}

	bool DOMToonStructuralDef::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		// No known structural changes in revision 3
		if (revision != 2 && revision != 3)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_guid)
			|| !reader.ReadU16(m_lengthOfName)
			|| !reader.ReadU32(m_structuralFlags)
			|| !reader.ReadBytes(m_unknown3, 2)
			|| !reader.ReadU32(m_animationFlags)
			|| !reader.ReadBytes(m_unknown4, 4)
			|| !reader.ReadU16(m_sectionID)
			|| !m_rect1.Load(reader, sp)
			|| !m_rect2.Load(reader, sp)
			|| !reader.ReadU32(m_unknown5)
			|| !reader.ReadU32(m_rateTimes10000)
			|| !reader.ReadU32(m_streamLocator)
			|| !reader.ReadU32(m_unknown6))
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

	DataObjectType DOMessengerModifier::GetType() const
	{
		return DataObjectType::kMessengerModifier;
	}

	bool DOMessengerModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3ea)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_guid)
			|| !reader.ReadBytes(m_unknown3, 6)
			|| !reader.ReadU32(m_unknown4)
			|| !reader.ReadBytes(m_unknown5, 4)
			|| !reader.ReadU16(m_lengthOfName))
			return false;

		if (m_lengthOfName > 0)
		{
			m_name.resize(m_lengthOfName);
			if (!reader.ReadBytes(&m_name[0], m_lengthOfName))
				return false;

			if (m_name[m_lengthOfName - 1] != 0)
				return false;
		}

		if (!reader.ReadU32(m_messageFlags)
			|| !reader.ReadU32(m_when.m_eventID)
			|| !m_send.Load(reader)
			|| !reader.ReadU16(m_unknown14)
			|| !reader.ReadU32(m_destination)
			|| !reader.ReadBytes(m_unknown11, 10)
			|| !reader.ReadU16(m_with)
			|| !reader.ReadBytes(m_unknown15, 4)
			|| !reader.ReadU32(m_withSourceGUID)
			|| !reader.ReadBytes(m_unknown12, 36)
			|| !reader.ReadU32(m_when.m_eventInfo)
			|| !reader.ReadU8(m_withSourceLength)
			|| !reader.ReadU8(m_unknown13))
			return false;

		if (m_withSourceLength > 0)
		{
			m_withSource.resize(m_withSourceLength + 1);
			if (!reader.ReadBytes(&m_withSource[0], m_withSourceLength))
				return false;
			m_withSource[m_withSourceLength] = 0;
		}

		return true;
	}

	DataObjectType DOSetModifier::GetType() const
	{
		return DataObjectType::kSetModifier;
	}

	bool DOSetModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader) || !reader.ReadBytes(m_unknown1, 4) || !reader.ReadU32(m_when.m_eventID) || !m_sourceLocator.Load(reader)
			|| !m_targetLocator.Load(reader) || !reader.ReadU32(m_when.m_eventInfo) || !reader.ReadU8(m_unknown3) || !reader.ReadU8(m_sourceNameLength)
			|| !reader.ReadU8(m_targetNameLength) || !reader.ReadBytes(m_unknown4, 3))
			return false;

		if (m_sourceNameLength > 0)
		{
			m_sourceName.resize(m_sourceNameLength + 1);
			if (!reader.ReadBytes(&m_sourceName[0], m_sourceNameLength))
				return false;
			m_sourceName[m_sourceNameLength] = 0;
		}

		if (m_targetNameLength > 0)
		{
			m_targetName.resize(m_targetNameLength + 1);
			if (!reader.ReadBytes(&m_targetName[0], m_targetNameLength))
				return false;
			m_targetName[m_targetNameLength] = 0;
		}

		return true;
	}

	DataObjectType DOIfMessengerModifier::GetType() const
	{
		return DataObjectType::kIfMessengerModifier;
	}

	bool DOIfMessengerModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3ea)
			return false;

		if (!m_modHeader.Load(reader))
			return false;

		if (!reader.ReadU32(m_messageFlags)
			|| !m_when.Load(reader)
			|| !m_send.Load(reader)
			|| !reader.ReadU16(m_unknown6)
			|| !reader.ReadU32(m_destination)
			|| !reader.ReadBytes(m_unknown7, 10)
			|| !reader.ReadU16(m_with)
			|| !reader.ReadBytes(m_unknown8, 4)
			|| !reader.ReadU32(m_withSourceGUID)
			|| !reader.ReadBytes(m_unknown9, 46)
			|| !reader.ReadU8(m_withSourceLength)
			|| !reader.ReadU8(m_unknown10))
			return false;

		if (m_withSourceLength > 0)
		{
			m_withSource.resize(m_withSourceLength + 1);
			if (!reader.ReadBytes(&m_withSource[0], m_withSourceLength))
				return false;
			m_withSource[m_withSourceLength] = 0;
		}

		if (!m_program.Load(reader, sp))
			return false;

		return true;
	}

	DataObjectType DOTimerMessengerModifier::GetType() const
	{
		return DataObjectType::kTimerMessengerModifier;
	}

	bool DOTimerMessengerModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3ea)
			return false;

		if (!m_modHeader.Load(reader))
			return false;

		if (!reader.ReadU32(m_messageAndTimerFlags)
			|| !m_executeWhen.Load(reader)
			|| !m_send.Load(reader)
			|| !m_terminateWhen.Load(reader)
			|| !reader.ReadU16(m_unknown2)
			|| !reader.ReadU32(m_destination)
			|| !reader.ReadBytes(m_unknown4, 10)
			|| !m_with.Load(reader)
			|| !reader.ReadU8(m_unknown5)
			|| !reader.ReadU8(m_minutes)
			|| !reader.ReadU8(m_seconds)
			|| !reader.ReadU8(m_hundredthsOfSeconds)
			|| !reader.ReadU32(m_unknown6)
			|| !reader.ReadU32(m_unknown7)
			|| !reader.ReadBytes(m_unknown8, 10)
			|| !reader.ReadU8(m_withSourceLength)
			|| !reader.ReadU8(m_unknown9))
			return false;

		if (m_withSourceLength > 0)
		{
			m_withSource.resize(m_withSourceLength + 1);
			if (!reader.ReadBytes(&m_withSource[0], m_withSourceLength))
				return false;
			m_withSource[m_withSourceLength] = 0;
		}

		return true;
	}

	DataObjectType DOCollisionDetectionMessengerModifier::GetType() const
	{
		return DataObjectType::kCollisionDetectionMessengerModifier;
	}

	bool DOCollisionDetectionMessengerModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3ea)
			return false;

		if (!m_modHeader.Load(reader))
			return false;

		if (!reader.ReadU32(m_messageAndModifierFlags)
			|| !m_enableWhen.Load(reader)
			|| !m_disableWhen.Load(reader)
			|| !m_send.Load(reader)
			|| !reader.ReadU16(m_unknown2)
			|| !reader.ReadU32(m_destination)
			|| !reader.ReadBytes(m_unknown3, 10)
			|| !m_with.Load(reader)
			|| !reader.ReadU8(m_withSourceLength)
			|| !reader.ReadU8(m_unknown4))
			return false;

		if (m_withSourceLength > 0)
		{
			m_withSource.resize(m_withSourceLength + 1);
			if (!reader.ReadBytes(&m_withSource[0], m_withSourceLength))
				return false;
			m_withSource[m_withSourceLength] = 0;
		}

		return true;
	}

	DataObjectType DOKeyboardMessengerModifier::GetType() const
	{
		return DataObjectType::kKeyboardMessengerModifier;
	}

	bool DOKeyboardMessengerModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3eb)
			return false;

		if (!m_modHeader.Load(reader))
			return false;

		if (!reader.ReadU32(m_messageFlagsAndKeyStates)
			|| !reader.ReadU16(m_unknown2)
			|| !reader.ReadU16(m_keyModifiers)
			|| !reader.ReadU8(m_keycode)
			|| !reader.ReadBytes(m_unknown4, 7)
			|| !m_message.Load(reader)
			|| !reader.ReadU16(m_unknown7)
			|| !reader.ReadU32(m_destination)
			|| !reader.ReadBytes(m_unknown9, 10)
			|| !reader.ReadU16(m_with)
			|| !reader.ReadBytes(m_unknown11, 4)
			|| !reader.ReadU32(m_withSourceGUID)
			|| !reader.ReadBytes(m_unknown13, 36)
			|| !reader.ReadU8(m_withSourceLength)
			|| !reader.ReadU8(m_unknown14))
			return false;

		if (m_withSourceLength > 0)
		{
			m_withSource.resize(m_withSourceLength + 1);
			if (!reader.ReadBytes(&m_withSource[0], m_withSourceLength))
				return false;
			m_withSource[m_withSourceLength] = 0;
		}

		return true;
	}

	DataObjectType DOBehaviorModifier::GetType() const
	{
		return DataObjectType::kBehaviorModifier;
	}

	bool DOBehaviorModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 1)
			return false;

		if (!reader.ReadU32(m_modifierFlags)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadBytes(m_unknown2, 2)
			|| !reader.ReadU32(m_guid)
			|| !reader.ReadU32(m_unknown4)
			|| !reader.ReadU16(m_unknown5)
			|| !reader.ReadU32(m_unknown6)
			|| !m_editorLayoutPosition.Load(reader, sp)
			|| !reader.ReadU16(m_lengthOfName)
			|| !reader.ReadU16(m_numChildren))
			return false;

		if (m_lengthOfName > 0)
		{
			m_name.resize(m_lengthOfName);
			if (!reader.ReadBytes(&m_name[0], m_lengthOfName))
				return false;

			if (m_name[m_lengthOfName - 1] != 0)
				return false;
		}

		if (!reader.ReadU32(m_flags)
			|| !m_enableWhen.Load(reader)
			|| !m_disableWhen.Load(reader)
			|| !reader.ReadBytes(m_unknown7, 2))
			return false;

		return true;
	}

	bool DOMiniscriptProgram::Load(DataReader& reader, const SerializationProperties& sp)
	{
		m_sp = sp;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeOfInstructions)
			|| !reader.ReadU32(m_numOfInstructions)
			|| !reader.ReadU32(m_numLocalRefs)
			|| !reader.ReadU32(m_numAttributes))
			return false;

		if (m_sizeOfInstructions > 0)
		{
			m_bytecode.resize(m_sizeOfInstructions);
			if (!reader.ReadBytes(&m_bytecode[0], m_sizeOfInstructions))
				return false;
		}

		if (m_numLocalRefs > 0)
		{
			m_localRefs.resize(m_numLocalRefs);
			for (size_t i = 0; i < m_numLocalRefs; i++)
			{
				LocalRef& localRef = m_localRefs[i];
				if (!reader.ReadU32(localRef.m_guid)
					|| !reader.ReadU8(localRef.m_lengthOfName)
					|| !reader.ReadU8(localRef.m_unknown10))
					return false;

				if (localRef.m_lengthOfName > 0)
				{
					localRef.m_name.resize(localRef.m_lengthOfName);
					if (!reader.ReadBytes(&localRef.m_name[0], localRef.m_lengthOfName) || localRef.m_name[localRef.m_lengthOfName - 1] != 0)
						return false;
				}
			}
		}

		if (m_numAttributes > 0)
		{
			m_attributes.resize(m_numAttributes);
			for (size_t i = 0; i < m_numAttributes; i++)
			{
				Attribute& attrib = m_attributes[i];
				if (!reader.ReadU8(attrib.m_lengthOfName) || !reader.ReadU8(attrib.m_unknown11))
					return false;

				if (attrib.m_lengthOfName > 0)
				{
					attrib.m_name.resize(attrib.m_lengthOfName);
					if (!reader.ReadBytes(&attrib.m_name[0], attrib.m_lengthOfName) || attrib.m_name[attrib.m_lengthOfName - 1] != 0)
						return false;
				}
			}
		}

		return true;
	}

	DataObjectType DOBooleanVariableModifier::GetType() const
	{
		return DataObjectType::kBooleanVariableModifier;
	}

	bool DOBooleanVariableModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader))
			return false;

		if (!reader.ReadU8(m_value)
			|| !reader.ReadU8(m_unknown5))
			return false;

		return true;
	}

	DataObjectType DOIntegerVariableModifier::GetType() const
	{
		return DataObjectType::kIntegerVariableModifier;
	}

	bool DOIntegerVariableModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader)
			|| !reader.ReadBytes(m_unknown1, 4)
			|| !reader.ReadS32(m_value))
			return false;

		return true;
	}

	DataObjectType DOStringVariableModifier::GetType() const
	{
		return DataObjectType::kStringVariableModifier;
	}

	bool DOStringVariableModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader)
			|| !reader.ReadU32(m_lengthOfString)
			|| !reader.ReadBytes(m_unknown1, 4))
			return false;

		if (m_lengthOfString > 0)
		{
			m_string.resize(m_lengthOfString);
			if (!reader.ReadBytes(&m_string[0], m_lengthOfString) || m_string[m_lengthOfString - 1] != 0)
				return false;
		}

		return true;
	}

	DataObjectType DOFloatVariableModifier::GetType() const
	{
		return DataObjectType::kFloatVariableModifier;
	}

	bool DOFloatVariableModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader)
			|| !reader.ReadBytes(m_unknown1, 4)
			|| !m_value.Load(reader, sp))
			return false;

		return true;
	}

	DataObjectType DOVectorVariableModifier::GetType() const
	{
		return DataObjectType::kVectorVariableModifier;
	}

	bool DOVectorVariableModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader)
			|| !reader.ReadBytes(m_unknown1, 4)
			|| !m_value.Load(reader, sp))
			return false;

		return true;
	}

	DataObjectType DOIntegerRangeVariableModifier::GetType() const
	{
		return DataObjectType::kIntegerRangeVariableModifier;
	}

	bool DOIntegerRangeVariableModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader)
			|| !reader.ReadBytes(m_unknown1, 4)
			|| !reader.ReadS32(m_min)
			|| !reader.ReadS32(m_max))
			return false;

		return true;
	};

	DataObjectType DOPointVariableModifier::GetType() const
	{
		return DataObjectType::kPointVariableModifier;
	}

	bool DOPointVariableModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader)
			|| !reader.ReadBytes(m_unknown5, 4)
			|| !m_value.Load(reader, sp))
			return false;

		return true;
	}

	DataObjectType DOMiniscriptModifier::GetType() const
	{
		return DataObjectType::kMiniscriptModifier;
	}

	bool DOMiniscriptModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3eb)
			return false;

		m_sp = sp;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_guid)
			|| !reader.ReadBytes(m_unknown3, 6)
			|| !reader.ReadU32(m_unknown4)
			|| !reader.ReadBytes(m_unknown5, 4)
			|| !reader.ReadU16(m_lengthOfName))
			return false;

		if (m_lengthOfName > 0)
		{
			m_name.resize(m_lengthOfName);
			if (!reader.ReadBytes(&m_name[0], m_lengthOfName) || m_name[m_lengthOfName - 1] != 0)
				return false;
		}

		if (!m_enableWhen.Load(reader)
			|| !reader.ReadBytes(m_unknown6, 11)
			|| !reader.ReadU8(m_unknown7)
			|| !m_program.Load(reader, sp))
			return false;

		return true;
	}

	DONotYetImplemented::DONotYetImplemented(uint32_t actualType, const char* name)
		: m_actualType(actualType)
		, m_name(name)
	{
	}

	DataObjectType DONotYetImplemented::GetType() const
	{
		return DataObjectType::kNotYetImplemented;
	}

	bool DONotYetImplemented::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		m_revision = revision;

		if (!reader.ReadU32(m_unknown)
			|| !reader.ReadU32(m_sizeIncludingTag))
			return false;

		if (m_sizeIncludingTag < 14)
			return false;

		if (!reader.Skip(m_sizeIncludingTag - 14))
			return false;

		return true;
	}

	PlugInObject::~PlugInObject()
	{
	}

	DOPlugInModifier::DOPlugInModifier()
		: m_plugInData(nullptr)
	{
	}

	DOPlugInModifier::~DOPlugInModifier()
	{
		if (m_plugInData)
			delete m_plugInData;
	}

	PlugInObjectType POCursorMod::GetType() const
	{
		return PlugInObjectType::kCursorMod;
	}

	bool POCursorMod::Load(const DOPlugInModifier& base, DataReader& reader, const SerializationProperties& sp)
	{
		if (!reader.ReadU16(m_unknown1)
			|| !m_applyWhen.Load(reader)
			|| !reader.ReadU16(m_unknown2)
			|| !m_removeWhen.Load(reader)
			|| !reader.ReadU16(m_unknown3)
			|| !reader.ReadU32(m_cursorID)
			|| !reader.ReadBytes(m_unknown4, 4))
			return false;

		return true;
	}

	PlugInObjectType POUnknown::GetType() const
	{
		return PlugInObjectType::kUnknown;
	}

	bool POUnknown::Load(const DOPlugInModifier& base, DataReader& reader, const SerializationProperties& sp)
	{
		m_data.resize(base.m_privateDataSize);
		if (base.m_privateDataSize > 0 && !reader.ReadBytes(&m_data[0], base.m_privateDataSize))
			return false;

		return true;
	}

	PlugInObjectType POMidiModifier::GetType() const
	{
		return PlugInObjectType::kMIDIModf;
	}

	bool POMidiModifier::Load(const DOPlugInModifier& base, DataReader& reader, const SerializationProperties& sp)
	{
		if (base.m_plugInRevision != 1 && base.m_plugInRevision != 2)
			return false;

		if (!reader.ReadU16(m_unknown1)
			|| !m_executeWhen.Load(reader)
			|| !reader.ReadU16(m_unknown2)
			|| !m_terminateWhen.Load(reader)
			|| !reader.ReadU8(m_embeddedFlag))
			return false;

		if (m_embeddedFlag)
		{
			if (!reader.ReadU8(m_typeDependent.m_embedded.m_hasFile))
				return false;
			if (m_typeDependent.m_embedded.m_hasFile)
			{
				if (!reader.ReadBytes(m_typeDependent.m_embedded.m_bigEndianLength, 4))
					return false;

				uint32_t length = (m_typeDependent.m_embedded.m_bigEndianLength[0] << 24)
					+ (m_typeDependent.m_embedded.m_bigEndianLength[1] << 16)
					+ (m_typeDependent.m_embedded.m_bigEndianLength[2] << 8)
					+ m_typeDependent.m_embedded.m_bigEndianLength[3];

				m_data.resize(length);
				if (length > 0 && !reader.ReadBytes(&m_data[0], length))
					return false;
			}

			if (!reader.ReadU8(m_typeDependent.m_embedded.m_loop)
				|| !reader.ReadU8(m_typeDependent.m_embedded.m_overrideTempo)
				|| !reader.ReadU8(m_typeDependent.m_embedded.m_volume)
				|| !reader.ReadU16(m_typeDependent.m_embedded.m_tempoTypeTag)
				|| !m_typeDependent.m_embedded.m_tempo.Load(reader, sp)
				|| !reader.ReadU16(m_typeDependent.m_embedded.m_fadeInTypeTag)
				|| !m_typeDependent.m_embedded.m_fadeIn.Load(reader, sp)
				|| !reader.ReadU16(m_typeDependent.m_embedded.m_fadeOutTypeTag)
				|| !m_typeDependent.m_embedded.m_fadeOut.Load(reader, sp))
				return false;
		}
		else
		{
			if (!reader.ReadU8(m_typeDependent.m_singleNote.m_channel)
				|| !reader.ReadU8(m_typeDependent.m_singleNote.m_note)
				|| !reader.ReadU8(m_typeDependent.m_singleNote.m_velocity)
				|| !reader.ReadU8(m_typeDependent.m_singleNote.m_program)
				|| !reader.ReadU16(m_typeDependent.m_singleNote.m_durationTypeTag)
				|| !m_typeDependent.m_singleNote.m_duration.Load(reader, sp))
				return false;
		}

		return true;
	}

	DataObjectType DOPlugInModifier::GetType() const
	{
		return DataObjectType::kPlugInModifier;
	}

	bool DOPlugInModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e9)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_weirdSize))
			return false;

		if (!reader.ReadBytes(m_plugin, 16)
			|| !reader.ReadU32(m_guid)
			|| !reader.ReadBytes(m_unknown2, 6)
			|| !reader.ReadU16(m_plugInRevision)
			|| !reader.ReadU32(m_unknown4)
			|| !reader.ReadBytes(m_unknown5, 4)
			|| !reader.ReadU16(m_lengthOfName))
			return false;

		m_plugin[16] = 0;

		if (m_lengthOfName > 0)
		{
			m_name.resize(m_lengthOfName);
			if (!reader.ReadBytes(&m_name[0], m_lengthOfName))
				return false;

			if (m_name[m_lengthOfName - 1] != 0)
				return false;
		}

		m_privateDataSize = m_weirdSize;
		if (sp.m_systemType == SystemType::kWindows)
		{
			// wtf??
			if (m_privateDataSize < m_lengthOfName * 255u)
				return false;
			m_privateDataSize -= m_lengthOfName * 256u;
		}
		else
			m_privateDataSize -= m_lengthOfName;

		if (m_privateDataSize < 52)
			return false;
		m_privateDataSize -= 52;

		if (!strcmp(m_plugin, "CursorMod"))
			m_plugInData = new POCursorMod();
		else if (!strcmp(m_plugin, "MIDIModf"))
			m_plugInData = new POMidiModifier();

		if (!m_plugInData)
			m_plugInData = new POUnknown();

		if (!m_plugInData->Load(*this, reader, sp))
			return false;

		return true;
	}

	DataObjectType DOMacOnlyCursorModifier::GetType() const
	{
		return DataObjectType::kMacOnlyCursorModifier;
	}

	bool DOMacOnlyCursorModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e9)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag)
			|| !reader.ReadU32(m_unknown2)
			|| !reader.ReadU32(m_unknown3)
			|| !reader.ReadU16(m_unknown4)
			|| !reader.ReadU32(m_unknown5)
			|| !reader.ReadBytes(m_unknown6, 4)
			|| !reader.ReadU16(m_lengthOfName)
			)
			return false;

		if (m_lengthOfName > 0)
		{
			m_name.resize(m_lengthOfName);
			if (!reader.ReadBytes(&m_name[0], m_lengthOfName))
				return false;

			if (m_name[m_lengthOfName - 1] != 0)
				return false;
		}

		if (sp.m_systemType == SystemType::kMac)
		{
			m_hasMacOnlyPart = true;


			if (!m_macOnlyPart.m_applyWhen.Load(reader)
				|| !reader.ReadU32(m_macOnlyPart.m_unknown1)
				|| !reader.ReadU16(m_macOnlyPart.m_unknown2)
				|| !reader.ReadU32(m_macOnlyPart.m_cursorIndex))
				return false;
		}
		else
			m_hasMacOnlyPart = false;

		return true;
	}

	DataObjectType DOGraphicModifier::GetType() const
	{
		return DataObjectType::kGraphicModifier;
	}

	bool DOGraphicModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e9)
			return false;

		if (!m_modHeader.Load(reader)
			|| !reader.ReadU16(m_unknown1)
			|| !m_applyWhen.Load(reader)
			|| !m_removeWhen.Load(reader)
			|| !reader.ReadBytes(m_unknown2, 2)
			|| !reader.ReadU16(m_inkMode)
			|| !reader.ReadU16(m_shape))
			return false;

		if (sp.m_systemType == mtdisasm::SystemType::kMac)
		{
			m_haveMacPart = true;
			if (!reader.ReadBytes(m_platform.m_mac.m_unknown4_1, 6)
				|| !m_backColor.Load(reader, sp)
				|| !m_foreColor.Load(reader, sp)
				|| !reader.ReadU16(m_borderSize)
				|| !m_borderColor.Load(reader, sp)
				|| !reader.ReadU16(m_shadowSize)
				|| !m_shadowColor.Load(reader, sp)
				|| !reader.ReadBytes(m_platform.m_mac.m_unknown4_2, 26))
				return false;
		}
		else
			m_haveMacPart = false;

		if (sp.m_systemType == mtdisasm::SystemType::kWindows)
		{
			m_haveWinPart = true;
			if (!reader.ReadBytes(m_platform.m_win.m_unknown5_1, 4)
				|| !m_backColor.Load(reader, sp)
				|| !m_foreColor.Load(reader, sp)
				|| !reader.ReadU16(m_borderSize)
				|| !m_borderColor.Load(reader, sp)
				|| !reader.ReadU16(m_shadowSize)
				|| !m_shadowColor.Load(reader, sp)
				|| !reader.ReadBytes(m_platform.m_win.m_unknown5_2, 22))
				return false;
		}
		else
			m_haveWinPart = false;

		if (!reader.ReadU16(m_numPolygonPoints)
			|| !reader.ReadBytes(m_unknown6, 8))
			return false;

		m_polyPoints.resize(m_numPolygonPoints);
		for (size_t i = 0; i < m_numPolygonPoints; i++)
		{
			if (!m_polyPoints[i].Load(reader, sp))
				return false;
		}

		return true;
	}

	DataObjectType DOTextStyleModifier::GetType() const
	{
		return DataObjectType::kTextStyleModifier;
	}

	bool DOTextStyleModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader))
			return false;

		if (!reader.ReadBytes(m_unknown1, 4)
			|| !reader.ReadU16(m_macFontID)
			|| !reader.ReadU8(m_flags)
			|| !reader.ReadU8(m_unknown2)
			|| !reader.ReadU16(m_size)
			|| !m_textColor.Load(reader, sp)
			|| !m_backgroundColor.Load(reader, sp)
			|| !reader.ReadU16(m_alignment)
			|| !reader.ReadU16(m_unknown3)
			|| !m_applyWhen.Load(reader)
			|| !m_removeWhen.Load(reader)
			|| !reader.ReadU16(m_lengthOfFontName))
			return false;

		if (m_lengthOfFontName > 0)
		{
			m_fontName.resize(m_lengthOfFontName + 1);
			if (!reader.ReadBytes(&m_fontName[0], m_lengthOfFontName))
				return false;

			m_fontName[m_lengthOfFontName] = 0;
		}

		return true;
	}

	DataObjectType DODragMotionModifier::GetType() const
	{
		return DataObjectType::kDragMotionModifier;
	}

	bool DODragMotionModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e8)
			return false;

		if (!m_modHeader.Load(reader))
			return false;

		if (!m_enableWhen.Load(reader)
			|| !m_disableWhen.Load(reader))
			return false;

		if (sp.m_systemType == mtdisasm::SystemType::kMac)
		{
			if (!reader.ReadU8(m_platform.m_mac.m_flags)
				|| !reader.ReadU8(m_platform.m_mac.m_unknown3))
				return false;

			m_haveMacPart = true;
		}
		else
			m_haveMacPart = false;

		if (sp.m_systemType == mtdisasm::SystemType::kWindows)
		{
			if (!reader.ReadU8(m_platform.m_win.m_unknown2)
				|| !reader.ReadU8(m_platform.m_win.m_constrainHorizontal)
				|| !reader.ReadU8(m_platform.m_win.m_constrainVertical)
				|| !reader.ReadU8(m_platform.m_win.m_constrainToParent))
				return false;

			m_haveWinPart = true;
		}
		else
			m_haveWinPart = false;

		if (!m_constraintMargin.Load(reader, sp)
			|| !reader.ReadU16(m_unknown1))
			return false;

		return true;
	}

	DataObjectType DOVectorMotionModifier::GetType() const
	{
		return DataObjectType::kVectorMotionModifier;
	}

	bool DOVectorMotionModifier::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0x3e9)
			return false;

		if (!m_modHeader.Load(reader))
			return false;

		if (!m_enableWhen.Load(reader)
			|| !m_disableWhen.Load(reader)
			|| !m_varSource.Load(reader)
			|| !reader.ReadU16(m_unknown1)
			|| !reader.ReadU8(m_varSourceNameLength)
			|| !reader.ReadU8(m_unknown2))
			return false;

		if (m_varSourceNameLength > 0)
		{
			m_varSourceName.resize(m_varSourceNameLength + 1);
			if (!reader.ReadBytes(&m_varSourceName[0], m_varSourceNameLength))
				return false;
			m_varSourceName[m_varSourceNameLength] = 0;
		}

		return true;
	}

	DataObjectType DOAudioAsset::GetType() const
	{
		return DataObjectType::kAudioAsset;
	}

	bool DOAudioAsset::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 2)
			return false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadU32(m_assetAndDataCombinedSize)
			|| !reader.ReadBytes(m_unknown2, 4)
			|| !reader.ReadU32(m_assetID)
			|| !reader.ReadBytes(m_unknown3, 20))
			return false;

		m_haveMacPart = false;
		m_haveWinPart = false;

		uint32_t extraDataSize = 0;
		if (sp.m_systemType == SystemType::kMac)
		{
			m_haveMacPart = true;

			if (!reader.ReadBytes(m_macPart.m_unknown4, 4)
				|| !reader.ReadU16(m_sampleRate1)
				|| !reader.ReadBytes(m_macPart.m_unknown5, 5)
				|| !reader.ReadU8(m_bitsPerSample)
				|| !reader.ReadU8(m_encoding1)
				|| !reader.ReadU8(m_channels)
				|| !reader.ReadBytes(m_codedDuration, 4)
				|| !reader.ReadBytes(m_macPart.m_unknown8, 20)
				|| !reader.ReadU16(m_sampleRate2))
				return false;
		}
		else if (sp.m_systemType == SystemType::kWindows)
		{
			m_haveWinPart = true;

			if (!reader.ReadU16(m_sampleRate1)
				|| !reader.ReadU8(m_bitsPerSample)
				|| !reader.ReadBytes(m_winPart.m_unknown9, 3)
				|| !reader.ReadU8(m_encoding1)
				|| !reader.ReadU8(m_channels)
				|| !reader.ReadBytes(m_codedDuration, 4)
				|| !reader.ReadBytes(m_winPart.m_unknown11, 18)
				|| !reader.ReadU16(m_sampleRate2)
				|| !reader.ReadBytes(m_winPart.m_unknown12_1, 2))
				return false;
		}
		else
			return false;

		if (!reader.ReadU32(m_cuePointDataSize)
			|| !reader.ReadU16(m_numCuePoints)
			|| !reader.ReadBytes(m_unknown14, 4)
			|| !reader.ReadU32(m_filePosition)
			|| !reader.ReadU32(m_size))
			return false;

		if (m_numCuePoints * 14 != m_cuePointDataSize)
			return false;

		m_cuePoints.resize(m_numCuePoints);
		for (size_t i = 0; i < m_numCuePoints; i++)
		{
			CuePoint& cuePoint = m_cuePoints[i];
			if (!reader.ReadBytes(cuePoint.m_unknown13, 2)
				|| !reader.ReadU32(cuePoint.m_unknown14)
				|| !reader.ReadU32(cuePoint.m_position)
				|| !reader.ReadU32(cuePoint.m_cuePointID))
				return false;
		}

		return true;
	}

	DataObjectType DOImageAsset::GetType() const
	{
		return DataObjectType::kImageAsset;
	}

	bool DOImageAsset::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 1)
			return false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadU32(m_unknown1)
			|| !reader.ReadBytes(m_unknown2, 4)
			|| !reader.ReadU32(m_assetID)
			|| !reader.ReadU32(m_unknown3))
			return false;

		m_haveWinPart = false;
		m_haveMacPart = false;

		if (sp.m_systemType == SystemType::kMac)
		{
			m_haveMacPart = true;
			if (!reader.ReadBytes(m_platform.m_mac.m_unknown7, 44))
				return false;
		}
		else if (sp.m_systemType == SystemType::kWindows)
		{
			m_haveWinPart = true;
			if (!reader.ReadBytes(m_platform.m_win.m_unknown8, 10))
				return false;
		}
		else
			return false;

		if (!m_rect1.Load(reader, sp)
			|| !reader.ReadU32(m_hdpiFixed)
			|| !reader.ReadU32(m_vdpiFixed)
			|| !reader.ReadU16(m_bitsPerPixel)
			|| !reader.ReadBytes(m_unknown4, 2)
			|| !reader.ReadBytes(m_unknown5, 4)
			|| !reader.ReadBytes(m_unknown6, 8)
			|| !m_rect2.Load(reader, sp)
			|| !reader.ReadU32(m_filePosition)
			|| !reader.ReadU32(m_size))
			return false;

		return true;
	}

	DataObjectType DOAssetDataSection::GetType() const
	{
		return DataObjectType::kAssetDataSection;
	}

	bool DOAssetDataSection::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0)
			return false;

		if (!reader.ReadU32(m_unknown1)
			|| !reader.ReadU32(m_sizeIncludingTag))
			return false;

		if (m_sizeIncludingTag < 14)
			return false;

		if (!reader.Skip(m_sizeIncludingTag - 14))
			return false;

		return true;
	}

	DataObjectType DOMovieAsset::GetType() const
	{
		return DataObjectType::kMovieAsset;
	}

	bool DOMovieAsset::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 0)
			return false;

		m_haveMacPart = false;
		m_haveWinPart = false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadU32(m_assetAndDataCombinedSize)
			|| !reader.ReadBytes(m_unknown1, 4)
			|| !reader.ReadU32(m_assetID)
			|| !reader.ReadBytes(m_unknown1_1, 4)
			|| !reader.ReadU16(m_extFileNameLength))
			return false;

		if (sp.m_systemType == SystemType::kMac)
		{
			m_haveMacPart = true;

			if (!reader.ReadBytes(m_macPart.m_unknown5_1, 66)
				|| !reader.ReadU32(m_movieDataSize)
				|| !reader.ReadBytes(m_macPart.m_unknown6, 12)
				|| !reader.ReadU32(m_moovAtomPos))
				return false;
		}
		else if (sp.m_systemType == SystemType::kWindows)
		{
			m_haveWinPart = true;

			if (!reader.ReadBytes(m_winPart.m_unknown3_1, 32)
				|| !reader.ReadU32(m_movieDataSize)
				|| !reader.ReadBytes(m_winPart.m_unknown4, 12)
				|| !reader.ReadU32(m_moovAtomPos)
				|| !reader.ReadBytes(m_winPart.m_unknown7, 12))
				return false;
		}
		else
			return false;

		if (m_extFileNameLength > 0)
		{
			m_extFileName.resize(m_extFileNameLength);
			if (!reader.ReadBytes(&m_extFileName[0], m_extFileNameLength) || m_extFileName[m_extFileNameLength - 1] != 0)
				return false;
		}

		m_movieDataPos = reader.TellGlobal();

		if (!reader.Skip(m_movieDataSize))
			return false;

		return true;
	}


	DataObjectType DOMToonAsset::GetType() const
	{
		return DataObjectType::kMToonAsset;
	}

	bool DOMToonAsset::Load(DataReader& reader, uint16_t revision, const SerializationProperties& sp)
	{
		if (revision != 1)
			return false;

		if (!reader.ReadU32(m_marker)
			|| !reader.ReadBytes(m_unknown1, 8)
			|| !reader.ReadU32(m_assetID))
			return false;

		m_haveMacPart = false;
		m_haveWinPart = false;

		if (sp.m_systemType == SystemType::kMac)
		{
			m_haveMacPart = true;

			if (!reader.ReadBytes(m_platform.m_mac.m_unknown10, 88))
				return false;
		}
		else if (sp.m_systemType == SystemType::kWindows)
		{
			m_haveWinPart = true;

			if (!reader.ReadBytes(m_platform.m_win.m_unknown11, 54))
				return false;
		}
		else
			return false;

		if (!reader.ReadU32(m_frameDataPosition)
			|| !reader.ReadU32(m_sizeOfFrameData)
			|| !reader.ReadU32(m_mtoonHeader[0])
			|| !reader.ReadU32(m_mtoonHeader[1])
			|| !reader.ReadU16(m_version)
			|| !reader.ReadBytes(m_unknown2, 4)
			|| !reader.ReadU32(m_encodingFlags)
			|| !m_rect.Load(reader, sp)
			|| !reader.ReadU16(m_numFrames)
			|| !reader.ReadBytes(m_unknown3, 14)
			|| !reader.ReadU16(m_bitsPerPixel)
			|| !reader.ReadU32(m_codecID)
			|| !reader.ReadBytes(m_unknown4_1, 8)
			|| !reader.ReadU32(m_codecDataSize)
			|| !reader.ReadBytes(m_unknown4_2, 4)
			)
			return false;

		if (m_mtoonHeader[0] != 0 || m_mtoonHeader[1] != 0x546f6f6e)
			return false;

		if (m_numFrames > 0)
		{
			m_frames.resize(m_numFrames);
			for (size_t i = 0; i < m_numFrames; i++)
			{
				FrameDef& frame = m_frames[i];

				if (!reader.ReadBytes(frame.m_unknown12, 4)
					|| !frame.m_rect1.Load(reader, sp)
					|| !reader.ReadU32(frame.m_dataOffset)
					|| !reader.ReadBytes(frame.m_unknown13, 2)
					|| !reader.ReadU32(frame.m_compressedSize)
					|| !reader.ReadU8(frame.m_unknown14)
					|| !reader.ReadU8(frame.m_keyframeFlag)
					|| !reader.ReadU8(frame.m_platformBit)
					|| !reader.ReadU8(frame.m_unknown15)
					|| !frame.m_rect2.Load(reader, sp)
					|| !reader.ReadU32(frame.m_hdpiFixed)
					|| !reader.ReadU32(frame.m_vdpiFixed)
					|| !reader.ReadU16(frame.m_bitsPerPixel)
					|| !reader.ReadU32(frame.m_unknown16)
					|| !reader.ReadU16(frame.m_decompressedBytesPerRow))
					return false;

				if (sp.m_systemType == SystemType::kMac)
				{
					if (!reader.ReadBytes(frame.m_platform.m_mac.m_unknown17, 4))
						return false;
				}
				else if (sp.m_systemType == SystemType::kWindows)
				{
					if (!reader.ReadBytes(frame.m_platform.m_win.m_unknown18, 2))
						return false;
				}
				else
					return false;

				if (!reader.ReadU32(frame.m_decompressedSize))
					return false;
			}
		}

		if (m_codecDataSize > 0)
		{
			m_codecData.resize(m_codecDataSize);
			if (!reader.ReadBytes(&m_codecData[0], m_codecDataSize))
				return false;
		}

		if (m_encodingFlags & kEncodingFlag_HasRanges)
		{
			if (!reader.ReadU32(m_frameRangesPart.m_tag)
				|| !reader.ReadU32(m_frameRangesPart.m_sizeIncludingTag)
				|| !reader.ReadU32(m_frameRangesPart.m_numFrameRanges))
				return false;

			if (m_frameRangesPart.m_tag != 1)
				return false;

			if (m_frameRangesPart.m_numFrameRanges > 0)
			{
				m_frameRangesPart.m_frameRanges.resize(m_frameRangesPart.m_numFrameRanges);
				for (size_t i = 0; i < m_frameRangesPart.m_numFrameRanges; i++)
				{
					FrameRangeDef& frameRange = m_frameRangesPart.m_frameRanges[i];

					if (!reader.ReadU32(frameRange.m_startFrame)
						|| !reader.ReadU32(frameRange.m_endFrame)
						|| !reader.ReadU8(frameRange.m_lengthOfName)
						|| !reader.ReadU8(frameRange.m_unknown14))
						return false;

					if (frameRange.m_lengthOfName > 0)
					{
						frameRange.m_name.resize(frameRange.m_lengthOfName);
						if (!reader.ReadBytes(&frameRange.m_name[0], frameRange.m_lengthOfName))
							return false;

						if (frameRange.m_name[frameRange.m_lengthOfName - 1] != 0)
							return false;
					}
				}
			}
		}

		return true;
	}
}
