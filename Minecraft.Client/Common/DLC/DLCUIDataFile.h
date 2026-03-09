#pragma once
#include "DLCFile.h"

class DLCUIDataFile : public DLCFile
{
private:
	PBYTE m_pbData;
	DWORD m_dwBytes;
	bool m_canDeleteData;

public:
	DLCUIDataFile(const wstring &path);
	~DLCUIDataFile() override;

	using DLCFile::addData;
	using DLCFile::addParameter;

	virtual void addData(PBYTE pbData, DWORD dwBytes,bool canDeleteData = false);
    PBYTE getData(DWORD &dwBytes) override;
};
