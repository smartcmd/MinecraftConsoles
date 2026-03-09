#include "stdafx.h"

#include "SonyCommerce_Vita.h"
#include "ShutdownManager.h"
#include <np_toolkit.h>
#include <libsysmodule.h>
#include <netcheck_dialog.h>

bool											SonyCommerce_Vita::m_bCommerceInitialised = false;
// SceNpCommerce2SessionInfo					SonyCommerce_Vita::m_sessionInfo;
SonyCommerce_Vita::State						SonyCommerce_Vita::m_state = e_state_noSession;
int												SonyCommerce_Vita::m_errorCode = 0;
LPVOID											SonyCommerce_Vita::m_callbackParam = nullptr;

void*											SonyCommerce_Vita::m_receiveBuffer = nullptr;
SonyCommerce_Vita::Event						SonyCommerce_Vita::m_event;
std::queue<SonyCommerce_Vita::Message>			SonyCommerce_Vita::m_messageQueue;
std::vector<SonyCommerce_Vita::ProductInfo>*	SonyCommerce_Vita::m_pProductInfoList = nullptr;
SonyCommerce_Vita::ProductInfoDetailed*		SonyCommerce_Vita::m_pProductInfoDetailed = nullptr;
SonyCommerce_Vita::ProductInfo*				SonyCommerce_Vita::m_pProductInfo = nullptr;

SonyCommerce_Vita::CategoryInfo*				SonyCommerce_Vita::m_pCategoryInfo = nullptr;
const char*										SonyCommerce_Vita::m_pProductID = nullptr;
char*											SonyCommerce_Vita::m_pCategoryID = nullptr;
SonyCommerce_Vita::CheckoutInputParams			SonyCommerce_Vita::m_checkoutInputParams;
SonyCommerce_Vita::DownloadListInputParams		SonyCommerce_Vita::m_downloadInputParams;

SonyCommerce_Vita::CallbackFunc				SonyCommerce_Vita::m_callbackFunc = nullptr;
// sys_memory_container_t						SonyCommerce_Vita::m_memContainer = SYS_MEMORY_CONTAINER_ID_INVALID;
bool											SonyCommerce_Vita::m_bUpgradingTrial = false;

SonyCommerce_Vita::CallbackFunc				SonyCommerce_Vita::m_trialUpgradeCallbackFunc;
LPVOID											SonyCommerce_Vita::m_trialUpgradeCallbackParam;

CRITICAL_SECTION								SonyCommerce_Vita::m_queueLock;

uint32_t										SonyCommerce_Vita::m_contextId=0;		///< The npcommerce2 context ID
bool											SonyCommerce_Vita::m_contextCreated=false;	///< npcommerce2 context ID created?
SonyCommerce_Vita::Phase						SonyCommerce_Vita::m_currentPhase = e_phase_stopped;		///< Current commerce2 util
// char											SonyCommerce_Vita::m_commercebuffer[SCE_NP_COMMERCE2_RECV_BUF_SIZE];

C4JThread*										SonyCommerce_Vita::m_tickThread = nullptr;
bool											SonyCommerce_Vita::m_bLicenseChecked=false;	 // Check the trial/full license for the game
bool											SonyCommerce_Vita::m_bLicenseInstalled=false;	// set to true when the licence has been downloaded and installed (but maybe not checked yet)
bool											SonyCommerce_Vita::m_bDownloadsPending=false;	// set to true if there are any downloads happening in the background, so we check for them completing, and install when finished
bool											SonyCommerce_Vita::m_bDownloadsReady=false;	// set to true if there are any downloads ready to install
bool											SonyCommerce_Vita::m_bInstallingContent=false;	// set to true while new content is being installed, so we don't fire it mulitple times
int												SonyCommerce_Vita::m_iClearDLCCountdown=0;			// tick for a set number of frames before clearing the DLC, as sometimes it doesn't register as being installed in time
bool											SonyCommerce_Vita::m_bPurchasabilityUpdated=false;	// set to when any purchase flags change
SonyCommerce_Vita::Message						SonyCommerce_Vita::m_lastMessage;

sce::Toolkit::NP::Utilities::Future<std::vector<sce::Toolkit::NP::ProductInfo> >	g_productList;
sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::CategoryInfo>					g_categoryInfo;
sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::ProductInfoDetailed>			g_detailedProductInfo;

//sce::Toolkit::NP::Utilities::Future<SceAppUtilBgdlStatus> g_bgdlStatus;
static bool s_showingPSStoreIcon = false;


SonyCommerce_Vita::ProductInfoDetailed s_trialUpgradeProductInfoDetailed;
void SonyCommerce_Vita::Delete()
{
	m_pProductInfoList=nullptr;
	m_pProductInfoDetailed=nullptr;
	m_pProductInfo=nullptr;
	m_pCategoryInfo = nullptr;
	m_pProductID = nullptr;
	m_pCategoryID = nullptr;
}

void SonyCommerce_Vita::Init()
{
	assert(m_state == e_state_noSession);
	if(!m_bCommerceInitialised)
	{
		m_bCommerceInitialised = true;
		m_pCategoryID=(char *)malloc(sizeof(char) * 100);
		InitializeCriticalSection(&m_queueLock);
		m_bLicenseInstalled = false;
		m_bDownloadsPending = false;
		m_bDownloadsReady = false;

	}
}



void SonyCommerce_Vita::CheckForTrialUpgradeKey_Callback(LPVOID param, bool bFullVersion)
{
	ProfileManager.SetFullVersion(bFullVersion);
	if(ProfileManager.IsFullVersion())
	{
		StorageManager.SetSaveDisabled(false);
		ConsoleUIController::handleUnlockFullVersionCallback();
		// licence has been checked, so we're ok to install the trophies now
		//  		ProfileManager.InitialiseTrophies(	SQRNetworkManager_Vita::GetSceNpCommsId(),
		//  										SQRNetworkManager_Vita::GetSceNpCommsSig());
		// 
	}
	m_bLicenseChecked=true;
	m_bLicenseInstalled = bFullVersion;
}

bool SonyCommerce_Vita::LicenseChecked() 
{
	return m_bLicenseChecked;
}

void SonyCommerce_Vita::CheckForTrialUpgradeKey()
{
	StorageManager.CheckForTrialUpgradeKey(CheckForTrialUpgradeKey_Callback, nullptr);
}

int SonyCommerce_Vita::Shutdown()
{
	int ret=0;
	if (m_contextCreated) 
	{
		m_contextId = 0;
		m_contextCreated = false;
	}	

	m_bCommerceInitialised = false;
	delete m_pCategoryID;
	DeleteCriticalSection(&m_queueLock);

	return ret;
}
 
void SonyCommerce_Vita::InstallContentCallback(LPVOID lpParam,int err)
{
	m_iClearDLCCountdown = 30;
	m_bInstallingContent = false;
	if(m_bLicenseInstalled && !ProfileManager.IsFullVersion())
		app.GetCommerce()->CheckForTrialUpgradeKey();
}

