#pragma once
using namespace std;
#include "DLCManager.h"

class DLCFile;
class DLCSkinFile;

class DLCPack
{
private:
	vector<DLCFile *> m_files[DLCManager::e_DLCType_Max];
	vector<DLCPack *> m_childPacks;
	DLCPack *m_parentPack;

	unordered_map<int, wstring> m_parameters;

	wstring m_packName;
	wstring m_dataPath;
	DWORD m_dwLicenseMask;
	int m_dlcMountIndex;
	XCONTENTDEVICEID m_dlcDeviceID;
#ifdef _XBOX_ONE
	wstring m_wsProductId;
#else
	ULONGLONG m_ullFullOfferId;
#endif
	bool m_isCorrupt;
	DWORD m_packId;
	DWORD m_packVersion;

	PBYTE m_data; // This pointer is for all the data used for this pack, so deleting it invalidates ALL of it's children.
public:

	DLCPack(const wstring &name,DWORD dwLicenseMask);
#ifdef _XBOX_ONE
	DLCPack(const wstring &name,const wstring &productID,DWORD dwLicenseMask);
#endif
	~DLCPack();

	wstring getFullDataPath() { return m_dataPath; }

	void SetDataPointer(PBYTE pbData) { m_data = pbData; }

	bool IsCorrupt() { return m_isCorrupt; }
	void SetIsCorrupt(bool val) { m_isCorrupt = val; }

	void SetPackId(DWORD id) { m_packId = id; }
	DWORD GetPackId() { return m_packId; }

	void SetPackVersion(DWORD version) { m_packVersion = version; }
	DWORD GetPackVersion() { return m_packVersion; }

	DLCPack * GetParentPack() { return m_parentPack; }
	DWORD GetParentPackId() { return m_parentPack->m_packId; }

	void SetDLCMountIndex(DWORD id) { m_dlcMountIndex = id; }
	DWORD GetDLCMountIndex();
	void SetDLCDeviceID(XCONTENTDEVICEID deviceId) { m_dlcDeviceID = deviceId; }
	XCONTENTDEVICEID GetDLCDeviceID();

	void addChildPack(DLCPack *childPack);
	void setParentPack(DLCPack *parentPack);

	void addParameter(DLCManager::EDLCParameterType type, const wstring &value);
	bool getParameterAsUInt(DLCManager::EDLCParameterType type, unsigned int &param);

	void updateLicenseMask( DWORD dwLicenseMask ) { m_dwLicenseMask = dwLicenseMask; }
	DWORD getLicenseMask( ) { return m_dwLicenseMask; }
	
	wstring getName() { return m_packName; }

	void UpdateLanguage();
#ifdef _XBOX_ONE
	wstring getPurchaseOfferId() { return m_wsProductId; }
#else
	ULONGLONG getPurchaseOfferId() { return m_ullFullOfferId; }
#endif

	DLCFile *addFile(DLCManager::EDLCType type, const wstring &path);
	DLCFile *getFile(DLCManager::EDLCType type, DWORD index);
	DLCFile *getFile(DLCManager::EDLCType type, const wstring &path);

	DWORD getDLCItemsCount(DLCManager::EDLCType type = DLCManager::e_DLCType_All);	
	DWORD getFileIndexAt(DLCManager::EDLCType type, const wstring &path, bool &found);
	bool doesPackContainFile(DLCManager::EDLCType type, const wstring &path);
	DWORD GetPackID() {return m_packId;}
	
	DWORD getSkinCount() { return getDLCItemsCount(DLCManager::e_DLCType_Skin); }
	DWORD getSkinIndexAt(const wstring &path, bool &found) { return getFileIndexAt(DLCManager::e_DLCType_Skin, path, found); }
	DLCSkinFile *getSkinFile(const wstring &path) { return (DLCSkinFile *)getFile(DLCManager::e_DLCType_Skin, path); }
	DLCSkinFile *getSkinFile(DWORD index) { return (DLCSkinFile *)getFile(DLCManager::e_DLCType_Skin, index); }
	bool doesPackContainSkin(const wstring &path) { return doesPackContainFile(DLCManager::e_DLCType_Skin, path); }

	bool hasPurchasedFile(DLCManager::EDLCType type, const wstring &path);
};
