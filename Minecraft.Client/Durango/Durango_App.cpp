#include "stdafx.h"
#include "../Common/Consoles_App.h"
#include "../User.h"
#include "../../Minecraft.Client/Minecraft.h"
#include "../../Minecraft.Client/MinecraftServer.h"
#include "../../Minecraft.Client/PlayerList.h"
#include "../../Minecraft.Client/ServerPlayer.h"
#include "../../Minecraft.World/Level.h"
#include "../../Minecraft.World/LevelSettings.h"
#include "../../Minecraft.World/BiomeSource.h"
#include "../../Minecraft.World/LevelType.h"
#include "ServiceConfig/Events-XBLA.8-149E11AEEvents.h"
#include "../../Minecraft.World/DurangoStats.h"
#include "../../Minecraft.Client/Durango/XML/xmlFilesCallback.h"

CConsoleMinecraftApp app;

CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp()
{
	memset(&m_ThumbnailBuffer,0,sizeof(ImageFileBuffer));
	m_bShutdown=false;

	m_bRead_TMS_DLCINFO_XML=false;
	m_bTMSPP_GlobalFileListRead=false;
	m_bTMSPP_UserFileListRead=false;

	for (int i=0; i<MAX_LOCAL_PLAYERS; i++)
	{
		m_iLastPresenceContext[i] = -1;
		m_xuidLastPresencePlayer[i] = INVALID_XUID;
	}
}


void CConsoleMinecraftApp::HandleDLCLicenseChange()
{
	// run through the DLC packs and update if required
	int iOfferC=app.GetDLCOffersCount();
	for(int i = 0; i < iOfferC; i++)
	{
		MARKETPLACE_CONTENTOFFER_INFO  xOffer = StorageManager.GetOffer(i);

		XCONTENT_DATA *pContentData=StorageManager.GetInstalledDLC(xOffer.wszProductID);

		if((pContentData!=NULL) &&(pContentData->bTrialLicense==false))
		{
			DLCPack *pack = app.m_dlcManager.getPackFromProductID(xOffer.wszProductID);
			if(pack)
			{
				// Clear the DLC installed flag so the scenes will pick up the new dlc (could be a full pack install)
				app.ClearDLCInstalled();
				app.DebugPrintf(">>> HandleDLCLicenseChange - Updating license for DLC [%ls]\n",xOffer.wszOfferName);
				pack->updateLicenseMask(1);
			}
			else
			{
				app.DebugPrintf(">>> HandleDLCLicenseChange - Couldn't find licensed DLC [%ls] in app.m_dlcManager\n",xOffer.wszOfferName);
			}
		}
	}

	ui.HandleDLCLicenseChange();
}


void CConsoleMinecraftApp::SetRichPresenceContext(int iPad, int contextId)
{
	if(iPad < XUSER_MAX_COUNT && Minecraft::GetInstance()->localplayers[iPad])
	{
		PlayerUID uid;
		ProfileManager.GetXUID(iPad, &uid, true);

		if (uid != INVALID_XUID)
		{
			// 4J-JEV: Player has changed, update cached player and ensure this next presence is sent.
			if (uid != m_xuidLastPresencePlayer[iPad])
			{
				m_xuidLastPresencePlayer[iPad] = uid;
				m_iLastPresenceContext[iPad] = -1;
			}

			if (m_iLastPresenceContext[iPad] != contextId)
			{
				app.DebugPrintf(">>>   EventWriteRichPresenceState(%ls,_,%d)\n", uid.toString().c_str(), contextId);
				EventWriteRichPresenceState(uid.toString().c_str(), DurangoStats::getPlayerSession(), contextId);

				m_iLastPresenceContext[iPad] = contextId;
			}
		}
	}
}

void CConsoleMinecraftApp::StoreLaunchData()
{
}
void CConsoleMinecraftApp::ExitGame()
{
	//Windows::ApplicationModel::Core::CoreApplication::Exit();
	m_bShutdown=true;
}
void CConsoleMinecraftApp::FatalLoadError()
{
	// 4J-PB -
	//for(int i=0;i<10;i++)
	{
#ifndef _CONTENT_PACKAGE
		OutputDebugStringA("FatalLoadError\n");
#endif
		//Sleep(5000);
	}
}

