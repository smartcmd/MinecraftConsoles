
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
#include "..\..\Minecraft.World\StringHelpers.h"
#include "PSVita\Network\SonyRemoteStorage_Vita.h"
#include "PSVita\Network\SonyCommerce_Vita.h"
#include "..\..\Common\Network\Sony\SonyRemoteStorage.h"
#include "PSVita/Network/PSVita_NPToolkit.h"
#include <message_dialog.h>
#include <savedata_dialog.h>
#include "Common\UI\UI.h"
#include "PSVita\PSVitaExtras\PSVitaStrings.h"

#define VITA_COMMERCE_ENABLED
CConsoleMinecraftApp app;

CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp()
{
	memset(&m_ThumbnailBuffer,0,sizeof(ImageFileBuffer));
	memset(&m_SaveImageBuffer,0,sizeof(ImageFileBuffer));
	memset(&ProductCodes,0,sizeof(PRODUCTCODES));

	m_bVoiceChatAndUGCRestricted=false;
	m_bDisplayFullVersionPurchase=false;

	m_ProductListA=NULL;

	m_pRemoteStorage = new SonyRemoteStorage_Vita;

	m_bSaveIncompleteDialogRunning = false;
	m_bSaveDataDeleteDialogState = eSaveDataDeleteState_idle;

	m_pSaveToDelete = NULL;
	m_pCheckoutProductInfo = NULL;
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
	return NULL;	// ProductCodes.chTexturePackID;
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
		return NULL;
	}
	return it->second;

	/*wstring wstrTemp=convStringToWstring(pchTitle);
	SONYDLC *pTemp=m_SONYDLCMap.at(wstrTemp);

	return pTemp;*/
}

SONYDLC *CConsoleMinecraftApp::GetSONYDLCInfo(int iTexturePackID)
{
    for (auto it = m_SONYDLCMap.begin(); it != m_SONYDLCMap.end(); ++it)
    {
		if(it->second->iConfig == iTexturePackID)
			return it->second;
	}
	return NULL;
}


