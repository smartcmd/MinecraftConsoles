#include "stdafx.h"
#include "DLCManager.h"
#include "DLCUIDataFile.h"

DLCUIDataFile::DLCUIDataFile(const wstring &path) : DLCFile(DLCManager::e_DLCType_UIData,path)
{	
	m_pbData = nullptr;
	m_dwBytes = 0;
	m_canDeleteData = false;
}

DLCUIDataFile::~DLCUIDataFile()
{
	if(m_canDeleteData && m_pbData != nullptr)
	{
		app.DebugPrintf("Deleting DLCUIDataFile data\n");
		delete [] m_pbData;
	}
}

void DLCUIDataFile::addData(PBYTE pbData, DWORD dwBytes,bool canDeleteData)
{
	m_pbData = pbData;
	m_dwBytes = dwBytes;
	m_canDeleteData = canDeleteData;
}

PBYTE DLCUIDataFile::getData(DWORD &dwBytes)
{
	dwBytes = m_dwBytes;
	return m_pbData;
}