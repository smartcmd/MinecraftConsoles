#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __PS3__
#include <np.h>
#include <np/drm.h>
#include <np/commerce2.h>
#elif defined __PSVITA__
#include <np.h>
#include <np_toolkit.h>
#else // __ORBIS__

#define SCE_NP_COMMERCE2_CATEGORY_ID_LEN				SCE_TOOLKIT_NP_COMMERCE_CATEGORY_ID_LEN					///< The size of the category ID.
#define SCE_NP_COMMERCE2_PRODUCT_ID_LEN					SCE_TOOLKIT_NP_COMMERCE_PRODUCT_ID_LEN					///< The size of the product ID.
#define SCE_NP_COMMERCE2_CATEGORY_NAME_LEN				SCE_TOOLKIT_NP_COMMERCE_CATEGORY_NAME_LEN				///< The size of the category name.
#define SCE_NP_COMMERCE2_CATEGORY_DESCRIPTION_LEN		SCE_TOOLKIT_NP_COMMERCE_CATEGORY_DESCRIPTION_LEN		///< The size of the category description.
#define SCE_NP_COMMERCE2_URL_LEN						SCE_TOOLKIT_NP_COMMERCE_URL_LEN							///< The size of the URL.
#define SCE_NP_COMMERCE2_PRODUCT_NAME_LEN				SCE_TOOLKIT_NP_COMMERCE_PRODUCT_NAME_LEN				///< The size of the product name.
#define SCE_NP_COMMERCE2_PRODUCT_SHORT_DESCRIPTION_LEN	SCE_TOOLKIT_NP_COMMERCE_PRODUCT_SHORT_DESCRIPTION_LEN	///< The size of the product short description.
#define SCE_NP_COMMERCE2_SP_NAME_LEN					SCE_TOOLKIT_NP_COMMERCE_SP_NAME_LEN						///< The size of the licensee (publisher) name.
#define SCE_NP_COMMERCE2_CURRENCY_CODE_LEN				SCE_TOOLKIT_NP_COMMERCE_CURRENCY_CODE_LEN				///< The size of currency code.
#define SCE_NP_COMMERCE2_CURRENCY_CODE_LEN				SCE_TOOLKIT_NP_COMMERCE_CURRENCY_CODE_LEN				
#define SCE_NP_COMMERCE2_CURRENCY_SYMBOL_LEN			SCE_TOOLKIT_NP_COMMERCE_CURRENCY_SYMBOL_LEN				///< The size of currency symbol.
#define SCE_NP_COMMERCE2_THOUSAND_SEPARATOR_LEN			SCE_TOOLKIT_NP_COMMERCE_THOUSAND_SEPARATOR_LEN			///< The size of the character separating every 3 digits of the price.
#define SCE_NP_COMMERCE2_DECIMAL_LETTER_LEN				SCE_TOOLKIT_NP_COMMERCE_DECIMAL_LETTER_LEN				///< The size of the character indicating the decimal point in the price.
#define SCE_NP_COMMERCE2_SKU_ID_LEN						SCE_TOOLKIT_NP_COMMERCE_SKU_ID_LEN						///< The size of the SKU ID.
#define SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN	SCE_TOOLKIT_NP_COMMERCE_PRODUCT_LONG_DESCRIPTION_LEN	///< The size of the product long description.
#define SCE_NP_COMMERCE2_PRODUCT_LEGAL_DESCRIPTION_LEN	SCE_TOOLKIT_NP_COMMERCE_PRODUCT_LEGAL_DESCRIPTION_LEN	///< The size of the product legal description.
#define SCE_NP_COMMERCE2_RATING_SYSTEM_ID_LEN			SCE_TOOLKIT_NP_COMMERCE_RATING_SYSTEM_ID_LEN			///< The size of the rating system ID.
#define SCE_NP_ENTITLEMENT_ID_SIZE						SCE_TOOLKIT_NP_COMMERCE_ENTITLEMENT_ID_LEN				///< The size of entitlement ID.
#endif

