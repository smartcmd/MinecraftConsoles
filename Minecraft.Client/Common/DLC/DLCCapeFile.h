#pragma once
#include "DLCFile.h"

class DLCCapeFile : public DLCFile
{
public:
	DLCCapeFile(const wstring &path);

    void addData(PBYTE pbData, DWORD dwBytes) override;
};