void CConsoleMinecraftApp::CaptureSaveThumbnail()
{
	RenderManager.CaptureThumbnail(&m_ThumbnailBuffer);
}
void CConsoleMinecraftApp::GetSaveThumbnail(PBYTE *pbData,DWORD *pdwSize)
{
	// on a save caused by a create world, the thumbnail capture won't have happened
	if(m_ThumbnailBuffer.Allocated())
	{
		if( pbData )
		{
			*pbData= new BYTE [m_ThumbnailBuffer.GetBufferSize()];
			*pdwSize=m_ThumbnailBuffer.GetBufferSize();
			memcpy(*pbData,m_ThumbnailBuffer.GetBufferPointer(),*pdwSize);
		}
		m_ThumbnailBuffer.Release();
	}
	else
	{
		if( pbData )
		{
			// use the default image
			StorageManager.GetDefaultSaveThumbnail(pbData,pdwSize);
		}
	}
}
void CConsoleMinecraftApp::ReleaseSaveThumbnail()
{

}

void CConsoleMinecraftApp::GetScreenshot(int iPad,PBYTE *pbData,DWORD *pdwSize)
{

}

int CConsoleMinecraftApp::GetLocalTMSFileIndex(WCHAR *wchTMSFile,bool bFilenameIncludesExtension,eFileExtensionType eEXT)
{
	return -1;
}


int CConsoleMinecraftApp::LoadLocalTMSFile(WCHAR *wchTMSFile)
{
	return -1;
}

int CConsoleMinecraftApp::LoadLocalTMSFile(WCHAR *wchTMSFile, eFileExtensionType eExt)
{
	return -1;
}

void CConsoleMinecraftApp::FreeLocalTMSFiles(eTMSFileType eType)
{

}

int CConsoleMinecraftApp::LoadLocalDLCImages()
{
	unordered_map<wstring,DLC_INFO * > *pDLCInfoA=app.GetDLCInfo();
	// 4J-PB - Any local graphic files for the Minecraft Store?
	for (auto it = pDLCInfoA->begin(); it != pDLCInfoA->end(); it++)
	{
		DLC_INFO * pDLCInfo=(*it).second;

		LoadLocalDLCImage(pDLCInfo->wchBanner,&pDLCInfo->pbImageData,&pDLCInfo->dwImageBytes);
	}
	return 0;
}

void CConsoleMinecraftApp::FreeLocalDLCImages()
{
	// 4J-PB - Any local graphic files for the Minecraft Store?
	unordered_map<wstring,DLC_INFO * > *pDLCInfoA=app.GetDLCInfo();

	for (auto it = pDLCInfoA->begin(); it != pDLCInfoA->end(); it++)
	{
		DLC_INFO * pDLCInfo=(*it).second;

		if(pDLCInfo->dwImageBytes!=0)
		{
			free(pDLCInfo->pbImageData);
			pDLCInfo->dwImageBytes=0;
			pDLCInfo->pbImageData=NULL;
		}
	}
}


