#pragma once

#include "AbstractTexturePack.h"

class DLCPack;
class StringTable;

class DLCTexturePack : public AbstractTexturePack
{
private:
	DLCPack *m_dlcInfoPack; // Description, icon etc
	DLCPack *m_dlcDataPack; // Actual textures
	StringTable *m_stringTable;
	bool bUILoaded;
	bool m_bLoadingData, m_bHasLoadedData;
	bool m_bUsingDefaultColourTable;
	//bool m_bHasAudio;
	ArchiveFile *m_archiveFile;



public:
	using AbstractTexturePack::getResource;

	DLCTexturePack(DWORD id, DLCPack *pack, TexturePack *fallback);
	~DLCTexturePack();

	virtual wstring getResource(const wstring& name);
	virtual DLCPack * getDLCPack();
	virtual wstring getDesc1() {return m_stringTable->getString(L"IDS_TP_DESCRIPTION");}
	virtual wstring getName()	{return m_stringTable->getString(L"IDS_DISPLAY_NAME");}
	virtual wstring getWorldName() { return m_stringTable->getString(L"IDS_WORLD_NAME");}

	// Added for sound banks with MashUp packs
#ifdef _XBOX
	IXACT3WaveBank	*m_pStreamedWaveBank;
	IXACT3SoundBank	*m_pSoundBank;
#endif
protected:
	//@Override
	void loadIcon();
	void loadComparison();
	void loadName();
	void loadDescription();
	InputStream *getResourceImplementation(const wstring &name); //throws IOException

public:
	//@Override
	bool hasFile(const wstring &name);
	bool isTerrainUpdateCompatible();

	// 4J Added
	virtual wstring getPath(bool bTitleUpdateTexture = false, const char *pchBDPatchFilename=nullptr);
	virtual wstring getAnimationString(const wstring &textureName, const wstring &path);
	virtual BufferedImage *getImageResource(const wstring& File, bool filenameHasExtension = false, bool bTitleUpdateTexture=false, const wstring &drive =L"");
	virtual void loadColourTable();
	virtual bool hasData() { return m_bHasLoadedData; }
	virtual bool isLoadingData() { return m_bLoadingData; }

private:
	static wstring getRootPath(DWORD packId, bool allowOverride, bool bAddDataFolder);
	static wstring getFilePath(DWORD packId, wstring filename, bool bAddDataFolder=true);

public:
	static int packMounted(LPVOID pParam,int iPad,DWORD dwErr,DWORD dwLicenceMask);
	virtual void loadData();
	virtual void loadUI();
	virtual void unloadUI();
	virtual wstring getXuiRootPath();
	virtual ArchiveFile *getArchiveFile() { return m_archiveFile; }

	virtual unsigned int getDLCParentPackId();
	virtual DLCPack *getDLCInfoParentPack();
	virtual unsigned char getDLCSubPackId();
	XCONTENTDEVICEID GetDLCDeviceID();
};