#define WRAPPED_READFILE(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped) {if(ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped)==FALSE) { return FALSE;}}
BOOL CConsoleMinecraftApp::ReadProductCodes()
{
	char chDLCTitle[64];

	// 4J-PB - Read the file containing the product codes. This will be different for the SCEE/SCEA/SCEJ builds
	HANDLE file = CreateFile("PSVita/PSVitaProductCodes.bin", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

		WRAPPED_READFILE(file,ProductCodes.chProductCode,PRODUCT_CODE_SIZE,&bytesRead,NULL);
		WRAPPED_READFILE(file,ProductCodes.chSaveFolderPrefix,SAVEFOLDERPREFIX_SIZE,&bytesRead,NULL);
		//WRAPPED_READFILE(file,ProductCodes.chDiscSaveFolderPrefix,SAVEFOLDERPREFIX_SIZE,&bytesRead,NULL);
		WRAPPED_READFILE(file,ProductCodes.chCommerceCategory,COMMERCE_CATEGORY_SIZE,&bytesRead,NULL);
		//WRAPPED_READFILE(file,ProductCodes.chTexturePackID,SCE_NP_COMMERCE2_CATEGORY_ID_LEN,&bytesRead,NULL);		// TODO
		WRAPPED_READFILE(file,ProductCodes.chUpgradeKey,UPGRADE_KEY_SIZE,&bytesRead,NULL);
		//WRAPPED_READFILE(file,ProductCodes.chSkuPostfix,SKU_POSTFIX_SIZE,&bytesRead,NULL);

		app.DebugPrintf("ProductCodes.chProductCode %s\n",ProductCodes.chProductCode);
		app.DebugPrintf("ProductCodes.chSaveFolderPrefix %s\n",ProductCodes.chSaveFolderPrefix);
		//app.DebugPrintf("ProductCodes.chDiscSaveFolderPrefix %s\n",ProductCodes.chDiscSaveFolderPrefix);
		app.DebugPrintf("ProductCodes.chCommerceCategory %s\n",ProductCodes.chCommerceCategory);
		//app.DebugPrintf("ProductCodes.chTexturePackID %s\n",ProductCodes.chTexturePackID);
		app.DebugPrintf("ProductCodes.chUpgradeKey %s\n",ProductCodes.chUpgradeKey);
		//app.DebugPrintf("ProductCodes.chSkuPostfix %s\n",ProductCodes.chSkuPostfix);

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

	if(strcmp(ProductCodes.chProductCode, "PCSB00560") == 0)
		ProductCodes.eProductSKU = e_sku_SCEE;
	else if(strcmp(ProductCodes.chProductCode, "PCSE00491") == 0)
		ProductCodes.eProductSKU = e_sku_SCEA;
	else if(strcmp(ProductCodes.chProductCode, "PCSG00302") == 0)
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
	RenderManager.CaptureThumbnail(&m_ThumbnailBuffer);
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

	app.setLevelGenerationOptions(NULL);

	// From CScene_Main::RunPlayGame
	Minecraft *pMinecraft=Minecraft::GetInstance();
	app.ReleaseSaveThumbnail();
	ProfileManager.SetLockedProfile(0);
	pMinecraft->user->name = L"Vita";
	app.ApplyGameSettingsChanged(0);

	////////////////////////////////////////////////////////////////////////////////////////////// From CScene_MultiGameJoinLoad::OnInit
	MinecraftServer::resetFlags();

	// From CScene_MultiGameJoinLoad::OnNotifyPressEx
	app.SetTutorialMode( false );
	app.SetCorruptSaveDeleted(false);

	////////////////////////////////////////////////////////////////////////////////////////////// From CScene_MultiGameCreate::CreateGame

	app.ClearTerrainFeaturePosition();
	wstring wWorldName = L"TestWorld";

	bool isFlat = false;
	__int64 seedValue = 0;//BiomeSource::findSeed(isFlat?LevelType::lvl_flat:LevelType::lvl_normal);	// 4J - was (new Random())->nextLong() - now trying to actually find a seed to suit our requirements

	NetworkGameInitData *param = new NetworkGameInitData();
	param->seed = seedValue;
	param->saveData = NULL;

	g_NetworkManager.HostGame(0,false,true,MINECRAFT_NET_MAX_PLAYERS,0);

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

	app.SetGameHostOption(eGameHostOption_MobGriefing, 1 );
	app.SetGameHostOption(eGameHostOption_KeepInventory, 0 );
	app.SetGameHostOption(eGameHostOption_DoMobSpawning, 1 );
	app.SetGameHostOption(eGameHostOption_DoMobLoot, 1 );
	app.SetGameHostOption(eGameHostOption_DoTileDrops, 1 );
	app.SetGameHostOption(eGameHostOption_NaturalRegeneration, 1 );
	app.SetGameHostOption(eGameHostOption_DoDaylightCycle, 1 );

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
#ifdef VITA_COMMERCE_ENABLED
	m_pCommerce= new SonyCommerce_Vita;
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
#ifdef VITA_COMMERCE_ENABLED
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
			((SonyCommerce_Vita*)m_pCommerce)->Checkout(&CConsoleMinecraftApp::CommerceCheckoutCallback, this,m_pCheckoutProductInfo);
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
#endif // VITA_COMMERCE_ENABLED
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

#ifdef VITA_COMMERCE_ENABLED
SonyCommerce::CategoryInfo *CConsoleMinecraftApp::GetCategoryInfo()
{
	if(m_bCommerceCategoriesRetrieved==false)
	{
		return NULL;
	}

	return &m_CategoryInfo;
}
#endif

void CConsoleMinecraftApp::ClearCommerceDetails()
{
#ifdef VITA_COMMERCE_ENABLED
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
#endif // #ifdef VITA_COMMERCE_ENABLED

}


void CConsoleMinecraftApp::GetDLCSkuIDFromProductList(char * pchDLCProductID, char *pchSkuID)
{
#ifdef VITA_COMMERCE_ENABLED

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
#endif // #ifdef VITA_COMMERCE_ENABLED

}

void CConsoleMinecraftApp::Checkout(char *pchSkuID)
{
	SonyCommerce::ProductInfo* productInfo = NULL;

	for(int i=0;i<m_ProductListCategoriesC;i++)
	{
		for(int j=0;j<m_ProductListA[i].size();j++)
		{
			std::vector<SonyCommerce::ProductInfo>* pProductList=&m_ProductListA[i];
            auto itEnd = pProductList->end();

            for (auto it = pProductList->begin(); it != itEnd; it++)
            {
				SonyCommerce::ProductInfo Info=*it;
				if(strcmp(pchSkuID,Info.skuId)==0)
				{
					productInfo = &(*it);
					break;
				}
			}
		}
	}

	if(productInfo)
	{
		if(m_eCommerce_State==eCommerce_State_Online)
		{
			strcpy(m_pchSkuID,productInfo->skuId);
			m_pCheckoutProductInfo = productInfo;
			m_eCommerce_State=eCommerce_State_Checkout;
		}
	}
	else
	{
		assert(0);
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
		C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_PRO_UNLOCKGAME_TITLE, IDS_NO_DLCOFFERS, uiIDA,1,ProfileManager.GetPrimaryPad());
		return true;
	}
	else
	{
		// commerce is busy
		return false;
	}
}

#ifdef VITA_COMMERCE_ENABLED
std::vector<SonyCommerce::ProductInfo>* CConsoleMinecraftApp::GetProductList(int iIndex)
{
	if((m_bCommerceProductListRetrieved==false) || (m_bProductListAdditionalDetailsRetrieved==false) )
	{
		return NULL;
	}

	return &m_ProductListA[iIndex];
}
#endif // #ifdef VITA_COMMERCE_ENABLED

bool CConsoleMinecraftApp::DLCAlreadyPurchased(char *pchTitle)
{
#ifdef VITA_COMMERCE_ENABLED
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
					if(Info.purchasabilityFlag==SCE_TOOLKIT_NP_COMMERCE_NOT_PURCHASED)
					{
						return false;
					}
					else
					{
						return true;
					}
				}
			}
		}
	}
