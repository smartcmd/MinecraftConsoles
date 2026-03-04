#include "stdafx.h"

#include "SonyCommerce_PS3.h"
#include "..\PS3Extras\ShutdownManager.h"
#include <sys/event.h>


bool									SonyCommerce_PS3::m_bCommerceInitialised = false;
SceNpCommerce2SessionInfo				SonyCommerce_PS3::m_sessionInfo;
SonyCommerce_PS3::State						SonyCommerce_PS3::m_state = e_state_noSession;
int										SonyCommerce_PS3::m_errorCode = 0;
LPVOID									SonyCommerce_PS3::m_callbackParam = NULL;

void*									SonyCommerce_PS3::m_receiveBuffer = NULL;
SonyCommerce_PS3::Event						SonyCommerce_PS3::m_event;
std::queue<SonyCommerce_PS3::Message>		SonyCommerce_PS3::m_messageQueue;
std::vector<SonyCommerce_PS3::ProductInfo>*	SonyCommerce_PS3::m_pProductInfoList = NULL;
SonyCommerce_PS3::ProductInfoDetailed*		SonyCommerce_PS3::m_pProductInfoDetailed = NULL;
SonyCommerce_PS3::ProductInfo*				SonyCommerce_PS3::m_pProductInfo = NULL;

SonyCommerce_PS3::CategoryInfo*				SonyCommerce_PS3::m_pCategoryInfo = NULL;
const char*								SonyCommerce_PS3::m_pProductID = NULL;
char*									SonyCommerce_PS3::m_pCategoryID = NULL;
SonyCommerce_PS3::CheckoutInputParams		SonyCommerce_PS3::m_checkoutInputParams;
SonyCommerce_PS3::DownloadListInputParams	SonyCommerce_PS3::m_downloadInputParams;

SonyCommerce_PS3::CallbackFunc				SonyCommerce_PS3::m_callbackFunc = NULL;
sys_memory_container_t					SonyCommerce_PS3::m_memContainer = SYS_MEMORY_CONTAINER_ID_INVALID;
bool									SonyCommerce_PS3::m_bUpgradingTrial = false;

SonyCommerce_PS3::CallbackFunc				SonyCommerce_PS3::m_trialUpgradeCallbackFunc;
LPVOID									SonyCommerce_PS3::m_trialUpgradeCallbackParam;

CRITICAL_SECTION						SonyCommerce_PS3::m_queueLock;



uint32_t				SonyCommerce_PS3::m_contextId=0;		///< The npcommerce2 context ID
bool					SonyCommerce_PS3::m_contextCreated=false;	///< npcommerce2 context ID created?
SonyCommerce_PS3::Phase		SonyCommerce_PS3::m_currentPhase = e_phase_stopped;		///< Current commerce2 util
char					SonyCommerce_PS3::m_commercebuffer[SCE_NP_COMMERCE2_RECV_BUF_SIZE];

C4JThread*				SonyCommerce_PS3::m_tickThread = NULL;
bool					SonyCommerce_PS3::m_bLicenseChecked=false;	 // Check the trial/full license for the game


SonyCommerce_PS3::ProductInfoDetailed s_trialUpgradeProductInfoDetailed;
void SonyCommerce_PS3::Delete()
{
	m_pProductInfoList=NULL;
	m_pProductInfoDetailed=NULL;
	m_pProductInfo=NULL;
	m_pCategoryInfo = NULL;
	m_pProductID = NULL;
	m_pCategoryID = NULL;
}
void SonyCommerce_PS3::Init()
{
	int ret;

	assert(m_state == e_state_noSession);
	if(!m_bCommerceInitialised)
	{
		ret =  sceNpCommerce2Init();
		if (ret < 0) 
		{
			app.DebugPrintf(4,"sceNpCommerce2Init failed (0x%x)\n", ret);
			return;
		}
		else
		{
			m_bCommerceInitialised = true;
		}
		m_pCategoryID=(char *)malloc(sizeof(char) * 100);
		InitializeCriticalSection(&m_queueLock);
	}

	return ;

}



void SonyCommerce_PS3::CheckForTrialUpgradeKey_Callback(LPVOID param, bool bFullVersion)
{
	ProfileManager.SetFullVersion(bFullVersion);
	if(ProfileManager.IsFullVersion())
	{
		StorageManager.SetSaveDisabled(false);
		ConsoleUIController::handleUnlockFullVersionCallback();
		// licence has been checked, so we're ok to install the trophies now
 		ProfileManager.InitialiseTrophies(	SQRNetworkManager_PS3::GetSceNpCommsId(),
 										SQRNetworkManager_PS3::GetSceNpCommsSig());

	}
	m_bLicenseChecked=true;
}

bool SonyCommerce_PS3::LicenseChecked() 
{
	return m_bLicenseChecked;
}

void SonyCommerce_PS3::CheckForTrialUpgradeKey()
{
	// 4J-PB - If we are the blu ray disc then we are the full version
	if(StorageManager.GetBootTypeDisc())
	{
		CheckForTrialUpgradeKey_Callback(NULL,true);
	}
	else
	{
		StorageManager.CheckForTrialUpgradeKey(CheckForTrialUpgradeKey_Callback, NULL);
	}
}

