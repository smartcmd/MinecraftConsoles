#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <np.h>
#include <np/drm.h>
#include <np/commerce2.h>
#include "Common\Network\Sony\SonyCommerce.h"

#define SCE_TOOLKIT_NP_SKU_PRICE_LEN	(SCE_NP_COMMERCE2_CURRENCY_CODE_LEN \
	+ SCE_NP_COMMERCE2_CURRENCY_SYMBOL_LEN \
	+ SCE_NP_COMMERCE2_THOUSAND_SEPARATOR_LEN \
	+ SCE_NP_COMMERCE2_DECIMAL_LETTER_LEN) ///< The maximum length of a price in characters.

class SonyCommerce_PS3 : public SonyCommerce
{
	enum State
	{
		e_state_noSession,
		e_state_creatingSession,
		e_state_createSessionDone, 
		e_state_idle,


	};
	/// This enum is used to verify the current utility that is running
	enum Phase
	{
		e_phase_stopped = 0,
		e_phase_idle,
		e_phase_voucherRedeemPhase,
		e_phase_productBrowsePhase,
		e_phase_creatingSessionPhase,
		e_phase_checkoutPhase,
		e_phase_downloadListPhase
	};

	enum Message
	{
		e_message_commerceNone, 
		e_message_commerceCreateSession,			///< Create a commerce session
		e_message_commerceGetCategoryInfo,			///< Information about a category in the Store
		e_message_commerceGetProductList,			///< Get a list of products available in the Store
		e_message_commerceGetDetailedProductInfo,	///< Get a list of products available in the Store, with additional details
		e_message_commerceAddDetailedProductInfo,	///< Add additional details to a ProdcutInfo already retrieved
		e_message_commerceStoreProductBrowse,		///< Launches the Store to a specified product 
		e_message_commerceUpgradeTrial,				///< Upgrade a trial to full game
		e_message_commerceRedeemVoucher,			///< Redeem a voucher code
		e_message_commerceGetEntitlementList,		///< Get a list of entitlements associated with the current PSN user.
		e_message_commerceConsumeEntitlement,		///< Consume an amount from a consumable entitlement.
		e_message_commerceCheckout,					///< Launch the Store checkout
		e_message_commerceDownloadList,				///< Launch the download list
		e_message_commerceEnd						///< End commerce2 processing
	};

	enum Event
	{
		e_event_none,				
		e_event_commerceSessionCreated,				///< An event generated when a commerce session has successfully been created.
		e_event_commerceSessionAborted,				///< An event generated when the creation of commerce session has been aborted.
		e_event_commerceGotCategoryInfo,			///< An event generated when some category information has been retrieved from the store.
		e_event_commerceGotProductList,				///< An event generated when a list of products that are available has been retrieved from the store.
		e_event_commerceGotDetailedProductInfo,		///< An event generated when some detailed product information has been retrieved from the store.
		e_event_commerceAddedDetailedProductInfo,		///< An event generated when some detailed product information has been retrieved from the store.
		e_event_commerceProductBrowseStarted,		///< An event generated when product overlay has started.
		e_event_commerceProductBrowseSuccess,		///< An event generated when a product browse was completed successfully, and the user purchased the product.
		e_event_commerceProductBrowseAborted,		///< An event generated when a product browse was aborted by the user (the user pressed back).
		e_event_commerceProductBrowseFinished,		///< An event generated when a product browse has finished and it is now safe to free memory.
		e_event_commerceVoucherInputStarted,		///< An event generated when a voucher code input overlay was started.
		e_event_commerceVoucherInputSuccess,		///< An event generated when a voucher code input completed successfully.
		e_event_commerceVoucherInputAborted,		///< An event generated when a voucher code input was aborted by the user (user pressed back).
		e_event_commerceVoucherInputFinished,		///< An event generated when a voucher code input has finished. It is now safe to free memory.
		e_event_commerceGotEntitlementList,			///< An event generated when a the list of entitlements has been received for the current user.
		e_event_commerceConsumedEntitlement,		///< An event generated when the has successfully consumed an entitlement.
		e_event_commerceCheckoutStarted,			///< An event generated when a store checkout overlay has started.
		e_event_commerceCheckoutSuccess,			///< An event generated when user has successfully purchased from the checkout.
		e_event_commerceCheckoutAborted,			///< An event generated when the checkout was aborted by the user (user pressed back).
		e_event_commerceCheckoutFinished,			///< An event generated when a store checkout overlay has finished.
		e_event_commerceDownloadListStarted,		///< An event generated when a download list overlay has started.
		e_event_commerceDownloadListSuccess,		///< An event generated when the user has ended the download list.
		e_event_commerceDownloadListFinished,		///< An event generated when a download list overlay has finished.
		e_event_commerceError						///< An event generated when a commerce error has occurred.
	};