#endif //#ifdef VITA_COMMERCE_ENABLED
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
#ifdef VITA_COMMERCE_ENABLED
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
#endif // #ifdef VITA_COMMERCE_ENABLED

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
#ifdef VITA_COMMERCE_ENABLED
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

#endif	//#ifdef VITA_COMMERCE_ENABLED

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

// bool g_bCalledJoin = false;
void CConsoleMinecraftApp::AppEventTick()
{
	int res = SCE_OK;
	SceAppMgrAppState appStatus;
	res = sceAppMgrGetAppState(&appStatus);

// 	if(!g_bCalledJoin)
// 	{
// 		SceAppUtilNpBasicJoinablePresenceParam joinParam = {0};
// 		strcpy(joinParam.npId.handle.data, "Mark4J");
// 		SQRNetworkManager_Vita::GetJoinablePresenceDataAndProcess(&joinParam);
// 	}
	if(res == SCE_OK)
	{
		if (appStatus.appEventNum > 0)
		{
			SceAppUtilAppEventParam	eventParam;
			memset(&eventParam, 0, sizeof(SceAppUtilAppEventParam));
			res = sceAppUtilReceiveAppEvent(&eventParam);
			if (res == SCE_OK)
			{
				if (SCE_APPUTIL_APPEVENT_TYPE_NP_APP_DATA_MESSAGE == eventParam.type)
				{
					PSVitaNPToolkit::getMessageData(&eventParam);
					//				Messaging::Interface::retrieveMessageAttachment(&eventParam,&s_attachment);
				}
				else if(SCE_APPUTIL_APPEVENT_TYPE_NP_BASIC_JOINABLE_PRESENCE == eventParam.type)
				{
					PSVitaNPToolkit::getMessageData(&eventParam);
				}
				else
				{
					app.DebugPrintf("unknown app event : 0x%08x\n", eventParam.type);
					assert(0);
				}
			}
		}

		if (appStatus.systemEventNum > 0)
		{
			SceAppMgrSystemEvent	systemEvent;
			memset(&systemEvent, 0, sizeof(SceAppMgrSystemEvent));
			res = sceAppMgrReceiveSystemEvent( &systemEvent );
			if (res == SCE_OK)
			{
				switch(systemEvent.systemEvent)
				{
				case SCE_APPMGR_SYSTEMEVENT_ON_STORE_PURCHASE:
					SonyCommerce_Vita::checkBackgroundDownloadStatus();
					break;
				case SCE_APPMGR_SYSTEMEVENT_ON_RESUME:
					app.DebugPrintf("SCE_APPMGR_SYSTEMEVENT_ON_RESUME event received\n");
					break;

				case SCE_APPMGR_SYSTEMEVENT_ON_NP_MESSAGE_ARRIVED:
					app.DebugPrintf("SCE_APPMGR_SYSTEMEVENT_ON_NP_MESSAGE_ARRIVED event received\n");
					break;
				case SCE_APPMGR_SYSTEMEVENT_ON_STORE_REDEMPTION:
					app.DebugPrintf("SCE_APPMGR_SYSTEMEVENT_ON_STORE_REDEMPTION event received\n");
					break;
				default:
					app.DebugPrintf("unknown sys event : 0x%08x\n", systemEvent.systemEvent);
					assert(0);
					break;
				}
			}
		}
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
		ProfileManager.ShowSystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_EMPTY_STORE, iPad );
	}

	return bEmptyStore;
}