int SonyCommerce_PS3::Shutdown()
{
	int ret=0;
	if (m_contextCreated) 
	{
		ret = sceNpCommerce2DestroyCtx(m_contextId);
		if (ret != 0) 
		{
			return ret;
		}

		m_contextId = 0;
		m_contextCreated = false;
	}	

	ret = sceNpCommerce2Term();
	m_bCommerceInitialised = false;
	if (ret != 0) 
	{
		return ret;
	}
	delete m_pCategoryID;
	DeleteCriticalSection(&m_queueLock);

	return ret;
}



int SonyCommerce_PS3::TickLoop(void* lpParam)
{
	ShutdownManager::HasStarted(ShutdownManager::eCommerceThread);
	while( (m_currentPhase != e_phase_stopped) && ShutdownManager::ShouldRun(ShutdownManager::eCommerceThread) )
	{
		processEvent();
		processMessage();
		Sleep(16); //  sleep for a frame
	}

	ShutdownManager::HasFinished(ShutdownManager::eCommerceThread);

	return 0;
}

int SonyCommerce_PS3::getProductList(std::vector<ProductInfo>* productList, char *categoryId)
{
	int											ret = 0;
	uint32_t									requestId;
	size_t										bufSize = sizeof(m_commercebuffer);
	size_t										fillSize = 0;
	SceNpCommerce2GetCategoryContentsResult		result;
	SceNpCommerce2CategoryInfo					categoryInfo;
	SceNpCommerce2ContentInfo					contentInfo;
	SceNpCommerce2GameProductInfo				productInfo;
	SceNpCommerce2GameSkuInfo					skuInfo;
	ProductInfo									tempInfo;
	std::vector<ProductInfo>					tempProductVec;

	if (!m_contextCreated) 
	{
		ret = createContext();
		if (ret < 0) 
		{
			setError(ret);
			return ret;
		}
	}

	// Create request ID
	ret = sceNpCommerce2GetCategoryContentsCreateReq(m_contextId, &requestId);
	if (ret < 0) 
	{
		setError(ret);
		return ret;
	}

	// Obtain category content data
	ret = sceNpCommerce2GetCategoryContentsStart(requestId, categoryId, 0, SCE_NP_COMMERCE2_GETCAT_MAX_COUNT);	
	if (ret < 0) 
	{
		sceNpCommerce2DestroyReq(requestId);
		setError(ret);
		return ret;
	}

	ret = sceNpCommerce2GetCategoryContentsGetResult(requestId, m_commercebuffer, bufSize, &fillSize);
	if (ret < 0) 
	{
		sceNpCommerce2DestroyReq(requestId);
		setError(ret);
		return ret;
	}

	ret = sceNpCommerce2DestroyReq(requestId);
	if (ret < 0) 
	{
		setError(ret);
		return ret;
	}

	// We have the initial category content data,
	// now to take out the category content information.
	ret = sceNpCommerce2InitGetCategoryContentsResult(&result, m_commercebuffer, fillSize);
	if (ret < 0) 
	{
		setError(ret);
		return ret;
	}

	// Get the category information
	ret = sceNpCommerce2GetCategoryInfo(&result, &categoryInfo);
	if (ret < 0) 
	{
		sceNpCommerce2DestroyGetCategoryContentsResult(&result);
		setError(ret);
		return ret;
	}

	if(categoryInfo.countOfProduct==0)
	{
		// There is no DLC
		return 0;
	}

	// Reserve some space
	tempProductVec.reserve(categoryInfo.countOfProduct);

	// For each product, obtain information
	for (int i = 0; i < result.rangeOfContents.count; i++) 
	{
		ret = sceNpCommerce2GetContentInfo(&result, i, &contentInfo);
		if (ret < 0) 
		{
			sceNpCommerce2DestroyGetCategoryContentsResult(&result);
			setError(ret);
			return ret;
		}

		// Only process if it is a product
		if (contentInfo.contentType == SCE_NP_COMMERCE2_CONTENT_TYPE_PRODUCT) 
		{

			// reset tempInfo
			memset(&tempInfo, 0x0, sizeof(tempInfo));

			// Get product info
			ret = sceNpCommerce2GetGameProductInfoFromContentInfo(&contentInfo, &productInfo);
			if (ret < 0) 
			{
				sceNpCommerce2DestroyGetCategoryContentsResult(&result);
				setError(ret);
				return ret;
			}

			// populate our temp struct

			strncpy(tempInfo.productId, productInfo.productId, SCE_NP_COMMERCE2_PRODUCT_ID_LEN);
			strncpy(tempInfo.productName, productInfo.productName, SCE_NP_COMMERCE2_PRODUCT_NAME_LEN);
			strncpy(tempInfo.shortDescription, productInfo.productShortDescription, SCE_NP_COMMERCE2_PRODUCT_SHORT_DESCRIPTION_LEN);
			if(tempInfo.longDescription[0]!=0)
			{
				strncpy(tempInfo.longDescription, productInfo.productLongDescription, SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN);
			}
			else
			{
#ifdef _DEBUG
				strcpy(tempInfo.longDescription,"Missing long description");
#endif
			}
			strncpy(tempInfo.spName, productInfo.spName, SCE_NP_COMMERCE2_SP_NAME_LEN);
			strncpy(tempInfo.imageUrl, productInfo.imageUrl, SCE_NP_COMMERCE2_URL_LEN);
			tempInfo.releaseDate = productInfo.releaseDate;

			if (productInfo.countOfSku == 1) 
			{
				// Get SKU info
				ret = sceNpCommerce2GetGameSkuInfoFromGameProductInfo(&productInfo, 0, &skuInfo);
				if (ret < 0) 
				{
					sceNpCommerce2DestroyGetCategoryContentsResult(&result);
					setError(ret);
					return ret;
				}
				tempInfo.purchasabilityFlag = skuInfo.purchasabilityFlag;

				// Take out the price. Nicely formatted
				// but also keep the price as a value in case it's 0 - we need to show "free" for that
				tempInfo.ui32Price= skuInfo.price;
				ret = sceNpCommerce2GetPrice(m_contextId, tempInfo.price, sizeof(tempInfo.price), skuInfo.price);
				
				if (ret < 0) 
				{
					sceNpCommerce2DestroyGetCategoryContentsResult(&result);
					setError(ret);
					return ret;
				}
			}
			tempProductVec.push_back(tempInfo);
		} 
	}

	// Set our result
	*productList = tempProductVec;

	// Destroy the category contents result
	ret = sceNpCommerce2DestroyGetCategoryContentsResult(&result);
	if (ret < 0) 
	{
		return ret;
	}

	return ret;
}

