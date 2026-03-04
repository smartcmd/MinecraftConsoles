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
	virtual bool requiresTexturePack() {return m_bRequiresTexturePack;}
	virtual UINT getRequiredTexturePackId() {return m_requiredTexturePackId;}
	virtual wstring getDefaultSaveName() {return m_defaultSaveName;}
	virtual LPCWSTR getWorldName() {return m_worldName.c_str();}
	virtual LPCWSTR getDisplayName() {return m_displayName.c_str();}
	virtual wstring getGrfPath() {return L"GameRules.grf";}

	virtual void setRequiresTexturePack(bool x) {m_bRequiresTexturePack = x;}
	virtual void setRequiredTexturePackId(UINT x) {m_requiredTexturePackId = x;}
	virtual void setDefaultSaveName(const wstring &x) {m_defaultSaveName = x;}
	virtual void setWorldName(const wstring & x) {m_worldName = x;}
	virtual void setDisplayName(const wstring & x) {m_displayName = x;}
	virtual void setGrfPath(const wstring & x) {m_grfPath = x;}

	LevelGenerationOptions *lgo;

public:
	DLCGameRulesHeader(const wstring &path);

	virtual void addData(PBYTE pbData, DWORD dwBytes);
	virtual PBYTE getData(DWORD &dwBytes);

	void setGrfData(PBYTE fData, DWORD fSize, StringTable *);

	virtual bool ready() { return m_hasData; }
};