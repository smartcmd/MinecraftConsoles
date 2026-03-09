#include "stdafx.h"
#include "DLCManager.h"
#include "DLCGameRulesFile.h"

DLCGameRulesFile::DLCGameRulesFile(const wstring &path) : DLCGameRules(DLCManager::e_DLCType_GameRules,path)
{	
	m_pbData = nullptr;
	m_dwBytes = 0;
}

void DLCGameRulesFile::addData(PBYTE pbData, DWORD dwBytes)
{
	m_pbData = pbData;
	m_dwBytes = dwBytes;
}

PBYTE DLCGameRulesFile::getData(DWORD &dwBytes)
{
	dwBytes = m_dwBytes;
	return m_pbData;
}