int SonyCommerce_PS3::getCategoryInfo(CategoryInfo *pInfo, char *categoryId)
{
	int											ret = 0;
	uint32_t									requestId;
	size_t										bufSize = sizeof(m_commercebuffer);
	size_t										fillSize = 0;
	SceNpCommerce2GetCategoryContentsResult		result;
	SceNpCommerce2CategoryInfo					categoryInfo;
	SceNpCommerce2ContentInfo					contentInfo;
	//CategoryInfo								tempCatInfo;
	CategoryInfoSub								tempSubCatInfo;

	if (!m_contextCreated) 
	{
		ret = createContext();
		if (ret < 0) 
		{
			m_errorCode = ret;
			return ret;
		}
	}

	// Create request ID
	ret = sceNpCommerce2GetCategoryContentsCreateReq(m_contextId, &requestId);
	if (ret < 0) 
	{
		m_errorCode = ret;
		return ret;
	}

	// Obtain category content data
	if (categoryId) 
	{
		ret = sceNpCommerce2GetCategoryContentsStart(requestId, categoryId, 0, SCE_NP_COMMERCE2_GETCAT_MAX_COUNT);	
	} 
	else 
	{
		ret = sceNpCommerce2GetCategoryContentsStart(requestId,categoryId,
			0, SCE_NP_COMMERCE2_GETCAT_MAX_COUNT);
	}
	if (ret < 0) 
	{
		sceNpCommerce2DestroyReq(requestId);
		m_errorCode = ret;
		return ret;
	}

	ret = sceNpCommerce2GetCategoryContentsGetResult(requestId, m_commercebuffer, bufSize, &fillSize);
	if (ret < 0) 
	{
		if(ret==SCE_NP_COMMERCE2_ERROR_SERVER_MAINTENANCE)
		{
			app.DebugPrintf(4,"\n--- SCE_NP_COMMERCE2_ERROR_SERVER_MAINTENANCE ---\n\n");
		}
		sceNpCommerce2DestroyReq(requestId);
		m_errorCode = ret;
		return ret;
	}

	ret = sceNpCommerce2DestroyReq(requestId);
	if (ret < 0) 
	{
		m_errorCode = ret;
		return ret;
	}

	// We have the initial category content data,
	// now to take out the category content information.
	ret = sceNpCommerce2InitGetCategoryContentsResult(&result, m_commercebuffer, fillSize);
	if (ret < 0) {
		m_errorCode = ret;
		return ret;
	}

	// Get the category information
	ret = sceNpCommerce2GetCategoryInfo(&result, &categoryInfo);
	if (ret < 0) {
		sceNpCommerce2DestroyGetCategoryContentsResult(&result);
		m_errorCode = ret;
		return ret;
	}

	strcpy(pInfo->current.categoryId, categoryInfo.categoryId);
	strcpy(pInfo->current.categoryName, categoryInfo.categoryName);
	strcpy(pInfo->current.categoryDescription, categoryInfo.categoryDescription);
	strcpy(pInfo->current.imageUrl, categoryInfo.imageUrl);
	pInfo->countOfProducts = categoryInfo.countOfProduct;
	pInfo->countOfSubCategories = categoryInfo.countOfSubCategory;

	if (categoryInfo.countOfSubCategory > 0) 
	{
		// For each  sub category, obtain information
		for (int i = 0; i < result.rangeOfContents.count; i++) 
		{

			ret = sceNpCommerce2GetContentInfo(&result, i, &contentInfo);
			if (ret < 0) 
			{
				sceNpCommerce2DestroyGetCategoryContentsResult(&result);
				m_errorCode = ret;
				return ret;
			}

			// Only process if it is a category	
			if (contentInfo.contentType == SCE_NP_COMMERCE2_CONTENT_TYPE_CATEGORY) 
			{

				ret = sceNpCommerce2GetCategoryInfoFromContentInfo(&contentInfo, &categoryInfo);
				if (ret < 0) 
				{
					sceNpCommerce2DestroyGetCategoryContentsResult(&result);
					m_errorCode = ret;
					return ret;
				}

				strcpy(tempSubCatInfo.categoryId, categoryInfo.categoryId);
				strcpy(tempSubCatInfo.categoryName, categoryInfo.categoryName);
				strcpy(tempSubCatInfo.categoryDescription, categoryInfo.categoryDescription);
				strcpy(tempSubCatInfo.imageUrl, categoryInfo.imageUrl);

				// Add to the list
				pInfo->subCategories.push_back(tempSubCatInfo);
			}
		}
	}

	// Set our result
	//*info = tempCatInfo;

	// Destroy the category contents result
	ret = sceNpCommerce2DestroyGetCategoryContentsResult(&result);
	if (ret < 0) {
		return ret;
	}

	return ret;
}


