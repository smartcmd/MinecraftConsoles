#include "stdafx.h"

#include "SonyCommerce_Orbis.h"
#include "PS3\PS3Extras\ShutdownManager.h"
#include <sys/event.h>


bool											SonyCommerce_Orbis::m_bCommerceInitialised = false;
// SceNpCommerce2SessionInfo					SonyCommerce_Orbis::m_sessionInfo;
SonyCommerce_Orbis::State						SonyCommerce_Orbis::m_state = e_state_noSession;
int												SonyCommerce_Orbis::m_errorCode = 0;
LPVOID											SonyCommerce_Orbis::m_callbackParam = NULL;

void*											SonyCommerce_Orbis::m_receiveBuffer = NULL;
SonyCommerce_Orbis::Event						SonyCommerce_Orbis::m_event;
std::queue<SonyCommerce_Orbis::Message>			SonyCommerce_Orbis::m_messageQueue;
std::vector<SonyCommerce_Orbis::ProductInfo>*	SonyCommerce_Orbis::m_pProductInfoList = NULL;
SonyCommerce_Orbis::ProductInfoDetailed*		SonyCommerce_Orbis::m_pProductInfoDetailed = NULL;
SonyCommerce_Orbis::ProductInfo*				SonyCommerce_Orbis::m_pProductInfo = NULL;

SonyCommerce_Orbis::CategoryInfo*				SonyCommerce_Orbis::m_pCategoryInfo = NULL;
const char*										SonyCommerce_Orbis::m_pProductID = NULL;
char*											SonyCommerce_Orbis::m_pCategoryID = NULL;
SonyCommerce_Orbis::CheckoutInputParams			SonyCommerce_Orbis::m_checkoutInputParams;
SonyCommerce_Orbis::DownloadListInputParams		SonyCommerce_Orbis::m_downloadInputParams;

SonyCommerce_Orbis::CallbackFunc				SonyCommerce_Orbis::m_callbackFunc = NULL;
// sys_memory_container_t						SonyCommerce_Orbis::m_memContainer = SYS_MEMORY_CONTAINER_ID_INVALID;
bool											SonyCommerce_Orbis::m_bUpgradingTrial = false;

SonyCommerce_Orbis::CallbackFunc				SonyCommerce_Orbis::m_trialUpgradeCallbackFunc;
LPVOID											SonyCommerce_Orbis::m_trialUpgradeCallbackParam;

CRITICAL_SECTION								SonyCommerce_Orbis::m_queueLock;

uint32_t										SonyCommerce_Orbis::m_contextId=0;		///< The npcommerce2 context ID
bool											SonyCommerce_Orbis::m_contextCreated=false;	///< npcommerce2 context ID created?
SonyCommerce_Orbis::Phase						SonyCommerce_Orbis::m_currentPhase = e_phase_stopped;		///< Current commerce2 util
// char											SonyCommerce_Orbis::m_commercebuffer[SCE_NP_COMMERCE2_RECV_BUF_SIZE];

C4JThread*										SonyCommerce_Orbis::m_tickThread = NULL;
bool											SonyCommerce_Orbis::m_bLicenseChecked=false;	 // Check the trial/full license for the game




sce::Toolkit::NP::Utilities::Future<std::vector<sce::Toolkit::NP::ProductInfo> >	g_productList;
sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::CategoryInfo>					g_categoryInfo;
sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::ProductInfoDetailed>			g_detailedProductInfo;


SonyCommerce_Orbis::ProductInfoDetailed s_trialUpgradeProductInfoDetailed;
void SonyCommerce_Orbis::Delete()
{
	m_pProductInfoList=NULL;
	m_pProductInfoDetailed=NULL;
	m_pProductInfo=NULL;
	m_pCategoryInfo = NULL;
	m_pProductID = NULL;
	m_pCategoryID = NULL;
}

void SonyCommerce_Orbis::Init()
{
	assert(m_state == e_state_noSession);
	if(!m_bCommerceInitialised)
	{
		m_bCommerceInitialised = true;
		m_pCategoryID=(char *)malloc(sizeof(char) * 100);
		InitializeCriticalSection(&m_queueLock);
	}
}



void SonyCommerce_Orbis::CheckForTrialUpgradeKey_Callback(LPVOID param, bool bFullVersion)
{
	ProfileManager.SetFullVersion(bFullVersion);
	if(ProfileManager.IsFullVersion())
	{
		StorageManager.SetSaveDisabled(false);
		ConsoleUIController::handleUnlockFullVersionCallback();
		// licence has been checked, so we're ok to install the trophies now
//  		ProfileManager.InitialiseTrophies(	SQRNetworkManager_Orbis::GetSceNpCommsId(),
//  										SQRNetworkManager_Orbis::GetSceNpCommsSig());
// 
	}
	m_bLicenseChecked=true;
}

