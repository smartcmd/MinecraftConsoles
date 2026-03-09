#pragma once

#include "DLCGameRules.h"
#include "..\GameRules\LevelGenerationOptions.h"

class DLCGameRulesHeader : public DLCGameRules, public JustGrSource
{
private:

	// GR-Header 
	PBYTE m_pbData;
	DWORD m_dwBytes;

	bool m_hasData;

public:
    bool requiresTexturePack() override
    {return m_bRequiresTexturePack;}

    UINT getRequiredTexturePackId() override
    {return m_requiredTexturePackId;}

    wstring getDefaultSaveName() override
    {return m_defaultSaveName;}

    LPCWSTR getWorldName() override
    {return m_worldName.c_str();}

    LPCWSTR getDisplayName() override
    {return m_displayName.c_str();}

    wstring getGrfPath() override
    {return L"GameRules.grf";}

    void setRequiresTexturePack(bool x) override
    {m_bRequiresTexturePack = x;}

    void setRequiredTexturePackId(UINT x) override
    {m_requiredTexturePackId = x;}

    void setDefaultSaveName(const wstring &x) override
    {m_defaultSaveName = x;}

    void setWorldName(const wstring & x) override
    {m_worldName = x;}

    void setDisplayName(const wstring & x) override
    {m_displayName = x;}

    void setGrfPath(const wstring & x) override
    {m_grfPath = x;}

	LevelGenerationOptions *lgo;

public:
	DLCGameRulesHeader(const wstring &path);

    void addData(PBYTE pbData, DWORD dwBytes) override;
    PBYTE getData(DWORD &dwBytes) override;

	void setGrfData(PBYTE fData, DWORD fSize, StringTable *);

    bool ready() override
    { return m_hasData; }
};