int SonyCommerce_PS3::getDetailedProductInfo(ProductInfoDetailed *pInfo, const char *productId, char *categoryId)
{
	int													ret = 0;
	uint32_t											requestId;
	size_t												bufSize = sizeof(m_commercebuffer);
	size_t												fillSize = 0;
	std::list<SceNpCommerce2ContentRatingDescriptor>	ratingDescList;
	SceNpCommerce2GetProductInfoResult					result;
	SceNpCommerce2ContentRatingInfo						ratingInfo;
	SceNpCommerce2GameProductInfo						productInfo;
	SceNpCommerce2GameSkuInfo							skuInfo;
	//ProductInfoDetailed									tempInfo;

	if (!m_contextCreated) {
		ret = createContext();
		if (ret < 0) {
			m_errorCode = ret;
			return ret;
		}
	}

	// Obtain product data
	ret = sceNpCommerce2GetProductInfoCreateReq(m_contextId, &requestId);
	if (ret < 0) {
		m_errorCode = ret;
		return ret;
	}

	if (categoryId && categoryId[0] != 0) {
		ret = sceNpCommerce2GetProductInfoStart(requestId, categoryId, productId);
	} else {
		ret = sceNpCommerce2GetProductInfoStart(requestId, NULL, productId);		
	}
	if (ret < 0) {
		sceNpCommerce2DestroyReq(requestId);
		m_errorCode = ret;
		return ret;
	}

	ret = sceNpCommerce2GetProductInfoGetResult(requestId, m_commercebuffer, bufSize, &fillSize);
	if (ret < 0) {
		sceNpCommerce2DestroyReq(requestId);
		m_errorCode = ret;
		return ret;
	}

	ret = sceNpCommerce2DestroyReq(requestId);
	if (ret < 0) 
	{
		m_errorCode = ret;
		return ret;
	}

	// Take Out Game Product Information
	ret = sceNpCommerce2InitGetProductInfoResult(&result, m_commercebuffer, fillSize);
	if (ret < 0) 
	{
		m_errorCode = ret;
		return ret;
	}

	ret = sceNpCommerce2GetGameProductInfo(&result, &productInfo);
	if (ret < 0) 
	{
		sceNpCommerce2DestroyGetProductInfoResult(&result);
		m_errorCode = ret;
		return ret;
	}

	// Get rating info
	ret = sceNpCommerce2GetContentRatingInfoFromGameProductInfo(&productInfo, &ratingInfo);
	if (ret < 0) 
	{
		sceNpCommerce2DestroyGetProductInfoResult(&result);
		m_errorCode = ret;
		return ret;
	}

	for (int index = 0; index < ratingInfo.countOfContentRatingDescriptor; index++) 
	{
		SceNpCommerce2ContentRatingDescriptor desc;
		sceNpCommerce2GetContentRatingDescriptor(&ratingInfo, index, &desc);
		ratingDescList.push_back(desc);		
	}

	// populate our temp struct
	pInfo->ratingDescriptors = ratingDescList;
	strncpy(pInfo->productId, productInfo.productId, SCE_NP_COMMERCE2_PRODUCT_ID_LEN);
	strncpy(pInfo->productName, productInfo.productName, SCE_NP_COMMERCE2_PRODUCT_NAME_LEN);
	strncpy(pInfo->shortDescription, productInfo.productShortDescription, SCE_NP_COMMERCE2_PRODUCT_SHORT_DESCRIPTION_LEN);
	strncpy(pInfo->longDescription, productInfo.productLongDescription, SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN);
	strncpy(pInfo->legalDescription, productInfo.legalDescription, SCE_NP_COMMERCE2_PRODUCT_LEGAL_DESCRIPTION_LEN);
	strncpy(pInfo->spName, productInfo.spName, SCE_NP_COMMERCE2_SP_NAME_LEN);
	strncpy(pInfo->imageUrl, productInfo.imageUrl, SCE_NP_COMMERCE2_URL_LEN);
	pInfo->releaseDate = productInfo.releaseDate;
	strncpy(pInfo->ratingSystemId, ratingInfo.ratingSystemId, SCE_NP_COMMERCE2_RATING_SYSTEM_ID_LEN);
	strncpy(pInfo->ratingImageUrl, ratingInfo.imageUrl, SCE_NP_COMMERCE2_URL_LEN);

	// Get SKU info
	if (productInfo.countOfSku == 1) 
	{
		ret = sceNpCommerce2GetGameSkuInfoFromGameProductInfo(&productInfo, 0, &skuInfo);
		if (ret < 0) 
		{
			sceNpCommerce2DestroyGetProductInfoResult(&result);
			m_errorCode = ret;
			return ret;
		}
		strncpy(pInfo->skuId, skuInfo.skuId, SCE_NP_COMMERCE2_SKU_ID_LEN);
		pInfo->purchasabilityFlag = skuInfo.purchasabilityFlag;

		// Take out the price. Nicely formatted
		// but also keep the price as a value in case it's 0 - we need to show "free" for that
		pInfo->ui32Price= skuInfo.price;
		ret = sceNpCommerce2GetPrice(m_contextId, pInfo->price, sizeof(pInfo->price), skuInfo.price);
		if (ret < 0) 
		{
			sceNpCommerce2DestroyGetProductInfoResult(&result);
			m_errorCode = ret;
			return ret;
		}
	}

	// Set our result
	//*info = tempInfo;

	ret = sceNpCommerce2DestroyGetProductInfoResult(&result);
	if (ret < 0) 
	{
		return ret;
	}

	return ret;
}