bool SonyCommerce_Orbis::LicenseChecked() 
{
	return m_bLicenseChecked;
}

void SonyCommerce_Orbis::CheckForTrialUpgradeKey()
{
	StorageManager.CheckForTrialUpgradeKey(CheckForTrialUpgradeKey_Callback, NULL);
}

int SonyCommerce_Orbis::Shutdown()
{
	int ret=0;
	m_bCommerceInitialised = false;
	if (m_contextCreated) 
	{
		m_contextId = 0;
		m_contextCreated = false;
	}	

	delete m_pCategoryID;
	DeleteCriticalSection(&m_queueLock);

	// clear any possible callback function
	m_callbackFunc = NULL;
	
	return ret;
}



int SonyCommerce_Orbis::TickLoop(void* lpParam)
{
	ShutdownManager::HasStarted(ShutdownManager::eCommerceThread);
	while( (m_currentPhase != e_phase_stopped) && ShutdownManager::ShouldRun(ShutdownManager::eCommerceThread) )
	{
		processEvent();
		processMessage();
		Sleep(16); //  sleep for a frame
// 		((SonyCommerce_Orbis*)app.GetCommerce())->Test();
	}

	Shutdown();
	ShutdownManager::HasFinished(ShutdownManager::eCommerceThread);

	return 0;
}

void SonyCommerce_Orbis::copyProductList(std::vector<ProductInfo>* pProductList, std::vector<sce::Toolkit::NP::ProductInfo>* pNPProductList)
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

int SonyCommerce_Orbis::getProductList(std::vector<ProductInfo>* productList, char *categoryId)
{
	int ret;
	sce::Toolkit::NP::ProductListInputParams params;
	SceUserServiceUserId userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

	params.userInfo.userId = userId;
	strcpy(params.categoryId, categoryId);
	params.serviceLabel = 0;
	app.DebugPrintf("Getting Product List ...\n");

	ret = sce::Toolkit::NP::Commerce::Interface::getProductList(&g_productList, params, true);
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



void SonyCommerce_Orbis::copyCategoryInfo(CategoryInfo *pInfo, sce::Toolkit::NP::CategoryInfo *pNPInfo)
{
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

int SonyCommerce_Orbis::getCategoryInfo(CategoryInfo *pInfo, char *categoryId)
{


	int ret;
	sce::Toolkit::NP::CategoryInfoInputParams params;
	SceUserServiceUserId userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

	params.userInfo.userId = userId;
	strcpy(params.categoryId, "");//categoryId);
	params.serviceLabel = 0;

	app.DebugPrintf("Getting Category Information...\n");

	ret = sce::Toolkit::NP::Commerce::Interface::getCategoryInfo(&g_categoryInfo, params, true);
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

void SonyCommerce_Orbis::copyDetailedProductInfo(ProductInfoDetailed *pInfo, sce::Toolkit::NP::ProductInfoDetailed* pNPInfo)
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
	pInfo->ui32Price= pNPInfo->intPrice;
	strncpy(pInfo->price, pNPInfo->price, SCE_TOOLKIT_NP_SKU_PRICE_LEN);

}
int SonyCommerce_Orbis::getDetailedProductInfo(ProductInfoDetailed *pInfo, const char *productId, char *categoryId)
{
	int ret;
	sce::Toolkit::NP::DetailedProductInfoInputParams params;
	SceUserServiceUserId userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

	params.userInfo.userId = userId;
	strcpy(params.categoryId, categoryId);
	strcpy(params.productId, productId);


	app.DebugPrintf("Getting Detailed Product Information ...  \n");			
	ret = sce::Toolkit::NP::Commerce::Interface::getDetailedProductInfo(&g_detailedProductInfo, params, true);
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

void SonyCommerce_Orbis::copyAddDetailedProductInfo(ProductInfo *pInfo, sce::Toolkit::NP::ProductInfoDetailed* pNPInfo)
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
	pInfo->ui32Price= pNPInfo->intPrice;
	strncpy(pInfo->price, pNPInfo->price, SCE_TOOLKIT_NP_SKU_PRICE_LEN);

}

int SonyCommerce_Orbis::addDetailedProductInfo(ProductInfo *pInfo, const char *productId, char *categoryId)
{
	int ret;
	sce::Toolkit::NP::DetailedProductInfoInputParams params;
	SceUserServiceUserId userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());

	params.userInfo.userId = userId;
	strcpy(params.categoryId, categoryId);
	strcpy(params.productId, productId);


	app.DebugPrintf("Getting Detailed Product Information ...  \n");			
	ret = sce::Toolkit::NP::Commerce::Interface::getDetailedProductInfo(&g_detailedProductInfo, params, true);
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


int SonyCommerce_Orbis::checkout(CheckoutInputParams &params)
{

	int ret;
	sce::Toolkit::NP::CheckoutInputParams npParams;
	SceUserServiceUserId userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());
	npParams.userInfo.userId = userId;
	npParams.serviceLabel = 0;

	std::list<const char*>::iterator iter = params.skuIds.begin();
	std::list<const char*>::iterator iterEnd = params.skuIds.end();
	while(iter != iterEnd) 
	{
		npParams.skuIds.push_back((char*)*iter);		// have to remove the const here, not sure why the libs pointers aren't const
		iter++;
	}

	app.DebugPrintf("Starting Checkout...\n");

	ret = sce::Toolkit::NP::Commerce::Interface::checkout(npParams, false);
	if (ret < 0) 
	{
		app.DebugPrintf("Sample menu checkout() error. ret = 0x%x\n", ret);
	}


	return ret;	
}


int SonyCommerce_Orbis::downloadList(DownloadListInputParams &params)
{
	
	int ret;
	sce::Toolkit::NP::DownloadListInputParams npParams;
	//memset(&npParams,0,sizeof(sce::Toolkit::NP::DownloadListInputParams));
	SceUserServiceUserId userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());
	npParams.userInfo.userId = userId;
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
	ret = sce::Toolkit::NP::Commerce::Interface::displayDownloadList(npParams, true);
	if (ret < 0) 
	{
		app.DebugPrintf("Commerce::Interface::displayDownloadList error: 0x%x\n", ret);
	}

	return ret;	
}