void SonyCommerce_Vita::checkBackgroundDownloadStatus()
{
	if(	m_bInstallingContent )
		return;
	 
	Future<SceAppUtilBgdlStatus> status;
	int ret = sce::Toolkit::NP::Commerce::Interface::getBgdlStatus(&status, false);
	if(ret == SCE_OK)
	{
		bool bInstallContent = false;
		// check for the license having been downloaded first
		if(!m_bLicenseInstalled && status.get()->licenseReady)
		{
			m_bLicenseInstalled = true;
			bInstallContent = true;
		}

		// and now any additional content
		m_bDownloadsReady = (status.get()->addcontNumReady > 0);

		if(m_bDownloadsReady)
			bInstallContent = true;
		// and if there are any downloads still pending, we'll call this function again
		m_bDownloadsPending = (status.get()->addcontNumNotReady > 0);

		// install the content
		if(bInstallContent)
		{
			InstallContent(InstallContentCallback, nullptr);
		}
	}
}

int SonyCommerce_Vita::TickLoop(void* lpParam)
{
	ShutdownManager::HasStarted(ShutdownManager::eCommerceThread);
	while( (m_currentPhase != e_phase_stopped) && ShutdownManager::ShouldRun(ShutdownManager::eCommerceThread) )
	{
		processEvent();
		processMessage();
		Sleep(16); //  sleep for a frame
		//((SonyCommerce_Vita*)app.GetCommerce())->Test();
		if(m_bDownloadsPending || m_bDownloadsReady)
		{
			checkBackgroundDownloadStatus();
		}
		if(m_iClearDLCCountdown > 0)	// tick for a set number of frames before clearing the DLC, as sometimes it doesn't register as being installed in time
		{
			m_iClearDLCCountdown--;
			if(m_iClearDLCCountdown == 0)
			{
				app.ClearDLCInstalled();
				if(g_NetworkManager.IsInSession()) // we're in-game, could be a purchase of a pack after joining an invite from another player
					app.StartInstallDLCProcess(0);
				else
					ui.HandleDLCInstalled(0);
			}

		}
	}
	ShutdownManager::HasFinished(ShutdownManager::eCommerceThread);

	return 0;
}

void SonyCommerce_Vita::copyProductList(std::vector<ProductInfo>* pProductList, std::vector<sce::Toolkit::NP::ProductInfo>* pNPProductList)
{
	ProductInfo									tempInfo;
	std::vector<ProductInfo>					tempProductVec;
	// Reserve some space
	int numProducts = pNPProductList->size();
	tempProductVec.reserve(numProducts);
	for(int i=0;i<numProducts;i++)
	{
		sce::Toolkit::NP::ProductInfo& npInfo = pNPProductList->at(i);

		// reset tempInfo
		memset(&tempInfo, 0x0, sizeof(tempInfo));
		strncpy(tempInfo.productId, npInfo.productId, SCE_NP_COMMERCE2_PRODUCT_ID_LEN);
		strncpy(tempInfo.productName, npInfo.productName, SCE_NP_COMMERCE2_PRODUCT_NAME_LEN);
		strncpy(tempInfo.shortDescription, npInfo.shortDescription, SCE_NP_COMMERCE2_PRODUCT_SHORT_DESCRIPTION_LEN);
		strcpy(tempInfo.longDescription,"Missing long description");
		strncpy(tempInfo.spName, npInfo.spName, SCE_NP_COMMERCE2_SP_NAME_LEN);
		strncpy(tempInfo.imageUrl, npInfo.imageUrl, SCE_NP_COMMERCE2_URL_LEN);
		tempInfo.releaseDate = npInfo.releaseDate;
		tempInfo.purchasabilityFlag = npInfo.purchasabilityFlag;
		m_bPurchasabilityUpdated = true;
		// Take out the price. Nicely formatted
		// but also keep the price as a value in case it's 0 - we need to show "free" for that
		tempInfo.ui32Price = -1;// not available here
		strncpy(tempInfo.price, npInfo.price, SCE_TOOLKIT_NP_SKU_PRICE_LEN);
		tempProductVec.push_back(tempInfo);
	}
	pNPProductList->clear();	// clear the vector now we're done, this doesn't happen automatically for the next query

	// Set our result
	*pProductList = tempProductVec;
}

int SonyCommerce_Vita::getProductList(std::vector<ProductInfo>* productList, char *categoryId)
{
	int ret;
	sce::Toolkit::NP::ProductListInputParams params;
	int userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

// 	params.userInfo.userId = userId;
	strcpy(params.categoryId, categoryId);
	params.serviceLabel = 0;
	app.DebugPrintf("Getting Product List ...\n");

	ret = sce::Toolkit::NP::Commerce::Interface::getProductList(&g_productList, params, true);

	app.DebugPrintf(" ----||||---- sce::Toolkit::NP::Commerce::Interface::getProductList : \n \t categoryId %s\n", categoryId);
	if (ret < 0) 
	{
		app.DebugPrintf("CommerceInterface::getProductList() error. ret = 0x%x\n", ret);
		return ret;
	}

	if (g_productList.hasResult())
	{
		// result has returned immediately (don't think this should happen, but was handled in the samples
		copyProductList(productList, g_productList.get());
		m_event = e_event_commerceGotProductList;
	}
	return ret;
}



void SonyCommerce_Vita::copyCategoryInfo(CategoryInfo *pInfo, sce::Toolkit::NP::CategoryInfo *pNPInfo)
{
	app.DebugPrintf("copyCategoryInfo %s\n", pNPInfo->current.categoryId);
	strcpy(pInfo->current.categoryId, pNPInfo->current.categoryId);
	strcpy(pInfo->current.categoryName, pNPInfo->current.categoryName);
	strcpy(pInfo->current.categoryDescription, pNPInfo->current.categoryDescription);
	strcpy(pInfo->current.imageUrl, pNPInfo->current.imageUrl);
	pInfo->countOfProducts = pNPInfo->countOfProducts;
	pInfo->countOfSubCategories = pNPInfo->countOfSubCategories;
	if(pInfo->countOfSubCategories > 0)
	{
		std::list<sce::Toolkit::NP::CategoryInfoSub>::iterator iter = pNPInfo->subCategories.begin();
		std::list<sce::Toolkit::NP::CategoryInfoSub>::iterator iterEnd = pNPInfo->subCategories.end();

		while(iter != iterEnd) 
		{
			// For each  sub category, obtain information
			app.DebugPrintf("copyCategoryInfo subcat - %s\n", iter->categoryId);

			CategoryInfoSub	tempSubCatInfo;
			strcpy(tempSubCatInfo.categoryId, iter->categoryId);
			strcpy(tempSubCatInfo.categoryName, iter->categoryName);
			strcpy(tempSubCatInfo.categoryDescription, iter->categoryDescription);
			strcpy(tempSubCatInfo.imageUrl, iter->imageUrl);
			// Add to the list
			pInfo->subCategories.push_back(tempSubCatInfo);
			iter++;
		}
	}
}

int SonyCommerce_Vita::getCategoryInfo(CategoryInfo *pInfo, char *categoryId)
{
	int ret;
	sce::Toolkit::NP::CategoryInfoInputParams params;
	int userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

	params.userInfo.userId = userId;
	strcpy(params.categoryId, "");//categoryId);
	params.serviceLabel = 0;

	app.DebugPrintf("Getting Category Information...\n");

	ret = sce::Toolkit::NP::Commerce::Interface::getCategoryInfo(&g_categoryInfo, params, true);
	app.DebugPrintf(" ----||||---- sce::Toolkit::NP::Commerce::Interface::getCategoryInfo : \n \t userID %d\n \t categoryId %s\n", userId, categoryId);
	if (ret < 0) 
	{
		// error
		app.DebugPrintf("Commerce::Interface::getCategoryInfo error: 0x%x\n", ret);
		return ret;
	}
	else if (g_categoryInfo.hasResult()) 
	{
		// result has returned immediately (don't think this should happen, but was handled in the samples
		copyCategoryInfo(pInfo, g_categoryInfo.get());
		m_event = e_event_commerceGotCategoryInfo;
	}
	return ret;
}