int SonyCommerce_PS3::addDetailedProductInfo(ProductInfo *info, const char *productId, char *categoryId)
{
	int													ret = 0;
	uint32_t											requestId;
	size_t												bufSize = sizeof(m_commercebuffer);
	size_t												fillSize = 0;
	std::list<SceNpCommerce2ContentRatingDescriptor>	ratingDescList;
	SceNpCommerce2GetProductInfoResult					result;
	SceNpCommerce2ContentRatingInfo						ratingInfo;
	SceNpCommerce2GameProductInfo						productInfo;
	SceNpCommerce2GameSkuInfo							skuInfo;
	//ProductInfoDetailed									tempInfo;

	if (!m_contextCreated) 
	{
		ret = createContext();
		if (ret < 0) 
		{
			m_errorCode = ret;
			return ret;
		}
	}

	// Obtain product data
	ret = sceNpCommerce2GetProductInfoCreateReq(m_contextId, &requestId);
	if (ret < 0) 
	{
		m_errorCode = ret;
		return ret;
	}

	if (categoryId && categoryId[0] != 0) 
	{
		ret = sceNpCommerce2GetProductInfoStart(requestId, categoryId, productId);
	} 
	else 
	{
		ret = sceNpCommerce2GetProductInfoStart(requestId, categoryId, productId);		
	}
	if (ret < 0) {
		sceNpCommerce2DestroyReq(requestId);
		m_errorCode = ret;
		return ret;
	}

	ret = sceNpCommerce2GetProductInfoGetResult(requestId, m_commercebuffer, bufSize, &fillSize);
	if (ret < 0) 
	{
		sceNpCommerce2DestroyReq(requestId);
		m_errorCode = ret;
		return ret;
	}

	ret = sceNpCommerce2DestroyReq(requestId);
	if (ret < 0) 
	{
		m_errorCode = ret;
		return ret;
	}

	// Take Out Game Product Information
	ret = sceNpCommerce2InitGetProductInfoResult(&result, m_commercebuffer, fillSize);
	if (ret < 0) 
	{
		m_errorCode = ret;
		return ret;
	}

	ret = sceNpCommerce2GetGameProductInfo(&result, &productInfo);
	if (ret < 0) 
	{
		sceNpCommerce2DestroyGetProductInfoResult(&result);
		m_errorCode = ret;
		return ret;
	}

	// Get rating info
	ret = sceNpCommerce2GetContentRatingInfoFromGameProductInfo(&productInfo, &ratingInfo);
	if (ret < 0) 
	{
		sceNpCommerce2DestroyGetProductInfoResult(&result);
		m_errorCode = ret;
		return ret;
	}

	for (int index = 0; index < ratingInfo.countOfContentRatingDescriptor; index++) 
	{
		SceNpCommerce2ContentRatingDescriptor desc;
		sceNpCommerce2GetContentRatingDescriptor(&ratingInfo, index, &desc);
		ratingDescList.push_back(desc);		
	}

	// populate our temp struct
// 	tempInfo.ratingDescriptors = ratingDescList;
// 	strncpy(tempInfo.productId, productInfo.productId, SCE_NP_COMMERCE2_PRODUCT_ID_LEN);
// 	strncpy(tempInfo.productName, productInfo.productName, SCE_NP_COMMERCE2_PRODUCT_NAME_LEN);
// 	strncpy(tempInfo.shortDescription, productInfo.productShortDescription, SCE_NP_COMMERCE2_PRODUCT_SHORT_DESCRIPTION_LEN);
	strncpy(info->longDescription, productInfo.productLongDescription, SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN);
// 	strncpy(tempInfo.legalDescription, productInfo.legalDescription, SCE_NP_COMMERCE2_PRODUCT_LEGAL_DESCRIPTION_LEN);
// 	strncpy(tempInfo.spName, productInfo.spName, SCE_NP_COMMERCE2_SP_NAME_LEN);
// 	strncpy(tempInfo.imageUrl, productInfo.imageUrl, SCE_NP_COMMERCE2_URL_LEN);
// 	tempInfo.releaseDate = productInfo.releaseDate;
// 	strncpy(tempInfo.ratingSystemId, ratingInfo.ratingSystemId, SCE_NP_COMMERCE2_RATING_SYSTEM_ID_LEN);
// 	strncpy(tempInfo.ratingImageUrl, ratingInfo.imageUrl, SCE_NP_COMMERCE2_URL_LEN);

	// Get SKU info
	if (productInfo.countOfSku == 1) 
	{
		ret = sceNpCommerce2GetGameSkuInfoFromGameProductInfo(&productInfo, 0, &skuInfo);
		if (ret < 0) 
		{
			sceNpCommerce2DestroyGetProductInfoResult(&result);
			m_errorCode = ret;
			return ret;
		}
		strncpy(info->skuId, skuInfo.skuId, SCE_NP_COMMERCE2_SKU_ID_LEN);
		info->purchasabilityFlag = skuInfo.purchasabilityFlag;
		info->annotation = skuInfo.annotation;

		// Take out the price. Nicely formatted
		// but also keep the price as a value in case it's 0 - we need to show "free" for that
		info->ui32Price= skuInfo.price;
		ret = sceNpCommerce2GetPrice(m_contextId, info->price, sizeof(info->price), skuInfo.price);
		if (ret < 0) 
		{
			sceNpCommerce2DestroyGetProductInfoResult(&result);
			m_errorCode = ret;
			return ret;
		}
	}
	else
	{
		// 4J-PB - more than one sku id! We have to be able to use the sku id returned for a product, so there is not supposed to be more than 1
		app.DebugPrintf("MORE THAN 1 SKU ID FOR %s\n",info->productName);
	}

	// Set our result
	//*info = tempInfo;

	ret = sceNpCommerce2DestroyGetProductInfoResult(&result);
	if (ret < 0) 
	{
		return ret;
	}

	return ret;
}