#ifndef __PSVITA__
#define SCE_TOOLKIT_NP_SKU_PRICE_LEN	(SCE_NP_COMMERCE2_CURRENCY_CODE_LEN \
	+ SCE_NP_COMMERCE2_CURRENCY_SYMBOL_LEN \
	+ SCE_NP_COMMERCE2_THOUSAND_SEPARATOR_LEN \
	+ SCE_NP_COMMERCE2_DECIMAL_LETTER_LEN) ///< The maximum length of a price in characters.
#endif

class SonyCommerce
{

public:
	typedef void (*CallbackFunc)(LPVOID lpParam, int error_code);


	/// @brief
	/// Contains information about a subcategory on the PlayStation(R)Store.
	///
	/// Contains information about a subcategory on the PlayStation(R)Store.
	typedef struct CategoryInfoSub
	{
		char		categoryId[SCE_NP_COMMERCE2_CATEGORY_ID_LEN];					///< The ID of the subcategory.
		char		categoryName[SCE_NP_COMMERCE2_CATEGORY_NAME_LEN];				///< The name of the subcategory.
		char		categoryDescription[SCE_NP_COMMERCE2_CATEGORY_DESCRIPTION_LEN];	///< The detailed description of the subcategory.
		char		imageUrl[SCE_NP_COMMERCE2_URL_LEN];								///< The image URL of the subcategory.
	}
	CategoryInfoSub;

	/// @brief
	/// Current category information
	///
	/// This structure contains information about a category on the PlayStation(R)Store
	typedef struct CategoryInfo
	{
		CategoryInfoSub				current;				///< The currently selected subcategory.
		std::list<CategoryInfoSub>	subCategories;			///< Information about the subcategories in this category.
		uint32_t					countOfProducts;		///< The number of products in the category.
		uint32_t					countOfSubCategories;	///< The number of subcategories.
	}
	CategoryInfo;

	/// Contains information about a product in the PlayStation(R)Store.
	typedef struct ProductInfo
	{
		uint32_t	purchasabilityFlag;													///< A flag that indicates whether the product can be purchased (<c>SCE_NP_COMMERCE2_SKU_PURCHASABILITY_FLAG_XXX</c>).
		uint32_t	annotation;															// SCE_NP_COMMERCE2_SKU_ANN_PURCHASED_CANNOT_PURCHASE_AGAIN or SCE_NP_COMMERCE2_SKU_ANN_PURCHASED_CAN_PURCHASE_AGAIN
		uint32_t	ui32Price;
		char		productId[SCE_NP_COMMERCE2_PRODUCT_ID_LEN];							///< The product ID.
		char		productName[SCE_NP_COMMERCE2_PRODUCT_NAME_LEN];						///< The name of the product.
		char		shortDescription[SCE_NP_COMMERCE2_PRODUCT_SHORT_DESCRIPTION_LEN];	///< A short description of the product.
		char		longDescription[SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN];		///< A long description of the product.
		char		skuId[SCE_NP_COMMERCE2_SKU_ID_LEN];									///< The SKU ID
		char		spName[SCE_NP_COMMERCE2_SP_NAME_LEN];								///< The service provider name.
		char		imageUrl[SCE_NP_COMMERCE2_URL_LEN];									///< The product image URL.
		char		price[SCE_TOOLKIT_NP_SKU_PRICE_LEN];								///< The price of the product. This is formatted to include the currency code.	
		char        padding[6];															///< Padding.	
#ifdef __PS3__
		CellRtcTick	releaseDate;														///< The product release date.
#else
		SceRtcTick releaseDate;
#endif
	}
	ProductInfo;