void SonyCommerce_Vita::copyDetailedProductInfo(ProductInfoDetailed *pInfo, sce::Toolkit::NP::ProductInfoDetailed* pNPInfo)
{
	// populate our temp struct
	//	pInfo->ratingDescriptors = npInfo.ratingSystemId;
	strncpy(pInfo->productId, pNPInfo->productId, SCE_NP_COMMERCE2_PRODUCT_ID_LEN);
	strncpy(pInfo->productName, pNPInfo->productName, SCE_NP_COMMERCE2_PRODUCT_NAME_LEN);
	strncpy(pInfo->shortDescription, pNPInfo->shortDescription, SCE_NP_COMMERCE2_PRODUCT_SHORT_DESCRIPTION_LEN);
	strncpy(pInfo->longDescription, pNPInfo->longDescription, SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN);
	strncpy(pInfo->legalDescription, pNPInfo->legalDescription, SCE_NP_COMMERCE2_PRODUCT_LEGAL_DESCRIPTION_LEN);
	strncpy(pInfo->spName, pNPInfo->spName, SCE_NP_COMMERCE2_SP_NAME_LEN);
	strncpy(pInfo->imageUrl, pNPInfo->imageUrl, SCE_NP_COMMERCE2_URL_LEN);
	pInfo->releaseDate = pNPInfo->releaseDate;
	strncpy(pInfo->ratingSystemId, pNPInfo->ratingSystemId, SCE_NP_COMMERCE2_RATING_SYSTEM_ID_LEN);
	strncpy(pInfo->ratingImageUrl, pNPInfo->imageUrl, SCE_NP_COMMERCE2_URL_LEN);
	strncpy(pInfo->skuId, pNPInfo->skuId, SCE_NP_COMMERCE2_SKU_ID_LEN);
	pInfo->purchasabilityFlag = pNPInfo->purchasabilityFlag;
	m_bPurchasabilityUpdated = true;
	pInfo->ui32Price= pNPInfo->intPrice;
	strncpy(pInfo->price, pNPInfo->price, SCE_TOOLKIT_NP_SKU_PRICE_LEN);

}
int SonyCommerce_Vita::getDetailedProductInfo(ProductInfoDetailed *pInfo, const char *productId, char *categoryId)
{
	int ret;
	sce::Toolkit::NP::DetailedProductInfoInputParams params;
	int userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

	//CD - userInfo no longer exists in DetailedProductInfoInputParams struct
	//params.userInfo.userId = userId;
	strcpy(params.categoryId, categoryId);
	strcpy(params.productId, productId);


	app.DebugPrintf("Getting Detailed Product Information ...  \n");		
	if(g_detailedProductInfo.get())		// MGH - clear the price out, in case something is hanging around from a previous call
	{
		g_detailedProductInfo.get()->intPrice = -1;
		g_detailedProductInfo.get()->price[0] = 0;
	}
	ret = sce::Toolkit::NP::Commerce::Interface::getDetailedProductInfo(&g_detailedProductInfo, params, true);
	app.DebugPrintf(" ----||||---- sce::Toolkit::NP::Commerce::Interface::getDetailedProductInfo : \n \t userID %d\n \t categoryId %s\n \t productId %s\n", userId, categoryId, productId);

	if (ret < 0) 
	{
		app.DebugPrintf("CommerceInterface::getDetailedProductInfo() error. ret = 0x%x\n", ret);
		return ret;
	}

	if (g_detailedProductInfo.hasResult()) 
	{
		// result has returned immediately (don't think this should happen, but was handled in the samples
		copyDetailedProductInfo(pInfo, g_detailedProductInfo.get());
		m_event = e_event_commerceGotDetailedProductInfo;
	}
	return ret;
}

void SonyCommerce_Vita::copyAddDetailedProductInfo(ProductInfo *pInfo, sce::Toolkit::NP::ProductInfoDetailed* pNPInfo)
{

	// populate our temp struct
	//	pInfo->ratingDescriptors = npInfo.ratingSystemId;
	// 	strncpy(pInfo->productId, npInfo.productId, SCE_NP_COMMERCE2_PRODUCT_ID_LEN);
	// 	strncpy(pInfo->productName, npInfo.productName, SCE_NP_COMMERCE2_PRODUCT_NAME_LEN);
	// 	strncpy(pInfo->shortDescription, npInfo.shortDescription, SCE_NP_COMMERCE2_PRODUCT_SHORT_DESCRIPTION_LEN);
	strncpy(pInfo->longDescription, pNPInfo->longDescription, SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN);
	// 	strncpy(pInfo->legalDescription, npInfo.legalDescription, SCE_NP_COMMERCE2_PRODUCT_LEGAL_DESCRIPTION_LEN);
	// 	strncpy(pInfo->spName, npInfo.spName, SCE_NP_COMMERCE2_SP_NAME_LEN);
	// 	strncpy(pInfo->imageUrl, npInfo.imageUrl, SCE_NP_COMMERCE2_URL_LEN);
	// 	pInfo->releaseDate = npInfo.releaseDate;
	// 	strncpy(pInfo->ratingSystemId, npInfo.ratingSystemId, SCE_NP_COMMERCE2_RATING_SYSTEM_ID_LEN);
	// 	strncpy(pInfo->ratingImageUrl, npInfo.imageUrl, SCE_NP_COMMERCE2_URL_LEN);
	strncpy(pInfo->skuId, pNPInfo->skuId, SCE_NP_COMMERCE2_SKU_ID_LEN);
	pInfo->purchasabilityFlag = pNPInfo->purchasabilityFlag;
	m_bPurchasabilityUpdated = true;
	pInfo->ui32Price= pNPInfo->intPrice;
	strncpy(pInfo->price, pNPInfo->price, SCE_TOOLKIT_NP_SKU_PRICE_LEN);

	app.DebugPrintf(" ---- description - %s\n", pInfo->longDescription);
	app.DebugPrintf(" ---- price - %d\n", pInfo->price);
	app.DebugPrintf(" ---- hasPurchased %d\n", pInfo->purchasabilityFlag);

}

int SonyCommerce_Vita::addDetailedProductInfo(ProductInfo *pInfo, const char *productId, char *categoryId)
{
	int ret;
	sce::Toolkit::NP::DetailedProductInfoInputParams params;
	int userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

	//CD - userInfo no longer exists in DetailedProductInfoInputParams struct
	//params.userInfo.userId = userId;
	strcpy(params.categoryId, categoryId);
	strcpy(params.productId, productId);


	app.DebugPrintf("Getting Detailed Product Information ...  \n");		
	if(g_detailedProductInfo.get())		// MGH - clear the price out, in case something is hanging around from a previous call
	{
		g_detailedProductInfo.get()->intPrice = -1;
		g_detailedProductInfo.get()->price[0] = 0;
	}
	ret = sce::Toolkit::NP::Commerce::Interface::getDetailedProductInfo(&g_detailedProductInfo, params, true);
	app.DebugPrintf(" ----||||---- sce::Toolkit::NP::Commerce::Interface::getDetailedProductInfo : \n \t userID %d\n \t categoryId %s\n \t productId %s\n", userId, categoryId, productId);

	if (ret < 0) 
	{
		app.DebugPrintf("CommerceInterface::addDetailedProductInfo() error. ret = 0x%x\n", ret);
	}

	if (g_detailedProductInfo.hasResult()) 
	{
		// result has returned immediately (don't think this should happen, but was handled in the samples
		copyAddDetailedProductInfo(pInfo, g_detailedProductInfo.get());
		m_event = e_event_commerceAddedDetailedProductInfo;
	}
	return ret;
}


