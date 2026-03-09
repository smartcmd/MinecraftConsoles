#pragma once
#include "DLCGameRules.h"

class DLCGameRulesFile : public DLCGameRules
{
private:
	PBYTE m_pbData;
	DWORD m_dwBytes;

public:
	DLCGameRulesFile(const wstring &path);

    void addData(PBYTE pbData, DWORD dwBytes) override;
    PBYTE getData(DWORD &dwBytes) override;
};