int CConsoleMinecraftApp::LoadLocalDLCImage(WCHAR *wchName,PBYTE *ppbImageData,DWORD *pdwBytes)
{
	// load the local file
	WCHAR wchFilename[64];


	// 4J-PB - Read the file containing the product codes. This will be different for the SCEE/SCEA/SCEJ builds
	swprintf(wchFilename,L"DLCImages/%s",wchName);
	HANDLE hFile = CreateFile(wchFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if( hFile == INVALID_HANDLE_VALUE )
	{
		app.DebugPrintf("Failed to open %ls\n", wchFilename);
		return FALSE;
	}

	DWORD dwHigh=0;
	*pdwBytes = GetFileSize(hFile,&dwHigh);

	if(*pdwBytes!=0)
	{
		DWORD dwBytesRead;
		PBYTE pbImageData=(PBYTE)malloc(*pdwBytes);

		if(ReadFile(hFile,pbImageData,*pdwBytes,&dwBytesRead,NULL)==FALSE)
		{
			// failed
			free(pbImageData);
			*pdwBytes=0;
		}
		else
		{
			*ppbImageData=pbImageData;
		}
	}

	CloseHandle(hFile);

	return 0;
}

void CConsoleMinecraftApp::TemporaryCreateGameStart()
{
	////////////////////////////////////////////////////////////////////////////////////////////// From CScene_Main::OnInit

	app.setLevelGenerationOptions(NULL);

	// From CScene_Main::RunPlayGame
	Minecraft *pMinecraft=Minecraft::GetInstance();
	app.ReleaseSaveThumbnail();
	ProfileManager.SetLockedProfile(0);
	pMinecraft->user->name = L"Durango";
	app.ApplyGameSettingsChanged(0);

	////////////////////////////////////////////////////////////////////////////////////////////// From CScene_MultiGameJoinLoad::OnInit
	MinecraftServer::resetFlags();

	// From CScene_MultiGameJoinLoad::OnNotifyPressEx
	app.SetTutorialMode( false );
	app.SetCorruptSaveDeleted(false);

	////////////////////////////////////////////////////////////////////////////////////////////// From CScene_MultiGameCreate::CreateGame

	app.ClearTerrainFeaturePosition();
	wstring wWorldName = L"TestWorld";

	StorageManager.ResetSaveData();
	StorageManager.SetSaveTitle(wWorldName.c_str());

	bool isFlat = false;
	int64_t seedValue = BiomeSource::findSeed(isFlat?LevelType::lvl_flat:LevelType::lvl_normal);	// 4J - was (new Random())->nextLong() - now trying to actually find a seed to suit our requirements

	NetworkGameInitData *param = new NetworkGameInitData();
	param->seed = seedValue;
	param->saveData = NULL;

	app.SetGameHostOption(eGameHostOption_Difficulty,0);
	app.SetGameHostOption(eGameHostOption_FriendsOfFriends,0);
	app.SetGameHostOption(eGameHostOption_Gamertags,1);
	app.SetGameHostOption(eGameHostOption_BedrockFog,1);

	app.SetGameHostOption(eGameHostOption_GameType,GameType::SURVIVAL->getId() );
	app.SetGameHostOption(eGameHostOption_LevelType, 0 );
	app.SetGameHostOption(eGameHostOption_Structures, 1 );
	app.SetGameHostOption(eGameHostOption_BonusChest, 0 );

	app.SetGameHostOption(eGameHostOption_PvP, 1);
	app.SetGameHostOption(eGameHostOption_TrustPlayers, 1 );
	app.SetGameHostOption(eGameHostOption_FireSpreads, 1 );
	app.SetGameHostOption(eGameHostOption_TNT, 1 );
	app.SetGameHostOption(eGameHostOption_HostCanFly, 1);
	app.SetGameHostOption(eGameHostOption_HostCanChangeHunger, 1);
	app.SetGameHostOption(eGameHostOption_HostCanBeInvisible, 1 );

	param->settings = app.GetGameHostOption( eGameHostOption_All );

	g_NetworkManager.FakeLocalPlayerJoined();

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = (LPVOID)param;

	// Reset the autosave time
	app.SetAutosaveTimerTime();

	C4JThread* thread = new C4JThread(loadingParams->func, loadingParams->lpParam, "RunNetworkGame");
	thread->Run();
}

typedef struct
{
	eDLCContentType e_DLC_Type;
	//WCHAR *wchDisplayName;
	WCHAR *wchProductId;
	WCHAR *wchBannerName;
	WCHAR *wchFirstSkin;
	int iConfig;
	unsigned int uiSortIndex;
}
DLC_DETAILS;

#define MAX_DLC_DETAILS 18
/*DLC_DETAILS DLCDetailsA[MAX_DLC_DETAILS] =
{
	// skin packs

	// Skin Pack Festive
	{ e_DLC_SkinPack,L"Festive Skin Pack",L"6dffc4d6-a2d2-4c8c-9284-c607f77e431a",L"SPF.png",L"dlcskin00000600.png",0,1},
	// Skin Pack 1
	{ e_DLC_SkinPack,L"Skin Pack 1",L"8ecf0f25-a119-4987-a32b-ee0a5925ad8d",L"SP1.png",L"dlcskin00000000.png",0,2},
	// Skin Pack 2
	{ e_DLC_SkinPack,L"Skin Pack 2",L"cc59b688-7cfb-4fa0-a76e-84aa55b92cae",L"SP2.png",L"dlcskin00000900.png",0,3},
	// Skin Pack 6
	//{ e_DLC_SkinPack,L"0037a29f-876e-4709-8bb8-a388738e6f51","SP6.png","dlcskin00000900.png",0,3},
	// Skin Pack Battle And Beasts
	{ e_DLC_SkinPack,L"Battle And Beasts",L"eeeb6489-02a3-4c6e-a8c9-2ace2aa1094d",L"SPC.png",L"dlcskin00000800.png",0,4},
	// Skin Pack Battle And Beasts 2
	{ e_DLC_SkinPack,L"Battle And Beasts 2",L"b858200a-59a8-4e1f-a049-f3e73db2d786",L"SPZ.png",L"dlcskin00001000.png",0,5},
	// Skin Pack Marvel Avengers
	{ e_DLC_SkinPack,L"Marvel Avengers",L"d59757dd-7757-4c5e-8499-dbe4743baa10",L"SPM.png",L"dlcskin00001700.png",0,6},
	// Skin Pack Marvel Spider-Man
	{ e_DLC_SkinPack,L"Marvel Spider-Man",L"cdca1bff-13d1-435a-8aee-e7a35002473f",L"SPI.png",L"dlcskin00001800.png",0,7},
	// Skin Pack Birthday 1
	{ e_DLC_SkinPack,L"Birthday Skin Pack",L"951e8062-3d4e-470c-8177-5eca91bc08b3",L"SPB.png",L"dlcskin00000700.png",0,8},
	// Skin Pack Birthday 2
	{ e_DLC_SkinPack,L"2nd Birthday Skin Pack",L"c7157788-468d-4ca8-9ecf-5d77a09850fc",L"SPB2.png",L"dlcskin00002200.png",0,9},

	// Texture Packs

	// Plastic Texture Pack
	{ e_DLC_TexturePacks,L"Plastic Texture Pack",L"52ecdcf1-d362-47a1-973b-1eeca0db0ea8",L"TP01.png",L"",2049,1},
	// Natural Texture Pack
	{ e_DLC_TexturePacks,L"Natural Texture Pack",L"1c56db0c-ff49-4bb1-b372-2122b0e813c1",L"TP02.png",L"",2053,2},
	// Halloween Texture Pack
	{ e_DLC_TexturePacks,L"Halloween Texture Pack",L"8cb331d1-8fa1-4367-a41a-d4830a80ce67",L"TP03.png",L"",2052,3},
	// Fantasy Texture Pack
	{ e_DLC_TexturePacks,L"Fantasy Texture Pack",L"375a1df4-5550-415b-b278-20f65b31a7a3",L"TP04.png",L"",2051,4},
	// City Texture Pack
	{ e_DLC_TexturePacks,L"City Texture Pack",L"ea5c7b40-e04d-4469-9382-8806467ca2c4",L"TP05.png",L"",2054,5},
	// Candy Texture Pack
	{ e_DLC_TexturePacks,L"Candy Texture Pack",L"94c75e45-0757-4886-916c-ab026ae27ca9",L"TP06.png",L"",2050,6},
	// Comic Texture Pack
	//{ e_DLC_TexturePacks,L"Comic Texture Pack",L"3e14cf0f-26eb-40df-897d-7af905456e58",L"TP07.png",L"",2055,7},

	// Mash-up Packs

	// Mass Effect
	{ e_DLC_MashupPacks,L"Mass Effect",L"ead4f3bb-b388-42da-8fa9-f1f91570b5c7",L"MPMA.png",L"dlcskin00001100.png",1024,1},
	// Skyrim
	{ e_DLC_MashupPacks,L"Skyrim",L"81cc4261-7b63-4e48-af1c-60b9ae099644",L"MPSR.png",L"dlcskin00001400.png",1025,2},
	// Halo
	{ e_DLC_MashupPacks,L"Halo",L"1e06dafc-ea27-475e-945c-fcee0c455f87",L"MPHA.png",L"dlcskin00001600.png",1026,3},
};*/

void CConsoleMinecraftApp::InitialiseDLCDetails()
{
	for(int i=0;i<18;i++)
	{
		//RegisterDLCData(DLCDetailsA[i].e_DLC_Type, DLCDetailsA[i].wchBannerName, DLCDetailsA[i].wchDisplayName, DLCDetailsA[i].wchProductId, DLCDetailsA[i].wchFirstSkin, DLCDetailsA[i].iConfig, DLCDetailsA[i].uiSortIndex);
	}
}

bool CConsoleMinecraftApp::UpdateProductId(XCONTENT_DATA &Data)
{
	// Do we have a product id for this?
	DLC_INFO *pDLCInfo=app.GetDLCInfoForProductName(Data.wszDisplayName);

	if(pDLCInfo!=NULL)
	{
		app.DebugPrintf("Updating product id for %ls\n",Data.wszDisplayName);
		swprintf_s(Data.wszProductID, 64,L"%ls",pDLCInfo->wsProductId.c_str());
		return true;
	}
	else
	{
		app.DebugPrintf("Couldn't find %ls\n",Data.wszDisplayName);
	}

	return false;
}

void CConsoleMinecraftApp::Shutdown()
{
	m_bShutdown=true;
}

bool CConsoleMinecraftApp::getShutdownFlag()
{
	return m_bShutdown;
}


// TMS
bool CConsoleMinecraftApp::TMSPP_ReadBannedList(int iPad,eTMSAction NextAction)
{
	app.DebugPrintf("CConsoleMinecraftApp::TMSPP_ReadBannedList\n");
	eTitleStorageState eResult;
	bool bSendBanFileRetrievedMsg=false;

	if(GetBanListRead(iPad)==false)
	{
		// Attempt to read the ban list
		// do we have one in our user filelist?
		//if(StorageManager.TMSPP_InFileList(C4JStorage::eGlobalStorage_TitleUser,iPad,L"BannedList"))
		{
			SetBanListRead(iPad,true);
			ClearBanList(iPad);

			eResult=StorageManager.TMSPP_ReadFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,L"BannedList",&CConsoleMinecraftApp::Callback_TMSPPReadBannedList,this,NextAction);
			if(eResult!=eTitleStorage_pending)
			{
				// something went wrong
				app.SetTMSAction(iPad,(eTMSAction)NextAction);
				bSendBanFileRetrievedMsg=true;
			}
		}
	}
	else
	{
		bSendBanFileRetrievedMsg=true;
	}

	if(bSendBanFileRetrievedMsg)
	{
		ui.HandleTMSBanFileRetrieved(iPad);
	}

	app.SetTMSAction(iPad,(eTMSAction)NextAction);

	return true;
}