void printSaveState()
{
#ifndef _CONTENT_PACKAGE
	string strState;
	switch (StorageManager.GetSaveState())
	{
	case C4JStorage::ESaveGame_Idle:								strState = "ESaveGame_Idle"; break;
	case C4JStorage::ESaveGame_Save:								strState = "ESaveGame_Save"; break;
	case C4JStorage::ESaveGame_SaveCompleteSuccess:					strState = "ESaveGame_SaveCompleteSuccess"; break;
	case C4JStorage::ESaveGame_SaveCompleteFail:					strState = "ESaveGame_SaveCompleteFail"; break;
	case C4JStorage::ESaveGame_SaveIncomplete:						strState = "ESaveGame_SaveIncomplete"; break;
	case C4JStorage::ESaveGame_SaveIncomplete_WaitingOnResponse:	strState = "ESaveGame_SaveIncomplete_WaitingOnResponse"; break;
	case C4JStorage::ESaveGame_Load:								strState = "ESaveGame_Load"; break;
	case C4JStorage::ESaveGame_LoadCompleteSuccess:					strState = "ESaveGame_LoadCompleteSuccess"; break;
	case C4JStorage::ESaveGame_LoadCompleteFail:					strState = "ESaveGame_LoadCompleteFail"; break;
	case C4JStorage::ESaveGame_Delete:								strState = "ESaveGame_Delete"; break;
	case C4JStorage::ESaveGame_DeleteSuccess:						strState = "ESaveGame_DeleteSuccess"; break;
	case C4JStorage::ESaveGame_DeleteFail:							strState = "ESaveGame_DeleteFail"; break;
	case C4JStorage::ESaveGame_Rename:								strState = "ESaveGame_Rename"; break;
	case C4JStorage::ESaveGame_RenameSuccess:						strState = "ESaveGame_RenameSuccess"; break;
	case C4JStorage::ESaveGame_RenameFail:							strState = "ESaveGame_RenameFail"; break;
	case C4JStorage::ESaveGame_GetSaveThumbnail:					strState = "ESaveGame_GetSaveThumbnail"; break;
	case C4JStorage::ESaveGame_GetSaveInfo:							strState = "ESaveGame_GetSaveInfo"; break;
	case C4JStorage::ESaveGame_SaveCache:							strState = "ESaveGame_SaveCache"; break;
	case C4JStorage::ESaveGame_ReconstructCache:					strState = "ESaveGame_ReconstructCache"; break;
	}

	app.DebugPrintf("[printSaveState] GetSaveState == %s.\n", strState.c_str());
#endif
}


void CConsoleMinecraftApp::SaveDataTick()
{
	//CD - We must check the savedata for odd failures that require messages
	//CD - This is based on the Orbis and Durango code and solves TRC issue

	//Are there any errors?
	//SaveData?

	if (m_bSaveIncompleteDialogRunning)
	{
		updateSaveIncompleteDialog();
		return;
	}

	if (sceSaveDataDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_NONE)
	{
		updateSaveDataDeleteDialog();
	}

	switch (m_bSaveDataDeleteDialogState)
	{
	case eSaveDataDeleteState_idle:
		return;

	case eSaveDataDeleteState_waitingForUser:
	case eSaveDataDeleteState_userConfirmation:
	case eSaveDataDeleteState_deleting:
		return;

	case eSaveDataDeleteState_abort:
	case eSaveDataDeleteState_continue:
		{
			C4JStorage::ESaveGameState eGameState = StorageManager.GetSaveState();
			printSaveState();

			if (eGameState == C4JStorage::ESaveGame_SaveIncomplete_WaitingOnResponse)
			{
				if (m_bSaveDataDeleteDialogState == eSaveDataDeleteState_abort)
				{
					app.DebugPrintf("[SaveDataTick] eSaveDataDeleteState_abort.\n");
					StorageManager.CancelIncompleteOperation();
				}
				else if (m_bSaveDataDeleteDialogState == eSaveDataDeleteState_continue)
				{
					app.DebugPrintf("[SaveDataTick] eSaveDataDeleteState_continue.\n");
					StorageManager.ContinueIncompleteOperation();
				}
			}
			else if (eGameState == C4JStorage::ESaveGame_Idle)
			{
				app.DebugPrintf("[SaveDataTick] Storage Manager is idle, SaveDataDialog reverting to idle state too.\n");
			}

			m_bSaveDataDeleteDialogState = eSaveDataDeleteState_idle;
		}
		return;
	}


#if 0
	C4JStorage::ESaveIncompleteType errorType = StorageManager.GetSaveError();
	if (errorType == C4JStorage::ESaveIncomplete_None) errorType = StorageManager.GetOptionsSaveError();

	if (errorType == C4JStorage::ESaveIncomplete_OutOfQuota)
	{
		initSaveDataDeleteDialog();
	}
	else if (errorType == C4JStorage::ESaveIncomplete_OutOfLocalStorage)
	{
		//initSaveIncompleteDialog(1);
	}
	else if (errorType != C4JStorage::ESaveIncomplete_None)
	{
		app.DebugPrintf("[SaveDataTick] Unknown save error from StorageManager.\n");
	}


	//TRC - Quota Failure
	if( errorType == C4JStorage::ESaveIncomplete_OutOfQuota )
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		C4JStorage::EMessageResult res = ui.RequestErrorMessage( IDS_SAVE_INCOMPLETE_TITLE, IDS_SAVE_INCOMPLETE_EXPLANATION_QUOTA, uiIDA, 1, ProfileManager.GetPrimaryPad());
		if( res != C4JStorage::EMessage_Busy )
		{
			//Clear the error now it's been dealt with
			StorageManager.ClearSaveError();
			StorageManager.ClearOptionsSaveError();
		}
	}
#endif
}