	/// @brief
	/// Contains detailed information about a product on the PlayStation(R)Store.
	///
	/// Contains detailed information about a product on the PlayStation(R)Store.
	typedef struct ProductInfoDetailed
	{
		uint32_t	purchasabilityFlag;													///< A flag that indicates whether the product can be purchased (<c>SCE_NP_COMMERCE2_SKU_PURCHASABILITY_FLAG_XXX</c>).
		uint32_t	ui32Price;
		char		skuId[SCE_NP_COMMERCE2_SKU_ID_LEN];									///< The SKU ID
		char		productId[SCE_NP_COMMERCE2_PRODUCT_ID_LEN];							///< The product ID.
		char		productName[SCE_NP_COMMERCE2_PRODUCT_NAME_LEN];						///< The name of the product.
		char		shortDescription[SCE_NP_COMMERCE2_PRODUCT_SHORT_DESCRIPTION_LEN];	///< A short description of the product.
		char		longDescription[SCE_NP_COMMERCE2_PRODUCT_LONG_DESCRIPTION_LEN];		///< A long description of the product.
		char		legalDescription[SCE_NP_COMMERCE2_PRODUCT_LEGAL_DESCRIPTION_LEN];	///< The legal description for the product.	
		char		spName[SCE_NP_COMMERCE2_SP_NAME_LEN];								///< The service provider name.
		char		imageUrl[SCE_NP_COMMERCE2_URL_LEN];									///< The product image URL.
		char		price[SCE_TOOLKIT_NP_SKU_PRICE_LEN];								///< The price of the product. This is formatted to include the currency code.
		char		ratingSystemId[SCE_NP_COMMERCE2_RATING_SYSTEM_ID_LEN];				///< The ID of the rating system (for example: PEGI, ESRB).
		char		ratingImageUrl[SCE_NP_COMMERCE2_URL_LEN];							///< The URL of the rating icon.
		char		padding[2];															///< Padding.	
#ifdef __PS3__
		std::list<SceNpCommerce2ContentRatingDescriptor> ratingDescriptors;				///< The list of rating descriptors.			
		CellRtcTick	releaseDate;														///< The product release date.
#else 
		SceRtcTick	releaseDate;														///< The product release date.
#endif
	}
	ProductInfoDetailed;

	/// @brief
	/// Checkout parameters
	///
	/// This structure contains list of SKUs to checkout to and a memory container
	typedef struct CheckoutInputParams 
	{
		std::list<const char *>		skuIds;			///< List of SKU IDs
#ifdef __PS3__
		sys_memory_container_t	*memContainer;	///< Memory container for checkout overlay
#endif
	}
	CheckoutInputParams;

	/// @brief
	/// Contains download list parameters.
	///
	/// Contains download list parameters.
	typedef struct DownloadListInputParams 
	{
		std::list<const char *>		skuIds;			///< The list of SKU IDs
#ifdef __PS3__
		sys_memory_container_t	*memContainer;	///< A memory container for checkout overlay.
#endif
		const char*				categoryID;
	}
	DownloadListInputParams;


public:
	virtual void CreateSession(CallbackFunc cb, LPVOID lpParam) = 0;
	virtual void CloseSession() = 0;

	virtual void GetCategoryInfo(CallbackFunc cb, LPVOID lpParam, CategoryInfo *info, const char *categoryId) = 0;
	virtual void GetProductList(CallbackFunc cb, LPVOID lpParam, std::vector<ProductInfo>* productList, const char *categoryId) = 0;
	virtual void GetDetailedProductInfo(CallbackFunc cb, LPVOID lpParam, ProductInfoDetailed* productInfoDetailed, const char *productId, const char *categoryId) = 0;
	virtual void AddDetailedProductInfo( CallbackFunc cb, LPVOID lpParam, ProductInfo* productInfo, const char *productId, const char *categoryId ) = 0;
	virtual void Checkout(CallbackFunc cb, LPVOID lpParam, const char* skuID) = 0;
	virtual void DownloadAlreadyPurchased(CallbackFunc cb, LPVOID lpParam, const char* skuID) = 0;
#if defined(__ORBIS__) || defined( __PSVITA__)
	virtual void Checkout_Game(CallbackFunc cb, LPVOID lpParam, const char* skuID) = 0;
	virtual void DownloadAlreadyPurchased_Game(CallbackFunc cb, LPVOID lpParam, const char* skuID) = 0;
#endif
	virtual void UpgradeTrial(CallbackFunc cb, LPVOID lpParam) = 0;
	virtual void CheckForTrialUpgradeKey() = 0;
	virtual bool LicenseChecked() = 0;

#if defined __ORBIS__ || defined __PSVITA__
	virtual void ShowPsStoreIcon() = 0;
	virtual void HidePsStoreIcon() = 0;
#endif
};