int CConsoleMinecraftApp::Callback_TMSPPReadBannedList(void *pParam,int iPad, int iUserData, LPVOID lpvData,WCHAR *wchFilename)
{
	app.DebugPrintf("CConsoleMinecraftApp::Callback_TMSPPReadBannedList\n");
	C4JStorage::PTMSPP_FILEDATA pFileData=(C4JStorage::PTMSPP_FILEDATA)lpvData;

	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;

	if(pFileData)
	{
		// put the entries into the vector
		int iEntries=pFileData->dwSize/sizeof(BANNEDLISTDATA);
		PBANNEDLISTDATA pData=(PBANNEDLISTDATA)pFileData->pbData;

		for(int i=0;i<iEntries;i++)
		{
			pClass->AddLevelToBannedLevelList(iPad,&pData[i], false);
		}
		// mark the level as not checked against banned levels - it'll be checked once the level starts
		app.SetBanListCheck(iPad,false);

		// Xbox One will clear things within the DownloadBlob
#ifndef _XBOX_ONE
		delete [] pFileData->pbData;
		delete [] pFileData;
#endif

		ui.HandleTMSBanFileRetrieved(iPad);
	}
	else
	{
		// read problem - set state to idel again
		StorageManager.TMSPP_ClearTitleStorageState(iPad);
	}

	// change the state to the next action
	pClass->SetTMSAction(iPad,(eTMSAction)iUserData);

	return 0;
}

