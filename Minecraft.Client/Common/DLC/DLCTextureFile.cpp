#include "stdafx.h"
#include "DLCManager.h"
#include "DLCTextureFile.h"

DLCTextureFile::DLCTextureFile(const wstring &path) : DLCFile(DLCManager::e_DLCType_Texture,path)
{
	m_bIsAnim = false;
	m_animString = L"";
	
	m_pbData = nullptr;
	m_dwBytes = 0;
}

void DLCTextureFile::addData(PBYTE pbData, DWORD dwBytes)
{
	//app.AddMemoryTextureFile(m_path,pbData,dwBytes);
	m_pbData = pbData;
	m_dwBytes = dwBytes;
}

PBYTE DLCTextureFile::getData(DWORD &dwBytes)
{
	dwBytes = m_dwBytes;
	return m_pbData;
}

void DLCTextureFile::addParameter(DLCManager::EDLCParameterType type, const wstring &value)
{
	switch(type)
	{
	case DLCManager::e_DLCParamType_Anim:
		m_animString = value;
		if(m_animString.empty()) m_animString = L",";
		m_bIsAnim = true;

		break;
	}
}

wstring DLCTextureFile::getParameterAsString(DLCManager::EDLCParameterType type)
{
	switch(type)
	{
	case DLCManager::e_DLCParamType_Anim:
		return m_animString;
	default:
		return L"";
	}
}

bool DLCTextureFile::getParameterAsBool(DLCManager::EDLCParameterType type)
{
	switch(type)
	{
	case DLCManager::e_DLCParamType_Anim:
		return m_bIsAnim;
	default:
		return false;
	}
}