int SonyCommerce_PS3::checkout(CheckoutInputParams &params)
{
	int ret = 0;
	const char *skuIdsTemp[SCE_NP_COMMERCE2_SKU_CHECKOUT_MAX];
	std::list<const char *>::iterator iter = params.skuIds.begin();
	std::list<const char *>::iterator iterEnd = params.skuIds.end();

	if (!m_contextCreated) {
		ret = createContext();
		if (ret < 0) {
			return ret;
		}
	}

	for (int i = 0; i < params.skuIds.size(); i++) {
		skuIdsTemp[i] = (const char *)(*iter);
		iter++;
	}

	ret = sceNpCommerce2DoCheckoutStartAsync(m_contextId, skuIdsTemp, params.skuIds.size(), *params.memContainer);
	if (ret < 0) {
		return ret;
	}

	return CELL_OK;	
}


int SonyCommerce_PS3::downloadList(DownloadListInputParams &params)
{
	int ret = 0;
	const char *skuIdsTemp[SCE_NP_COMMERCE2_SKU_CHECKOUT_MAX];
	std::list<const char *>::iterator iter = params.skuIds.begin();
	std::list<const char *>::iterator iterEnd = params.skuIds.end();

	if (!m_contextCreated) {
		ret = createContext();
		if (ret < 0) {
			return ret;
		}
	}

	for (int i = 0; i < params.skuIds.size(); i++) {
		skuIdsTemp[i] = (const char *)(*iter);
		iter++;
	}
	ret = sceNpCommerce2DoDlListStartAsync(m_contextId, app.GetCommerceCategory(), skuIdsTemp, params.skuIds.size(), *params.memContainer);
	if (ret < 0) {
		return ret;
	}

	return CELL_OK;	
}

void SonyCommerce_PS3::UpgradeTrialCallback2(LPVOID lpParam,int err)
{
	SonyCommerce* pCommerce = (SonyCommerce*)lpParam;
	app.DebugPrintf(4,"SonyCommerce_UpgradeTrialCallback2 : err : 0x%08x\n", err);
	pCommerce->CheckForTrialUpgradeKey();
	if(err != CELL_OK)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		C4JStorage::EMessageResult result = ui.RequestMessageBox( IDS_PRO_UNLOCKGAME_TITLE, IDS_NO_DLCOFFERS, uiIDA,1,ProfileManager.GetPrimaryPad());
	}
	m_trialUpgradeCallbackFunc(m_trialUpgradeCallbackParam, m_errorCode);
}

