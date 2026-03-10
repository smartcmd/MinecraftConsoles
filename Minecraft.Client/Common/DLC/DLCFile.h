#pragma once
#include "DLCManager.h"

class DLCFile
{
protected:
	DLCManager::EDLCType m_type;
	wstring m_path;
	DWORD m_dwSkinId;

public:
	DLCFile(DLCManager::EDLCType type, const wstring &path);
	virtual ~DLCFile() {}

	DLCManager::EDLCType getType() const { return m_type; }
	wstring getPath()				{ return m_path; }
	DWORD getSkinID() const { return m_dwSkinId; }

	virtual void addData(PBYTE pbData, DWORD dwBytes) {}
	virtual PBYTE getData(DWORD &dwBytes) { dwBytes = 0; return nullptr; }
	virtual void addParameter(DLCManager::EDLCParameterType type, const wstring &value) {}

	virtual wstring getParameterAsString(DLCManager::EDLCParameterType type) { return L""; }
	virtual bool getParameterAsBool(DLCManager::EDLCParameterType type) { return false;}
};