void CConsoleMinecraftApp::TMSPP_ReadDLCFile(int iPad,eTMSAction NextAction)
{
	app.DebugPrintf("CConsoleMinecraftApp::TMSPP_ReadDLCFile\n");
	bool bRetrievedDLCFile=false;
	// try reading the DLC.xml file (from TMS global) - only allowed to do this once an hour at the most, but we'll just read once each time the game launches
	eTitleStorageState eResult;
	if(m_bRead_TMS_DLCINFO_XML==false)
	{
// 4J-PB - we're reading this info from a local file now

		eResult=StorageManager.TMSPP_ReadFile(iPad,C4JStorage::eGlobalStorage_Title,C4JStorage::TMS_FILETYPE_BINARY,L"DLCXbox1.cmp",&CConsoleMinecraftApp::Callback_TMSPPReadDLCFile,this,NextAction);
		if(eResult!=eTitleStorage_pending)
		{
			// something went wrong
			app.SetTMSAction(iPad,(eTMSAction)NextAction);
			bRetrievedDLCFile=true;
			m_bRead_TMS_DLCINFO_XML=true;
		}
	}
	else
	{
		bRetrievedDLCFile=true;
	}

	if(bRetrievedDLCFile)
	{
		ui.HandleTMSDLCFileRetrieved(iPad);

		app.SetTMSAction(iPad,(eTMSAction)NextAction);
	}
}