void CConsoleMinecraftApp::Callback_SaveGameIncomplete(void *pParam, C4JStorage::ESaveIncompleteType saveIncompleteType, int blocksRequired)
{
	app.DebugPrintf(
		"[Callback_SaveGameIncomplete] saveIncompleteType=%i, blocksRequired=%i,\n",
		saveIncompleteType, blocksRequired
		);

	if (saveIncompleteType == C4JStorage::ESaveIncomplete_OutOfQuota || saveIncompleteType == C4JStorage::ESaveIncomplete_OutOfLocalStorage)
	{
		if(UIScene_LoadOrJoinMenu::isSaveTransferRunning())
		{
			// 4J MGH - if we're trying to save from the save transfer stuff, only show "ok", and we won't try to save again
			if(saveIncompleteType == C4JStorage::ESaveIncomplete_OutOfQuota) blocksRequired = -1;
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
			C4JStorage::EMessageResult res = ui.RequestErrorMessage( IDS_SAVE_INCOMPLETE_TITLE, IDS_SAVE_INCOMPLETE_EXPLANATION_QUOTA, uiIDA, 1, ProfileManager.GetPrimaryPad(), &NoSaveSpaceReturned, (void *)blocksRequired);

		}
		else
		{
			// 4J Stu - If it's quota then we definitely have to delete our saves, so don't show the system UI for this case
			if(saveIncompleteType == C4JStorage::ESaveIncomplete_OutOfQuota) blocksRequired = -1;
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			C4JStorage::EMessageResult res = ui.RequestErrorMessage( IDS_SAVE_INCOMPLETE_TITLE, IDS_SAVE_INCOMPLETE_EXPLANATION_QUOTA, uiIDA, 2, ProfileManager.GetPrimaryPad(), &NoSaveSpaceReturned, (void *)blocksRequired);
		}
	}
}

int	CConsoleMinecraftApp::NoSaveSpaceReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	if(result==C4JStorage::EMessage_ResultAccept && !UIScene_LoadOrJoinMenu::isSaveTransferRunning())		// MGH - we won't try to save again during a save tranfer
	{
		int blocksRequired = (int)pParam;
		if(blocksRequired > 0)
		{
			app.initSaveIncompleteDialog(blocksRequired);
		}
		else
		{
			app.initSaveDataDeleteDialog();
		}
	}
	else
	{
		StorageManager.CancelIncompleteOperation();
		StorageManager.ClearSaveError();
		StorageManager.ClearOptionsSaveError();
	}

	return 0;
}

int CConsoleMinecraftApp::cbConfirmDeleteMessageBox(void *pParam, int iPad, const C4JStorage::EMessageResult result)
{
	CConsoleMinecraftApp *pClass = (CConsoleMinecraftApp*) pParam;

	if (pClass != NULL && pClass->m_pSaveToDelete != NULL)
	{
		if (result == C4JStorage::EMessage_ResultDecline)
		{
			pClass->m_bSaveDataDeleteDialogState = eSaveDataDeleteState_deleting;
			C4JStorage::ESaveGameState eDeleteStatus = StorageManager.DeleteSaveData(pClass->m_pSaveToDelete, cbSaveDataDeleted, pClass);
		}
		else
		{
			pClass->initSaveDataDeleteDialog();
		}
	}
	else
	{
		pClass->initSaveDataDeleteDialog();

		// 4J-JEV: This could leave the storage library in a waiting for user state.
		//pClass->m_bSaveDataDeleteDialogState = eSaveDataDeleteState_idle;
	}
	return 0;
}

void CConsoleMinecraftApp::initSaveIncompleteDialog(int spaceNeeded)
{
	SceSaveDataDialogParam param;
	sceSaveDataDialogParamInit(&param);

	SceSaveDataDialogSystemMessageParam sysParam;
	ZeroMemory(&sysParam,sizeof(SceSaveDataDialogSystemMessageParam));
	param.sysMsgParam = &sysParam;

	param.mode = SCE_SAVEDATA_DIALOG_MODE_SYSTEM_MSG;
	param.dispType = SCE_SAVEDATA_DIALOG_TYPE_SAVE;

	param.sysMsgParam->sysMsgType = SCE_SAVEDATA_DIALOG_SYSMSG_TYPE_NOSPACE_CONTINUABLE;
	param.sysMsgParam->value = (SceInt32) spaceNeeded;

	SceInt32 ret = sceSaveDataDialogInit(&param);
	if (ret == SCE_OK)
	{
		m_bSaveIncompleteDialogRunning = true;
		InputManager.SetMenuDisplayed(0,true);
		ProfileManager.SetSysUIShowing(true);
		ui.SetSysUIShowing(true);
	}
	else
	{
		app.DebugPrintf("[SaveDataIncompleteDialog] ERROR: INITIALISING DIALOG, sceSaveDataDialogInit() (0x%x).\n", ret);
	}

}

