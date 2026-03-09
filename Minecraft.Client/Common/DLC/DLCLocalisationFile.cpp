#include "stdafx.h"
#include "DLCManager.h"
#include "DLCLocalisationFile.h"
#include "..\..\StringTable.h"

DLCLocalisationFile::DLCLocalisationFile(const wstring &path) : DLCFile(DLCManager::e_DLCType_LocalisationData,path)
{	
	m_strings = nullptr;
}

void DLCLocalisationFile::addData(PBYTE pbData, DWORD dwBytes)
{
	m_strings = new StringTable(pbData, dwBytes);
}