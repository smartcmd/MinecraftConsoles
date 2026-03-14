#pragma once

class C4JStringTable;
//#include <np_toolkit.h>

#include "../../Common/Network/Sony/SonyCommerce.h"
#include "../../Common/Network/Sony/SonyRemoteStorage.h"

#define PRODUCT_CODE_SIZE 9
#define SAVEFOLDERPREFIX_SIZE 10
#define COMMERCE_CATEGORY_SIZE 19
#define UPGRADE_KEY_SIZE 59
#define SKU_POSTFIX_SIZE 4

enum EProductSKU
{
	e_sku_SCEE,
	e_sku_SCEA,
	e_sku_SCEJ
};

typedef struct 
{
	char chProductCode[PRODUCT_CODE_SIZE+1];
	char chSaveFolderPrefix[SAVEFOLDERPREFIX_SIZE+1];
	char chDiscSaveFolderPrefix[SAVEFOLDERPREFIX_SIZE+1];
	char chCommerceCategory[COMMERCE_CATEGORY_SIZE+1];
//	char chTexturePackID[SCE_TOOLKIT_NP_COMMERCE_CATEGORY_ID_LEN+1];
	char chUpgradeKey[UPGRADE_KEY_SIZE+1];
	char chSkuPostfix[SKU_POSTFIX_SIZE+1];
	EProductSKU	eProductSKU;
}
PRODUCTCODES;	

//class SonyRemoteStorage;

// MGH - moved these to the storage lib, as we need this data when parsing the DLC folders 
// enum e_SONYDLCType
// {
// 	eSONYDLCType_SkinPack=0,
// 	eSONYDLCType_TexturePack,
// 	eSONYDLCType_MashUpPack,
// 	eSONYDLCType_All
// };
// 
// typedef struct
// {
// 	char chDLCKeyname[16];
// 	//char chDLCTitle[64];
// 	e_SONYDLCType eDLCType;
// 	int iFirstSkin;
// 	int iConfig; // used for texture pack data files
// }
// SONYDLC;
//

struct SceSaveDataDialogParam;

class CConsoleMinecraftApp : public CMinecraftApp 
{
	ImageFileBuffer m_ThumbnailBuffer;
	ImageFileBuffer m_SaveImageBuffer;
public:
	CConsoleMinecraftApp();	

	virtual void SetRichPresenceContext(int iPad, int contextId);

	virtual void StoreLaunchData();
	virtual void ExitGame();
	virtual void FatalLoadError();

