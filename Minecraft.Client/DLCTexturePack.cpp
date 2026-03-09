#include "stdafx.h"
#include "Common\DLC\DLCGameRulesFile.h"
#include "Common\DLC\DLCGameRulesHeader.h"
#include "Common\DLC\DLCGameRules.h"
#include "DLCTexturePack.h"
#include "Common\DLC\DLCColourTableFile.h"
#include "Common\DLC\DLCUIDataFile.h"
#include "Common\DLC\DLCTextureFile.h"
#include "Common\DLC\DLCLocalisationFile.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "StringTable.h"
#include "Common/UI/UI.h"
#include "Common\DLC\DLCAudioFile.h"

#if defined _XBOX || defined _WINDOWS64
#include "Xbox\XML\ATGXmlParser.h"
#include "Xbox\XML\xmlFilesCallback.h"
#endif

DLCTexturePack::DLCTexturePack(DWORD id, DLCPack *pack, TexturePack *fallback) : AbstractTexturePack(id, nullptr, pack->getName(), fallback)
{
	m_dlcInfoPack = pack;
	m_dlcDataPack = nullptr;
	bUILoaded = false;
	m_bLoadingData = false;
	m_bHasLoadedData = false;
	m_archiveFile = nullptr;
	if (app.getLevelGenerationOptions()) app.getLevelGenerationOptions()->setLoadedData();
	m_bUsingDefaultColourTable = true;

	m_stringTable = nullptr;

#ifdef _XBOX
	m_pStreamedWaveBank=nullptr;
	m_pSoundBank=nullptr;
#endif

	if(m_dlcInfoPack->doesPackContainFile(DLCManager::e_DLCType_LocalisationData, L"languages.loc"))
	{
		DLCLocalisationFile *localisationFile = static_cast<DLCLocalisationFile *>(m_dlcInfoPack->getFile(DLCManager::e_DLCType_LocalisationData, L"languages.loc"));
		m_stringTable = localisationFile->getStringTable();
	}

	// 4J Stu - These calls need to be in the most derived version of the class
	loadIcon();
	loadName();
	loadDescription();
	//loadDefaultHTMLColourTable();
}

void DLCTexturePack::loadIcon()
{
	if(m_dlcInfoPack->doesPackContainFile(DLCManager::e_DLCType_Texture, L"icon.png"))
	{
		DLCTextureFile *textureFile = static_cast<DLCTextureFile *>(m_dlcInfoPack->getFile(DLCManager::e_DLCType_Texture, L"icon.png"));
		m_iconData = textureFile->getData(m_iconSize);
	}
	else
	{
		AbstractTexturePack::loadIcon();
	}
}

void DLCTexturePack::loadComparison()
{
	if(m_dlcInfoPack->doesPackContainFile(DLCManager::e_DLCType_Texture, L"comparison.png"))
	{
		DLCTextureFile *textureFile = static_cast<DLCTextureFile *>(m_dlcInfoPack->getFile(DLCManager::e_DLCType_Texture, L"comparison.png"));
		m_comparisonData = textureFile->getData(m_comparisonSize);
	}
}

void DLCTexturePack::loadName()
{
	texname = L"";

	if(m_dlcInfoPack->GetPackID()&1024)
	{
		if(m_stringTable != nullptr)
		{
			texname = m_stringTable->getString(L"IDS_DISPLAY_NAME");
			m_wsWorldName=m_stringTable->getString(L"IDS_WORLD_NAME");
		}
	}
	else
	{	
		if(m_stringTable != nullptr)
		{
			texname = m_stringTable->getString(L"IDS_DISPLAY_NAME");
		}
	}

}

void DLCTexturePack::loadDescription()
{
	desc1 = L"";

	if(m_stringTable != nullptr)
	{
		desc1 = m_stringTable->getString(L"IDS_TP_DESCRIPTION");
	}
}