int SonyCommerce_Orbis::checkout_game(CheckoutInputParams &params)
{

	int ret;
	sce::Toolkit::NP::CheckoutInputParams npParams;
	SceUserServiceUserId userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());
	npParams.userInfo.userId = userId;
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
	Myparams.userInfo.userId = userId; 
	strncpy(Myparams.productId, "MINECRAFTPS40000", strlen("MINECRAFTPS40000")); 

	ret = sce::Toolkit::NP::Commerce::Interface::productBrowse(Myparams, false); 
	if (ret < 0) { 
		// Error handling 
	} 


	//ret = sce::Toolkit::NP::Commerce::Interface::checkout(npParams, false);
	if (ret < 0) 
	{
		app.DebugPrintf("Sample menu checkout() error. ret = 0x%x\n", ret);
	}


	return ret;	
}

int SonyCommerce_Orbis::downloadList_game(DownloadListInputParams &params)
{

	int ret;
	sce::Toolkit::NP::DownloadListInputParams npParams;
	//memset(&npParams,0,sizeof(sce::Toolkit::NP::DownloadListInputParams));
	SceUserServiceUserId userId = ProfileManager.getUserID(ProfileManager.GetPrimaryPad());
	npParams.userInfo.userId = userId;
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
	Myparams.userInfo.userId = userId; 
	strncpy(Myparams.productId, "MINECRAFTPS40000", strlen("MINECRAFTPS40000")); 

	ret = sce::Toolkit::NP::Commerce::Interface::productBrowse(Myparams, false); 
	if (ret < 0) { 
		// Error handling 
		app.DebugPrintf("Commerce::Interface::displayDownloadList error: 0x%x\n", ret);
	} 



	return ret;	
}

void SonyCommerce_Orbis::UpgradeTrialCallback2(LPVOID lpParam,int err)
{
	SonyCommerce* pCommerce = (SonyCommerce*)lpParam;
	app.DebugPrintf(4,"SonyCommerce_UpgradeTrialCallback2 : err : 0x%08x\n", err);
	pCommerce->CheckForTrialUpgradeKey();
	if(err != SCE_OK)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		C4JStorage::EMessageResult result = ui.RequestMessageBox( IDS_PRO_UNLOCKGAME_TITLE, IDS_NO_DLCOFFERS, uiIDA,1,ProfileManager.GetPrimaryPad());
	}
	m_trialUpgradeCallbackFunc(m_trialUpgradeCallbackParam, m_errorCode);
}

void SonyCommerce_Orbis::UpgradeTrialCallback1(LPVOID lpParam,int err)
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
		C4JStorage::EMessageResult result = ui.RequestMessageBox( IDS_PRO_UNLOCKGAME_TITLE, IDS_NO_DLCOFFERS, uiIDA,1,ProfileManager.GetPrimaryPad());
		m_trialUpgradeCallbackFunc(m_trialUpgradeCallbackParam, m_errorCode);
	}
}