void CConsoleMinecraftApp::updateSaveIncompleteDialog()
{
	SceCommonDialogStatus dialogStatus = sceSaveDataDialogGetStatus();
	if (dialogStatus == SCE_COMMON_DIALOG_STATUS_RUNNING)
	{
		SceCommonDialogStatus dialogSubStatus = sceSaveDataDialogGetSubStatus();

		if (dialogSubStatus == SCE_COMMON_DIALOG_STATUS_RUNNING)
		{
			// Wait for user.
		}
		else if (dialogSubStatus == SCE_COMMON_DIALOG_STATUS_FINISHED)
		{
			SceSaveDataDialogFinishParam finishParam;
			ZeroMemory(&finishParam, sizeof(SceSaveDataDialogFinishParam));
			finishParam.flag = SCE_SAVEDATA_DIALOG_FINISH_FLAG_DEFAULT;

			SceInt32 ret = sceSaveDataDialogFinish(&finishParam);
			if (ret != SCE_OK)
			{
				app.DebugPrintf("[SaveDataIncompleteDialog] ERROR: UPDATING DIALOG, sceSaveDataDialogFinish() (0x%x).\n", ret);
			}
		}
	}
	else if (dialogStatus == SCE_COMMON_DIALOG_STATUS_FINISHED)
	{
		SceInt32 ret = sceSaveDataDialogTerm();
		if (ret == SCE_OK)
		{
			finishSaveIncompleteDialog();
		}
		else
		{
			app.DebugPrintf("[SaveDataIncompleteDialog] ERROR: TERMINATING DIALOG, sceSaveDataDialogTerm() (0x%x).\n", ret);
		}
	}
}

void CConsoleMinecraftApp::finishSaveIncompleteDialog()
{
	m_bSaveIncompleteDialogRunning = false;
	InputManager.SetMenuDisplayed(0,false);
	ProfileManager.SetSysUIShowing(false);
	ui.SetSysUIShowing(false);

	StorageManager.ClearSaveError();
	StorageManager.ClearOptionsSaveError();

	initSaveDataDeleteDialog();
}

void CConsoleMinecraftApp::initSaveDataDeleteDialog()
{
	SceSaveDataDialogParam param;
	getSaveDataDeleteDialogParam( &param );

	SceInt32 ret = sceSaveDataDialogInit(&param);
	if (ret == SCE_OK)
	{
		app.DebugPrintf("[SaveDataDeleteDialog] Successfully initialised SaveDataDelete dialog.\n");

		m_bSaveDataDeleteDialogState = eSaveDataDeleteState_waitingForUser;

		InputManager.SetMenuDisplayed(0,true);
		ProfileManager.SetSysUIShowing(true);
		ui.SetSysUIShowing(true);

		// Start getting saves data to use when deleting.
		if (StorageManager.ReturnSavesInfo() == NULL)
		{
			C4JStorage::ESaveGameState eSGIStatus
				= StorageManager.GetSavesInfo(
					ProfileManager.GetPrimaryPad(),
					NULL,
					this,
					"save"
				);
		}

		// Dim background because sony doesn't do that.
		ui.showComponent(0, eUIComponent_MenuBackground, eUILayer_Tooltips, eUIGroup_Fullscreen, true);

		StorageManager.SetSaveDisabled(true);
		EnterSaveNotificationSection();
	}
	else
	{
		app.DebugPrintf("[SaveDataDeleteDialog] ERROR: INITIALISING DIALOG, sceSaveDataDialogInit() (0x%x).\n", ret);
	}

	releaseSaveDataDeleteDialogParam( &param );
}