wstring DLCTexturePack::getResource(const wstring& name)
{
	// 4J Stu - We should never call this function
#ifndef __CONTENT_PACKAGE
	__debugbreak();
#endif
	return L"";
}

InputStream *DLCTexturePack::getResourceImplementation(const wstring &name) //throws IOException
{
	// 4J Stu - We should never call this function
#ifndef _CONTENT_PACKAGE
	__debugbreak();
	if(hasFile(name)) return nullptr;
#endif
	return nullptr; //resource;
}

bool DLCTexturePack::hasFile(const wstring &name)
{
	bool hasFile = false;
	if(m_dlcDataPack != nullptr) hasFile = m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_Texture, name);
	return hasFile;
}

bool DLCTexturePack::isTerrainUpdateCompatible()
{
	return true;
}

wstring DLCTexturePack::getPath(bool bTitleUpdateTexture /*= false*/, const char *pchBDPatchFilename)
{
	return L"";
}

wstring DLCTexturePack::getAnimationString(const wstring &textureName, const wstring &path)
{
	wstring result = L"";

	wstring fullpath = L"res/" + path + textureName + L".png"; 
	if(hasFile(fullpath))
	{
		result = m_dlcDataPack->getFile(DLCManager::e_DLCType_Texture, fullpath)->getParameterAsString(DLCManager::e_DLCParamType_Anim);
	}

	return result;
}

BufferedImage *DLCTexturePack::getImageResource(const wstring& File, bool filenameHasExtension /*= false*/, bool bTitleUpdateTexture /*=false*/, const wstring &drive /*=L""*/)
{
	if(m_dlcDataPack) return new BufferedImage(m_dlcDataPack, L"/" + File, filenameHasExtension);
	else return fallback->getImageResource(File, filenameHasExtension, bTitleUpdateTexture, drive);
}

DLCPack * DLCTexturePack::getDLCPack()
{
	return m_dlcDataPack;
}

void DLCTexturePack::loadColourTable()
{
	// Load the game colours
	if(m_dlcDataPack != nullptr && m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_ColourTable, L"colours.col"))
	{
		DLCColourTableFile *colourFile = static_cast<DLCColourTableFile *>(m_dlcDataPack->getFile(DLCManager::e_DLCType_ColourTable, L"colours.col"));
		m_colourTable = colourFile->getColourTable();
		m_bUsingDefaultColourTable = false;
	}
	else
	{
		// 4J Stu - We can delete the default colour table, but not the one from the DLCColourTableFile
		if(!m_bUsingDefaultColourTable) m_colourTable = nullptr;
		loadDefaultColourTable();
		m_bUsingDefaultColourTable = true;
	}

	// Load the text colours
#ifdef _XBOX
	if(m_dlcDataPack != nullptr && m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_UIData, L"TexturePack.xzp"))
	{
		DLCUIDataFile *dataFile = (DLCUIDataFile *)m_dlcDataPack->getFile(DLCManager::e_DLCType_UIData, L"TexturePack.xzp");

		DWORD dwSize = 0;
		PBYTE pbData = dataFile->getData(dwSize);

		const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
		WCHAR szResourceLocator[ LOCATOR_SIZE ];
		
		// Try and load the HTMLColours.col based off the common XML first, before the deprecated xuiscene_colourtable	
		swprintf(szResourceLocator, LOCATOR_SIZE,L"memory://%08X,%04X#HTMLColours.col",pbData, dwSize);
		BYTE *data;
		UINT dataLength;
		if(XuiResourceLoadAll(szResourceLocator, &data, &dataLength) == S_OK)
		{
			m_colourTable->loadColoursFromData(data,dataLength);

			XuiFree(data);
		}
		else
		{

		swprintf(szResourceLocator, LOCATOR_SIZE,L"memory://%08X,%04X#xuiscene_colourtable.xur",pbData, dwSize);
		HXUIOBJ hScene;
		HRESULT hr = XuiSceneCreate(szResourceLocator,szResourceLocator, nullptr, &hScene);

		if(HRESULT_SUCCEEDED(hr))
		{
			loadHTMLColourTableFromXuiScene(hScene);
		}
		else
		{			
			loadDefaultHTMLColourTable();
		}
	}
	}
	else
	{
		loadDefaultHTMLColourTable();
	}