// global func, so we can call from the profile lib
void SonyCommerce_UpgradeTrial()
{
	// we're now calling the app function here, which manages pending requests
	app.UpgradeTrial();
}

void SonyCommerce_Orbis::UpgradeTrial(CallbackFunc cb, LPVOID lpParam)
{
	m_trialUpgradeCallbackFunc = cb;
	m_trialUpgradeCallbackParam = lpParam;
	
// 	static char szTrialUpgradeSkuID[64];
// 	sprintf(szTrialUpgradeSkuID, "%s-TRIALUPGRADE0001", app.GetCommerceCategory());//, szSKUSuffix);
	GetDetailedProductInfo(UpgradeTrialCallback1, this, &s_trialUpgradeProductInfoDetailed, app.GetUpgradeKey(), app.GetCommerceCategory());
}


int SonyCommerce_Orbis::createContext()
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
// 	ret = sceNpCommerce2CreateCtx(SCE_NP_COMMERCE2_VERSION, &npId, commerce2Handler, NULL, &m_contextId);
// 	if (ret < 0) 
// 	{
// 		app.DebugPrintf(4,"createContext sceNpCommerce2CreateCtx problem\n");
// 		return ret;
// 	}

	m_contextCreated = true;

	return SCE_OK;
}

int SonyCommerce_Orbis::createSession()
{
	// From the Sony docs
	//
	// sce::Toolkit::NP::Commerce::Interface::CreateSession
	// 
	// 	This function is provided to maintain compatibility with the PlayStation®Vita and PlayStation®3 platforms. Because commerce on the PlayStation®4 is not session based, SCE_TOOLKIT_NP_SUCCESS is always returned.

	int ret = sce::Toolkit::NP::Commerce::Interface::createSession();

	if (ret < 0) 
	{
		return ret;
	}
	m_currentPhase = e_phase_creatingSessionPhase;

	return ret;
}


