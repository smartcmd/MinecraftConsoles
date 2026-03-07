#pragma once
using namespace std;
#include <vector>
class DLCPack;
class DLCSkinFile;

class DLCManager
{
public:
	enum EDLCType
	{
		e_DLCType_Skin = 0,
		e_DLCType_Cape,
		e_DLCType_Texture,
		e_DLCType_UIData,
		e_DLCType_PackConfig,
		e_DLCType_TexturePack,
		e_DLCType_LocalisationData,
		e_DLCType_GameRules,
		e_DLCType_Audio,
		e_DLCType_ColourTable,
		e_DLCType_GameRulesHeader,

		e_DLCType_Max,
		e_DLCType_All,
	};

	// If you add to the Enum,then you need to add the array of type names
	// These are the names used in the XML for the parameters
	enum EDLCParameterType
	{
		e_DLCParamType_Invalid = -1,

		e_DLCParamType_DisplayName = 0,
		e_DLCParamType_ThemeName,
		e_DLCParamType_Free, // identify free skins
		e_DLCParamType_Credit, // legal credits for DLC
		e_DLCParamType_Cape,
		e_DLCParamType_Box,
		e_DLCParamType_Anim,
		e_DLCParamType_PackId,
		e_DLCParamType_NetherParticleColour,
		e_DLCParamType_EnchantmentTextColour,
		e_DLCParamType_EnchantmentTextFocusColour,
		e_DLCParamType_DataPath,
		e_DLCParamType_PackVersion,

		e_DLCParamType_Max,

	};
	static const WCHAR *wchTypeNamesA[e_DLCParamType_Max];

private:
	vector<DLCPack *> m_packs;
	//bool m_bNeedsUpdated;
	bool m_bNeedsCorruptCheck;
	DWORD m_dwUnnamedCorruptDLCCount;
public:
	DLCManager();
	~DLCManager();

	static EDLCParameterType getParameterType(const wstring &paramName);

	DWORD getPackCount(EDLCType type = e_DLCType_All);

	//bool NeedsUpdated() { return m_bNeedsUpdated; }
	//void SetNeedsUpdated(bool val) { m_bNeedsUpdated = val; }

	bool NeedsCorruptCheck() { return m_bNeedsCorruptCheck; }
	void SetNeedsCorruptCheck(bool val) { m_bNeedsCorruptCheck = val; }

	void resetUnnamedCorruptCount() { m_dwUnnamedCorruptDLCCount = 0; }
	void incrementUnnamedCorruptCount() { ++m_dwUnnamedCorruptDLCCount; }

	void addPack(DLCPack *pack);
	void removePack(DLCPack *pack);
	void removeAllPacks(void);
	void LanguageChanged(void);

	DLCPack *getPack(const wstring &name);
#ifdef _XBOX_ONE
	DLCPack *DLCManager::getPackFromProductID(const wstring &productID);
#endif
	DLCPack *getPack(DWORD index, EDLCType type = e_DLCType_All);
	DWORD getPackIndex(DLCPack *pack, bool &found, EDLCType type = e_DLCType_All);
	DLCSkinFile *getSkinFile(const wstring &path); // Will hunt all packs of type skin to find the right skinfile

	DLCPack *getPackContainingSkin(const wstring &path);
	DWORD getPackIndexContainingSkin(const wstring &path, bool &found);

	DWORD checkForCorruptDLCAndAlert(bool showMessage = true);

	bool readDLCDataFile(DWORD &dwFilesProcessed, const wstring &path, DLCPack *pack, bool fromArchive = false);
	bool readDLCDataFile(DWORD &dwFilesProcessed, const string &path, DLCPack *pack, bool fromArchive = false);
	DWORD retrievePackIDFromDLCDataFile(const string &path, DLCPack *pack);

private:
	bool processDLCDataFile(DWORD &dwFilesProcessed, PBYTE pbData, DWORD dwLength, DLCPack *pack);

	DWORD retrievePackID(PBYTE pbData, DWORD dwLength, DLCPack *pack);
};