#else
	if(app.hasArchiveFile(L"HTMLColours.col"))
	{
		byteArray textColours = app.getArchiveFile(L"HTMLColours.col");
		m_colourTable->loadColoursFromData(textColours.data,textColours.length);

		delete [] textColours.data;
	}
#endif
}

void DLCTexturePack::loadData()
{
	int mountIndex = m_dlcInfoPack->GetDLCMountIndex();

	if(mountIndex > -1)
	{
#ifdef _DURANGO
		if(StorageManager.MountInstalledDLC(ProfileManager.GetPrimaryPad(),mountIndex,&DLCTexturePack::packMounted,this,L"TPACK")!=ERROR_IO_PENDING)
#else
		if(StorageManager.MountInstalledDLC(ProfileManager.GetPrimaryPad(),mountIndex,&DLCTexturePack::packMounted,this,"TPACK")!=ERROR_IO_PENDING)
#endif
		{
			// corrupt DLC
			m_bHasLoadedData = true;
			if (app.getLevelGenerationOptions()) app.getLevelGenerationOptions()->setLoadedData();
			app.DebugPrintf("Failed to mount texture pack DLC %d for pad %d\n",mountIndex,ProfileManager.GetPrimaryPad());	
		}
		else
		{
			m_bLoadingData = true;
			app.DebugPrintf("Attempted to mount DLC data for texture pack %d\n", mountIndex);
		}
	}
	else
	{
		m_bHasLoadedData = true;
		if (app.getLevelGenerationOptions()) app.getLevelGenerationOptions()->setLoadedData();
		app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ReloadTexturePack);
	}
}





wstring DLCTexturePack::getFilePath(DWORD packId, wstring filename, bool bAddDataFolder)
{
	return app.getFilePath(packId,filename,bAddDataFolder);
}

