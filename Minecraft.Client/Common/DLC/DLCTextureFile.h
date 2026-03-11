#pragma once
#include "DLCFile.h"

class DLCTextureFile : public DLCFile
{

private:
	bool m_bIsAnim;
	wstring m_animString;

	PBYTE m_pbData;
	DWORD m_dwBytes;

public:
	DLCTextureFile(const wstring &path);

    void addData(PBYTE pbData, DWORD dwBytes) override;
    PBYTE getData(DWORD &dwBytes) override;

    void addParameter(DLCManager::EDLCParameterType type, const wstring &value) override;

    wstring getParameterAsString(DLCManager::EDLCParameterType type) override;
    bool getParameterAsBool(DLCManager::EDLCParameterType type) override;
};