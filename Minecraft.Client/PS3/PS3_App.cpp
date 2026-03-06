
#include "stdafx.h"
#include "..\Common\Consoles_App.h"
#include "..\User.h"
#include "..\..\Minecraft.Client\Minecraft.h"
#include "..\..\Minecraft.Client\MinecraftServer.h"
#include "..\..\Minecraft.Client\PlayerList.h"
#include "..\..\Minecraft.Client\ServerPlayer.h"
#include "..\..\Minecraft.World\Level.h"
#include "..\..\Minecraft.World\LevelSettings.h"
#include "..\..\Minecraft.World\BiomeSource.h"
#include "..\..\Minecraft.World\LevelType.h"
#include "..\..\PS3\Network\SonyCommerce_PS3.h"
#include "..\..\Minecraft.World\StringHelpers.h"
#include "PS3Extras\ShutdownManager.h"
#include "PS3\Network\SonyRemoteStorage_PS3.h"



// #define SAVE_GAME_TO_LOAD L"Saves\\v0170-39728.00.109.56268.00.mcs"

CConsoleMinecraftApp app;

CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp()
{
	memset(&m_ThumbnailBuffer,0,sizeof(ImageFileBuffer));
	memset(&m_SaveImageBuffer,0,sizeof(ImageFileBuffer));
	memset(&m_ScreenshotBuffer,0,sizeof(ImageFileBuffer));

	memset(&ProductCodes,0,sizeof(PRODUCTCODES));

	m_bVoiceChatAndUGCRestricted=false;
	m_bDisplayFullVersionPurchase=false;
// #ifdef _DEBUG_MENUS_ENABLED
// 	debugOverlayCreated = false;
// #endif

	m_ProductListA=NULL;
	m_bBootedFromDiscPatch=false;
	memset(m_usrdirPathBDPatch,0,128);
	m_pRemoteStorage = new SonyRemoteStorage_PS3;

}

void CConsoleMinecraftApp::SetRichPresenceContext(int iPad, int contextId)
{
	ProfileManager.SetRichPresenceContextValue(iPad,CONTEXT_GAME_STATE,contextId);
}

char *CConsoleMinecraftApp::GetProductCode()
{
	return ProductCodes.chProductCode;
}
char *CConsoleMinecraftApp::GetDiscProductCode()
{
	return ProductCodes.chDiscProductCode;
}
char *CConsoleMinecraftApp::GetSaveFolderPrefix()
{
	//4J-PB - need to check if we are the disc version
	if(StorageManager.GetBootTypeDisc())
	{
		return ProductCodes.chDiscSaveFolderPrefix;
	}
	else
	{
		return ProductCodes.chSaveFolderPrefix;
	}
}
char *CConsoleMinecraftApp::GetCommerceCategory()
{
	return ProductCodes.chCommerceCategory;
}
char *CConsoleMinecraftApp::GetTexturePacksCategoryID()
{
	return ProductCodes.chTexturePackID;
}
char *CConsoleMinecraftApp::GetUpgradeKey()
{
	return ProductCodes.chUpgradeKey;
}
EProductSKU CConsoleMinecraftApp::GetProductSKU()
{
	return ProductCodes.eProductSKU;
}
bool CConsoleMinecraftApp::IsJapaneseSKU()
{
	return ProductCodes.eProductSKU == e_sku_SCEJ;

}
bool CConsoleMinecraftApp::IsEuropeanSKU()
{
	return ProductCodes.eProductSKU == e_sku_SCEE;

}
bool CConsoleMinecraftApp::IsAmericanSKU()
{
	return ProductCodes.eProductSKU == e_sku_SCEA;

}
// char *CConsoleMinecraftApp::GetSKUPostfix()
// {
// 	return ProductCodes.chSkuPostfix;
// }

SONYDLC *CConsoleMinecraftApp::GetSONYDLCInfo(char *pchTitle)
{
	wstring wstrTemp=convStringToWstring(pchTitle);
	SONYDLC *pTemp=m_SONYDLCMap.at(wstrTemp);

	return pTemp;
}