int DLCTexturePack::packMounted(LPVOID pParam,int iPad,DWORD dwErr,DWORD dwLicenceMask)
{
	DLCTexturePack *texturePack = static_cast<DLCTexturePack *>(pParam);
	texturePack->m_bLoadingData = false;
	if(dwErr!=ERROR_SUCCESS)
	{
		// corrupt DLC
		app.DebugPrintf("Failed to mount DLC for pad %d: %d\n",iPad,dwErr);
	}
	else
	{
		app.DebugPrintf("Mounted DLC for texture pack, attempting to load data\n");
		texturePack->m_dlcDataPack = new DLCPack(texturePack->m_dlcInfoPack->getName(), dwLicenceMask);
		texturePack->setHasAudio(false);
		DWORD dwFilesProcessed = 0;
		// Load the DLC textures
		wstring dataFilePath = texturePack->m_dlcInfoPack->getFullDataPath();
		if(!dataFilePath.empty())
		{
			if(!app.m_dlcManager.readDLCDataFile(dwFilesProcessed, getFilePath(texturePack->m_dlcInfoPack->GetPackID(), dataFilePath),texturePack->m_dlcDataPack))
			{
				delete texturePack->m_dlcDataPack;
				texturePack->m_dlcDataPack = nullptr;
			}

			// Load the UI data
			if(texturePack->m_dlcDataPack != nullptr)
			{
#ifdef _XBOX
				File xzpPath(getFilePath(texturePack->m_dlcInfoPack->GetPackID(), wstring(L"TexturePack.xzp") ) );

				if(xzpPath.exists())
				{
					const char *pchFilename=wstringtofilename(xzpPath.getPath());
					HANDLE fileHandle = CreateFile(
						pchFilename, // file name
						GENERIC_READ, // access mode
						0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
						nullptr, // Unused
						OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
						FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
						nullptr // Unsupported
						);

					if( fileHandle != INVALID_HANDLE_VALUE )
					{
						DWORD dwFileSize = xzpPath.length();
						DWORD bytesRead;
						PBYTE pbData =  (PBYTE) new BYTE[dwFileSize];
						BOOL success = ReadFile(fileHandle,pbData,dwFileSize,&bytesRead,nullptr);
						CloseHandle(fileHandle);
						if(success)
						{
							DLCUIDataFile *uiDLCFile = (DLCUIDataFile *)texturePack->m_dlcDataPack->addFile(DLCManager::e_DLCType_UIData,L"TexturePack.xzp");
							uiDLCFile->addData(pbData,bytesRead,true);
						
						}
					}
				}
#else
				File archivePath(getFilePath(texturePack->m_dlcInfoPack->GetPackID(), wstring(L"media.arc") ) );
				if(archivePath.exists()) texturePack->m_archiveFile = new ArchiveFile(archivePath);
#endif

				/**
					4J-JEV:
						For all the GameRuleHeader files we find
				*/
				DLCPack *pack = texturePack->m_dlcInfoPack->GetParentPack();
				LevelGenerationOptions *levelGen = app.getLevelGenerationOptions();
				if (levelGen != nullptr && !levelGen->hasLoadedData())
				{
					int gameRulesCount = pack->getDLCItemsCount(DLCManager::e_DLCType_GameRulesHeader);
					for(int i = 0; i < gameRulesCount; ++i)
					{
						DLCGameRulesHeader *dlcFile = static_cast<DLCGameRulesHeader *>(pack->getFile(DLCManager::e_DLCType_GameRulesHeader, i));
					
						if (!dlcFile->getGrfPath().empty())
						{
							File grf( getFilePath(texturePack->m_dlcInfoPack->GetPackID(), dlcFile->getGrfPath() ) );
							if (grf.exists())
							{
#ifdef _UNICODE
								wstring path = grf.getPath();
								const WCHAR *pchFilename=path.c_str();
								HANDLE fileHandle = CreateFile(
									pchFilename, // file name
									GENERIC_READ, // access mode
									0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
									nullptr, // Unused
									OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
									FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
									nullptr // Unsupported
									);
#else
								const char *pchFilename=wstringtofilename(grf.getPath());
								HANDLE fileHandle = CreateFile(
									pchFilename, // file name
									GENERIC_READ, // access mode
									0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
									nullptr, // Unused
									OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
									FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
									nullptr // Unsupported
									);
#endif

								if( fileHandle != INVALID_HANDLE_VALUE )
								{
									DWORD dwFileSize = grf.length();
									DWORD bytesRead;
									PBYTE pbData =  (PBYTE) new BYTE[dwFileSize];
									BOOL bSuccess = ReadFile(fileHandle,pbData,dwFileSize,&bytesRead,nullptr);
									if(bSuccess==FALSE)
									{
										app.FatalLoadError();
									}
									CloseHandle(fileHandle);

									// 4J-PB - is it possible that we can get here after a read fail and it's not an error?
									dlcFile->setGrfData(pbData, dwFileSize, texturePack->m_stringTable);

									delete [] pbData;

									app.m_gameRules.setLevelGenerationOptions( dlcFile->lgo );
								}
							}
						}
					}
					if(levelGen->requiresBaseSave() && !levelGen->getBaseSavePath().empty() )
					{
						File grf(getFilePath(texturePack->m_dlcInfoPack->GetPackID(), levelGen->getBaseSavePath() ));
						if (grf.exists())
						{
#ifdef _UNICODE
							wstring path = grf.getPath();
							const WCHAR *pchFilename=path.c_str();
							HANDLE fileHandle = CreateFile(
								pchFilename, // file name
								GENERIC_READ, // access mode
								0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
								nullptr, // Unused
								OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
								FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
								nullptr // Unsupported
								);
#else
							const char *pchFilename=wstringtofilename(grf.getPath());
							HANDLE fileHandle = CreateFile(
								pchFilename, // file name
								GENERIC_READ, // access mode
								0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
								nullptr, // Unused
								OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
								FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
								nullptr // Unsupported
								);
#endif

							if( fileHandle != INVALID_HANDLE_VALUE )
							{
								DWORD bytesRead,dwFileSize = GetFileSize(fileHandle,nullptr);
								PBYTE pbData =  (PBYTE) new BYTE[dwFileSize];
								BOOL bSuccess = ReadFile(fileHandle,pbData,dwFileSize,&bytesRead,nullptr);
								if(bSuccess==FALSE)
								{
									app.FatalLoadError();
								}
								CloseHandle(fileHandle);

								// 4J-PB - is it possible that we can get here after a read fail and it's not an error?
								levelGen->setBaseSaveData(pbData, dwFileSize);
							}
						}
					}
				}
				

				// any audio data?
#ifdef _XBOX				
				File audioXSBPath(getFilePath(texturePack->m_dlcInfoPack->GetPackID(), wstring(L"MashUp.xsb") ) );
				File audioXWBPath(getFilePath(texturePack->m_dlcInfoPack->GetPackID(), wstring(L"MashUp.xwb") ) );
				
				if(audioXSBPath.exists() && audioXWBPath.exists())
				{

					texturePack->setHasAudio(true);
					const char *pchXWBFilename=wstringtofilename(audioXWBPath.getPath());
					Minecraft::GetInstance()->soundEngine->CreateStreamingWavebank(pchXWBFilename,&texturePack->m_pStreamedWaveBank);
					const char *pchXSBFilename=wstringtofilename(audioXSBPath.getPath());
					Minecraft::GetInstance()->soundEngine->CreateSoundbank(pchXSBFilename,&texturePack->m_pSoundBank);	

				}
#else 
				//DLCPack *pack = texturePack->m_dlcInfoPack->GetParentPack();
				if(pack->getDLCItemsCount(DLCManager::e_DLCType_Audio)>0)
				{
					DLCAudioFile *dlcFile = static_cast<DLCAudioFile *>(pack->getFile(DLCManager::e_DLCType_Audio, 0));
					texturePack->setHasAudio(true);
					// init the streaming sound ids for this texture pack
					int iOverworldStart, iNetherStart, iEndStart;
					int iOverworldC, iNetherC, iEndC;

					iOverworldStart=0;
					iOverworldC=dlcFile->GetCountofType(DLCAudioFile::e_AudioType_Overworld);
					iNetherStart=iOverworldC;
					iNetherC=dlcFile->GetCountofType(DLCAudioFile::e_AudioType_Nether);
					iEndStart=iOverworldC+iNetherC;
					iEndC=dlcFile->GetCountofType(DLCAudioFile::e_AudioType_End);

					Minecraft::GetInstance()->soundEngine->SetStreamingSounds(iOverworldStart,iOverworldStart+iOverworldC-1,
						iNetherStart,iNetherStart+iNetherC-1,iEndStart,iEndStart+iEndC-1,iEndStart+iEndC); // push the CD start to after
				}
#endif
}
			texturePack->loadColourTable();
		}

		// 4J-PB - we need to leave the texture pack mounted if it contained streaming audio
		if(texturePack->hasAudio()==false)
		{
#ifdef _XBOX
			StorageManager.UnmountInstalledDLC("TPACK");
#endif
		}
	}
	
	texturePack->m_bHasLoadedData = true;
	if (app.getLevelGenerationOptions()) app.getLevelGenerationOptions()->setLoadedData();
	app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ReloadTexturePack);

	return 0;
}