void CConsoleMinecraftApp::TMSPP_RetrieveFileList(int iPad,C4JStorage::eGlobalStorage eStorageFacility,eTMSAction NextAction)
{
	app.DebugPrintf("CConsoleMinecraftApp::TMSPP_RetrieveFileList\n");

	if(eStorageFacility==C4JStorage::eGlobalStorage_Title)
	{
		if(m_bTMSPP_GlobalFileListRead==false)
		{
			m_bTMSPP_GlobalFileListRead=true;
			StorageManager.TMSPP_ReadFileList(iPad,eStorageFacility,&CConsoleMinecraftApp::Callback_TMSPPRetrieveFileList,this,NextAction);
		}
		else
		{
			SetTMSAction(iPad,NextAction);
		}
	}
	else
	{
		if(m_bTMSPP_UserFileListRead==false)
		{
			m_bTMSPP_UserFileListRead=true;
			StorageManager.TMSPP_ReadFileList(iPad,eStorageFacility,&CConsoleMinecraftApp::Callback_TMSPPRetrieveFileList,this,NextAction);
		}
		else
		{
			SetTMSAction(iPad,NextAction);
		}
	}
}

int CConsoleMinecraftApp::Callback_TMSPPRetrieveFileList(void *pParam,int iPad, int iUserData, LPVOID lpvData, WCHAR *wchFilename)
{
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;
	app.DebugPrintf("CConsoleMinecraftApp::Callback_TMSPPRetrieveFileList\n");
	if(lpvData!=NULL)
	{
		vector<C4JStorage::PTMSPP_FILE_DETAILS> *pvTmsFileDetails=(vector<C4JStorage::PTMSPP_FILE_DETAILS> *)lpvData;

		if(pvTmsFileDetails->size()>0)
		{
	#ifdef _DEBUG
			// dump out the file list
			app.DebugPrintf("TMSPP filecount - %d\nFiles - \n",pvTmsFileDetails->size());
			int iCount=0;
			auto itEnd = pvTmsFileDetails->end();
			for (auto it = pvTmsFileDetails->begin(); it != itEnd; it++)
			{
				C4JStorage::PTMSPP_FILE_DETAILS fd = *it;
				app.DebugPrintf("%2d. %ls (size - %d)\n",iCount++,fd->wchFilename,fd->ulFileSize);
			}

	#endif
		}
	}
	// change the state to the next action
	pClass->SetTMSAction(iPad,(eTMSAction)iUserData);
	return 0;
}