int SonyCommerce_Vita::checkout(CheckoutInputParams &params)
{
	int ret;
	sce::Toolkit::NP::CheckoutInputParams npParams;
	int userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

	//CD - userInfo no longer exists in CheckoutInputParams struct
	//npParams.userInfo.userId = userId;
	npParams.serviceLabel = 0;

	std::list<const char*>::iterator iter = params.skuIds.begin();
	std::list<const char*>::iterator iterEnd = params.skuIds.end();
	while(iter != iterEnd) 
	{
		npParams.skuIds.push_back((char*)*iter);		// have to remove the const here, not sure why the libs pointers aren't const
		iter++;
	}

	app.DebugPrintf("Starting SonyCommerce_Vita::checkout...\n");
	ret = sce::Toolkit::NP::Commerce::Interface::checkout(npParams, false);
	if (ret < 0) 
	{
		app.DebugPrintf("checkout() error. ret = 0x%x\n", ret);
	}
	return ret;	
}


int SonyCommerce_Vita::downloadList(DownloadListInputParams &params)
{	
	int ret;
	sce::Toolkit::NP::DownloadListInputParams npParams;
	int userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());
	//CD - userInfo no longer exists in DownloadListInputParams struct
	//npParams.userInfo.userId = userId;
	npParams.serviceLabel = 0;

	std::list<const char*>::iterator iter = params.skuIds.begin();
	std::list<const char*>::iterator iterEnd = params.skuIds.end();
	while(iter != iterEnd) 
	{
		npParams.skuIds.push_back((char*)*iter);		// have to remove the const here, not sure why the libs pointers aren't const
		iter++;
	}

	app.DebugPrintf("Starting Store Download List...\n");
	ret = sce::Toolkit::NP::Commerce::Interface::displayDownloadList(npParams, true);
	if (ret < 0) 
	{
		app.DebugPrintf("Commerce::Interface::displayDownloadList error: 0x%x\n", ret);
	}
	return ret;	
}

int SonyCommerce_Vita::checkout_game(CheckoutInputParams &params)
{

	int ret;
	sce::Toolkit::NP::CheckoutInputParams npParams;
	npParams.serviceLabel = 0;

	std::list<const char*>::iterator iter = params.skuIds.begin();
	std::list<const char*>::iterator iterEnd = params.skuIds.end();
	while(iter != iterEnd) 
	{
		npParams.skuIds.push_back((char*)*iter);		// have to remove the const here, not sure why the libs pointers aren't const
		iter++;
	}

	app.DebugPrintf("Starting Checkout...\n");
	sce::Toolkit::NP::ProductBrowseParams Myparams; 

	Myparams.serviceLabel = 0; 
	strncpy(Myparams.productId, app.GetUpgradeKey(), strlen(app.GetUpgradeKey())); 

	ret = sce::Toolkit::NP::Commerce::Interface::productBrowse(Myparams, false); 

	//ret = sce::Toolkit::NP::Commerce::Interface::checkout(npParams, false);
	if (ret < 0) 
	{
		app.DebugPrintf("Sample menu checkout() error. ret = 0x%x\n", ret);
	}

	// we don't seem to get any of the productBrowse completion callbacks on Vita, so just force us into that state next
	m_event = e_event_commerceProductBrowseFinished;

	return ret;	
}

int SonyCommerce_Vita::downloadList_game(DownloadListInputParams &params)
{

	int ret;
	sce::Toolkit::NP::DownloadListInputParams npParams;
	//memset(&npParams,0,sizeof(sce::Toolkit::NP::DownloadListInputParams));
	npParams.serviceLabel = 0;
	npParams.skuIds.clear();

	std::list<const char*>::iterator iter = params.skuIds.begin();
	std::list<const char*>::iterator iterEnd = params.skuIds.end();
	while(iter != iterEnd) 
	{
		npParams.skuIds.push_back((char*)*iter);		// have to remove the const here, not sure why the libs pointers aren't const
		iter++;
	}

	app.DebugPrintf("Starting Store Download List...\n");
	// 	ret = sce::Toolkit::NP::Commerce::Interface::displayDownloadList(npParams, true);
	// 	if (ret < 0) 
	// 	{
	// 		app.DebugPrintf("Commerce::Interface::displayDownloadList error: 0x%x\n", ret);
	// 	}

	sce::Toolkit::NP::ProductBrowseParams Myparams; 

	Myparams.serviceLabel = 0; 
	strncpy(Myparams.productId, "EP4433-PCSB00560_00-MINECRAFTVIT0452", strlen("EP4433-PCSB00560_00-MINECRAFTVIT0452")); 

	ret = sce::Toolkit::NP::Commerce::Interface::productBrowse(Myparams, false); 
	if (ret < 0) 
	{ 
		// Error handling 
		app.DebugPrintf("Commerce::Interface::displayDownloadList error: 0x%x\n", ret);
	} 



	// we don't seem to get any of the productBrowse completion callbacks on Vita, so just force us into that state next
	m_event = e_event_commerceProductBrowseFinished;

	return ret;	
}

int SonyCommerce_Vita::installContent()
{
	int ret;
	ret = sce::Toolkit::NP::Commerce::Interface::installContent();
	return ret;	
}


void SonyCommerce_Vita::UpgradeTrialCallback2(LPVOID lpParam,int err)
{
	SonyCommerce* pCommerce = (SonyCommerce*)lpParam;
	app.DebugPrintf(4,"SonyCommerce_UpgradeTrialCallback2 : err : 0x%08x\n", err);
	pCommerce->CheckForTrialUpgradeKey();
	if(err != SCE_OK)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_PRO_UNLOCKGAME_TITLE, IDS_NO_DLCOFFERS, uiIDA,1,ProfileManager.GetPrimaryPad());
	}
	m_trialUpgradeCallbackFunc(m_trialUpgradeCallbackParam, m_errorCode);
}

void SonyCommerce_Vita::UpgradeTrialCallback1(LPVOID lpParam,int err)
{
	SonyCommerce* pCommerce = (SonyCommerce*)lpParam;
	app.DebugPrintf(4,"SonyCommerce_UpgradeTrialCallback1 : err : 0x%08x\n", err);
	if(err == SCE_OK)
	{
		char* skuID = s_trialUpgradeProductInfoDetailed.skuId;
		if(s_trialUpgradeProductInfoDetailed.purchasabilityFlag == SCE_TOOLKIT_NP_COMMERCE_NOT_PURCHASED)
		{
			app.DebugPrintf(4,"UpgradeTrialCallback1 - Checkout\n");
			pCommerce->Checkout_Game(UpgradeTrialCallback2, pCommerce, skuID);
		}
		else
		{
			app.DebugPrintf(4,"UpgradeTrialCallback1 - DownloadAlreadyPurchased\n");
			pCommerce->DownloadAlreadyPurchased_Game(UpgradeTrialCallback2, pCommerce, skuID);
		}
	}
	else
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_PRO_UNLOCKGAME_TITLE, IDS_NO_DLCOFFERS, uiIDA,1,ProfileManager.GetPrimaryPad());
		m_trialUpgradeCallbackFunc(m_trialUpgradeCallbackParam, m_errorCode);
	}
}



