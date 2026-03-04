#pragma once
#include "DLCFile.h"
#include "..\..\..\Minecraft.Client\HumanoidModel.h"

class DLCSkinFile : public DLCFile
{

private:
	wstring m_displayName;
	wstring m_themeName;
	wstring m_cape;
	unsigned int m_uiAnimOverrideBitmask;
	bool m_bIsFree;
	vector<SKIN_BOX *> m_AdditionalBoxes;

public:

	DLCSkinFile(const wstring &path);

	virtual void addData(PBYTE pbData, DWORD dwBytes);
	virtual void addParameter(DLCManager::EDLCParameterType type, const wstring &value);

	virtual wstring getParameterAsString(DLCManager::EDLCParameterType type);
	virtual bool getParameterAsBool(DLCManager::EDLCParameterType type);
	vector<SKIN_BOX *> *getAdditionalBoxes();
	int getAdditionalBoxesCount();
	unsigned int getAnimOverrideBitmask() { return m_uiAnimOverrideBitmask;}
	bool isFree() {return m_bIsFree;}
};