	static bool							m_bLicenseChecked;
	static bool							m_bCommerceInitialised;
	static SceNpCommerce2SessionInfo	m_sessionInfo;
	static State						m_state;
	static int							m_errorCode;
	static LPVOID						m_callbackParam;
	static Event						m_event;
	static Message						m_message;
// 	static uint32_t						m_requestID;
	static void*						m_receiveBuffer;
	static std::vector<ProductInfo>		*m_pProductInfoList;
	static ProductInfoDetailed			*m_pProductInfoDetailed;
	static ProductInfo					*m_pProductInfo;
	static CategoryInfo*				m_pCategoryInfo;
	static char*						m_pCategoryID;
	static const char*					m_pProductID;
	static std::queue<Message>			m_messageQueue;
	static CallbackFunc					m_callbackFunc;
	static CheckoutInputParams			m_checkoutInputParams;
	static DownloadListInputParams		m_downloadInputParams;
	static sys_memory_container_t		m_memContainer;
	static bool							m_bUpgradingTrial;
	static C4JThread*					m_tickThread;
	static CallbackFunc					m_trialUpgradeCallbackFunc;
	static LPVOID						m_trialUpgradeCallbackParam;
	static CRITICAL_SECTION				m_queueLock;

	static void runCallback()
	{
		assert(m_callbackFunc);
		CallbackFunc func = m_callbackFunc;
		m_callbackFunc = nullptr;
		if(func)
			func(m_callbackParam, m_errorCode);
		m_errorCode = CELL_OK;
	}
	static void setCallback(CallbackFunc cb,LPVOID lpParam)
	{
		assert(m_callbackFunc == nullptr);
		m_callbackFunc = cb;
		m_callbackParam = lpParam;
	}


	static uint32_t		m_contextId;		///< The npcommerce2 context ID
	static bool			m_contextCreated;	///< npcommerce2 context ID created?
	static Phase		m_currentPhase;		///< Current commerce2 util
	static char			m_commercebuffer[SCE_NP_COMMERCE2_RECV_BUF_SIZE];



	static void commerce2Handler(uint32_t contextId, uint32_t subjectId, int event, int errorCode, void *arg);
	static void processMessage();
	static void processEvent();

	static int createContext();
	static int createSession();
	static void setError(int err) { m_errorCode = err; }
	static int getCategoryInfo(CategoryInfo *info, char *categoryId);
	static int getProductList(std::vector<ProductInfo>* productList, char *categoryId);
	static int getDetailedProductInfo(ProductInfoDetailed *info, const char *productId, char *categoryId);
	static int addDetailedProductInfo(ProductInfo *info, const char *productId, char *categoryId);
	static int checkout(CheckoutInputParams &params);
	static int downloadList(DownloadListInputParams &params);
	static void UpgradeTrialCallback1(LPVOID lpParam,int err);
	static void UpgradeTrialCallback2(LPVOID lpParam,int err);
	static void Delete();


	static int commerceEnd();
// 	static int upgradeTrial();

	static int TickLoop(void* lpParam);

	static void Init();
	static int Shutdown();

	static void CheckForTrialUpgradeKey_Callback(LPVOID param, bool bFullVersion);

public:

	virtual void CreateSession(CallbackFunc cb, LPVOID lpParam);
	virtual void CloseSession();

	virtual void GetCategoryInfo(CallbackFunc cb, LPVOID lpParam, CategoryInfo *info, const char *categoryId);
	virtual void GetProductList(CallbackFunc cb, LPVOID lpParam, std::vector<ProductInfo>* productList, const char *categoryId);
	virtual void GetDetailedProductInfo(CallbackFunc cb, LPVOID lpParam, ProductInfoDetailed* productInfoDetailed, const char *productId, const char *categoryId);
	virtual void AddDetailedProductInfo( CallbackFunc cb, LPVOID lpParam, ProductInfo* productInfo, const char *productId, const char *categoryId );
	virtual void Checkout(CallbackFunc cb, LPVOID lpParam, const char* skuID);
	virtual void DownloadAlreadyPurchased(CallbackFunc cb, LPVOID lpParam, const char* skuID);
	virtual void UpgradeTrial(CallbackFunc cb, LPVOID lpParam);
	virtual void CheckForTrialUpgradeKey();
	virtual bool LicenseChecked();

};