void CConsoleMinecraftApp::updateSaveDataDeleteDialog()
{
	SceCommonDialogStatus dialogStatus = sceSaveDataDialogGetStatus();
	if (dialogStatus == SCE_COMMON_DIALOG_STATUS_RUNNING)
	{
		SceCommonDialogStatus dialogSubStatus = sceSaveDataDialogGetSubStatus();

		if (dialogSubStatus == SCE_COMMON_DIALOG_STATUS_RUNNING)
		{
			// Wait for user.
		}
		else if (dialogSubStatus == SCE_COMMON_DIALOG_STATUS_FINISHED)
		{
			SceSaveDataDialogResult dialogResult;
			ZeroMemory(&dialogResult, sizeof(SceSaveDataDialogResult));

			SceInt32 ret = sceSaveDataDialogGetResult(&dialogResult);
			if (ret == SCE_OK)
			{
				bool finishDialog = false;

				if (	dialogResult.result == SCE_COMMON_DIALOG_RESULT_USER_CANCELED
					||	dialogResult.result == SCE_COMMON_DIALOG_RESULT_ABORTED )
				{
					app.DebugPrintf("[SaveDataDeleteDialog] CANCELED OR ABORTED!\n");

					// 4J-JEV: Check to ensure that finishedDeletingSaves is called only once.
					if (m_bSaveDataDeleteDialogState == eSaveDataDeleteState_waitingForUser)
					{
						finishedDeletingSaves(false);
					}

					finishDialog = true;
				}

				if ( dialogResult.result == SCE_COMMON_DIALOG_RESULT_OK )
				{
					SceAppUtilSaveDataSlotParam slotParam;
					ret = sceAppUtilSaveDataSlotGetParam( dialogResult.slotId, &slotParam, NULL );

					if (ret == SCE_OK)
					{
						int saveindex = -1;
						PSAVE_INFO pSaveInfo = NULL;
						PSAVE_DETAILS pSaveDetails = StorageManager.ReturnSavesInfo();

						if (pSaveDetails != NULL)
						{
							app.DebugPrintf("[SaveDataDeleteDialog] Searching for save files:\n");

							for (unsigned int i = 0; i < pSaveDetails->iSaveC; i++)
							{
								app.DebugPrintf("\t- '%s'\n", pSaveDetails->SaveInfoA[i].UTF8SaveFilename);

								void *buf1, *buf2;
								buf1 = &(pSaveDetails->SaveInfoA[i].UTF8SaveFilename);
								buf2 = &slotParam.title;

								if ( 0 == memcmp(buf1, buf2, MAX_SAVEFILENAME_LENGTH) )
								{
									pSaveInfo = &pSaveDetails->SaveInfoA[i];
									saveindex = i;
									break;
								}
							}
						}
						else
						{
							app.DebugPrintf("[SaveDataDeleteDialog] ERROR: PERFORMING DELETE OPERATION, pSavesDetails is null.\n");
						}

						if (pSaveInfo != NULL)
						{
							app.DebugPrintf(
								"[SaveDataDeleteDialog] User wishes to delete slot_%d:\n\t"
								"4jsaveindex=%d, filename='%s', title='%s', subtitle='%s', size=%dKiB.\n",
								dialogResult.slotId,
								saveindex,
								pSaveInfo->UTF8SaveFilename,
								slotParam.title,
								slotParam.subTitle,
								slotParam.sizeKiB
								);

							UINT uiIDA[] =
							{
								IDS_CONFIRM_CANCEL,
								IDS_CONFIRM_OK
							};

							ui.RequestErrorMessage(
								IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE,
								uiIDA, 2,
								0,
								&cbConfirmDeleteMessageBox, this
								);

							m_bSaveDataDeleteDialogState = eSaveDataDeleteState_userConfirmation;

							m_pSaveToDelete = pSaveInfo;
						}
						else
						{
							app.DebugPrintf(
								"[SaveDataDeleteDialog] ERROR: PERFORMING DELETE OPERATION, cannot find file in our saves list:\n"
								"\t slotId=%i, title=%s, subtitle=%s,\n",
								dialogResult.slotId, slotParam.title, slotParam.subTitle
								);
						}

						finishDialog = true;
					}
					else
					{
						app.DebugPrintf("[SaveDataDeleteDialog] ERROR: UPDATING DIALOG, sceAppUtilSaveDataGetParam() (0x%x).\n", ret);
					}
				}

				if (finishDialog)
				{
					SceSaveDataDialogFinishParam finishParam;
					ZeroMemory(&finishParam, sizeof(SceSaveDataDialogFinishParam));
					finishParam.flag = SCE_SAVEDATA_DIALOG_FINISH_FLAG_DEFAULT;

					sceSaveDataDialogFinish(&finishParam);
					if (ret == SCE_OK)	app.DebugPrintf("[SaveDataDeleteDialog] Successfully finished saveDataDialog.\n");
					else				app.DebugPrintf("[SaveDataDeleteDialog] ERROR: UPDATING DIALOG, sceSaveDataDialogFinish() (0x%x).\n", ret);
				}

			}
			else
			{
				app.DebugPrintf("[SaveDataDeleteDialog] ERROR: UPDATING DIALOG, sceSaveDataDialogGetResult() (0x%x).\n", ret);
			}
		}
	}
	else if (dialogStatus == SCE_COMMON_DIALOG_STATUS_FINISHED)
	{
		SceInt32 ret = sceSaveDataDialogTerm();
		if (ret == SCE_OK)
		{
			finishSaveDataDeleteDialog();
		}
		else
		{
			app.DebugPrintf("[SaveDataDeleteDialog] ERROR: TERMINATING DIALOG, sceSaveDataDialogTerm() (0x%x).\n", ret);
		}
	}
}

void CConsoleMinecraftApp::finishSaveDataDeleteDialog()
{
	ProfileManager.SetSysUIShowing(false);
	InputManager.SetMenuDisplayed(0,false);
	ui.SetSysUIShowing(false);
	ui.removeComponent(eUIComponent_MenuBackground, eUILayer_Tooltips, eUIGroup_Fullscreen);
}