// global func, so we can call from the profile lib
void SonyCommerce_UpgradeTrial()
{
	// we're now calling the app function here, which manages pending requests
	app.UpgradeTrial();
}

void SonyCommerce_Vita::UpgradeTrial(CallbackFunc cb, LPVOID lpParam)
{
	m_trialUpgradeCallbackFunc = cb;
	m_trialUpgradeCallbackParam = lpParam;

	GetDetailedProductInfo(UpgradeTrialCallback1, this, &s_trialUpgradeProductInfoDetailed, app.GetUpgradeKey(), app.GetCommerceCategory());
}


int SonyCommerce_Vita::createContext()
{
	// 	SceNpId npId;
	// 	int ret = sceNpManagerGetNpId(&npId);
	// 	if(ret < 0)
	// 	{
	// 		app.DebugPrintf(4,"createContext sceNpManagerGetNpId problem\n");
	// 		return ret;
	// 	}
	// 
	// 	if (m_contextCreated) {
	// 		ret = sceNpCommerce2DestroyCtx(m_contextId);
	// 		if (ret < 0) 
	// 		{
	// 			app.DebugPrintf(4,"createContext sceNpCommerce2DestroyCtx problem\n");
	// 			return ret;
	// 		}
	// 	}
	// 
	// 	// Create commerce2 context
	// 	ret = sceNpCommerce2CreateCtx(SCE_NP_COMMERCE2_VERSION, &npId, commerce2Handler, nullptr, &m_contextId);
	// 	if (ret < 0) 
	// 	{
	// 		app.DebugPrintf(4,"createContext sceNpCommerce2CreateCtx problem\n");
	// 		return ret;
	// 	}

	m_contextCreated = true;

	return SCE_OK;
}

int SonyCommerce_Vita::createSession()
{
	// this does nothing now, we only catch session expired errors now and recreate the session when needed.
	int ret = 0;
	EnterCriticalSection(&m_queueLock);
	m_messageQueue.push(e_message_commerceEnd);
	m_event = e_event_commerceSessionCreated;
	LeaveCriticalSection(&m_queueLock);

	return ret;
}

int SonyCommerce_Vita::recreateSession()
{
	int ret = 0;
	ret = sce::Toolkit::NP::Commerce::Interface::createSession();
	app.DebugPrintf(" ----||||---- sce::Toolkit::NP::Commerce::Interface::createSession \n");

	if (ret < 0) 
	{
		return ret;
	}
	m_currentPhase = e_phase_creatingSessionPhase;
	return ret;
}



