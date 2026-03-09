
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
#include "..\..\Orbis\Network\SonyCommerce_Orbis.h"
#include "..\..\Minecraft.World\StringHelpers.h"
#include "Network/Orbis_NPToolkit.h"
#include "Orbis\Network\SonyRemoteStorage_Orbis.h"

#include <system_service.h>
#include "..\..\Common\Network\Sony\SonyRemoteStorage.h"
#include <save_data_dialog.h>
#include <error_dialog.h>

#define ORBIS_COMMERCE_ENABLED
CConsoleMinecraftApp app;



CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp()
{
	memset(&m_ThumbnailBuffer,0,sizeof(ImageFileBuffer));
	memset(&m_SaveImageBuffer,0,sizeof(ImageFileBuffer));
// 	memset(&m_ScreenshotBuffer,0,sizeof(ImageFileBuffer));

	memset(&ProductCodes,0,sizeof(PRODUCTCODES));

	m_bVoiceChatAndUGCRestricted=false;
	m_bDisplayFullVersionPurchase=false;
	// #ifdef _DEBUG_MENUS_ENABLED
	// 	debugOverlayCreated = false;
	// #endif

	m_ProductListA=nullptr;

	m_pRemoteStorage = new SonyRemoteStorage_Orbis;

	m_bSaveDataDialogRunning = false;
	m_bOptionsSaveDataDialogRunning=false;
	m_bPatchAvailableDialogRunning = false;
}

void CConsoleMinecraftApp::SetRichPresenceContext(int iPad, int contextId)
{
	ProfileManager.SetRichPresenceContextValue(iPad,CONTEXT_GAME_STATE,contextId);
}

char *CConsoleMinecraftApp::GetProductCode()
{
	return ProductCodes.chProductCode;
}
char *CConsoleMinecraftApp::GetSaveFolderPrefix()
{
	return ProductCodes.chSaveFolderPrefix;
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

    auto it = m_SONYDLCMap.find(wstrTemp);
    if(it == m_SONYDLCMap.end())
	{
		app.DebugPrintf("Couldn't find DLC info for %s\n", pchTitle);
		assert(0);
		return nullptr;
	}
	return it->second;
}

SONYDLC *CConsoleMinecraftApp::GetSONYDLCInfoFromKeyname(char *pchKeyName)
{

    for ( auto& it : m_SONYDLCMap )
    {
		SONYDLC *pDLCInfo=it.second;

		if(strcmp(pDLCInfo->chDLCKeyname,pchKeyName)==0)
		{
			return pDLCInfo;
		}
	}

	return nullptr;
}