void CConsoleMinecraftApp::getSaveDataDeleteDialogParam(SceSaveDataDialogParam *baseParam)
{
	sceSaveDataDialogParamInit(baseParam);

	static SceSaveDataDialogListParam listParam;
	ZeroMemory(&listParam, sizeof(SceSaveDataDialogListParam));

	{
		vector<const SceAppUtilSaveDataSlot> slots;
		for (unsigned int i = 2; i < SCE_APPUTIL_SAVEDATA_SLOT_MAX; i++)
		{
			SceAppUtilSaveDataSlotParam slotParam;
			int ret = sceAppUtilSaveDataSlotGetParam( i, &slotParam, NULL );

			if (ret == SCE_OK)
			{
				SceAppUtilSaveDataSlot slot;
				ZeroMemory( &slot, sizeof(SceAppUtilSaveDataSlot) );

				slot.id = i;
				slot.status = slotParam.status;
				slot.userParam = 0;

				slots.push_back( slot );
			}
		}

		SceAppUtilSaveDataSlot *pSavesList = new SceAppUtilSaveDataSlot[slots.size()];

		int slotIndex = 0;

		vector<const SceAppUtilSaveDataSlot>::iterator itr;
		for (itr = slots.begin(); itr != slots.end(); itr++)
		{
			pSavesList[slotIndex] = *itr;
			slotIndex++;
		}

		listParam.slotListSize = slots.size();
		listParam.slotList = pSavesList;
	}

	if (listParam.slotListSize > 0)	listParam.focusPos = SCE_SAVEDATA_DIALOG_FOCUS_POS_LISTHEAD;
	else							listParam.focusPos = SCE_SAVEDATA_DIALOG_FOCUS_POS_EMPTYHEAD;

	// static SceCommonDialogColor s_bgColor, s_dColor;
	// s_bgColor.r = s_dColor.r = 50;
	// s_bgColor.g = s_dColor.g = 50;
	// s_bgColor.b = s_dColor.b = 50;
	// s_bgColor.a = s_dColor.a = 125;
	// baseParam->commonParam.bgColor = &s_bgColor;
	// baseParam->commonParam.dimmerColor = &s_dColor;


	static uint8_t *strPtr = NULL;
	if (strPtr != NULL) delete strPtr;
	strPtr = mallocAndCreateUTF8ArrayFromString( IDS_TOOLTIPS_DELETESAVE );

	listParam.listTitle = (const SceChar8 *) strPtr;
	listParam.itemStyle = SCE_SAVEDATA_DIALOG_LIST_ITEM_STYLE_TITLE_SUBTITLE_DATE;

	baseParam->mode = SCE_SAVEDATA_DIALOG_MODE_LIST;
	baseParam->dispType = SCE_SAVEDATA_DIALOG_TYPE_DELETE;
	baseParam->listParam = &listParam;

	baseParam->flag = SCE_SAVEDATA_DIALOG_ENV_FLAG_DEFAULT;
}

void CConsoleMinecraftApp::releaseSaveDataDeleteDialogParam(SceSaveDataDialogParam *baseParam)
{
	//delete baseParam->listParam;
	//delete baseParam->commonParam.dimmerColor;
	//delete baseParam->commonParam.bgColor;
	//delete baseParam->listParam.listTitle;
}

int CConsoleMinecraftApp::cbSaveDataDeleted( void *pParam, const bool success )
{
	app.DebugPrintf("[SaveDataDeleteDialog] cbSaveDataDeleted(%s)\n", (success?"success":"fail"));

	CConsoleMinecraftApp *pApp = (CConsoleMinecraftApp*) pParam;
	if ( pApp->m_bSaveDataDeleteDialogState == eSaveDataDeleteState_deleting )
	{
		/* SceSaveDataDialogParam param;
		pApp->getSaveDataDeleteDialogParam( &param );

		SceInt32 ret = sceSaveDataDialogContinue(&param);
		if (ret != SCE_OK)	app.DebugPrintf("[SaveDataDeleteDialog] ERROR: UPDATING DIALOG, sceSaveDataDialogContinue() (0x%x).\n", ret);

		pApp->m_bSaveDataDeleteDialogState = eSaveDataDeleteState_waitingForUser;
		pApp->releaseSaveDataDeleteDialogParam( &param ); */

		pApp->finishedDeletingSaves(true);
	}

	return 0;
}

void CConsoleMinecraftApp::finishedDeletingSaves(bool bContinue)
{
	app.DebugPrintf( "[finishedDeletingSaves] %s.\n", (bContinue?"Continuing":"Aborting") );

	StorageManager.SetSaveDisabled(false);
	LeaveSaveNotificationSection();

	StorageManager.ClearSaveError();
	StorageManager.ClearOptionsSaveError();


	if (bContinue)	m_bSaveDataDeleteDialogState = eSaveDataDeleteState_continue;
	else			m_bSaveDataDeleteDialogState = eSaveDataDeleteState_abort;
}