void SonyCommerce_Vita::commerce2Handler( const sce::Toolkit::NP::Event& event)
{
	// 	Event reply;
	// 	reply.service = Toolkit::NP::commerce;
	// 
	
	// make sure we're initialised
	Init();

	app.DebugPrintf("commerce2Handler returnCode = 0x%08x\n", event.returnCode);


	EnterCriticalSection(&m_queueLock);

	if(event.returnCode == SCE_NP_COMMERCE2_SERVER_ERROR_SESSION_EXPIRED)
	{
		// this will happen on the first commerce call, since there is no session, so we create and then queue the request again
		m_messageQueue.push(e_message_commerceRecreateSession);
		LeaveCriticalSection(&m_queueLock);
		return;
	}


	switch (event.event)
	{
	case sce::Toolkit::NP::Event::UserEvent::commerceNoEntitlements:
		app.DebugPrintf("commerce2Handler : commerceNoEntitlements\n");
		StorageManager.EntitlementsCallback(false);
		break;

	case sce::Toolkit::NP::Event::UserEvent::commerceGotEntitlementList:
		app.DebugPrintf("commerce2Handler : commerceGotEntitlementList\n");
		StorageManager.EntitlementsCallback(true);
		break;

	case sce::Toolkit::NP::Event::UserEvent::commerceError:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_errorCode = event.returnCode;
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceSessionCreated:
		{
			// the seesion has been recreated after an error, so queue the old request back up now we're running again
			m_messageQueue.push(m_lastMessage);
			m_event = e_event_commerceSessionRecreated;
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceSessionAborted:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceSessionAborted;		
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceCheckoutStarted:
		{
			m_currentPhase = e_phase_checkoutPhase;
			m_event = e_event_commerceCheckoutStarted;
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceGotCategoryInfo:
		{
			// 		int ret = sce::Toolkit::NP::Commerce::Interface::getBgdlStatus(&status, false);
			// 		if(ret == SCE_OK)
			// 		{
			copyCategoryInfo(m_pCategoryInfo, g_categoryInfo.get());
			m_pCategoryInfo = nullptr;
			m_event = e_event_commerceGotCategoryInfo;
			// 		}

			break;
		}

	case sce::Toolkit::NP::Event::UserEvent::commerceGotProductList:
		{
			copyProductList(m_pProductInfoList, g_productList.get());
			m_pProductInfoDetailed = nullptr;
			m_event = e_event_commerceGotProductList;
			break;
		}

	case sce::Toolkit::NP::Event::UserEvent::commerceGotDetailedProductInfo:
		{
			if(m_pProductInfoDetailed)
			{
				copyDetailedProductInfo(m_pProductInfoDetailed, g_detailedProductInfo.get());
				m_pProductInfoDetailed = nullptr;
			}
			else
			{
				copyAddDetailedProductInfo(m_pProductInfo, g_detailedProductInfo.get());
				m_pProductInfo = nullptr;
			}
			m_event = e_event_commerceGotDetailedProductInfo;
			break;
		}



		// 	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_SUCCESS:
		// 		{
		// 			m_messageQueue.push(e_message_commerceEnd);
		// 			m_event = e_event_commerceCheckoutSuccess;
		// 			break;
		// 		}
		// 	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_BACK:
		// 		{
		// 			m_messageQueue.push(e_message_commerceEnd);
		// 			m_event = e_event_commerceCheckoutAborted;
		// 			break;
		// 		}
	case sce::Toolkit::NP::Event::UserEvent::commerceCheckoutFinished:
		{
			m_messageQueue.push(e_message_commerceEnd);  // MGH - fixes an assert when switching to adhoc mode after this
			m_event = e_event_commerceCheckoutFinished;
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceDownloadListStarted:
		{
			m_currentPhase = e_phase_downloadListPhase;
			m_event = e_event_commerceDownloadListStarted;	
			break;
		}
		// 	case SCE_NP_COMMERCE2_EVENT_DO_DL_LIST_SUCCESS:
		// 		{
		// 			m_messageQueue.push(e_message_commerceEnd);
		// 			m_event = e_event_commerceDownloadListSuccess;
		// 			break;
		// 		}
	case sce::Toolkit::NP::Event::UserEvent::commerceDownloadListFinished:
		{
			m_event = e_event_commerceDownloadListFinished;
			break;
		}

	case sce::Toolkit::NP::Event::UserEvent::commerceProductBrowseStarted:
		{
			m_currentPhase = e_phase_productBrowsePhase;
			m_event = e_event_commerceProductBrowseStarted;
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceProductBrowseSuccess:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceProductBrowseSuccess;		
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceProductBrowseAborted:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceProductBrowseAborted;
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceProductBrowseFinished:
		{
			m_event = e_event_commerceProductBrowseFinished;
			break;
		}

	case sce::Toolkit::NP::Event::UserEvent::commerceInstallStarted:
		{
			m_event = e_event_commerceInstallContentStarted;
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceInstallFinished:
		{
			m_event = e_event_commerceInstallContentFinished;
			break;
		}


		// 	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_OPENED:
		// 		break;
		// 	case SCE_NP_COMMERCE2_EVENT_DO_PRODUCT_CODE_STARTED:
		// 		{
		// 			m_currentPhase = e_phase_voucherRedeemPhase;
		// 			m_event = e_event_commerceVoucherInputStarted;
		// 			break;
		// 		}
		// 	case SCE_NP_COMMERCE2_EVENT_DO_PRODUCT_CODE_SUCCESS:
		// 		{
		// 			m_messageQueue.push(e_message_commerceEnd);
		// 			m_event = e_event_commerceVoucherInputSuccess;		
		// 			break;
		// 		}
		// 	case SCE_NP_COMMERCE2_EVENT_DO_PRODUCT_CODE_BACK:
		// 		{
		// 			m_messageQueue.push(e_message_commerceEnd);
		// 			m_event = e_event_commerceVoucherInputAborted;
		// 			break;
		// 		}
		// 	case SCE_NP_COMMERCE2_EVENT_DO_PRODUCT_CODE_FINISHED:
		// 		{
		// 			m_event = e_event_commerceVoucherInputFinished;
		// 			break;
		// 		}
	default:
		break;
	};

	LeaveCriticalSection(&m_queueLock);
}



void SonyCommerce_Vita::processMessage()
{
	EnterCriticalSection(&m_queueLock);
	int ret;
	if(m_messageQueue.empty())
	{
		LeaveCriticalSection(&m_queueLock);
		return;
	}
	Message msg = m_messageQueue.front();
	if(msg != e_message_commerceRecreateSession)
		m_lastMessage = msg;
	m_messageQueue.pop();

	switch (msg)
	{

	case e_message_commerceCreateSession:
		ret = createSession();
		if (ret < 0) 
		{
			m_event = e_event_commerceError;
			m_errorCode = ret;
		}
		break;

	case e_message_commerceRecreateSession:
		ret = recreateSession();
		if (ret < 0) 
		{
			m_event = e_event_commerceError;
			m_errorCode = ret;
		}
		break;

	case e_message_commerceGetCategoryInfo:
		{
			ret = getCategoryInfo(m_pCategoryInfo, m_pCategoryID);
			if (ret < 0) 
			{
				m_event = e_event_commerceError;
				app.DebugPrintf(4,"ERROR - e_event_commerceGotCategoryInfo - %s\n",m_pCategoryID);
				m_errorCode = ret;
			} 
			break;
		}

	case e_message_commerceGetProductList:
		{
			ret = getProductList(m_pProductInfoList, m_pCategoryID);
			if (ret < 0) 
			{
				m_event = e_event_commerceError;
			}
			break;
		}

	case e_message_commerceGetDetailedProductInfo:
		{
			ret = getDetailedProductInfo(m_pProductInfoDetailed, m_pProductID, m_pCategoryID);
			if (ret < 0) 
			{
				m_event = e_event_commerceError;
				m_errorCode = ret;
			} 
			break;
		}
	case e_message_commerceAddDetailedProductInfo:
		{
			ret = addDetailedProductInfo(m_pProductInfo, m_pProductID, m_pCategoryID);
			if (ret < 0) 
			{
				m_event = e_event_commerceError;
				m_errorCode = ret;
			} 
			break;
		}

		// 
		// 	case e_message_commerceStoreProductBrowse:
		// 		{
		// 			ret = productBrowse(*(ProductBrowseParams *)msg.inputArgs);
		// 			if (ret < 0) {
		// 				m_event = e_event_commerceError;
		// 				m_errorCode = ret;
		// 			} 
		// 			_TOOLKIT_NP_DEL (ProductBrowseParams *)msg.inputArgs;
		// 			break;
		// 		}
		// 
		// 	case e_message_commerceUpgradeTrial:
		// 		{
		// 			ret = upgradeTrial();
		// 			if (ret < 0) {
		// 				m_event = e_event_commerceError;
		// 				m_errorCode = ret;
		// 			} 
		// 			break;
		// 		}
		// 
		// 	case e_message_commerceRedeemVoucher:
		// 		{
		// 			ret = voucherCodeInput(*(VoucherInputParams *)msg.inputArgs);
		// 			if (ret < 0) {
		// 				m_event = e_event_commerceError;
		// 				m_errorCode = ret;
		// 			} 
		// 			_TOOLKIT_NP_DEL (VoucherInputParams *)msg.inputArgs;
		// 			break;
		// 		}
		// 
		// 	case e_message_commerceGetEntitlementList:
		// 		{			
		// 			Job<std::vector<SceNpEntitlement> > tmpJob(static_cast<Future<std::vector<SceNpEntitlement> > *>(msg.output));
		// 
		// 			int state = 0;
		// 			int ret = sceNpManagerGetStatus(&state);
		// 
		// 			// We don't want to process this if we are offline
		// 			if (ret < 0 || state != SCE_NP_MANAGER_STATUS_ONLINE) {
		// 				m_event = e_event_commerceError;
		// 				reply.returnCode = SCE_TOOLKIT_NP_OFFLINE;
		// 				tmpJob.setError(SCE_TOOLKIT_NP_OFFLINE);
		// 			} else {
		// 				getEntitlementList(&tmpJob);
		// 			}
		// 			break;
		// 		}
		// 
		// 	case e_message_commerceConsumeEntitlement:
		// 		{			
		// 			int state = 0;
		// 			int ret = sceNpManagerGetStatus(&state);
		// 
		// 			// We don't want to process this if we are offline
		// 			if (ret < 0 || state != SCE_NP_MANAGER_STATUS_ONLINE) {
		// 				m_event = e_event_commerceError;
		// 				reply.returnCode = SCE_TOOLKIT_NP_OFFLINE;
		// 			} else {
		// 
		// 				ret = consumeEntitlement(*(EntitlementToConsume *)msg.inputArgs);
		// 				if (ret < 0) {
		// 					m_event = e_event_commerceError;
		// 					m_errorCode = ret;
		// 				} else {		
		// 					m_event = e_event_commerceConsumedEntitlement;
		// 				}
		// 			}
		// 			_TOOLKIT_NP_DEL (EntitlementToConsume *)msg.inputArgs;
		// 
		// 			break;
		// 		}
		// 
	case e_message_commerceCheckout:
		{
			ret = checkout(m_checkoutInputParams);
			if (ret < 0) {
				m_event = e_event_commerceError;
				m_errorCode = ret;
			} 
			break;
		}

	case e_message_commerceDownloadList:
		{
			ret = downloadList(m_downloadInputParams);
			if (ret < 0) {
				m_event = e_event_commerceError;
				m_errorCode = ret;
			} 
			break;
		}

	case e_message_commerceCheckout_Game:
		{
			ret = checkout_game(m_checkoutInputParams);
			if (ret < 0) {
				m_event = e_event_commerceError;
				m_errorCode = ret;
			} 
			break;
		}

	case e_message_commerceDownloadList_Game:
		{
			ret = downloadList_game(m_downloadInputParams);
			if (ret < 0) {
				m_event = e_event_commerceError;
				m_errorCode = ret;
			} 
			break;
		}

	case e_message_commerceInstallContent:
		{
			ret = installContent();
			if (ret < 0) {
				m_event = e_event_commerceError;
				m_errorCode = ret;
			} 
			break;
		}


	case e_message_commerceEnd:	
		app.DebugPrintf("XXX - e_message_commerceEnd!\n");
		ret = commerceEnd();
		if (ret < 0) 
		{
			m_event = e_event_commerceError;
			m_errorCode = ret;
		} 
		// 4J-PB - we don't seem to handle the error code here
		else if(m_errorCode!=0)
		{
			m_event = e_event_commerceError;
		}
		break;

	default:
		break;
	}

	LeaveCriticalSection(&m_queueLock);
}


void SonyCommerce_Vita::processEvent()
{
	int ret = 0;

	switch (m_event) 
	{
	case e_event_none:
		break;
	case e_event_commerceSessionRecreated:
		app.DebugPrintf(4,"Commerce Session Created.\n");
		break;
	case e_event_commerceSessionCreated:
		app.DebugPrintf(4,"Commerce Session Created.\n");
		runCallback();
		break;
	case e_event_commerceSessionAborted:
		app.DebugPrintf(4,"Commerce Session aborted.\n");
		runCallback();
		break;
	case e_event_commerceGotProductList:
		app.DebugPrintf(4,"Got product list.\n");
		runCallback();
		break;
	case e_event_commerceGotCategoryInfo:
		app.DebugPrintf(4,"Got category info\n");
		runCallback();
		break;
	case e_event_commerceGotDetailedProductInfo:
		app.DebugPrintf(4,"Got detailed product info.\n");
		runCallback();
		break;
	case e_event_commerceAddedDetailedProductInfo:
		app.DebugPrintf(4,"Added detailed product info.\n");
		runCallback();
		break;
	case e_event_commerceProductBrowseStarted:
		break;
	case e_event_commerceProductBrowseSuccess:
		break;
	case e_event_commerceProductBrowseAborted:
		break;
	case e_event_commerceProductBrowseFinished:
		app.DebugPrintf(4,"e_event_commerceProductBrowseFinished succeeded: 0x%x\n", m_errorCode);
		if(m_callbackFunc!=nullptr)
		{
			runCallback();
		}
		m_bDownloadsPending = true;

//		assert(0);
		// 		ret = sys_memory_container_destroy(s_memContainer);
		// 		if (ret < 0) {
		// 			printf("Failed to destroy memory container");
		// 		}
		// 		s_memContainer = SYS_MEMORY_CONTAINER_ID_INVALID;
		break;
	case e_event_commerceVoucherInputStarted:
		break;
	case e_event_commerceVoucherInputSuccess:
		break;
	case e_event_commerceVoucherInputAborted:
		break;
	case e_event_commerceVoucherInputFinished:
		assert(0);
		// 		ret = sys_memory_container_destroy(s_memContainer);
		// 		if (ret < 0) {
		// 			printf("Failed to destroy memory container");
		// 		}
		// 		s_memContainer = SYS_MEMORY_CONTAINER_ID_INVALID;
		break;
	case e_event_commerceGotEntitlementList:
		break;
	case e_event_commerceConsumedEntitlement:
		break;
	case e_event_commerceCheckoutStarted:
		app.DebugPrintf(4,"Checkout Started\n");
		ProfileManager.SetSysUIShowing(true);
		break;
	case e_event_commerceCheckoutSuccess:
		app.DebugPrintf(4,"Checkout succeeded: 0x%x\n", m_errorCode);
		// clear the DLC installed and check again
		ProfileManager.SetSysUIShowing(false);
		break;
	case e_event_commerceCheckoutAborted:
		app.DebugPrintf(4,"Checkout aborted: 0x%x\n", m_errorCode);
		ProfileManager.SetSysUIShowing(false);
		break;
	case e_event_commerceCheckoutFinished:
		app.DebugPrintf(4,"Checkout Finished: 0x%x\n", m_errorCode);
		if (ret < 0) {
			app.DebugPrintf(4,"Failed to destroy memory container");
		}
		ProfileManager.SetSysUIShowing(false);

		// 4J-PB - if there's been an error - like dlc already purchased, the runcallback has already happened, and will crash this time
		if(m_callbackFunc!=nullptr)
		{
			// get the detailed product info again, to see if the purchase has happened or not
 			EnterCriticalSection(&m_queueLock);
 			m_messageQueue.push(e_message_commerceAddDetailedProductInfo);
 			LeaveCriticalSection(&m_queueLock);
 
//			runCallback();
		}
		m_bDownloadsPending = true;
		break;
	case e_event_commerceDownloadListStarted:
		app.DebugPrintf(4,"Download List Started\n");
		ProfileManager.SetSysUIShowing(true);
		break;
	case e_event_commerceDownloadListSuccess:
		app.DebugPrintf(4,"Download succeeded: 0x%x\n", m_errorCode);
		ProfileManager.SetSysUIShowing(false);
		m_bDownloadsPending = true;
		break;
	case e_event_commerceDownloadListFinished:
		app.DebugPrintf(4,"Download Finished: 0x%x\n", m_errorCode);
		if (ret < 0) {
			app.DebugPrintf(4,"Failed to destroy memory container");
		}
		ProfileManager.SetSysUIShowing(false);

		// 4J-PB - if there's been an error - like dlc already purchased, the runcallback has already happened, and will crash this time
		if(m_callbackFunc!=nullptr)
		{
			runCallback();
		}
		m_bDownloadsPending = true;
		break;

	case e_event_commerceInstallContentStarted:
		app.DebugPrintf(4,"Install content Started\n");
		ProfileManager.SetSysUIShowing(true);
		break;
	case e_event_commerceInstallContentFinished:
		app.DebugPrintf(4,"Install content finished: 0x%x\n", m_errorCode);
		ProfileManager.SetSysUIShowing(false);
		runCallback();
		break;

	case e_event_commerceError:
		app.DebugPrintf(4,"Commerce Error 0x%x\n", m_errorCode);
		runCallback();
		break;
	default:
		break;
	}
	m_event = e_event_none;
}


int SonyCommerce_Vita::commerceEnd()
{
	int ret = 0;

	// 	if (m_currentPhase == e_phase_voucherRedeemPhase)
	// 		ret = sceNpCommerce2DoProductCodeFinishAsync(m_contextId);
	// 	else if (m_currentPhase == e_phase_productBrowsePhase)
	// 		ret = sceNpCommerce2DoProductBrowseFinishAsync(m_contextId);
	// 	else if (m_currentPhase == e_phase_creatingSessionPhase)
	// 		ret = sceNpCommerce2CreateSessionFinish(m_contextId, &m_sessionInfo);
	// 	else if (m_currentPhase == e_phase_checkoutPhase)
	// 		ret = sceNpCommerce2DoCheckoutFinishAsync(m_contextId);
	// 	else if (m_currentPhase == e_phase_downloadListPhase)
	// 		ret = sceNpCommerce2DoDlListFinishAsync(m_contextId);

	m_currentPhase = e_phase_idle;

	return ret;
}

void SonyCommerce_Vita::CreateSession( CallbackFunc cb, LPVOID lpParam )
{
	// 4J-PB - reset any previous error code
	// I had this happen when I was offline on Vita, and accepted the PSN sign-in
	// the m_errorCode was picked up in the message queue after the commerce init call
	if(m_errorCode!=0)
	{
		app.DebugPrintf("m_errorCode was set!\n");
		m_errorCode=0;
	}
	Init();
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_messageQueue.push(e_message_commerceCreateSession);
// 	m_messageQueue.push(e_message_commerceEnd);
// 	m_event = e_event_commerceSessionCreated;

	if(m_tickThread && (m_tickThread->isRunning() == false))
	{
		delete m_tickThread;
		m_tickThread = nullptr;
	}
	if(m_tickThread == nullptr)
		m_tickThread = new C4JThread(TickLoop, nullptr, "SonyCommerce_Vita tick");
	if(m_tickThread->isRunning() == false)
	{
		m_currentPhase = e_phase_idle;
		m_tickThread->Run();
	}
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Vita::CloseSession()
{
// 	assert(m_currentPhase == e_phase_idle);
	m_currentPhase = e_phase_stopped;
	Shutdown();
}

void SonyCommerce_Vita::GetProductList( CallbackFunc cb, LPVOID lpParam, std::vector<ProductInfo>* productList, const char *categoryId)
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pProductInfoList = productList;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceGetProductList);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Vita::GetDetailedProductInfo( CallbackFunc cb, LPVOID lpParam, ProductInfoDetailed* productInfo, const char *productId, const char *categoryId )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pProductInfoDetailed = productInfo;
	m_pProductID = productId;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceGetDetailedProductInfo);
	LeaveCriticalSection(&m_queueLock);
}

// 4J-PB - fill out the long description and the price for the product
void SonyCommerce_Vita::AddDetailedProductInfo( CallbackFunc cb, LPVOID lpParam, ProductInfo* productInfo, const char *productId, const char *categoryId )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pProductInfo = productInfo;
	m_pProductID = productId;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceAddDetailedProductInfo);
	LeaveCriticalSection(&m_queueLock);
}
void SonyCommerce_Vita::GetCategoryInfo( CallbackFunc cb, LPVOID lpParam, CategoryInfo *info, const char *categoryId )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pCategoryInfo = info;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceGetCategoryInfo);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Vita::Checkout( CallbackFunc cb, LPVOID lpParam, ProductInfo* productInfo )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_checkoutInputParams.skuIds.clear();
	m_checkoutInputParams.skuIds.push_back(productInfo->skuId);

	m_pProductInfo = productInfo;
	m_pProductID = productInfo->productId;

	m_messageQueue.push(e_message_commerceCheckout);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Vita::Checkout( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	assert(0);
}