#define WRAPPED_READFILE(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped) {if(ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped)==FALSE) { return FALSE;}}
BOOL CConsoleMinecraftApp::ReadProductCodes()
{
	char chDLCTitle[64];

	// 4J-PB - Read the file containing the product codes. This will be different for the SCEE/SCEA/SCEJ builds
	//HANDLE file = CreateFile("orbis/DLCImages/TP01_360x360.png", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	HANDLE file = CreateFile("orbis/PS4ProductCodes.bin", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if( file == INVALID_HANDLE_VALUE )
	{
		DWORD error = GetLastError();
		app.DebugPrintf("Failed to open ProductCodes.bin with error code %d (%x)\n", error, error);
		return FALSE;
	}

	DWORD dwHigh=0;
	DWORD dwFileSize = GetFileSize(file,&dwHigh);

	if(dwFileSize!=0)
	{
		DWORD bytesRead;

		WRAPPED_READFILE(file,ProductCodes.chProductCode,PRODUCT_CODE_SIZE,&bytesRead,nullptr);
		WRAPPED_READFILE(file,ProductCodes.chSaveFolderPrefix,SAVEFOLDERPREFIX_SIZE,&bytesRead,nullptr);
		WRAPPED_READFILE(file,ProductCodes.chCommerceCategory,COMMERCE_CATEGORY_SIZE,&bytesRead,nullptr);
		WRAPPED_READFILE(file,ProductCodes.chTexturePackID,SCE_NP_COMMERCE2_CATEGORY_ID_LEN,&bytesRead,nullptr);
		WRAPPED_READFILE(file,ProductCodes.chUpgradeKey,UPGRADE_KEY_SIZE,&bytesRead,nullptr);
		WRAPPED_READFILE(file,ProductCodes.chSkuPostfix,SKU_POSTFIX_SIZE,&bytesRead,nullptr);

		app.DebugPrintf("ProductCodes.chProductCode %s\n",ProductCodes.chProductCode);
		app.DebugPrintf("ProductCodes.chSaveFolderPrefix %s\n",ProductCodes.chSaveFolderPrefix);
		app.DebugPrintf("ProductCodes.chCommerceCategory %s\n",ProductCodes.chCommerceCategory);
		app.DebugPrintf("ProductCodes.chTexturePackID %s\n",ProductCodes.chTexturePackID);
		app.DebugPrintf("ProductCodes.chUpgradeKey %s\n",ProductCodes.chUpgradeKey);
		app.DebugPrintf("ProductCodes.chSkuPostfix %s\n",ProductCodes.chSkuPostfix);

		// DLC
		unsigned int uiDLC;
		WRAPPED_READFILE(file,&uiDLC,sizeof(int),&bytesRead,nullptr);

		for(unsigned int i=0;i<uiDLC;i++)
		{
			SONYDLC *pDLCInfo= new SONYDLC;
			memset(pDLCInfo,0,sizeof(SONYDLC));
			memset(chDLCTitle,0,64);

			unsigned int uiVal;
			WRAPPED_READFILE(file,&uiVal,sizeof(int),&bytesRead,nullptr);
			WRAPPED_READFILE(file,pDLCInfo->chDLCKeyname,sizeof(char)*uiVal,&bytesRead,nullptr);

			WRAPPED_READFILE(file,&uiVal,sizeof(int),&bytesRead,nullptr);
			WRAPPED_READFILE(file,chDLCTitle,sizeof(char)*uiVal,&bytesRead,nullptr);
			app.DebugPrintf("DLC title %s\n",chDLCTitle);

			WRAPPED_READFILE(file,&pDLCInfo->eDLCType,sizeof(int),&bytesRead,nullptr);

			WRAPPED_READFILE(file,&uiVal,sizeof(int),&bytesRead,nullptr);
			WRAPPED_READFILE(file,pDLCInfo->chDLCPicname,sizeof(char)*uiVal,&bytesRead,nullptr);

			WRAPPED_READFILE(file,&pDLCInfo->iFirstSkin,sizeof(int),&bytesRead,nullptr);
			WRAPPED_READFILE(file,&pDLCInfo->iConfig,sizeof(int),&bytesRead,nullptr);

			// push this into a vector

			wstring wstrTemp=convStringToWstring(chDLCTitle);
			m_SONYDLCMap[wstrTemp]=pDLCInfo;
		}
		CloseHandle(file);
	}

	if(strcmp(ProductCodes.chProductCode, "CUSA00265") == 0)
		ProductCodes.eProductSKU = e_sku_SCEE;
	else if(strcmp(ProductCodes.chProductCode, "CUSA00744") == 0)
		ProductCodes.eProductSKU = e_sku_SCEA;
	else if(strcmp(ProductCodes.chProductCode, "CUSA00283") == 0)
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
	assert(0);
}

void CConsoleMinecraftApp::CaptureSaveThumbnail()
{
	RenderManager.CaptureThumbnail(&m_ThumbnailBuffer,&m_SaveImageBuffer);
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

}

void CConsoleMinecraftApp::GetScreenshot(int iPad,PBYTE *pbData,DWORD *pdwSize)
{

}

int CConsoleMinecraftApp::GetLocalTMSFileIndex(WCHAR *wchTMSFile,bool bFilenameIncludesExtension,eFileExtensionType eEXT)
{
	return -1;
}


int CConsoleMinecraftApp::LoadLocalDLCImages()
{
	// 4J-PB - Any local graphic files for the Minecraft Store?
	unordered_map<wstring, SONYDLC *>*pDLCInfoA=app.GetSonyDLCMap();
    for (auto& it : *pDLCInfoA )
    {
		SONYDLC * pDLCInfo = it.second;

		LoadLocalDLCImage(pDLCInfo);
	}
	return 0;
}

void CConsoleMinecraftApp::FreeLocalDLCImages()
{
	// 4J-PB - Any local graphic files for the Minecraft Store?
	unordered_map<wstring, SONYDLC *>*pDLCInfoA=app.GetSonyDLCMap();
	for( auto& it : *pDLCInfoA )
	{
		SONYDLC * pDLCInfo = it.second;

		if(pDLCInfo->dwImageBytes!=0)
		{
			free(pDLCInfo->pbImageData);
			pDLCInfo->dwImageBytes=0;
			pDLCInfo->pbImageData=nullptr;
		}
	}
}