void SonyCommerce_PS3::UpgradeTrialCallback1(LPVOID lpParam,int err)
{
	SonyCommerce* pCommerce = (SonyCommerce*)lpParam;
	app.DebugPrintf(4,"SonyCommerce_UpgradeTrialCallback1 : err : 0x%08x\n", err);
	if(err == CELL_OK)
	{
		const char* skuID = s_trialUpgradeProductInfoDetailed.skuId;
		if(s_trialUpgradeProductInfoDetailed.purchasabilityFlag == SCE_NP_COMMERCE2_SKU_PURCHASABILITY_FLAG_OFF)
		{
			app.DebugPrintf(4,"UpgradeTrialCallback1 - DownloadAlreadyPurchased\n");
			pCommerce->DownloadAlreadyPurchased(UpgradeTrialCallback2, pCommerce, skuID);
		}
		else
		{
			app.DebugPrintf(4,"UpgradeTrialCallback1 - Checkout\n");
			pCommerce->Checkout(UpgradeTrialCallback2, pCommerce, skuID);
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

void SonyCommerce_PS3::UpgradeTrial(CallbackFunc cb, LPVOID lpParam)
{
	m_trialUpgradeCallbackFunc = cb;
	m_trialUpgradeCallbackParam = lpParam;
	
// 	static char szTrialUpgradeSkuID[64];
// 	sprintf(szTrialUpgradeSkuID, "%s-TRIALUPGRADE0001", app.GetCommerceCategory());//, szSKUSuffix);
	GetDetailedProductInfo(UpgradeTrialCallback1, this, &s_trialUpgradeProductInfoDetailed, app.GetUpgradeKey(), app.GetCommerceCategory());
}


int SonyCommerce_PS3::createContext()
{
	SceNpId npId;
	int ret = sceNpManagerGetNpId(&npId);
	if(ret < 0)
	{
		app.DebugPrintf(4,"createContext sceNpManagerGetNpId problem\n");
		return ret;
	}

	if (m_contextCreated) {
		ret = sceNpCommerce2DestroyCtx(m_contextId);
		if (ret < 0) 
		{
			app.DebugPrintf(4,"createContext sceNpCommerce2DestroyCtx problem\n");
			return ret;
		}
	}

	// Create commerce2 context
	ret = sceNpCommerce2CreateCtx(SCE_NP_COMMERCE2_VERSION, &npId, commerce2Handler, NULL, &m_contextId);
	if (ret < 0) 
	{
		app.DebugPrintf(4,"createContext sceNpCommerce2CreateCtx problem\n");
		return ret;
	}

	m_contextCreated = true;

	return CELL_OK;
}

int SonyCommerce_PS3::createSession()
{
	int ret = createContext();
	if (ret < 0) {
		return ret;
	}

	m_currentPhase = e_phase_creatingSessionPhase;
	ret = sceNpCommerce2CreateSessionStart(m_contextId);
	if (ret < 0) {
		return ret;
	}
	return ret;
}


void SonyCommerce_PS3::commerce2Handler(uint32_t contextId, uint32_t subjectId, int event, int errorCode, void *arg)
{
// 	Event reply;
// 	reply.service = Toolkit::NP::commerce;
// 
	EnterCriticalSection(&m_queueLock);

	switch (event) {
	case SCE_NP_COMMERCE2_EVENT_REQUEST_ERROR:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_errorCode = errorCode;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_CREATE_SESSION_DONE:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceSessionCreated;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_CREATE_SESSION_ABORT:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceSessionAborted;		
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_STARTED:
		{
			m_currentPhase = e_phase_checkoutPhase;
			m_event = e_event_commerceCheckoutStarted;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_SUCCESS:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceCheckoutSuccess;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_BACK:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceCheckoutAborted;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_CHECKOUT_FINISHED:
		{
			m_event = e_event_commerceCheckoutFinished;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_DL_LIST_STARTED:
		{
			m_currentPhase = e_phase_downloadListPhase;
			m_event = e_event_commerceDownloadListStarted;	
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_DL_LIST_SUCCESS:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceDownloadListSuccess;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_DL_LIST_FINISHED:
		{
			m_event = e_event_commerceDownloadListFinished;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_STARTED:
		m_currentPhase = e_phase_productBrowsePhase;
		m_event = e_event_commerceProductBrowseStarted;
		break;
	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_SUCCESS:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceProductBrowseSuccess;		
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_BACK:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceProductBrowseAborted;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_FINISHED:
		{
			m_event = e_event_commerceProductBrowseFinished;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_PROD_BROWSE_OPENED:
		break;
	case SCE_NP_COMMERCE2_EVENT_DO_PRODUCT_CODE_STARTED:
		{
			m_currentPhase = e_phase_voucherRedeemPhase;
			m_event = e_event_commerceVoucherInputStarted;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_PRODUCT_CODE_SUCCESS:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceVoucherInputSuccess;		
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_PRODUCT_CODE_BACK:
		{
			m_messageQueue.push(e_message_commerceEnd);
			m_event = e_event_commerceVoucherInputAborted;
			break;
		}
	case SCE_NP_COMMERCE2_EVENT_DO_PRODUCT_CODE_FINISHED:
		{
			m_event = e_event_commerceVoucherInputFinished;
			break;
		}
	default:
		break;
	};

	LeaveCriticalSection(&m_queueLock);
}



void SonyCommerce_PS3::processMessage()
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
			else 
			{
				m_event = e_event_commerceGotCategoryInfo;
				app.DebugPrintf(4,"e_event_commerceGotCategoryInfo - %s\n",m_pCategoryID);
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
			else
			{
				m_event = e_event_commerceGotProductList;
				app.DebugPrintf(4,"e_event_commerceGotProductList - %s\n",m_pCategoryID);
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
			else 
			{
				m_event = e_event_commerceGotDetailedProductInfo;
				app.DebugPrintf(4,"e_event_commerceGotDetailedProductInfo - %s\n",m_pCategoryID);
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
			else 
			{
				m_event = e_event_commerceAddedDetailedProductInfo;
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


void SonyCommerce_PS3::processEvent()
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
		assert(0);
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
		ret = sys_memory_container_destroy(m_memContainer);
		if (ret < 0) {
			app.DebugPrintf(4,"Failed to destroy memory container");
		}

		m_memContainer = SYS_MEMORY_CONTAINER_ID_INVALID;
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
		// 4J-PB - they may or may not have downloaded DLC
		// clear the DLC installed and check again
		app.ClearDLCInstalled();
		ui.HandleDLCInstalled(0);
		break;
	case e_event_commerceDownloadListFinished:
		app.DebugPrintf(4,"Download Finished: 0x%x\n", m_errorCode);
		ret = sys_memory_container_destroy(m_memContainer);
		if (ret < 0) {
			app.DebugPrintf(4,"Failed to destroy memory container");
		}

		m_memContainer = SYS_MEMORY_CONTAINER_ID_INVALID;
		// 4J-PB - if there's been an error - like dlc already purchased, the runcallback has already happened, and will crash this time
		if(m_callbackFunc!=NULL)
		{
			runCallback();
		}
		break;
	case e_event_commerceError:
		app.DebugPrintf(4,"Commerce Error 0x%x\n", m_errorCode);

		if(m_memContainer != SYS_MEMORY_CONTAINER_ID_INVALID)
		{
			ret = sys_memory_container_destroy(m_memContainer);
			if (ret < 0) {
				app.DebugPrintf(4,"Failed to destroy memory container");
			}

			m_memContainer = SYS_MEMORY_CONTAINER_ID_INVALID;
		}

		runCallback();
		break;
	default:
		break;
	}
	m_event = e_event_none;
}


int SonyCommerce_PS3::commerceEnd()
{
	int ret = 0;

	if (m_currentPhase == e_phase_voucherRedeemPhase)
		ret = sceNpCommerce2DoProductCodeFinishAsync(m_contextId);
	else if (m_currentPhase == e_phase_productBrowsePhase)
		ret = sceNpCommerce2DoProductBrowseFinishAsync(m_contextId);
	else if (m_currentPhase == e_phase_creatingSessionPhase)
		ret = sceNpCommerce2CreateSessionFinish(m_contextId, &m_sessionInfo);
	else if (m_currentPhase == e_phase_checkoutPhase)
		ret = sceNpCommerce2DoCheckoutFinishAsync(m_contextId);
	else if (m_currentPhase == e_phase_downloadListPhase)
		ret = sceNpCommerce2DoDlListFinishAsync(m_contextId);
	 
	m_currentPhase = e_phase_idle;

	return ret;
}

void SonyCommerce_PS3::CreateSession( CallbackFunc cb, LPVOID lpParam )
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
	if(m_tickThread == NULL)
		m_tickThread = new C4JThread(TickLoop, NULL, "SonyCommerce_PS3 tick");
	if(m_tickThread->isRunning() == false)
	{
		m_currentPhase = e_phase_idle;
		m_tickThread->Run();
	}
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_PS3::CloseSession()
{
	assert(m_currentPhase == e_phase_idle);
	m_currentPhase = e_phase_stopped;
	Shutdown();
}

void SonyCommerce_PS3::GetProductList( CallbackFunc cb, LPVOID lpParam, std::vector<ProductInfo>* productList, const char *categoryId)
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pProductInfoList = productList;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceGetProductList);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_PS3::GetDetailedProductInfo( CallbackFunc cb, LPVOID lpParam, ProductInfoDetailed* productInfo, const char *productId, const char *categoryId )
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
void SonyCommerce_PS3::AddDetailedProductInfo( CallbackFunc cb, LPVOID lpParam, ProductInfo* productInfo, const char *productId, const char *categoryId )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pProductInfo = productInfo;
	m_pProductID = productId;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceAddDetailedProductInfo);
	LeaveCriticalSection(&m_queueLock);
}
void SonyCommerce_PS3::GetCategoryInfo( CallbackFunc cb, LPVOID lpParam, CategoryInfo *info, const char *categoryId )
{
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	m_pCategoryInfo = info;
	strcpy(m_pCategoryID,categoryId);
	m_messageQueue.push(e_message_commerceGetCategoryInfo);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_PS3::Checkout( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	if(m_memContainer != SYS_MEMORY_CONTAINER_ID_INVALID)
	{
		return;
	}
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	int ret = sys_memory_container_create(&m_memContainer, SCE_NP_COMMERCE2_DO_CHECKOUT_MEMORY_CONTAINER_SIZE);
	if (ret < 0) 
	{
		app.DebugPrintf(4,"sys_memory_container_create() failed. ret = 0x%x\n", ret);
	}

	m_checkoutInputParams.memContainer = &m_memContainer;
	m_checkoutInputParams.skuIds.clear();
	m_checkoutInputParams.skuIds.push_back(skuID);
	m_messageQueue.push(e_message_commerceCheckout);
	LeaveCriticalSection(&m_queueLock);
}

void SonyCommerce_PS3::DownloadAlreadyPurchased( CallbackFunc cb, LPVOID lpParam, const char* skuID )
{
	if(m_memContainer != SYS_MEMORY_CONTAINER_ID_INVALID)
		return;
	EnterCriticalSection(&m_queueLock);
	setCallback(cb,lpParam);
	int ret = sys_memory_container_create(&m_memContainer, SCE_NP_COMMERCE2_DO_CHECKOUT_MEMORY_CONTAINER_SIZE);
	if (ret < 0) 
	{
		app.DebugPrintf(4,"sys_memory_container_create() failed. ret = 0x%x\n", ret);
	}

	m_downloadInputParams.memContainer = &m_memContainer;
	m_downloadInputParams.skuIds.clear();
	m_downloadInputParams.skuIds.push_back(skuID);
	m_messageQueue.push(e_message_commerceDownloadList);
	LeaveCriticalSection(&m_queueLock);
}