#define WRAPPED_READFILE(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped) {if(ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped)==FALSE) { return FALSE;}}
BOOL CConsoleMinecraftApp::ReadProductCodes()
{
	char chDLCTitle[64];

	// 4J-PB - Read the file containing the product codes. This will be different for the SCEE/SCEA/SCEJ builds
	HANDLE file = CreateFile("PS3/PS3ProductCodes.bin", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if( file == INVALID_HANDLE_VALUE )
	{
		//DWORD error = GetLastError();
		app.DebugPrintf("Failed to open ProductCodes.bin\n");// with error code %d (%x)\n", error, error);
		return FALSE;
	}

	DWORD dwHigh=0;
	DWORD dwFileSize = GetFileSize(file,&dwHigh);

	if(dwFileSize!=0)
	{
		DWORD bytesRead;

		WRAPPED_READFILE(file,ProductCodes.chProductCode,PRODUCT_CODE_SIZE,&bytesRead,NULL);
		WRAPPED_READFILE(file,ProductCodes.chDiscProductCode,PRODUCT_CODE_SIZE,&bytesRead,NULL);
		WRAPPED_READFILE(file,ProductCodes.chSaveFolderPrefix,SAVEFOLDERPREFIX_SIZE,&bytesRead,NULL);
		WRAPPED_READFILE(file,ProductCodes.chDiscSaveFolderPrefix,SAVEFOLDERPREFIX_SIZE,&bytesRead,NULL);
		WRAPPED_READFILE(file,ProductCodes.chCommerceCategory,COMMERCE_CATEGORY_SIZE,&bytesRead,NULL);
		WRAPPED_READFILE(file,ProductCodes.chTexturePackID,SCE_NP_COMMERCE2_CATEGORY_ID_LEN,&bytesRead,NULL);
		WRAPPED_READFILE(file,ProductCodes.chUpgradeKey,UPGRADE_KEY_SIZE,&bytesRead,NULL);
		WRAPPED_READFILE(file,ProductCodes.chSkuPostfix,SKU_POSTFIX_SIZE,&bytesRead,NULL);

		app.DebugPrintf("ProductCodes.chProductCode %s\n",ProductCodes.chProductCode);
		app.DebugPrintf("ProductCodes.chDiscProductCode %s\n",ProductCodes.chDiscProductCode);
		app.DebugPrintf("ProductCodes.chSaveFolderPrefix %s\n",ProductCodes.chSaveFolderPrefix);
		app.DebugPrintf("ProductCodes.chDiscSaveFolderPrefix %s\n",ProductCodes.chDiscSaveFolderPrefix);
		app.DebugPrintf("ProductCodes.chCommerceCategory %s\n",ProductCodes.chCommerceCategory);
		app.DebugPrintf("ProductCodes.chTexturePackID %s\n",ProductCodes.chTexturePackID);
		app.DebugPrintf("ProductCodes.chUpgradeKey %s\n",ProductCodes.chUpgradeKey);
		app.DebugPrintf("ProductCodes.chSkuPostfix %s\n",ProductCodes.chSkuPostfix);

		// DLC
		unsigned int uiDLC;
		WRAPPED_READFILE(file,&uiDLC,sizeof(int),&bytesRead,NULL);

		for(unsigned int i=0;i<uiDLC;i++)
		{
			SONYDLC *pDLCInfo= new SONYDLC;
			memset(pDLCInfo,0,sizeof(SONYDLC));
			memset(chDLCTitle,0,64);

			unsigned int uiVal;
			WRAPPED_READFILE(file,&uiVal,sizeof(int),&bytesRead,NULL);
			WRAPPED_READFILE(file,pDLCInfo->chDLCKeyname,sizeof(char)*uiVal,&bytesRead,NULL);

			WRAPPED_READFILE(file,&uiVal,sizeof(int),&bytesRead,NULL);
			WRAPPED_READFILE(file,chDLCTitle,sizeof(char)*uiVal,&bytesRead,NULL);
			app.DebugPrintf("DLC title %s\n",chDLCTitle);

			WRAPPED_READFILE(file,&pDLCInfo->eDLCType,sizeof(int),&bytesRead,NULL);
			WRAPPED_READFILE(file,&pDLCInfo->iFirstSkin,sizeof(int),&bytesRead,NULL);
			WRAPPED_READFILE(file,&pDLCInfo->iConfig,sizeof(int),&bytesRead,NULL);

			// push this into a vector

			wstring wstrTemp=convStringToWstring(chDLCTitle);
			m_SONYDLCMap[wstrTemp]=pDLCInfo;
		}
		CloseHandle(file);
	}

	if(strcmp(ProductCodes.chProductCode, "NPEB01899") == 0)
		ProductCodes.eProductSKU = e_sku_SCEE;
	else if(strcmp(ProductCodes.chProductCode, "NPUB31419") == 0)
		ProductCodes.eProductSKU = e_sku_SCEA;
	else if(strcmp(ProductCodes.chProductCode, "NPJB00549") == 0)
		ProductCodes.eProductSKU = e_sku_SCEJ;
	else
	{
		// unknown product ID
		assert(0);
	}

	return TRUE;
}


void CConsoleMinecraftApp::StoreLaunchData()
{
}
void CConsoleMinecraftApp::ExitGame()
{

}
void CConsoleMinecraftApp::FatalLoadError()
{
	wchar_t *aStrings[3];
	LPCWSTR wszButtons[1];

	app.DebugPrintf("CConsoleMinecraftApp::FatalLoadError\n");

	// IDS_FATAL_ERROR_TITLE
	// IDS_FATAL_ERROR_TEXT
	// IDS_EXIT_GAME

	switch (XGetLanguage())
	{
	case XC_LANGUAGE_GERMAN:
		aStrings[0] = L"Ladefehler";
		aStrings[1] = L"Minecraft: PlayStation®3 Edition konnte nicht geladen werden und kann daher nicht fortgesetzt werden.";
		aStrings[2] = L"Spiel verlassen";
		break;
	case XC_LANGUAGE_SPANISH:
		aStrings[0] = L"Error al cargar";
		aStrings[1] = L"Se ha producido un error al cargar Minecraft: PlayStation®3 Edition y no es posible continuar.";
		aStrings[2] = L"Salir del juego";
		break;
	case XC_LANGUAGE_FRENCH:
		aStrings[0] = L"Échec du chargement";
		aStrings[1] = L"Le chargement de Minecraft: PlayStation®3 Edition a échoué : impossible de continuer.";
		aStrings[2] = L"Quitter le jeu";
		break;
	case XC_LANGUAGE_ITALIAN:
		aStrings[0] = L"Errore caricamento";
		aStrings[1] = L"Caricamento \"Minecraft: PlayStation®3 Edition\" non riuscito, impossibile continuare.";
		aStrings[2] = L"Esci dal gioco";
		break;
	case XC_LANGUAGE_JAPANESE:
		aStrings[0] = L"ロード エラー";
		aStrings[1] = L"Minecraft  PlayStation®3 版のロードに失敗しました。続行できません";
		aStrings[2] = L"ゲームを終了";
		break;
	case XC_LANGUAGE_KOREAN:
		aStrings[0] = L"불러오기 오류";
		aStrings[1] = L"\"Minecraft: PlayStation®3 Edition\"을 불러오는 중에 오류가 발생하여 계속할 수 없습니다.";
		aStrings[2] = L"게임 나가기";
		break;
	case XC_LANGUAGE_PORTUGUESE:
		// can only tell if it's brazilian when we've read the productcodes.bin, which might have failed, so stick with Portuguese

			aStrings[0] = L"Erro de Carregamento";
			aStrings[1] = L"Não foi possível carregar Minecraft: PlayStation®3 Edition e não é possível continuar.";
			aStrings[2] = L"Sair do Jogo";
		break;
		// can only tell if it's brazilian when we've read the productcodes.bin, which might have failed
/*	case XC_LANGUAGE_BRAZILIAN:
			// Brazilian Portuguese
			aStrings[0] = L"Erro de carregamento";
			aStrings[1] = L"\"Minecraft: PlayStation®3 Edition\" falhou ao carregar e não é possível continuar.";
			aStrings[2] = L"Sair do Jogo";
		break;*/
	case XC_LANGUAGE_RUSSIAN:
		// Brazilian Portuguese
		aStrings[0] = L"Ошибка при загрузке";
		aStrings[1] = L"Не удалось загрузить игру \"Minecraft: PlayStation®3 Edition\". Продолжить загрузку невозможно.";
		aStrings[2] = L"Выйти из игры";
		break;
	case XC_LANGUAGE_TCHINESE:
		aStrings[0] = L"載入錯誤";
		aStrings[1] = L"無法載入「Minecraft: PlayStation®3 Edition」，因此無法繼續。";
		aStrings[2] = L"離開遊戲";
		break;

	// new TU14 languages
	case XC_LANGUAGE_DUTCH:
		aStrings[0] = L"Fout tijdens het laden";
		aStrings[1] = L"Minecraft: PlayStation®3 Edition kan niet worden geladen.";
		aStrings[2] = L"Game afsluiten";
		break;
	case XC_LANGUAGE_FINISH:
		aStrings[0] = L"Latausvirhe";
		aStrings[1] = L"\"Minecraft: PlayStation®3 Edition\" -pelin lataaminen ei onnistunut, eikä sitä voi jatkaa.";
		aStrings[2] = L"Poistu pelistä";
		break;
	case XC_LANGUAGE_SWEDISH:
		aStrings[0] = L"Laddningsfel";
		aStrings[1] = L"\"Minecraft: PlayStation®3 Edition\" kunde inte laddas och kan inte fortsätta.";
		aStrings[2] = L"Avsluta";
		break;
	case XC_LANGUAGE_DANISH:
		aStrings[0] = L"Indlæsningsfejl";
		aStrings[1] = L"\"Minecraft: PlayStation®3 Edition\" kunne ikke blive indlæst og kan derfor ikke fortsætte.";
		aStrings[2] = L"Afslut spil";
		break;
	case XC_LANGUAGE_BNORWEGIAN:
		aStrings[0] = L"Innlastingsfeil";
		aStrings[1] = L"Feil under innlasting av Minecraft: PlayStation®3 Edition. Kan ikke fortsette.";
		aStrings[2] = L"Avslutt spill";
		break;
	case XC_LANGUAGE_POLISH:
		aStrings[0] = L"Błąd wczytywania";
		aStrings[1] = L"Wystąpił błąd podczas wczytywania Minecraft: Edycja PlayStation®3. Dalsze działanie jest niemożliwe.";
		aStrings[2] = L"Wyjdź z gry";
		break;
	case XC_LANGUAGE_TURKISH:
		aStrings[0] = L"Yükleme Hatası";
		aStrings[1] = L"Minecraft: PlayStation®3 Edition yüklenemedi ve devam edemiyor.";
		aStrings[2] = L"Oyundan Çık";
		break;
	case XC_LANGUAGE_LATINAMERICANSPANISH:
		aStrings[0] = L"Error al cargar";
		aStrings[1] = L"Se produjo un error al cargar Minecraft: PlayStation®3 Edition y no es posible continuar.";
		aStrings[2] = L"Salir de la partida";
		break;

	default:
		aStrings[0] = L"Loading Error";
		aStrings[1] = L"\"Minecraft: PlayStation®3 Edition\" has failed to load, and cannot continue.";
		aStrings[2] = L"Exit Game";
		break;
	}
	wszButtons[0] = aStrings[2];

	//MESSAGEBOX_RESULT MessageResult;

	// convert to utf8
	uint8_t u8Message[256];
	size_t srclen,dstlen;
	srclen=256;
	dstlen=256;
	L10nResult lres= UTF16stoUTF8s((uint16_t *)aStrings[1],&srclen,u8Message,&dstlen);

	// 4J-PB - let's have a sleep here, to give any disc eject system message time to come in
	Sleep(250);
	// Get the system events if there are any
	cellSysutilCheckCallback() ;

	if(ShutdownManager::ShouldRun(ShutdownManager::eMainThread)==false)
	{
		app.DebugPrintf("Disc Eject received\n");
		app.DebugPrintf("The Fatal Error message box will possibly fail, so just exit\n");
		ShutdownManager::MainThreadHandleShutdown();
		_Exit(0);
	}

	app.DebugPrintf("Requesting Message Box for Fatal Error\n");
	EMsgBoxResult eResult=InputManager.RequestMessageBox(EMSgBoxType_None,0,NULL,this,(char *)u8Message,0);

	while ((eResult==EMsgBox_Busy) || (eResult==EMsgBox_SysUtilBusy))
	{
		Sleep(250);
		// Get the system events if there are any
		cellSysutilCheckCallback() ;
		// Check if we should shutdown due to a disc eject
		if(!ShutdownManager::ShouldRun(ShutdownManager::eMainThread))
		{
			ShutdownManager::MainThreadHandleShutdown();
			_Exit(0);
		}
		app.DebugPrintf("Requesting Message Box for Fatal Error again due to BUSY\n");
		eResult=InputManager.RequestMessageBox(EMSgBoxType_None,0,NULL,this,(char *)u8Message,0);
	}

	while(1)
	{
		RenderManager.Tick();

		RenderManager.Present();
		Sleep(250);
		// Get the system events if there are any
		cellSysutilCheckCallback() ;

		// Check if we should shutdown due to a disc eject
		if(!ShutdownManager::ShouldRun(ShutdownManager::eMainThread))
		{
			ShutdownManager::MainThreadHandleShutdown();
			_Exit(0);
		}
	}
}

void CConsoleMinecraftApp::CaptureSaveThumbnail()
{
	MemSect(53);
#ifdef __PS3__
	RenderManager.CaptureThumbnail(&m_ThumbnailBuffer,&m_SaveImageBuffer);
#else
	RenderManager.CaptureThumbnail(&m_ThumbnailBuffer);
#endif
	MemSect(0);
}

void CConsoleMinecraftApp::GetSaveThumbnail(PBYTE *ppbThumbnailData,DWORD *pdwThumbnailSize,PBYTE *ppbDataImage,DWORD *pdwSizeImage)
{
	// on a save caused by a create world, the thumbnail capture won't have happened
	if(m_ThumbnailBuffer.Allocated())
	{
		if( ppbThumbnailData )
		{
			*ppbThumbnailData= new BYTE [m_ThumbnailBuffer.GetBufferSize()];
			*pdwThumbnailSize=m_ThumbnailBuffer.GetBufferSize();
			memcpy(*ppbThumbnailData,m_ThumbnailBuffer.GetBufferPointer(),*pdwThumbnailSize);
		}
		m_ThumbnailBuffer.Release();
	}
	else
	{
		if( ppbThumbnailData )
		{
			// use the default image
			StorageManager.GetDefaultSaveThumbnail(ppbThumbnailData,pdwThumbnailSize);
		}
	}

	if(m_SaveImageBuffer.Allocated())
	{
		if( ppbDataImage )
		{
			*ppbDataImage= new BYTE [m_SaveImageBuffer.GetBufferSize()];
			*pdwSizeImage=m_SaveImageBuffer.GetBufferSize();
			memcpy(*ppbDataImage,m_SaveImageBuffer.GetBufferPointer(),*pdwSizeImage);
		}
		m_SaveImageBuffer.Release();
	}
	else
	{
		if( ppbDataImage )
		{
			// use the default image
			StorageManager.GetDefaultSaveImage(ppbDataImage,pdwSizeImage);
		}
	}
}
void CConsoleMinecraftApp::ReleaseSaveThumbnail()
{
	if(m_ThumbnailBuffer.Allocated())
	{
		m_ThumbnailBuffer.Release();
	}
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

LoadSaveDataThreadParam* LoadSaveFromDisk(const wstring& pathName)
{
	File saveFile(pathName);
	int64_t fileSize = saveFile.length();
	FileInputStream fis(saveFile);
	byteArray ba(fileSize);
	fis.read(ba);
	fis.close();

	LoadSaveDataThreadParam *saveData = new LoadSaveDataThreadParam(ba.data, ba.length, saveFile.getName());
	return saveData;
}

void CConsoleMinecraftApp::TemporaryCreateGameStart()
{
	////////////////////////////////////////////////////////////////////////////////////////////// From CScene_Main::OnInit

	app.setLevelGenerationOptions(NULL);

	// From CScene_Main::RunPlayGame
	Minecraft *pMinecraft=Minecraft::GetInstance();
	app.ReleaseSaveThumbnail();
	ProfileManager.SetLockedProfile(0);
	pMinecraft->user->name = L"PS3";
	app.ApplyGameSettingsChanged(0);

	////////////////////////////////////////////////////////////////////////////////////////////// From CScene_MultiGameJoinLoad::OnInit
	MinecraftServer::resetFlags();

	// From CScene_MultiGameJoinLoad::OnNotifyPressEx
	app.SetTutorialMode( false );
	app.SetCorruptSaveDeleted(false);

	////////////////////////////////////////////////////////////////////////////////////////////// From CScene_MultiGameCreate::CreateGame

	app.ClearTerrainFeaturePosition();	wstring wWorldName = L"TestWorld";

	StorageManager.ResetSaveData();
	StorageManager.SetSaveTitle(wWorldName.c_str());

	bool isFlat = false;
	int64_t seedValue = 0;//BiomeSource::findSeed(isFlat?LevelType::lvl_flat:LevelType::lvl_normal);	// 4J - was (new Random())->nextLong() - now trying to actually find a seed to suit our requirements
//	int64_t seedValue =  0xfd97203ebdbf5c6f;
	unsigned int seedLow = (unsigned int )(seedValue & 0xffffffff);
	unsigned int  seedHigh = (unsigned int )(seedValue>>32);
#ifndef _CONTENT_PACKAGE
	printf("----------------------------------------------------------------\n");
	printf("    Seed value - 0x%08x%08x\n", seedHigh, seedLow);
	printf("----------------------------------------------------------------\n");
#endif
	NetworkGameInitData *param = new NetworkGameInitData();
	param->seed = seedValue;
#ifdef SAVE_GAME_TO_LOAD
	param->saveData =  LoadSaveFromDisk(wstring(SAVE_GAME_TO_LOAD));
#else
 	param->saveData =  NULL;
#endif
	app.SetGameHostOption(eGameHostOption_Difficulty,0);
	app.SetGameHostOption(eGameHostOption_FriendsOfFriends,0);
	app.SetGameHostOption(eGameHostOption_Gamertags,1);
	app.SetGameHostOption(eGameHostOption_BedrockFog,1);

	app.SetGameHostOption(eGameHostOption_GameType,GameType::CREATIVE->getId());
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

	g_NetworkManager.HostGame(3, true, false, 8, 0);

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = (LPVOID)param;

	// Reset the autosave time
	app.SetAutosaveTimerTime();

	C4JThread* thread = new C4JThread(loadingParams->func, loadingParams->lpParam, "RunNetworkGame");
	thread->Run();
}

// COMMERCE / DLC

void CConsoleMinecraftApp::CommerceInit()
{
	m_bCommerceCategoriesRetrieved=false;
	m_bCommerceProductListRetrieved=false;
	m_bCommerceInitialised=false;
	m_bProductListAdditionalDetailsRetrieved=false;
	m_pCommerce= new SonyCommerce_PS3;
	m_eCommerce_State=eCommerce_State_Offline; // can only init when we have a PSN user
	m_ProductListRetrievedC=0;
	m_ProductListAdditionalDetailsC=0;
	m_ProductListCategoriesC=0;
	m_iCurrentCategory=0;
	m_iCurrentProduct=0;
	memset(m_pchSkuID,0,48);
}

void CConsoleMinecraftApp::CommerceTick()
{
	// only tick this if the primary user is signed in to the PSN
	if(ProfileManager.IsSignedInLive(0))
	{
		switch(m_eCommerce_State)
		{
		case eCommerce_State_Offline:
			m_eCommerce_State=eCommerce_State_Init;
			break;
		case eCommerce_State_Init:
			m_eCommerce_State=eCommerce_State_Init_Pending;
			m_pCommerce->CreateSession(&CConsoleMinecraftApp::CommerceInitCallback, this);
			break;
		case eCommerce_State_GetCategories:
			m_eCommerce_State=eCommerce_State_GetCategories_Pending;
			// get all categories for this product
			m_pCommerce->GetCategoryInfo(&CConsoleMinecraftApp::CommerceGetCategoriesCallback, this, &m_CategoryInfo,app.GetCommerceCategory());

			break;
		case eCommerce_State_GetProductList:
			{
				m_eCommerce_State=eCommerce_State_GetProductList_Pending;
				SonyCommerce::CategoryInfo *pCategories=app.GetCategoryInfo();
				std::list<SonyCommerce::CategoryInfoSub>::iterator iter = pCategories->subCategories.begin();

				for(int i=0;i<m_ProductListRetrievedC;i++)
				{
					iter++;
				}
				SonyCommerce::CategoryInfoSub category = (SonyCommerce::CategoryInfoSub)(*iter);

				m_pCommerce->GetProductList(&CConsoleMinecraftApp::CommerceGetProductListCallback, this, &m_ProductListA[m_ProductListRetrievedC],category.categoryId);
			}

			break;
		case eCommerce_State_AddProductInfoDetailed:
			{
				m_eCommerce_State=eCommerce_State_AddProductInfoDetailed_Pending;

				// for each of the products in the categories, get the detailed info. We really only need the long description and price info.
				SonyCommerce::CategoryInfo *pCategories=app.GetCategoryInfo();
				std::list<SonyCommerce::CategoryInfoSub>::iterator iter = pCategories->subCategories.begin();
				for(int i=0;i<m_iCurrentCategory;i++)
				{
					iter++;
				}

				SonyCommerce::CategoryInfoSub category = (SonyCommerce::CategoryInfoSub)(*iter);
				std::vector<SonyCommerce::ProductInfo>*pvProductList=&m_ProductListA[m_iCurrentCategory];

				// 4J-PB - there may be no products in the category
				if(pvProductList->size()==0)
				{
					CConsoleMinecraftApp::CommerceAddDetailedProductInfoCallback(this,0);
				}
				else
				{
					assert(pvProductList->size() > m_iCurrentProduct);
					SonyCommerce::ProductInfo *pProductInfo=&(pvProductList->at(m_iCurrentProduct));
					m_pCommerce->AddDetailedProductInfo(&CConsoleMinecraftApp::CommerceAddDetailedProductInfoCallback, this, pProductInfo,pProductInfo->productId,category.categoryId);
				}
			}
			break;
		case eCommerce_State_Checkout:
			m_pCommerce->CreateSession(&CConsoleMinecraftApp::CheckoutSessionStartedCallback, this);
			m_eCommerce_State=eCommerce_State_Checkout_WaitingForSession;
			break;
		case eCommerce_State_Checkout_SessionStarted:
			m_eCommerce_State=eCommerce_State_Checkout_Pending;
			m_pCommerce->Checkout(&CConsoleMinecraftApp::CommerceCheckoutCallback, this,m_pchSkuID);
			break;

		case eCommerce_State_RegisterDLC:
			{
				m_eCommerce_State=eCommerce_State_Online;
				// register the DLC info
				SonyCommerce::CategoryInfo *pCategories=app.GetCategoryInfo();
				std::list<SonyCommerce::CategoryInfoSub>::iterator iter = pCategories->subCategories.begin();
				for(int i=0;i<m_iCurrentCategory;i++)
				{
					std::vector<SonyCommerce::ProductInfo>*pvProductList=&m_ProductListA[i];
					for(int j=0;j<pvProductList->size();j++)
					{
						SonyCommerce::ProductInfo *pProductInfo=&(pvProductList->at(j));
						// just want the final 16 characters of the product id
						RegisterDLCData(&pProductInfo->productId[20],0,pProductInfo->imageUrl);
					}
					iter++;
				}
			}
			break;

		case eCommerce_State_DownloadAlreadyPurchased:
			m_pCommerce->CreateSession(&CConsoleMinecraftApp::DownloadAlreadyPurchasedSessionStartedCallback, this);
			m_eCommerce_State=eCommerce_State_DownloadAlreadyPurchased_WaitingForSession;
			break;
		case eCommerce_State_DownloadAlreadyPurchased_SessionStarted:
			m_eCommerce_State=eCommerce_State_DownloadAlreadyPurchased_Pending;
			m_pCommerce->DownloadAlreadyPurchased(&CConsoleMinecraftApp::CommerceCheckoutCallback, this,m_pchSkuID);
			break;


		case eCommerce_State_UpgradeTrial:
			m_pCommerce->CreateSession(&CConsoleMinecraftApp::UpgradeTrialSessionStartedCallback, this);
			m_eCommerce_State=eCommerce_State_UpgradeTrial_WaitingForSession;
			break;
		case eCommerce_State_UpgradeTrial_SessionStarted:
			m_pCommerce->UpgradeTrial(&CConsoleMinecraftApp::CommerceCheckoutCallback, this);
			m_eCommerce_State=eCommerce_State_UpgradeTrial_Pending;
			break;
		}

		// 4J-PB - bit of a hack to display the full version purchase after signing in during a trial trophy popup
		if(m_bDisplayFullVersionPurchase && ((m_eCommerce_State==eCommerce_State_Online) || (m_eCommerce_State==eCommerce_State_Error)))
		{
			m_bDisplayFullVersionPurchase=false;
			ProfileManager.DisplayFullVersionPurchase(false,ProfileManager.GetPrimaryPad(),eSen_UpsellID_Full_Version_Of_Game);
		}
	}
	else
	{
		// was the primary player signed in and is now signed out?
		if(m_eCommerce_State!=eCommerce_State_Offline)
		{
			m_eCommerce_State=eCommerce_State_Offline;

			// clear out all the product info
			ClearCommerceDetails();

			m_pCommerce->CloseSession();
		}
	}
}

bool CConsoleMinecraftApp::GetCommerceCategoriesRetrieved()
{
	return m_bCommerceCategoriesRetrieved;
}

bool  CConsoleMinecraftApp::GetCommerceProductListRetrieved()
{
	return m_bCommerceProductListRetrieved;
}

bool  CConsoleMinecraftApp::GetCommerceProductListInfoRetrieved()
{
	return m_bProductListAdditionalDetailsRetrieved;
}

SonyCommerce::CategoryInfo *CConsoleMinecraftApp::GetCategoryInfo()
{
	if(m_bCommerceCategoriesRetrieved==false)
	{
		return NULL;
	}

	return &m_CategoryInfo;
}

void CConsoleMinecraftApp::ClearCommerceDetails()
{
	for(int i=0;i<m_ProductListCategoriesC;i++)
	{
		std::vector<SonyCommerce::ProductInfo>* pProductList=&m_ProductListA[i];
		pProductList->clear();
	}

	if(m_ProductListA!=NULL)
	{
		delete [] m_ProductListA;
		m_ProductListA=NULL;
	}

	m_ProductListRetrievedC=0;
	m_ProductListAdditionalDetailsC=0;
	m_ProductListCategoriesC=0;
	m_iCurrentCategory=0;
	m_iCurrentProduct=0;
	m_bCommerceCategoriesRetrieved=false;
	m_bCommerceInitialised=false;
	m_bCommerceProductListRetrieved=false;
	m_bProductListAdditionalDetailsRetrieved=false;

	m_CategoryInfo.subCategories.clear();

}


void CConsoleMinecraftApp::GetDLCSkuIDFromProductList(char * pchDLCProductID, char *pchSkuID)
{

	// find the DLC
	for(int i=0;i<m_ProductListCategoriesC;i++)
	{
		for(int j=0;j<m_ProductListA[i].size();j++)
		{
			std::vector<SonyCommerce::ProductInfo>* pProductList=&m_ProductListA[i];
            auto itEnd = pProductList->end();

            for (auto it = pProductList->begin(); it != itEnd; it++)
            {
				SonyCommerce::ProductInfo Info=*it;
				if(strcmp(pchDLCProductID,Info.productId)==0)
				{
					memcpy(pchSkuID,Info.skuId,SCE_NP_COMMERCE2_SKU_ID_LEN);
					return;
				}
			}
		}
	}
	return;
}

void CConsoleMinecraftApp::Checkout(char *pchSkuID)
{
	if(m_eCommerce_State==eCommerce_State_Online)
	{
		strcpy(m_pchSkuID,pchSkuID);
		m_eCommerce_State=eCommerce_State_Checkout;
	}
}

void CConsoleMinecraftApp::DownloadAlreadyPurchased(char *pchSkuID)
{
	if(m_eCommerce_State==eCommerce_State_Online)
	{
		strcpy(m_pchSkuID,pchSkuID);
		m_eCommerce_State=eCommerce_State_DownloadAlreadyPurchased;
	}
}

bool CConsoleMinecraftApp::UpgradeTrial()
{
	if(m_eCommerce_State==eCommerce_State_Online)
	{
		m_eCommerce_State=eCommerce_State_UpgradeTrial;
		return true;
	}
	else if(m_eCommerce_State==eCommerce_State_Error)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		C4JStorage::EMessageResult result = ui.RequestMessageBox( IDS_PRO_UNLOCKGAME_TITLE, IDS_NO_DLCOFFERS, uiIDA,1,ProfileManager.GetPrimaryPad());
		return true;
	}
	else
	{
		// commerce is busy
		return false;
	}
}

std::vector<SonyCommerce::ProductInfo>* CConsoleMinecraftApp::GetProductList(int iIndex)
{
	if((m_bCommerceProductListRetrieved==false) || (m_bProductListAdditionalDetailsRetrieved==false) )
	{
		return NULL;
	}

	return &m_ProductListA[iIndex];
}

bool CConsoleMinecraftApp::DLCAlreadyPurchased(char *pchTitle)
{
	// find the DLC
	for(int i=0;i<m_ProductListCategoriesC;i++)
	{
		for(int j=0;j<m_ProductListA[i].size();j++)
		{
			std::vector<SonyCommerce::ProductInfo>* pProductList=&m_ProductListA[i];
            auto itEnd = pProductList->end();

            for (auto it = pProductList->begin(); it != itEnd; it++)
            {
				SonyCommerce::ProductInfo Info=*it;
				if(strcmp(pchTitle,Info.skuId)==0)
				{
					if(Info.purchasabilityFlag==SCE_NP_COMMERCE2_SKU_PURCHASABILITY_FLAG_OFF)
					{
						return true;
					}
					else
					{
						return false;
					}
				}
			}
		}
	}
	return false;
}



////////////////////
// Commerce callbacks
/////////////////////
void CConsoleMinecraftApp::CommerceInitCallback(LPVOID lpParam,int err)
{
	CConsoleMinecraftApp *pClass=(CConsoleMinecraftApp *)lpParam;

	if(err==0)
	{
		pClass->m_eCommerce_State=eCommerce_State_GetCategories;
	}
	else
	{
		pClass->m_eCommerce_State=eCommerce_State_Error;
		pClass->m_ProductListCategoriesC=0;
		pClass->m_bCommerceCategoriesRetrieved=true;
	}
}


void CConsoleMinecraftApp::CommerceGetCategoriesCallback(LPVOID lpParam,int err)
{
	CConsoleMinecraftApp *pClass=(CConsoleMinecraftApp *)lpParam;

	if(err==0)
	{
		pClass->m_ProductListCategoriesC=pClass->m_CategoryInfo.countOfSubCategories;
		// allocate the memory for the product info for each categories
		if(pClass->m_CategoryInfo.countOfSubCategories>0)
		{
			pClass->m_ProductListA = (std::vector<SonyCommerce::ProductInfo> *) new std::vector<SonyCommerce::ProductInfo> [pClass->m_CategoryInfo.countOfSubCategories];
			pClass->m_eCommerce_State=eCommerce_State_GetProductList;
		}
		else
		{
			pClass->m_eCommerce_State=eCommerce_State_Online;
		}
	}
	else
	{
		pClass->m_ProductListCategoriesC=0;
		pClass->m_eCommerce_State=eCommerce_State_Error;
	}

	pClass->m_bCommerceCategoriesRetrieved=true;
}

void CConsoleMinecraftApp::CommerceGetProductListCallback(LPVOID lpParam,int err)
{
	CConsoleMinecraftApp *pClass=(CConsoleMinecraftApp *)lpParam;

	if(err==0)
	{
		pClass->m_ProductListRetrievedC++;
		// if we have more info to get, keep going with the next call
		if(pClass->m_ProductListRetrievedC==pClass->m_CategoryInfo.countOfSubCategories)
		{
			// we're done, so now retrieve the additional product details for each product
			pClass->m_eCommerce_State=eCommerce_State_AddProductInfoDetailed;
			pClass->m_bCommerceProductListRetrieved=true;
		}
		else
		{
			pClass->m_eCommerce_State=eCommerce_State_GetProductList;
		}
	}
	else
	{
		pClass->m_eCommerce_State=eCommerce_State_Error;
		pClass->m_bCommerceProductListRetrieved=true;
	}
}

// void CConsoleMinecraftApp::CommerceGetDetailedProductInfoCallback(LPVOID lpParam,int err)
// {
// 	CConsoleMinecraftApp *pScene=(CConsoleMinecraftApp *)lpParam;
//
// 	if(err==0)
// 	{
// 		pScene->m_eCommerce_State=eCommerce_State_Idle;
// 		//pScene->m_bCommerceProductListRetrieved=true;
// 	}
// 	//printf("Callback hit, error 0x%08x\n", err);
//
// }

void CConsoleMinecraftApp::CommerceAddDetailedProductInfoCallback(LPVOID lpParam,int err)
{
	CConsoleMinecraftApp *pClass=(CConsoleMinecraftApp *)lpParam;

	if(err==0)
	{
		// increment the current product counter. When this gets to the end of the products, move to the next category
		pClass->m_iCurrentProduct++;

		std::vector<SonyCommerce::ProductInfo>*pvProductList=&pClass->m_ProductListA[pClass->m_iCurrentCategory];

		// if there are no more products in this category, move to the next category (there may be no products in the category)
		if(pClass->m_iCurrentProduct>=pvProductList->size())
		{
			// MGH - change this to a while loop so we can skip empty categories.
			do
			{
				pClass->m_iCurrentCategory++;
			}while(pClass->m_ProductListA[pClass->m_iCurrentCategory].size() == 0 && pClass->m_iCurrentCategory<pClass->m_ProductListCategoriesC);

			pClass->m_iCurrentProduct=0;
			if(pClass->m_iCurrentCategory==pClass->m_ProductListCategoriesC)
			{
				// there are no more categories, so we're done
				pClass->m_eCommerce_State=eCommerce_State_RegisterDLC;
				pClass->m_bProductListAdditionalDetailsRetrieved=true;
			}
			else
			{
				// continue with the next category
				pClass->m_eCommerce_State=eCommerce_State_AddProductInfoDetailed;
			}
		}
		else
		{
			// continue with the next product
			pClass->m_eCommerce_State=eCommerce_State_AddProductInfoDetailed;
		}
	}
	else
	{
		pClass->m_eCommerce_State=eCommerce_State_Error;
		pClass->m_bProductListAdditionalDetailsRetrieved=true;
		pClass->m_iCurrentProduct=0;
		pClass->m_iCurrentCategory=0;
	}
}

void CConsoleMinecraftApp::CommerceCheckoutCallback(LPVOID lpParam,int err)
{
	CConsoleMinecraftApp *pClass=(CConsoleMinecraftApp *)lpParam;

	if(err==0)
	{
	}
	pClass->m_eCommerce_State=eCommerce_State_Online;
}

void CConsoleMinecraftApp::CheckoutSessionStartedCallback(LPVOID lpParam,int err)
{
	CConsoleMinecraftApp *pClass=(CConsoleMinecraftApp *)lpParam;
	if(err==0)
		pClass->m_eCommerce_State=eCommerce_State_Checkout_SessionStarted;
	else
		pClass->m_eCommerce_State=eCommerce_State_Error;
}

void CConsoleMinecraftApp::DownloadAlreadyPurchasedSessionStartedCallback(LPVOID lpParam,int err)
{
	CConsoleMinecraftApp *pClass=(CConsoleMinecraftApp *)lpParam;
	if(err==0)
		pClass->m_eCommerce_State=eCommerce_State_DownloadAlreadyPurchased_SessionStarted;
	else
		pClass->m_eCommerce_State=eCommerce_State_Error;
}

void CConsoleMinecraftApp::UpgradeTrialSessionStartedCallback(LPVOID lpParam,int err)
{
	CConsoleMinecraftApp *pClass=(CConsoleMinecraftApp *)lpParam;
	if(err==0)
		pClass->m_eCommerce_State=eCommerce_State_UpgradeTrial_SessionStarted;
	else
		pClass->m_eCommerce_State=eCommerce_State_Error;
}


bool CConsoleMinecraftApp::GetTrialFromName(char *pchDLCName)
{
	if(pchDLCName[0]=='T')
	{
		return true;
	}

	return false;
}

eDLCContentType CConsoleMinecraftApp::GetDLCTypeFromName(char *pchDLCName)
{
	char chDLCType[3];

	chDLCType[0]=pchDLCName[1];
	chDLCType[1]=pchDLCName[2];
	chDLCType[2]=0;

	app.DebugPrintf(6,"DLC - %s\n",pchDLCName);

	if(strcmp(chDLCType,"SP")==0)
	{
		 return e_DLC_SkinPack;
	}
	else if(strcmp(chDLCType,"GP")==0)
	{
		return e_DLC_Gamerpics;
	}
	else if(strcmp(chDLCType,"TH")==0)
	{
		return e_DLC_Themes;
	}
	else if(strcmp(chDLCType,"AV")==0)
	{
		return e_DLC_AvatarItems;
	}
	else if(strcmp(chDLCType,"MP")==0)
	{
		return e_DLC_MashupPacks;
	}
	else if(strcmp(chDLCType,"TP")==0)
	{
		return e_DLC_TexturePacks;
	}
	else
	{
		return e_DLC_NotDefined;
	}
}

int CConsoleMinecraftApp::GetiConfigFromName(char *pchName)
{
	char pchiConfig[5];
	int iStrlen=strlen(pchName);
	// last four character of DLC product name are the iConfig value
	pchiConfig[0]=pchName[iStrlen-4];
	pchiConfig[1]=pchName[iStrlen-3];
	pchiConfig[2]=pchName[iStrlen-2];
	pchiConfig[3]=pchName[iStrlen-1];
	pchiConfig[4]=0;

	return atoi(pchiConfig);
}

int CConsoleMinecraftApp::GetiFirstSkinFromName(char *pchName)
{
	char pchiFirstSkin[5];
	int iStrlen=strlen(pchName);
	// last four character of DLC product name are the iConfig value
	// four before that are the first skin id
	pchiFirstSkin[0]=pchName[iStrlen-8];
	pchiFirstSkin[1]=pchName[iStrlen-7];
	pchiFirstSkin[2]=pchName[iStrlen-6];
	pchiFirstSkin[3]=pchName[iStrlen-5];
	pchiFirstSkin[4]=0;

	return atoi(pchiFirstSkin);
}

// void CConsoleMinecraftApp::SetVoiceChatAndUGCRestricted(bool bRestricted)
//{
//	m_bVoiceChatAndUGCRestricted=bRestricted;
//}

// bool CConsoleMinecraftApp::GetVoiceChatAndUGCRestricted(void)
//{
//	return m_bVoiceChatAndUGCRestricted;
//}


int CConsoleMinecraftApp::GetCommerceState()
{
	return m_eCommerce_State;
}

void CConsoleMinecraftApp::SetBootedFromDiscPatch()
{
	m_bBootedFromDiscPatch=true;
}
bool CConsoleMinecraftApp::GetBootedFromDiscPatch()
{
	return m_bBootedFromDiscPatch;
}

void CConsoleMinecraftApp::SetDiscPatchUsrDir(char *chPatchDir)
{
	strcpy(m_usrdirPathBDPatch,chPatchDir);
}
char * CConsoleMinecraftApp::GetDiscPatchUsrDir()
{
	return m_usrdirPathBDPatch;
}

char *PatchFilelist[] =
{
	"PS3/PS3ProductCodes.bin",
	"Common/Media/MediaPS3.arc",
	// patch 7
	"PS3/Sound/Minecraft.msscmp",
	"font/Default.png",
// 	"font/Mojangles_7.png",
// 	"font/Mojangles_11.png",
// 	"/TitleUpdate/res/font/Default.png",
 	"/TitleUpdate/res/font/Mojangles_7.png",
 	"/TitleUpdate/res/font/Mojangles_11.png",

	"music/music/creative1.binka",
	"music/music/creative2.binka",
	"music/music/creative3.binka",
	"music/music/creative4.binka",
	"music/music/creative5.binka",
	"music/music/creative6.binka",
	"music/music/menu1.binka",
	"music/music/menu2.binka",
	"music/music/menu3.binka",
	"music/music/menu4.binka",




	NULL
};

bool CConsoleMinecraftApp::IsFileInPatchList(LPCSTR lpFileName)
{
	int i = 0;
	app.DebugPrintf("*** CConsoleMinecraftApp::IsFileInPatchList\n");
	while(PatchFilelist[i])
	{
#ifndef _CONTENT_PACKAGE
		app.DebugPrintf("*** Patch file? Comparing %s to %s\n",lpFileName,PatchFilelist[i]);
#endif
		if(strcmp(lpFileName,PatchFilelist[i])==0)
		{
			return true;
		}
		i++;
	}
	return false;
}

char * CConsoleMinecraftApp::GetBDUsrDirPath(const char *pchFilename)
{
	char *pchUsrDir;

	DebugPrintf("*** CConsoleMinecraftApp::GetBDUsrDirPath - check %s\n",pchFilename);

	if(IsFileInPatchList(pchFilename))
	{
		// does this file need to be loaded from the patch directory?
		DebugPrintf("*** BufferedImage::BufferedImage - Found a patched file %s\n",pchFilename);

		return GetDiscPatchUsrDir();
	}
	else
	{
		return getUsrDirPath();
	}
}

bool CConsoleMinecraftApp::CheckForEmptyStore(int iPad)
{
	SonyCommerce::CategoryInfo *pCategories=app.GetCategoryInfo();

	bool bEmptyStore=true;
	if(pCategories!=NULL)
	{
		if(pCategories->countOfProducts>0)
		{
			bEmptyStore=false;
		}
		else
		{
			for(int i=0;i<pCategories->countOfSubCategories;i++)
			{
				std::vector<SonyCommerce::ProductInfo>*pvProductInfo=app.GetProductList(i);
				if(pvProductInfo->size()>0)
				{
					bEmptyStore=false;
					break;
				}
			}
		}
	}

	if(bEmptyStore)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		C4JStorage::EMessageResult result = ui.RequestMessageBox( IDS_DOWNLOADABLE_CONTENT_OFFERS, IDS_NO_DLCOFFERS, uiIDA,1,iPad);
	}

	return bEmptyStore;
}