int CConsoleMinecraftApp::LoadLocalDLCImage(SONYDLC *pDLCInfo)
{
	// load the local file
	char pchFilename[64];

	sprintf(pchFilename,"orbis/DLCImages/%s_360x360.png",pDLCInfo->chDLCPicname);
	// 4J-PB - Read the file containing the product codes. This will be different for the SCEE/SCEA/SCEJ builds
	HANDLE hFile = CreateFile(pchFilename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if( hFile == INVALID_HANDLE_VALUE )
	{
		app.DebugPrintf("Failed to open %s\n", pchFilename);
		return FALSE;
	}

	DWORD dwHigh=0;
	pDLCInfo->dwImageBytes = GetFileSize(hFile,&dwHigh);

	if(pDLCInfo->dwImageBytes!=0)
	{
		DWORD dwBytesRead;
		pDLCInfo->pbImageData=(PBYTE)malloc(pDLCInfo->dwImageBytes);

		if(ReadFile(hFile,pDLCInfo->pbImageData,pDLCInfo->dwImageBytes,&dwBytesRead,nullptr)==FALSE)
		{
			// failed
			free(pDLCInfo->pbImageData);
			pDLCInfo->dwImageBytes=0;
		}
	}

	CloseHandle(hFile);

	return 0;
}

int CConsoleMinecraftApp::LoadLocalTMSFile(char *chTMSFile)
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

void CConsoleMinecraftApp::TemporaryCreateGameStart()
{
	////////////////////////////////////////////////////////////////////////////////////////////// From CScene_Main::OnInit

	app.setLevelGenerationOptions(nullptr);

	// From CScene_Main::RunPlayGame
	Minecraft *pMinecraft=Minecraft::GetInstance();
	app.ReleaseSaveThumbnail();
	ProfileManager.SetLockedProfile(0);
	pMinecraft->user->name = L"Orbis";
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
	param->saveData = nullptr;

	app.SetGameHostOption(eGameHostOption_Difficulty,0);
	app.SetGameHostOption(eGameHostOption_FriendsOfFriends,0);
	app.SetGameHostOption(eGameHostOption_Gamertags,1);
	app.SetGameHostOption(eGameHostOption_BedrockFog,1);

	app.SetGameHostOption(eGameHostOption_GameType,GameType::SURVIVAL->getId());
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





// COMMERCE / DLC

void CConsoleMinecraftApp::CommerceInit()
{
	m_bCommerceCategoriesRetrieved=false;
	m_bCommerceProductListRetrieved=false;
	m_bCommerceInitialised=false;
	m_bProductListAdditionalDetailsRetrieved=false;
#ifdef ORBIS_COMMERCE_ENABLED
	m_pCommerce= new SonyCommerce_Orbis;
#endif
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
#ifdef ORBIS_COMMERCE_ENABLED
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
#endif // ORBIS_COMMERCE_ENABLED
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

#ifdef ORBIS_COMMERCE_ENABLED
SonyCommerce::CategoryInfo *CConsoleMinecraftApp::GetCategoryInfo()
{
	if(m_bCommerceCategoriesRetrieved==false)
	{
		return nullptr;
	}

	return &m_CategoryInfo;
}
#endif

void CConsoleMinecraftApp::ClearCommerceDetails()
{
#ifdef ORBIS_COMMERCE_ENABLED
	for(int i=0;i<m_ProductListCategoriesC;i++)
	{
		std::vector<SonyCommerce::ProductInfo>* pProductList=&m_ProductListA[i];
		pProductList->clear();
	}

	if(m_ProductListA!=nullptr)
	{
		delete [] m_ProductListA;
		m_ProductListA=nullptr;
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
#endif // #ifdef ORBIS_COMMERCE_ENABLED

}


void CConsoleMinecraftApp::GetDLCSkuIDFromProductList(char * pchDLCProductID, char *pchSkuID)
{
#ifdef ORBIS_COMMERCE_ENABLED

	// find the DLC
	for(int i=0;i<m_ProductListCategoriesC;i++)
	{
		for(size_t j=0;j<m_ProductListA[i].size();j++)
		{
			std::vector<SonyCommerce::ProductInfo>* pProductList=&m_ProductListA[i];
			for ( SonyCommerce::ProductInfo& : *pProductList )
			{
				if(strcmp(pchDLCProductID,Info.productId)==0)
				{
					memcpy(pchSkuID,Info.skuId,SCE_NP_COMMERCE2_SKU_ID_LEN);
					return;
				}
			}
		}
	}
	return;
#endif // #ifdef ORBIS_COMMERCE_ENABLED

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

#ifdef ORBIS_COMMERCE_ENABLED
std::vector<SonyCommerce::ProductInfo>* CConsoleMinecraftApp::GetProductList(int iIndex)
{
	if((m_bCommerceProductListRetrieved==false) || (m_bProductListAdditionalDetailsRetrieved==false) )
	{
		return nullptr;
	}

	return &m_ProductListA[iIndex];
}
#endif // #ifdef ORBIS_COMMERCE_ENABLED

bool CConsoleMinecraftApp::DLCAlreadyPurchased(char *pchTitle)
{
#ifdef ORBIS_COMMERCE_ENABLED
	// purchasability flag is not return on PS4
	return false;
#endif // #ifdef ORBIS_COMMERCE_ENABLED

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
#ifdef ORBIS_COMMERCE_ENABLED
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
#endif // #ifdef ORBIS_COMMERCE_ENABLED

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
#ifdef ORBIS_COMMERCE_ENABLED
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

#endif	//#ifdef ORBIS_COMMERCE_ENABLED

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


void CConsoleMinecraftApp::SystemServiceTick()
{
	SceSystemServiceStatus status;
	SceSystemServiceEvent event;

	int ret = sceSystemServiceGetStatus(&status);
	if ((ret == SCE_OK) && (status.eventNum > 0))
	{
		for (int i = 0; i < status.eventNum; i++)
		{
			ret = sceSystemServiceReceiveEvent(&event);
			if (ret == SCE_OK)
			{
				switch(event.eventType)
				{
				case SCE_SYSTEM_SERVICE_EVENT_GAME_CUSTOM_DATA:
				{
					OrbisNPToolkit::getMessageData((SceNpGameCustomDataEventParam*)event.data.param);
					// Processing after invitation
					//SceNpSessionInvitationEventParam* pInvite = (SceNpSessionInvitationEventParam*)event.data.param;
					//SQRNetworkManager_Orbis::GetInviteDataAndProcess(pInvite);
					break;
				}
				case SCE_SYSTEM_SERVICE_EVENT_ON_RESUME:
					// Resume means that the user signed out (but came back), sensible thing to do is exit to main menu
					app.SetAction(0, eAppAction_ExitWorld);
					break;
				case SCE_SYSTEM_SERVICE_EVENT_SESSION_INVITATION:
				case SCE_SYSTEM_SERVICE_EVENT_GAME_LIVE_STREAMING_STATUS_UPDATE:
					break;
				case SCE_SYSTEM_SERVICE_EVENT_ENTITLEMENT_UPDATE:
					app.GetCommerce()->CheckForTrialUpgradeKey();
					// clear the DLC installed and check again
					app.ClearDLCInstalled();
					ui.HandleDLCInstalled(0);
					break;
				case SCE_SYSTEM_SERVICE_EVENT_DISPLAY_SAFE_AREA_UPDATE:
				case SCE_SYSTEM_SERVICE_EVENT_URL_OPEN:
				case SCE_SYSTEM_SERVICE_EVENT_LAUNCH_APP:
				default:
					break;

				}
			}
		}
	}
}

void CConsoleMinecraftApp::SaveDataDialogTick()
{
	if(m_bOptionsSaveDataDialogRunning)
	{
		SceCommonDialogStatus status = sceSaveDataDialogUpdateStatus();
		if( status == SCE_COMMON_DIALOG_STATUS_FINISHED )
		{
			SceSaveDataDialogResult result;
			memset(&result, 0, sizeof(result));
			int ret = sceSaveDataDialogGetResult(&result);
			m_bOptionsSaveDataDialogRunning = false;
			ret = sceSaveDataDialogTerminate();

			// R4099 doesn't say we need give the user a delete dialog to create space, so we won't
		}
	}
	else if( m_bSaveDataDialogRunning )
	{
		SceCommonDialogStatus status = sceSaveDataDialogUpdateStatus();
		if( status == SCE_COMMON_DIALOG_STATUS_FINISHED || status == SCE_COMMON_DIALOG_STATUS_NONE )
		{
			SceSaveDataDialogResult result;
			memset(&result, 0, sizeof(result));
			int ret = sceSaveDataDialogGetResult(&result);
			m_bSaveDataDialogRunning = false;
			ret = sceSaveDataDialogTerminate();

			UINT uiIDA[3];
			uiIDA[0]=IDS_SAVE_INCOMPLETE_RETRY_SAVING;
			uiIDA[1]=IDS_SAVE_INCOMPLETE_DISABLE_SAVING;
			uiIDA[2]=IDS_SAVE_INCOMPLETE_DELETE_SAVES;

			int message;
			if( m_eSaveIncompleteType == C4JStorage::ESaveIncomplete_OutOfQuota )
			{
				message = IDS_SAVE_INCOMPLETE_EXPLANATION_QUOTA;
			}
			else
			{
				message = IDS_SAVE_INCOMPLETE_EXPLANATION_LOCAL_STORAGE;
			}

			if( ui.RequestMessageBox( IDS_SAVE_INCOMPLETE_TITLE, message, uiIDA,3,XUSER_INDEX_ANY,Callback_SaveGameIncompleteMessageBoxReturned,this, app.GetStringTable()) == C4JStorage::EMessage_Busy)
			{
				// If this failed to display, continue as if we cancelled. This isn't ideal, but the user should already have had some system notification of being out of memory,
				// and if we instantly retry then they may not be able to navigate whatever other error is blocking this from appearing
				Callback_SaveGameIncompleteMessageBoxReturned(this, 0, C4JStorage::EMessage_Cancelled);
			}
		}
	}
}


void CConsoleMinecraftApp::Callback_SaveGameIncomplete(void *pParam, C4JStorage::ESaveIncompleteType saveIncompleteType, int blocksRequired)
{
	CConsoleMinecraftApp* pClass = (CConsoleMinecraftApp*)pParam;

	SceSaveDataDialogParam param;
	SceSaveDataDialogSystemMessageParam sysParam;
	SceSaveDataDialogItems items;
	SceSaveDataDirName dirName;

	sceSaveDataDialogParamInitialize(&param);
	param.mode = SCE_SAVE_DATA_DIALOG_MODE_SYSTEM_MSG;
	param.dispType = SCE_SAVE_DATA_DIALOG_TYPE_SAVE;
	memset(&sysParam,0,sizeof(sysParam));
	param.sysMsgParam = &sysParam;
	param.sysMsgParam->sysMsgType = SCE_SAVE_DATA_DIALOG_SYSMSG_TYPE_NOSPACE_CONTINUABLE;
	param.sysMsgParam->value = blocksRequired;
	memset(&items, 0, sizeof(items));
	param.items = &items;

	SceUserServiceUserId userId;
	int ret = sceUserServiceGetInitialUser(&userId);

	param.items->userId = userId;

	ret = sceSaveDataDialogInitialize();
	ret = sceSaveDataDialogOpen(&param);

	pClass->m_bSaveDataDialogRunning = true;
	pClass->m_eSaveIncompleteType = saveIncompleteType;

	StorageManager.SetSaveDisabled(true);
	pClass->EnterSaveNotificationSection();
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
			ui.NavigateToScene(iPad, eUIScene_InGameSaveManagementMenu, nullptr, eUILayer_Error, eUIGroup_Fullscreen);
			break;
	}
	return 0;
}

bool CConsoleMinecraftApp::CheckForEmptyStore(int iPad)
{
	SonyCommerce::CategoryInfo *pCategories=app.GetCategoryInfo();

	bool bEmptyStore=true;
	if(pCategories!=nullptr)
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
		ProfileManager.DisplaySystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_EMPTY_STORE, iPad );
	}

	return bEmptyStore;
}

void CConsoleMinecraftApp::ShowPatchAvailableError()
{
	int32_t ret=sceErrorDialogInitialize();
	if (  ret==SCE_OK )
	{
		m_bPatchAvailableDialogRunning = true;

		SceErrorDialogParam param;
		sceErrorDialogParamInitialize( &param );
		// 4J-PB - We want to display the option to get the patch now
		param.errorCode = SCE_NP_ERROR_LATEST_PATCH_PKG_DOWNLOADED;
		ret = sceUserServiceGetInitialUser( &param.userId );
		if ( ret == SCE_OK )
		{
			ret=sceErrorDialogOpen( &param );
		}
	}
}

void CConsoleMinecraftApp::PatchAvailableDialogTick()
{
	if(m_bPatchAvailableDialogRunning)
	{
		SceErrorDialogStatus stat = sceErrorDialogUpdateStatus();
		if( stat == SCE_ERROR_DIALOG_STATUS_FINISHED )
		{
			sceErrorDialogTerminate();

			UINT uiIDA[1];
			uiIDA[0]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION_PATCH_AVAILABLE, uiIDA, 1, ProfileManager.GetPrimaryPad(), nullptr, nullptr, app.GetStringTable());
			m_bPatchAvailableDialogRunning=false;
		}
	}
}