void SonyCommerce_Orbis::commerce2Handler( const sce::Toolkit::NP::Event& event)
{
// 	Event reply;
// 	reply.service = Toolkit::NP::commerce;
// 
	if(m_bCommerceInitialised==false)
	{
		// 4J-PB - this happens when we've signed out of the PSn, and we were in the process of retrieving DLC product info
		// Ignore this event
		app.DebugPrintf("@@@@@@@@ IGNORING COMMERCE EVENT AFTER COMMERCE SHUTDOWN @@@@@@@@@\n");
		return;
	}
	EnterCriticalSection(&m_queueLock);

	switch (event.event)
	{
	case sce::Toolkit::NP::Event::UserEvent::commerceError:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_errorCode = event.returnCode;
			break;
		}
	case sce::Toolkit::NP::Event::UserEvent::commerceSessionCreated:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceSessionCreated;
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
			copyCategoryInfo(m_pCategoryInfo, g_categoryInfo.get());
			m_pCategoryInfo = NULL;
			m_event = e_event_commerceGotCategoryInfo;
			break;
		}

	case sce::Toolkit::NP::Event::UserEvent::commerceGotProductList:
		{
			copyProductList(m_pProductInfoList, g_productList.get());
			m_pProductInfoDetailed = NULL;
			m_event = e_event_commerceGotProductList;
			break;
		}

	case sce::Toolkit::NP::Event::UserEvent::commerceGotDetailedProductInfo:
		{
			if(m_pProductInfoDetailed)
			{
				copyDetailedProductInfo(m_pProductInfoDetailed, g_detailedProductInfo.get());
				m_pProductInfoDetailed = NULL;
			}
			else
			{
				copyAddDetailedProductInfo(m_pProductInfo, g_detailedProductInfo.get());
				m_pProductInfo = NULL;
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



void SonyCommerce_Orbis::processMessage()
{
	EnterCriticalSection(&m_queueLock);
	int ret;
	if(m_messageQueue.empty())
	{
		LeaveCriticalSection(&m_queueLock);
		return;
	}
	Message msg = m_messageQueue.front();
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


void SonyCommerce_Orbis::processEvent()
{
	int ret = 0;

	switch (m_event) 
	{
	case e_event_none:
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

		if(m_callbackFunc!=NULL)
		{
			runCallback();
		}

		//assert(0);
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
		break;
	case e_event_commerceCheckoutSuccess:
		app.DebugPrintf(4,"Checkout succeeded: 0x%x\n", m_errorCode);
		// clear the DLC installed and check again
		app.ClearDLCInstalled();
		ui.HandleDLCInstalled(0);
		break;
	case e_event_commerceCheckoutAborted:
		app.DebugPrintf(4,"Checkout aborted: 0x%x\n", m_errorCode);
		break;
	case e_event_commerceCheckoutFinished:
		app.DebugPrintf(4,"Checkout Finished: 0x%x\n", m_errorCode);
		if (ret < 0) {
			app.DebugPrintf(4,"Failed to destroy memory container");
		}

		// 4J-PB - if there's been an error - like dlc already purchased, the runcallback has already happened, and will crash this time
		if(m_callbackFunc!=NULL)
		{
			runCallback();
		}
		break;
	case e_event_commerceDownloadListStarted:
		app.DebugPrintf(4,"Download List Started\n");
		break;
	case e_event_commerceDownloadListSuccess:
		app.DebugPrintf(4,"Download succeeded: 0x%x\n", m_errorCode);
		break;
	case e_event_commerceDownloadListFinished:
		app.DebugPrintf(4,"Download Finished: 0x%x\n", m_errorCode);
		if (ret < 0) {
			app.DebugPrintf(4,"Failed to destroy memory container");
		}

		// 4J-PB - if there's been an error - like dlc already purchased, the runcallback has already happened, and will crash this time
		if(m_callbackFunc!=NULL)
		{
			runCallback();
		}
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


int SonyCommerce_Orbis::commerceEnd()
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

void SonyCommerce_Orbis::CreateSession( CallbackFunc cb, LPVOID lpParam )
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

	// We don't need to create a session on PS4, from the Sony docs - 
//	This function is provided to maintain compatibility with the PlayStation®Vita and PlayStation®3 
//	platforms. Because commerce on the PlayStation®4 is not session based, SCE_TOOLKIT_NP_SUCCESS is always returned.



//	m_messageQueue.push(e_message_commerceCreateSession);
	m_messageQueue.push(e_message_commerceEnd);
	m_event = e_event_commerceSessionCreated;

	if(m_tickThread == NULL)
		m_tickThread = new C4JThread(TickLoop, NULL, "SonyCommerce_Orbis tick");
	if(m_tickThread->isRunning() == false)
	{
		m_currentPhase = e_phase_idle;
		m_tickThread->Run();
	}
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Orbis::CloseSession()
{
	assert(m_currentPhase == e_phase_idle);
	m_currentPhase = e_phase_stopped;
	// 4J-PB - don't call shutdown here - the SonyCommerce_Orbis::TickLoop thread is still running and could crash accessing the critical section that Shutdown destroys
	//Shutdown();
}

void SonyCommerce_Orbis::GetProductList( CallbackFunc cb, LPVOID lpParam, std::vector<ProductInfo>* productList, const char *categoryId)
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pProductInfoList = productList;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceGetProductList);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Orbis::GetDetailedProductInfo( CallbackFunc cb, LPVOID lpParam, ProductInfoDetailed* productInfo, const char *productId, const char *categoryId )
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
void SonyCommerce_Orbis::AddDetailedProductInfo( CallbackFunc cb, LPVOID lpParam, ProductInfo* productInfo, const char *productId, const char *categoryId )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pProductInfo = productInfo;
	m_pProductID = productId;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceAddDetailedProductInfo);
	LeaveCriticalSection(&m_queueLock);
}
void SonyCommerce_Orbis::GetCategoryInfo( CallbackFunc cb, LPVOID lpParam, CategoryInfo *info, const char *categoryId )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pCategoryInfo = info;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceGetCategoryInfo);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Orbis::Checkout( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_checkoutInputParams.skuIds.clear();
	m_checkoutInputParams.skuIds.push_back(skuID);
	m_messageQueue.push(e_message_commerceCheckout);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Orbis::DownloadAlreadyPurchased( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_downloadInputParams.skuIds.clear();
	m_downloadInputParams.skuIds.push_back(skuID);
	m_messageQueue.push(e_message_commerceDownloadList);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_Orbis::Checkout_Game( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_checkoutInputParams.skuIds.clear();
	m_checkoutInputParams.skuIds.push_back(skuID);
	m_messageQueue.push(e_message_commerceCheckout_Game);
	LeaveCriticalSection(&m_queueLock);
}
void SonyCommerce_Orbis::DownloadAlreadyPurchased_Game( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_downloadInputParams.skuIds.clear();
	m_downloadInputParams.skuIds.push_back(skuID);
	m_messageQueue.push(e_message_commerceDownloadList_Game);
	LeaveCriticalSection(&m_queueLock);
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

void SonyCommerce_Orbis::Test()
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