	virtual void CaptureSaveThumbnail();
	virtual void GetSaveThumbnail(PBYTE*,DWORD*) {}; // NOT USED
	virtual void GetSaveThumbnail(PBYTE *ppbThumbnailData,DWORD *pdwThumbnailSize,PBYTE *ppbDataImage,DWORD *pdwSizeImage);
	virtual void ReleaseSaveThumbnail();
	virtual void GetScreenshot(int iPad,PBYTE *pbData,DWORD *pdwSize);

	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile);
	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile, eFileExtensionType eExt);
	virtual void FreeLocalTMSFiles(eTMSFileType eType);
	virtual int GetLocalTMSFileIndex(WCHAR *wchTMSFile,bool bFilenameIncludesExtension,eFileExtensionType eEXT=eFileExtensionType_PNG);

	// BANNED LEVEL LIST
	virtual void ReadBannedList(int iPad, eTMSAction action=(eTMSAction)0, bool bCallback=false) {}

	C4JStringTable *GetStringTable()																									{ return nullptr;}

	// original code
	virtual void TemporaryCreateGameStart();




	BOOL ReadProductCodes();
	char *GetProductCode();
	char *GetSaveFolderPrefix();
	char *GetCommerceCategory();
	char *GetTexturePacksCategoryID();
	char *GetUpgradeKey();
	EProductSKU GetProductSKU();
	bool IsJapaneseSKU();
	bool IsEuropeanSKU();
	bool IsAmericanSKU();
	//char *GetSKUPostfix();
	SONYDLC *GetSONYDLCInfo(char *pchTitle);
	SONYDLC *GetSONYDLCInfo(int iTexturePackID);

	int GetiFirstSkinFromName(char *pchName);
	int GetiConfigFromName(char *pchName);
	eDLCContentType GetDLCTypeFromName(char *pchDLCName);
	bool GetTrialFromName(char *pchDLCName);

	// PS3 COMMERCE
	enum eUI_DLC_State
	{
		eCommerce_State_Offline,
		eCommerce_State_Online,
		eCommerce_State_Error,
		eCommerce_State_Init,
		eCommerce_State_Init_Pending,
		eCommerce_State_GetCategories,
		eCommerce_State_GetCategories_Pending,
		eCommerce_State_GetProductList,
		eCommerce_State_GetProductList_Pending,
		eCommerce_State_AddProductInfoDetailed,
		eCommerce_State_AddProductInfoDetailed_Pending,
		eCommerce_State_RegisterDLC,
		eCommerce_State_Checkout,
		eCommerce_State_Checkout_WaitingForSession,
		eCommerce_State_Checkout_SessionStarted,
		eCommerce_State_Checkout_Pending,
		eCommerce_State_DownloadAlreadyPurchased,
		eCommerce_State_DownloadAlreadyPurchased_WaitingForSession,
		eCommerce_State_DownloadAlreadyPurchased_SessionStarted,
		eCommerce_State_DownloadAlreadyPurchased_Pending,
		eCommerce_State_UpgradeTrial,
		eCommerce_State_UpgradeTrial_WaitingForSession,
		eCommerce_State_UpgradeTrial_SessionStarted,
		eCommerce_State_UpgradeTrial_Pending,
	};

	void AppEventTick();

	bool CheckForEmptyStore(int iPad);

	void CommerceInit();
	void CommerceTick();
	bool GetCommerceCategoriesRetrieved();
	bool GetCommerceProductListRetrieved();
	bool GetCommerceProductListInfoRetrieved();
	int GetCommerceState();
	SonyCommerce* GetCommerce() { return m_pCommerce; }
	SonyCommerce::CategoryInfo *GetCategoryInfo();
	std::vector<SonyCommerce::ProductInfo>* GetProductList(int iIndex); // default to fail if the additional details are not retrieved
	SonyCommerce::ProductInfoDetailed *GetProductInfoDetailed();
	void ClearCommerceDetails(); // wipe out details on a PSN sign out
	void Checkout(char *pchSkuID);
	void DownloadAlreadyPurchased(char *pchSkuID);
	bool UpgradeTrial();
	bool DLCAlreadyPurchased(char *pchTitle);
	char *GetSkuIDFromProductList();
	void GetDLCSkuIDFromProductList(char *,char *);
	unordered_map<wstring, SONYDLC *>* GetSonyDLCMap() { return &m_SONYDLCMap; }
	static void CommerceInitCallback(LPVOID lpParam,int err);
	static void CommerceGetCategoriesCallback(LPVOID lpParam,int err);
	static void CommerceGetProductListCallback(LPVOID lpParam,int err);
	//	static void CommerceGetDetailedProductInfoCallback(LPVOID lpParam,int err);
	static void CommerceAddDetailedProductInfoCallback(LPVOID lpParam,int err);
	static void CommerceCheckoutCallback(LPVOID lpParam,int err);

	static void CheckoutSessionStartedCallback(LPVOID lpParam,int err);
	static void DownloadAlreadyPurchasedSessionStartedCallback(LPVOID lpParam,int err);
	static void UpgradeTrialSessionStartedCallback(LPVOID lpParam,int err);

	SonyRemoteStorage* getRemoteStorage() { return m_pRemoteStorage; }

	void SaveDataTick();
	static void Callback_SaveGameIncomplete(void *pParam, C4JStorage::ESaveIncompleteType saveIncompleteType, int blocksRequired);
	static int	NoSaveSpaceReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int cbConfirmDeleteMessageBox(void *pParam,int iPad,const C4JStorage::EMessageResult);

private:
	bool m_bSaveIncompleteDialogRunning;

	void initSaveIncompleteDialog(int spaceNeeded);
	void updateSaveIncompleteDialog();
	void finishSaveIncompleteDialog();

	enum ESaveDataDeleteDialogState
	{
		eSaveDataDeleteState_idle,
		eSaveDataDeleteState_waitingForUser,
		eSaveDataDeleteState_userConfirmation,
		eSaveDataDeleteState_deleting,

		eSaveDataDeleteState_continue,
		eSaveDataDeleteState_abort,

	} m_bSaveDataDeleteDialogState;

	void initSaveDataDeleteDialog();
	void updateSaveDataDeleteDialog();
	void finishSaveDataDeleteDialog();
	
	void getSaveDataDeleteDialogParam(SceSaveDataDialogParam *baseParam);
	void releaseSaveDataDeleteDialogParam(SceSaveDataDialogParam *baseParam);

	static int cbSaveDataDeleted(LPVOID pParam, const bool);

	PSAVE_INFO m_pSaveToDelete;

	void finishedDeletingSaves(bool bContinue);

	bool m_bCommerceCategoriesRetrieved;
	bool m_bCommerceInitialised;
	bool m_bCommerceProductListRetrieved;
	bool m_bProductListAdditionalDetailsRetrieved;
	char m_pchSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

	int m_eCommerce_State;
	int m_ProductListRetrievedC;
	int m_ProductListAdditionalDetailsC;
	int m_ProductListCategoriesC;
	int m_iCurrentCategory;
	int m_iCurrentProduct;

 	SonyCommerce *m_pCommerce;
	SonyCommerce::CategoryInfo m_CategoryInfo;
	std::vector<SonyCommerce::ProductInfo>* m_ProductListA;
	SonyCommerce::ProductInfo* m_pCheckoutProductInfo;
	//	SonyCommerce::ProductInfoDetailed m_ProductInfoDetailed;

	PRODUCTCODES ProductCodes;
	unordered_map<wstring, SONYDLC *> m_SONYDLCMap;


	bool m_bVoiceChatAndUGCRestricted;
	SonyRemoteStorage* m_pRemoteStorage;

};

extern CConsoleMinecraftApp app;