void SonyCommerce_Vita::DownloadAlreadyPurchased( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_downloadInputParams.skuIds.clear();
	m_downloadInputParams.skuIds.push_back(skuID);
	m_messageQueue.push(e_message_commerceDownloadList);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Vita::Checkout_Game( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_checkoutInputParams.skuIds.clear();
	m_checkoutInputParams.skuIds.push_back(skuID);
	m_messageQueue.push(e_message_commerceCheckout_Game);
	LeaveCriticalSection(&m_queueLock);
}
void SonyCommerce_Vita::DownloadAlreadyPurchased_Game( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_downloadInputParams.skuIds.clear();
	m_downloadInputParams.skuIds.push_back(skuID);
	m_messageQueue.push(e_message_commerceDownloadList_Game);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Vita::InstallContent( CallbackFunc cb, LPVOID lpParam )
{
	if(m_callbackFunc == nullptr && m_messageQueue.size() == 0)		// wait till other processes have finished
	{
		EnterCriticalSection(&m_queueLock);
		m_bInstallingContent = true;
		setCallback(cb,lpParam);
		m_messageQueue.push(e_message_commerceInstallContent);
		LeaveCriticalSection(&m_queueLock);
	}
}

bool SonyCommerce_Vita::getPurchasabilityUpdated()
{
	bool retVal = m_bPurchasabilityUpdated; 
	m_bPurchasabilityUpdated = false; 
	return retVal;
}

bool SonyCommerce_Vita::getDLCUpgradePending()
{
	if(m_bDownloadsPending || m_bInstallingContent || (m_iClearDLCCountdown > 0))
		return true;
	return false;
}


void SonyCommerce_Vita::ShowPsStoreIcon()
{
	if(!s_showingPSStoreIcon)
	{
		sceNpCommerce2ShowPsStoreIcon(SCE_NP_COMMERCE2_ICON_DISP_RIGHT);
		s_showingPSStoreIcon = true;
	}
}

void SonyCommerce_Vita::HidePsStoreIcon()
{
	if(s_showingPSStoreIcon)
	{
		sceNpCommerce2HidePsStoreIcon();
		s_showingPSStoreIcon = false;
	}
}



/*
bool g_bDoCommerceCreateSession = false;
bool g_bDoCommerceGetProductList = false;
bool g_bDoCommerceGetCategoryInfo = false;
bool g_bDoCommerceGetProductInfoDetailed = false;
bool g_bDoCommerceCheckout = false;
bool g_bDoCommerceCloseSession = false;
const char* g_category = "EP4433-CUSA00265_00";
const char* g_skuID = "SKINPACK00000001-E001";
std::vector<SonyCommerce::ProductInfo> g_productInfo;
SonyCommerce::CategoryInfo g_categoryInfo2;
SonyCommerce::ProductInfoDetailed g_productInfoDetailed;

void testCallback(LPVOID lpParam, int error_code)
{
	app.DebugPrintf("Callback hit, error 0x%08x\n", error_code);
}

void SonyCommerce_Vita::Test()
{
	int err = SCE_OK;
	if(g_bDoCommerceCreateSession)
	{
		CreateSession(testCallback, this);
		g_bDoCommerceCreateSession = false;
	}
	if(g_bDoCommerceGetProductList)
	{
		GetProductList(testCallback, this, &g_productInfo, g_category);
		g_bDoCommerceGetProductList = false;
	}

	if(g_bDoCommerceGetCategoryInfo)
	{
		GetCategoryInfo(testCallback, this, &g_categoryInfo2, g_category);
		g_bDoCommerceGetCategoryInfo = false;
	}

	if(g_bDoCommerceGetProductInfoDetailed)
	{
		GetDetailedProductInfo(testCallback, this, &g_productInfoDetailed, g_productInfo[0].productId, g_category);
		g_bDoCommerceGetProductInfoDetailed = false;
	}

	if(g_bDoCommerceCheckout)
	{
		//Checkout(testCallback, this, g_skuID);//g_productInfoDetailed.skuId);
		Checkout(testCallback, this, g_productInfoDetailed.skuId);
		g_bDoCommerceCheckout = false;
	}
	if(g_bDoCommerceCloseSession)
	{
		CloseSession();
		g_bDoCommerceCloseSession = false;
	}

}
*/