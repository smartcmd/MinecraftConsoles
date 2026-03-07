#include "stdafx.h"
#include "DLCPack.h"
#include "DLCSkinFile.h"
#include "DLCCapeFile.h"
#include "DLCTextureFile.h"
#include "DLCUIDataFile.h"
#include "DLCLocalisationFile.h"
#include "DLCGameRulesFile.h"
#include "DLCGameRulesHeader.h"
#include "DLCAudioFile.h"
#include "DLCColourTableFile.h"
#include "../../../Minecraft.World/StringHelpers.h"

DLCPack::DLCPack(const wstring &name,DWORD dwLicenseMask)
{
	m_dataPath = L"";
	m_packName = name;
	m_dwLicenseMask=dwLicenseMask;
#ifdef _XBOX_ONE
	m_wsProductId = L"";
#else
	m_ullFullOfferId = 0LL;
#endif
	m_isCorrupt = false;
	m_packId = 0;
	m_packVersion = 0;
	m_parentPack = NULL;
	m_dlcMountIndex = -1;
#ifdef _XBOX
	m_dlcDeviceID = XCONTENTDEVICE_ANY;
#endif
	
	// This pointer is for all the data used for this pack, so deleting it invalidates ALL of it's children.
	m_data = NULL;
}

#ifdef _XBOX_ONE
DLCPack::DLCPack(const wstring &name,const wstring &productID,DWORD dwLicenseMask)
{
	m_dataPath = L"";
	m_packName = name;
	m_dwLicenseMask=dwLicenseMask;
	m_wsProductId = productID;
	m_isCorrupt = false;
	m_packId = 0;
	m_packVersion = 0;
	m_parentPack = NULL;
	m_dlcMountIndex = -1;

	// This pointer is for all the data used for this pack, so deleting it invalidates ALL of it's children.
	m_data = NULL;
}
#endif

DLCPack::~DLCPack()
{
	for( auto& it : m_childPacks )
	{
		if ( it )
			delete it;
	}

	for(unsigned int i = 0; i < DLCManager::e_DLCType_Max; ++i)
	{
		for (auto& it : m_files[i] )
		{
			if ( it )
				delete it;
		}
	}

	// This pointer is for all the data used for this pack, so deleting it invalidates ALL of it's children.
	if(m_data)
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"Deleting data for DLC pack %ls\n", m_packName.c_str());
#endif
		// For the same reason, don't delete data pointer for any child pack as it just points to a region within the parent pack that has already been freed
		if( m_parentPack == NULL )
		{
			delete [] m_data;
		}
	}
}

DWORD DLCPack::GetDLCMountIndex()
{
	if(m_parentPack != NULL)
	{
		return m_parentPack->GetDLCMountIndex();
	}
	return m_dlcMountIndex;
}

XCONTENTDEVICEID DLCPack::GetDLCDeviceID()
{
	if(m_parentPack != NULL )
	{
		return m_parentPack->GetDLCDeviceID();
	}
	return m_dlcDeviceID;
}

void DLCPack::addChildPack(DLCPack *childPack)
{
	int packId = childPack->GetPackId();
#ifndef _CONTENT_PACKAGE
	if(packId < 0 || packId > 15)
	{
		__debugbreak();
	}
#endif
	childPack->SetPackId( (packId<<24) | m_packId );
	m_childPacks.push_back(childPack);
	childPack->setParentPack(this);
	childPack->m_packName = m_packName + childPack->getName();
}

void DLCPack::setParentPack(DLCPack *parentPack)
{
	m_parentPack = parentPack;
}

void DLCPack::addParameter(DLCManager::EDLCParameterType type, const wstring &value)
{
	switch(type)
	{
	case DLCManager::e_DLCParamType_PackId:
		{
			DWORD packId = 0;

			std::wstringstream ss;
			// 4J Stu - numbered using decimal to make it easier for artists/people to number manually
			ss << std::dec << value.c_str();
			ss >> packId;

			SetPackId(packId);
		}
		break;
	case DLCManager::e_DLCParamType_PackVersion:
		{
			DWORD version = 0;

			std::wstringstream ss;
			// 4J Stu - numbered using decimal to make it easier for artists/people to number manually
			ss << std::dec << value.c_str();
			ss >> version;

			SetPackVersion(version);
		}
		break;
	case DLCManager::e_DLCParamType_DisplayName:
		m_packName = value;
		break;
	case DLCManager::e_DLCParamType_DataPath:
		m_dataPath = value;
		break;
	default:
		m_parameters[(int)type] = value;
		break;
	}
}

