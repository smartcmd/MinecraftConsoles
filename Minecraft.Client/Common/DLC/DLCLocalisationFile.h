#pragma once
#include "DLCFile.h"

class StringTable;

class DLCLocalisationFile : public DLCFile
{
private:
	StringTable *m_strings;

public:
	DLCLocalisationFile(const wstring &path);
	DLCLocalisationFile(PBYTE pbData, DWORD dwBytes); // when we load in a texture pack details file from TMS++

    void addData(PBYTE pbData, DWORD dwBytes) override;

	StringTable *getStringTable() { return m_strings; }
};