void DLCTexturePack::loadUI()
{
#ifdef _XBOX
//Syntax: "memory://" + Address + "," + Size + "#" + File
//L"memory://0123ABCD,21A3#skin_default.xur"

	// Load new skin
	if(m_dlcDataPack != nullptr && m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_UIData, L"TexturePack.xzp"))
	{
		DLCUIDataFile *dataFile = (DLCUIDataFile *)m_dlcDataPack->getFile(DLCManager::e_DLCType_UIData, L"TexturePack.xzp");

		DWORD dwSize = 0;
		PBYTE pbData = dataFile->getData(dwSize);

		const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
		WCHAR szResourceLocator[ LOCATOR_SIZE ];
		swprintf(szResourceLocator, LOCATOR_SIZE,L"memory://%08X,%04X#skin_Minecraft.xur",pbData, dwSize);

		XuiFreeVisuals(L"");


		HRESULT hr = app.LoadSkin(szResourceLocator,nullptr);//L"TexturePack");
		if(HRESULT_SUCCEEDED(hr))
		{
			bUILoaded = true;
			//CXuiSceneBase::GetInstance()->SetVisualPrefix(L"TexturePack");
			//CXuiSceneBase::GetInstance()->SkinChanged(CXuiSceneBase::GetInstance()->m_hObj);
		}
	}