bool DLCPack::getParameterAsUInt(DLCManager::EDLCParameterType type, unsigned int &param)
{
	auto it = m_parameters.find((int)type);
	if(it != m_parameters.end())
	{
		switch(type)
		{
		case DLCManager::e_DLCParamType_NetherParticleColour:
		case DLCManager::e_DLCParamType_EnchantmentTextColour:
		case DLCManager::e_DLCParamType_EnchantmentTextFocusColour:
			{
				std::wstringstream ss;
				ss << std::hex << it->second.c_str();
				ss >> param;
			}
			break;
		default:
			param = _fromString<unsigned int>(it->second);
		}
		return true;
	}
	return false;
}

DLCFile *DLCPack::addFile(DLCManager::EDLCType type, const wstring &path)
{
	DLCFile *newFile = NULL;

	switch(type)
	{
	case DLCManager::e_DLCType_Skin:
		{
			wstring newPath = replaceAll(path, L"\\", L"/");
			std::vector<std::wstring> splitPath = stringSplit(newPath,L'/');
			wstring strippedPath = splitPath.back();

			newFile = new DLCSkinFile(strippedPath);

			// check to see if we can get the full offer id using this skin name
#ifdef _XBOX_ONE
			app.GetDLCFullOfferIDForSkinID(strippedPath,m_wsProductId);
#else
			ULONGLONG ullVal=0LL;

			if(app.GetDLCFullOfferIDForSkinID(strippedPath,&ullVal))
			{
				m_ullFullOfferId=ullVal;
			}
#endif
		}
		break;
	case DLCManager::e_DLCType_Cape:
		{
			wstring newPath = replaceAll(path, L"\\", L"/");
			std::vector<std::wstring> splitPath = stringSplit(newPath,L'/');
			wstring strippedPath = splitPath.back();
			newFile = new DLCCapeFile(strippedPath);
		}
		break;
	case DLCManager::e_DLCType_Texture:
		newFile = new DLCTextureFile(path);
		break;
	case DLCManager::e_DLCType_UIData:
		newFile = new DLCUIDataFile(path);
		break;
	case DLCManager::e_DLCType_LocalisationData:
		newFile = new DLCLocalisationFile(path);
		break;
	case DLCManager::e_DLCType_GameRules:
		newFile = new DLCGameRulesFile(path);
		break;
	case DLCManager::e_DLCType_Audio:
		newFile = new DLCAudioFile(path);
		break;
	case DLCManager::e_DLCType_ColourTable:
		newFile = new DLCColourTableFile(path);
		break;
	case DLCManager::e_DLCType_GameRulesHeader:
		newFile = new DLCGameRulesHeader(path);
		break;
	};

	if( newFile != NULL )
	{
		m_files[newFile->getType()].push_back(newFile);
	}

	return newFile;
}

// MGH - added this comp func, as the embedded func in find_if was confusing the PS3 compiler
static const wstring *g_pathCmpString = NULL;
static bool pathCmp(DLCFile *val)
{
	return (g_pathCmpString->compare(val->getPath()) == 0); 
}

bool DLCPack::doesPackContainFile(DLCManager::EDLCType type, const wstring &path)
{
	bool hasFile = false;
	if(type == DLCManager::e_DLCType_All)
	{
		for(DLCManager::EDLCType currentType = (DLCManager::EDLCType)0; currentType < DLCManager::e_DLCType_Max; currentType = (DLCManager::EDLCType)(currentType + 1))
		{
			hasFile = doesPackContainFile(currentType,path);
			if(hasFile) break;
		}
	}
	else
	{
		g_pathCmpString = &path;
		auto it = find_if(m_files[type].begin(), m_files[type].end(), pathCmp);
		hasFile = it != m_files[type].end();
		if(!hasFile && m_parentPack )
		{
			hasFile = m_parentPack->doesPackContainFile(type,path);
		}
	}
	return hasFile;
}