//#define WRITE_DLCINFO 1
int CConsoleMinecraftApp::Callback_TMSPPReadDLCFile(void *pParam,int iPad, int iUserData, LPVOID lpvData ,WCHAR *pwchFilename)
{
	app.DebugPrintf("CConsoleMinecraftApp::Callback_TMSPPReadDLCFile\n");
	C4JStorage::PTMSPP_FILEDATA pFileData= (C4JStorage::PTMSPP_FILEDATA)lpvData;
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;

#ifdef WRITE_DLCINFO
	if(0)
#else
	if(pFileData && pFileData->dwSize>0)
#endif
	{
		// the DLC.xml file is now compressed

		unsigned int uiDecompSize=*(unsigned int *)pFileData->pbData;
		unsigned int uiCompSize=((unsigned int *)pFileData->pbData)[1];

		BYTE *pDecompressedData = new BYTE [uiDecompSize];

		Compression::ECompressionTypes eOriginalCompressionType=Compression::getCompression()->GetDecompressionType();
		Compression::getCompression()->SetDecompressionType(Compression::eCompressionType_LZXRLE);
		Compression::getCompression()->Decompress(pDecompressedData,&uiDecompSize,&((unsigned int *)pFileData->pbData)[2],uiCompSize);
		Compression::getCompression()->SetDecompressionType(eOriginalCompressionType);

		ATG::XMLParser xmlParser;
		xmlDLCInfoCallback xmlCallback;

		xmlParser.RegisterSAXCallbackInterface( &xmlCallback );
		xmlParser.ParseXMLBuffer((CHAR *)pDecompressedData,uiDecompSize);

		pClass->m_bRead_TMS_DLCINFO_XML=true;

		delete pDecompressedData;

		// apply the dlc info to the locally installed DLC
		StorageManager.UpdateDLCProductIDs();

		ui.HandleTMSDLCFileRetrieved(iPad);
	}
	else
	{

		// if there was a read error, reset to idle
		StorageManager.TMSPP_ClearTitleStorageState(iPad);

#ifdef WRITE_DLCINFO
		HANDLE file;
		DWORD dwHigh=0;
		DWORD dwFileSize;

		// hack for now to upload the file
		// open the local file
		file = CreateFile(L"DLCXbox1.cmp", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( file == INVALID_HANDLE_VALUE )
		{
			DWORD error = GetLastError();
			app.DebugPrintf("Failed to open DLCXbox1.cmp with error code %d (%x)\n", error, error);
			__debugbreak();
			return 0;
		}

		dwHigh=0;
		dwFileSize = GetFileSize(file,&dwHigh);

		if(dwFileSize!=0)
		{
			DWORD bytesRead;

			PBYTE pbData= new BYTE [dwFileSize];

			ReadFile(file,pbData,dwFileSize,&bytesRead,NULL);

			if(bytesRead==dwFileSize)
			{
				//StorageManager.TMSPP_WriteFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,L"DLCXbox1.cmp",(PBYTE) pbData, dwFileSize,NULL,NULL, 0);
				StorageManager.TMSPP_WriteFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,L"TP06.cmp",(PBYTE) pbData, dwFileSize,NULL,NULL, 0);
			}
			Sleep(5000);
		}

		CloseHandle(file);

		/*
		// now the icon
		file = CreateFile(L"TP06.png", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if( file == INVALID_HANDLE_VALUE )
		{
			DWORD error = GetLastError();
			app.DebugPrintf("Failed to open DLCXbox1.cmp with error code %d (%x)\n", error, error);
			return 0;
		}

		dwHigh=0;
		dwFileSize = GetFileSize(file,&dwHigh);

		if(dwFileSize!=0)
		{
			DWORD bytesRead;

			PBYTE pbData= new BYTE [dwFileSize];

			ReadFile(file,pbData,dwFileSize,&bytesRead,NULL);

			if(bytesRead==dwFileSize)
			{
				StorageManager.TMSPP_WriteFile(iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,L"TP06.png",(PBYTE) pbData, dwFileSize,NULL,NULL, 0);
			}
			Sleep(5000);
		}

		CloseHandle(file);*/
#endif
	}

	// change the state to the next action
	pClass->SetTMSAction(iPad,(eTMSAction)iUserData);

	return 0;
}