#else
	if(m_archiveFile && m_archiveFile->hasFile(L"skin.swf"))
	{
		ui.ReloadSkin();
		bUILoaded = true;
	}
#endif
	else
	{		
		loadDefaultUI();
		bUILoaded = true;
	}

	AbstractTexturePack::loadUI();
#ifndef _XBOX
	if(hasAudio()==false && !ui.IsReloadingSkin())
	{
#ifdef _DURANGO
		StorageManager.UnmountInstalledDLC(L"TPACK");
#else
		StorageManager.UnmountInstalledDLC("TPACK");
#endif
	}
#endif
}

void DLCTexturePack::unloadUI()
{
	// Unload skin
	if(bUILoaded)
	{
#ifdef _XBOX
		XuiFreeVisuals(L"TexturePack");
		XuiFreeVisuals(L"");
		CXuiSceneBase::GetInstance()->SetVisualPrefix(L"");
		CXuiSceneBase::GetInstance()->SkinChanged(CXuiSceneBase::GetInstance()->m_hObj);
#endif
		setHasAudio(false);
	}
	AbstractTexturePack::unloadUI();

	app.m_dlcManager.removePack(m_dlcDataPack);
	m_dlcDataPack = nullptr;
	delete m_archiveFile;
	m_bHasLoadedData = false;

	bUILoaded = false;
}

wstring DLCTexturePack::getXuiRootPath()
{
	wstring path = L"";
	if(m_dlcDataPack != nullptr && m_dlcDataPack->doesPackContainFile(DLCManager::e_DLCType_UIData, L"TexturePack.xzp"))
	{
		DLCUIDataFile *dataFile = static_cast<DLCUIDataFile *>(m_dlcDataPack->getFile(DLCManager::e_DLCType_UIData, L"TexturePack.xzp"));

		DWORD dwSize = 0;
		PBYTE pbData = dataFile->getData(dwSize);

		const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
		WCHAR szResourceLocator[ LOCATOR_SIZE ];
		swprintf(szResourceLocator, LOCATOR_SIZE,L"memory://%08X,%04X#",pbData, dwSize);
		path = szResourceLocator;
	}
	return path;
}

unsigned int DLCTexturePack::getDLCParentPackId()
{
	return m_dlcInfoPack->GetParentPackId();
}

unsigned char DLCTexturePack::getDLCSubPackId()
{
	return (m_dlcInfoPack->GetPackId()>>24)&0xFF;
}

DLCPack * DLCTexturePack::getDLCInfoParentPack()
{
	return m_dlcInfoPack->GetParentPack();
}

XCONTENTDEVICEID DLCTexturePack::GetDLCDeviceID()
{
	return m_dlcInfoPack->GetDLCDeviceID();
}