DLCFile *DLCPack::getFile(DLCManager::EDLCType type, DWORD index)
{
	DLCFile *file = NULL;
	if(type == DLCManager::e_DLCType_All)
	{
		for(DLCManager::EDLCType currentType = (DLCManager::EDLCType)0; currentType < DLCManager::e_DLCType_Max; currentType = (DLCManager::EDLCType)(currentType + 1))
		{
			file = getFile(currentType,index);
			if(file != NULL) break;
		}
	}
	else
	{
		if(m_files[type].size() > index) file = m_files[type][index];
		if(!file && m_parentPack)
		{
			file = m_parentPack->getFile(type,index);
		}
	}
	return file;
}

DLCFile *DLCPack::getFile(DLCManager::EDLCType type, const wstring &path)
{
	DLCFile *file = NULL;
	if(type == DLCManager::e_DLCType_All)
	{
		for(DLCManager::EDLCType currentType = (DLCManager::EDLCType)0; currentType < DLCManager::e_DLCType_Max; currentType = (DLCManager::EDLCType)(currentType + 1))
		{
			file = getFile(currentType,path);
			if(file != NULL) break;
		}
	}
	else
	{
		g_pathCmpString = &path;
		auto it = find_if(m_files[type].begin(), m_files[type].end(), pathCmp);

		if(it == m_files[type].end())
		{
			// Not found
			file = NULL; 
		}
		else
		{
			file = *it;
		}
		if(!file && m_parentPack)
		{
			file = m_parentPack->getFile(type,path);
		}
	}
	return file;
}

DWORD DLCPack::getDLCItemsCount(DLCManager::EDLCType type /*= DLCManager::e_DLCType_All*/)
{
	DWORD count = 0;

	switch(type)
	{
	case DLCManager::e_DLCType_All:
		for(int i = 0; i < DLCManager::e_DLCType_Max; ++i)
		{
			count += getDLCItemsCount((DLCManager::EDLCType)i);
		}
		break;
	default:
		count = (DWORD)m_files[(int)type].size();
		break;
	};
	return count;
};

DWORD DLCPack::getFileIndexAt(DLCManager::EDLCType type, const wstring &path, bool &found)
{
	if(type == DLCManager::e_DLCType_All)
	{
		app.DebugPrintf("Unimplemented\n");
#ifndef __CONTENT_PACKAGE
		__debugbreak();
#endif
		return 0;
	}

	DWORD foundIndex = 0;
	found = false;
	DWORD index = 0;
	for( auto& it : m_files[type] )
	{
		if(path.compare(it->getPath()) == 0)
		{
			foundIndex = index;
			found = true;
			break;
		}
		++index;
	}

	return foundIndex;
}

bool DLCPack::hasPurchasedFile(DLCManager::EDLCType type, const wstring &path)
{
	// Patch all DLC to be "purchased"
	return true;
	
	/*if(type == DLCManager::e_DLCType_All)
	{
		app.DebugPrintf("Unimplemented\n");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
		return false;
	}
#ifndef _CONTENT_PACKAGE
	if( app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_UnlockAllDLC) )
	{
		return true;
	}
	else
#endif
	if ( m_dwLicenseMask == 0 )
	{
		//not purchased.
		return false;
	}
	else
	{
		//purchased
		return true;
	}*/
}

void  DLCPack::UpdateLanguage()
{
	// find the language file
	DLCManager::e_DLCType_LocalisationData;
	DLCFile *file = NULL;

	if(m_files[DLCManager::e_DLCType_LocalisationData].size() > 0)
	{
		file = m_files[DLCManager::e_DLCType_LocalisationData][0];
		DLCLocalisationFile *localisationFile = (DLCLocalisationFile *)getFile(DLCManager::e_DLCType_LocalisationData, L"languages.loc");
		StringTable *strTable = localisationFile->getStringTable();
		strTable->ReloadStringTable();
	}

}