void CConsoleMinecraftApp::Callback_SaveGameIncomplete(void *pParam, C4JStorage::ESaveIncompleteType saveIncompleteType)
{
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;

	if (	saveIncompleteType == C4JStorage::ESaveIncomplete_OutOfQuota
		||	saveIncompleteType == C4JStorage::ESaveIncomplete_OutOfLocalStorage )
	{
		StorageManager.SetSaveDisabled(true);
		pClass->EnterSaveNotificationSection();

		int message;
		if (saveIncompleteType == C4JStorage::ESaveIncomplete_OutOfQuota)	message = IDS_SAVE_INCOMPLETE_EXPLANATION_QUOTA;
		else																message = IDS_SAVE_INCOMPLETE_EXPLANATION_LOCAL_STORAGE;

		UINT uiIDA[3] =
		{
			IDS_SAVE_INCOMPLETE_RETRY_SAVING,
			IDS_SAVE_INCOMPLETE_DISABLE_SAVING,
			IDS_SAVE_INCOMPLETE_DELETE_SAVES
		};

		if ( ui.RequestMessageBox( IDS_SAVE_INCOMPLETE_TITLE, message, uiIDA,3,0,Callback_SaveGameIncompleteMessageBoxReturned,pClass, app.GetStringTable()) == C4JStorage::EMessage_Busy)
		{
			// If this failed to display, continue as if we cancelled. This isn't ideal, but the user should already have had some system notification of being out of memory,
			// and if we instantly retry then they may not be able to navigate whatever other error is blocking this from appearing
			Callback_SaveGameIncompleteMessageBoxReturned(pParam, 0, C4JStorage::EMessage_Cancelled);
		}
	}
	else
	{
		// 4J-JEV: Unknown error, just cancel the operation.
		Callback_SaveGameIncompleteMessageBoxReturned(pParam, 0, C4JStorage::EMessage_Cancelled);
	}
}

int CConsoleMinecraftApp::Callback_SaveGameIncompleteMessageBoxReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;

	switch(result)
	{
		case C4JStorage::EMessage_ResultAccept:
			pClass->LeaveSaveNotificationSection();
			StorageManager.SetSaveDisabled(false);
			StorageManager.ContinueIncompleteOperation();
			break;
		case C4JStorage::EMessage_ResultDecline:
		case C4JStorage::EMessage_Cancelled:
			pClass->LeaveSaveNotificationSection();
			// Set the global flag, so that we don't disable saving again once the message box is complete
			app.SetGameHostOption(eGameHostOption_DisableSaving, 1);
			StorageManager.CancelIncompleteOperation();
			break;
		case C4JStorage::EMessage_ResultThirdOption:
			ui.NavigateToScene(iPad, eUIScene_InGameSaveManagementMenu, NULL, eUILayer_Error, eUIGroup_Fullscreen);
			break;
	}
	return 0;
}

void CConsoleMinecraftApp::ReadLocalDLCList(void)
{
	char szFile[255];
	DWORD dwLength;
	// read the local dlc list
	File fDLCList(L"CU/DLCXbox1.cmp") ;
	if(fDLCList.exists())
	{
		dwLength = fDLCList.length();
		byteArray data(dwLength);

		FileInputStream fis(fDLCList);
		fis.read(data,0,dwLength);
		fis.close();

		unsigned int uiDecompSize=*(unsigned int *)data.data;
		unsigned int uiCompSize=((unsigned int *)data.data)[1];

		BYTE *pDecompressedData = new BYTE [uiDecompSize];

		Compression::ECompressionTypes eOriginalCompressionType=Compression::getCompression()->GetDecompressionType();
		Compression::getCompression()->SetDecompressionType(Compression::eCompressionType_LZXRLE);
		Compression::getCompression()->Decompress(pDecompressedData,&uiDecompSize,&((unsigned int *)data.data)[2],uiCompSize);
		Compression::getCompression()->SetDecompressionType(eOriginalCompressionType);

		ATG::XMLParser xmlParser;
		xmlDLCInfoCallback xmlCallback;

		xmlParser.RegisterSAXCallbackInterface( &xmlCallback );
		xmlParser.ParseXMLBuffer((CHAR *)pDecompressedData,uiDecompSize);

		delete pDecompressedData;
	}
}