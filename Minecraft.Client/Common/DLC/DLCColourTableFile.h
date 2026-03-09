#pragma once
#include "DLCFile.h"

class ColourTable;

class DLCColourTableFile : public DLCFile
{
private:
	ColourTable *m_colourTable;

public:
	DLCColourTableFile(const wstring &path);
	~DLCColourTableFile() override;

    void addData(PBYTE pbData, DWORD dwBytes) override;

	ColourTable *getColourTable() const { return m_colourTable; }
};