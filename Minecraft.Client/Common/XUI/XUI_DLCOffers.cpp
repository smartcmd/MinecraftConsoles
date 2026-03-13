// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../../../Minecraft.World/ByteArrayInputStream.h"
#include "../../../Minecraft.World/BufferedReader.h"
#include "../../../Minecraft.World/InputStreamReader.h"
#include "../../../Minecraft.World/ArrayWithLength.h"
#include <assert.h>
#include "XUI_Ctrl_4JIcon.h"
#include "XUI_DLCOffers.h"
#include "../../../Minecraft.World/StringHelpers.h"
#ifdef _XBOX
#include <xavatar.h>
#endif

#define TIMER_ID_NETWORK_CONNECTION 1
#define TIMER_ID_NAVIGATE_BACK 2
// Constants

//const wstring CScene_DLCOffers::DEFAULT_BANNER = L"Graphics/banner.png";

// DLC Main

HRESULT CScene_DLCMain::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	iPad = *static_cast<int *>(pInitData->pvInitData);

	MapChildControls();
	
	app.SetTickTMSDLCFiles(true);

	XuiControlSetText(xList,app.GetString(IDS_DOWNLOADABLE_CONTENT_OFFERS));

	//if(app.GetTMSDLCInfoRead())
	{
		m_Timer.SetShow(FALSE);
		m_bIgnoreInput=false;

		VOID *pObj;
		XuiObjectFromHandle( xList, &pObj );
		list = static_cast<CXuiCtrl4JList *>(pObj);

		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK );

		CXuiCtrl4JList::LIST_ITEM_INFO *pListInfo = new CXuiCtrl4JList::LIST_ITEM_INFO [e_DLC_MAX_MinecraftStore];
		ZeroMemory(pListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO)*e_DLC_MAX_MinecraftStore);

		m_bAllDLCContentRetrieved=false;
		pListInfo[e_DLC_SkinPack].pwszText = app.GetString(IDS_DLC_MENU_SKINPACKS);
		pListInfo[e_DLC_SkinPack].fEnabled=TRUE;
		list->AddData(pListInfo[e_DLC_SkinPack]);

		pListInfo[e_DLC_TexturePacks].pwszText = app.GetString(IDS_DLC_MENU_TEXTUREPACKS);
		pListInfo[e_DLC_TexturePacks].fEnabled=TRUE;
		list->AddData(pListInfo[e_DLC_TexturePacks]);

		pListInfo[e_DLC_MashupPacks].pwszText = app.GetString(IDS_DLC_MENU_MASHUPPACKS);
		pListInfo[e_DLC_MashupPacks].fEnabled=TRUE;
		list->AddData(pListInfo[e_DLC_MashupPacks]);

		pListInfo[e_DLC_Themes].pwszText = app.GetString(IDS_DLC_MENU_THEMES);
		pListInfo[e_DLC_Themes].fEnabled=TRUE;
		list->AddData(pListInfo[e_DLC_Themes]);

		pListInfo[e_DLC_AvatarItems].pwszText = app.GetString(IDS_DLC_MENU_AVATARITEMS);
		pListInfo[e_DLC_AvatarItems].fEnabled=TRUE;
		list->AddData(pListInfo[e_DLC_AvatarItems]);

		pListInfo[e_DLC_Gamerpics].pwszText = app.GetString(IDS_DLC_MENU_GAMERPICS);
		pListInfo[e_DLC_Gamerpics].fEnabled=TRUE;
		list->AddData(pListInfo[e_DLC_Gamerpics]);

		app.AddDLCRequest(e_Marketplace_Content); // content is skin packs, texture packs and mash-up packs
		app.AddDLCRequest(e_Marketplace_Gamerpics);
		app.AddDLCRequest(e_Marketplace_Themes);
		app.AddDLCRequest(e_Marketplace_AvatarItems);

		// start retrieving the images needed from TMS
		app.AddTMSPPFileTypeRequest(e_DLC_SkinPack);
		app.AddTMSPPFileTypeRequest(e_DLC_Gamerpics);
		app.AddTMSPPFileTypeRequest(e_DLC_Themes);
		app.AddTMSPPFileTypeRequest(e_DLC_AvatarItems);
		app.AddTMSPPFileTypeRequest(e_DLC_TexturePacks);
		app.AddTMSPPFileTypeRequest(e_DLC_MashupPacks);
	}

	XuiElementInitUserFocus(xList, ProfileManager.GetPrimaryPad(), TRUE);
	TelemetryManager->RecordMenuShown(iPad, eUIScene_DLCMainMenu, 0); // 4J JEV ?

	return S_OK;
}

HRESULT CScene_DLCMain::OnDestroy()
{
	return S_OK;
}

HRESULT CScene_DLCMain::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	if(pData->nId==TIMER_ID_NETWORK_CONNECTION)
	{	
		if(ProfileManager.GetLiveConnectionStatus()!=XONLINE_S_LOGON_CONNECTION_ESTABLISHED)
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;

			StorageManager.ClearDLCOffers();
			app.ClearAndResetDLCDownloadQueue();
		}
	}
	else if(pData->nId==TIMER_ID_NAVIGATE_BACK)
	{
		if(app.CheckTMSDLCCanStop())
		{
			XuiKillTimer(m_hObj,TIMER_ID_NAVIGATE_BACK);
			app.NavigateBack(XUSER_INDEX_ANY);
		}
	}

	return S_OK;
}


HRESULT CScene_DLCMain::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		app.SetTickTMSDLCFiles(false);

		// set the timer running to navigate back when any tms retrieval has come in
		XuiSetTimer(m_hObj,TIMER_ID_NAVIGATE_BACK,50);
		m_bIgnoreInput=true;
		m_Timer.SetShow(TRUE);
		//app.NavigateBack(XUSER_INDEX_ANY);
		rfHandled = TRUE;

		break;
	}

	return S_OK;
}

HRESULT CScene_DLCMain::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);
	
	if(hObjPressed==xList)
	{
		int iIndex;
		CXuiControl pItem;
		iIndex=xList.GetCurSel(&pItem);

		DLCOffersParam *param = new DLCOffersParam();
		param->iPad = iPad;
		param->iType = iIndex;

		// promote the DLC content request type 
		app.AddDLCRequest(static_cast<eDLCMarketplaceType>(iIndex), true);
		app.NavigateToScene(iPad,eUIScene_DLCOffersMenu, param);
	}
	return S_OK;
}

HRESULT CScene_DLCMain::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK );

	return S_OK;
}

// DLC OFFERS

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_DLCOffers::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	DLCOffersParam *param = static_cast<DLCOffersParam *>(pInitData->pvInitData);
	m_iPad = param->iPad;
	m_iType = param->iType;
	m_iOfferC = app.GetDLCOffersCount();
	m_bIsFemale = false;
	m_pNoImageFor_DLC=nullptr;
	bNoDLCToDisplay=true;
	//hCostText=nullptr;

	
	// 4J JEV: Deleting this here seems simpler.
	delete param;

	// If this is the avatar items, we need to init the avatar system to allow us to find out what the player avatar gender is (since we should only display tshirts etc with the right gender)
	// The init reserves 3MB of memory, so we shut down on leaving this.
	if(m_iType==e_DLC_AvatarItems)
	{
		XAVATAR_METADATA AvatarMetadata;
		HRESULT hRes;


		hRes=XAvatarInitialize(XAVATAR_COORDINATE_SYSTEM_LEFT_HANDED,0,0,0,nullptr);

		// get the avatar gender
		hRes=XAvatarGetMetadataLocalUser(m_iPad,&AvatarMetadata,nullptr);

		m_bIsFemale= (XAVATAR_BODY_TYPE_FEMALE == XAvatarMetadataGetBodyType(&AvatarMetadata));
		// shutdown the avatar system

		XAvatarShutdown();
	}

	m_bIsSD=!RenderManager.IsHiDef() && !RenderManager.IsWidescreen();

	MapChildControls();	
	
	XuiControlSetText(m_List,app.GetString(IDS_DOWNLOADABLE_CONTENT_OFFERS));

	m_bIgnorePress=true;

	VOID *pObj;
	m_hXuiBrush=nullptr;

	XuiObjectFromHandle( m_List, &pObj );
	m_pOffersList = static_cast<CXuiCtrl4JList *>(pObj);
	m_bAllDLCContentRetrieved=false;

	XuiElementInitUserFocus(m_hObj,ProfileManager.GetPrimaryPad(),TRUE);
	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_DLCOffersMenu, 0);
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1,IDS_TOOLTIPS_BACK);

	// Disable the price tag display
	m_PriceTag.SetShow(FALSE);

	// If we don't yet have this DLC, we need to display a timer
	m_bDLCRequiredIsRetrieved=false;

	// Is the DLC we're looking for available?
	if(!m_bDLCRequiredIsRetrieved)
	{
		if(app.DLCContentRetrieved(static_cast<eDLCMarketplaceType>(m_iType)))
		{	
			m_bDLCRequiredIsRetrieved=true;

			// Retrieve the info
			GetDLCInfo(app.GetDLCOffersCount(), false);
			m_bIgnorePress=false;
		}
	}

	XuiSetTimer(m_hObj,TIMER_ID_NETWORK_CONNECTION,50);

	return S_OK;
}

HRESULT CScene_DLCOffers::GetDLCInfo( int iOfferC, bool bUpdateOnly )
{
	CXuiCtrl4JList::LIST_ITEM_INFO *pListInfo=nullptr;
	//XMARKETPLACE_CONTENTOFFER_INFO xOffer;
	XMARKETPLACE_CURRENCY_CONTENTOFFER_INFO xOffer;
	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
	WCHAR szResourceLocator[ LOCATOR_SIZE ];
	ZeroMemory(szResourceLocator,sizeof(WCHAR)*LOCATOR_SIZE);
	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(nullptr);
	int iCount=0;

	if(bUpdateOnly) // Just update the info on the current list
	{
		for(int i=0;i<iOfferC;i++)
		{
			xOffer = StorageManager.GetOffer(i);
			// Check that this is in the list of known DLC
			DLC_INFO *pDLC=app.GetDLCInfoForFullOfferID(xOffer.qwOfferID);
			if(pDLC==nullptr)
			{
				// try the trial version
				pDLC=app.GetDLCInfoForTrialOfferID(xOffer.qwOfferID);
			}

			if(pDLC==nullptr)
			{
				// skip this one
#ifdef _DEBUG
				app.DebugPrintf("Unknown offer - ");
				OutputDebugStringW(xOffer.wszOfferName);
				app.DebugPrintf("\n");
#endif
				continue;
			}

			// If the item has a gender, then skip ones that are the other gender
			if((pDLC->iGender==1) && (m_bIsFemale==true))
			{
				app.DebugPrintf("Wrong gender\n");
				continue;
			}

			// can't trust the offer type - partnernet is giving avatar items the CONTENT type
			//if(Offer.dwOfferType==app.GetDLCContentType((eDLCContentType)m_iType))
			if(pDLC->eDLCType==static_cast<eDLCContentType>(m_iType))
			{		
				if(xOffer.fUserHasPurchased)
				{
					HXUIBRUSH hBrush;

					if(RenderManager.IsHiDef() || RenderManager.IsWidescreen())
					{
						swprintf(szResourceLocator, LOCATOR_SIZE, L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/Graphics/DLC_Tick.png");
						XuiCreateTextureBrush(szResourceLocator,&hBrush);
					}
					else
					{
						swprintf(szResourceLocator, LOCATOR_SIZE, L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/Graphics/DLC_TickSmall.png");
						XuiCreateTextureBrush(szResourceLocator,&hBrush);
					}
					m_pOffersList->UpdateGraphic(i,hBrush );
				}

				iCount++;
			}
		}

		if(iCount>0)
		{
			bNoDLCToDisplay=false;
		}
	}
	else
	{	
		if(iOfferC!=0)
		{
			pListInfo = new CXuiCtrl4JList::LIST_ITEM_INFO [iOfferC];
			ZeroMemory(pListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO)*iOfferC);
		}

		for(int i = 0; i < iOfferC; i++)
		{
			xOffer = StorageManager.GetOffer(i);

			// Check that this is in the list of known DLC
			DLC_INFO *pDLC=app.GetDLCInfoForFullOfferID(xOffer.qwOfferID);
			if(pDLC==nullptr)
			{
				// try the trial version
				pDLC=app.GetDLCInfoForTrialOfferID(xOffer.qwOfferID);
			}

			if(pDLC==nullptr)
			{
				// skip this one
#ifdef _DEBUG
				app.DebugPrintf("Unknown offer - ");
				OutputDebugStringW(xOffer.wszOfferName);
				app.DebugPrintf("\n");
#endif
				continue;
			}

			// can't trust the offer type - partnernet is giving avatar items the CONTENT type
			//if(Offer.dwOfferType==app.GetDLCContentType((eDLCContentType)m_iType))
			if(pDLC->eDLCType==static_cast<eDLCContentType>(m_iType))
			{		
				wstring wstrTemp=xOffer.wszOfferName;

				// If the string starts with Minecraft, removed that

				// Bug 49249 - JPN: Code Defect: Missing Text: String 'Minecraft' is missing in contents download screen.
				// Looks like we shouldn't be removing this text for Japanese, and probably Chinese & Korean

				DWORD dwLanguage = XGetLanguage( );
				switch(dwLanguage)
				{
				case XC_LANGUAGE_KOREAN:	
				case XC_LANGUAGE_JAPANESE:
				case XC_LANGUAGE_TCHINESE:
					pListInfo[iCount].pwszText = xOffer.wszOfferName;
					break;
				default:
					if(wstrTemp.compare(0,10,L"Minecraft ")==0)
					{
						pListInfo[iCount].pwszText = &xOffer.wszOfferName[10];
					}
					else
					{
						pListInfo[iCount].pwszText = xOffer.wszOfferName;
					}
					break;
				}

				pListInfo[iCount].fEnabled=TRUE;

				// store the offer index
				pListInfo[iCount].iData=i;
				pListInfo[iCount].iSortIndex=static_cast<int>(pDLC->uiSortIndex);
#ifdef _DEBUG
				app.DebugPrintf("Adding ");
				OutputDebugStringW(pListInfo[iCount].pwszText);
				app.DebugPrintf(" at %d\n",i);
#endif

				m_pOffersList->AddData(pListInfo[iCount],0,CXuiCtrl4JList::eSortList_Index);
				//offerIndexes.push_back(i);

				if(xOffer.fUserHasPurchased)
				{
					HXUIBRUSH hBrush;

					if(RenderManager.IsHiDef() || RenderManager.IsWidescreen())
					{
						swprintf(szResourceLocator, LOCATOR_SIZE, L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/Graphics/DLC_Tick.png");
						XuiCreateTextureBrush(szResourceLocator,&hBrush);
					}
					else
					{
						swprintf(szResourceLocator, LOCATOR_SIZE, L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/Graphics/DLC_TickSmall.png");
						XuiCreateTextureBrush(szResourceLocator,&hBrush);
					}
					m_pOffersList->UpdateGraphicFromiData(i,hBrush);
				}

				/** 4J JEV:
					* We've filtered results out from the list, need to keep track
					* of the 'actual' list index.
					*/
				iCount++;			
			}
		}

		// Check if there is nothing to display, and display the default "nothing available at this time"
		if(iCount>0)
		{
			bNoDLCToDisplay=false;
		}
	}

	// turn off the timer display
	m_Timer.SetShow(FALSE);
	if(iCount!=0)
	{	
		// get the right index for the first list item - it will have been re-sorted internally in the list
		int iIndex=0;
		xOffer=StorageManager.GetOffer(m_pOffersList->GetData(iIndex).iData);
		m_pOffersList->SetCurSelVisible(0);

		DLC_INFO *dlc = app.GetDLCInfoForFullOfferID(xOffer.qwOfferID);
		if (dlc != nullptr)
		{
			BYTE *pData=nullptr;
			UINT uiSize=0;
			DWORD dwSize=0;

			WCHAR *cString = dlc->wchBanner;
			// is the file in the TMS XZP?	 
			int iIndex = app.GetLocalTMSFileIndex(cString, true);

			if(iIndex!=-1)
			{
				// it's in the xzp
				if(m_hXuiBrush!=nullptr)
				{
					XuiDestroyBrush(m_hXuiBrush);
					// clear the TMS XZP vector memory
					//app.FreeLocalTMSFiles();
				}
				app.LoadLocalTMSFile(cString);
				XuiCreateTextureBrushFromMemory(app.TMSFileA[iIndex].pbData,app.TMSFileA[iIndex].uiSize,&m_hXuiBrush);
			}
			else
			{
				bool bPresent = app.IsFileInMemoryTextures(cString);
				if (!bPresent)
				{
					// Image has not come in yet
					// Set the item monitored in the timer, so we can set the image when it comes in
					m_pNoImageFor_DLC=dlc;
				}
				else
				{
					if(m_hXuiBrush!=nullptr)
					{
						XuiDestroyBrush(m_hXuiBrush);
						// clear the TMS XZP vector memory
						//app.FreeLocalTMSFiles();
					}
					app.GetMemFileDetails(cString,&pData,&dwSize);
					XuiCreateTextureBrushFromMemory(pData,dwSize,&m_hXuiBrush);
				}			
			}
		}

		wchar_t formatting[40];
		wstring wstrTemp = xOffer.wszSellText;
		swprintf(formatting, 40, L"<font size=\"%d\">", m_bIsSD?12:14);
		wstrTemp = formatting + wstrTemp;

		m_SellText.SetText(wstrTemp.c_str());
		m_SellText.SetShow(TRUE);

		// set the price info	
		m_PriceTag.SetShow(TRUE);
// 		swprintf(formatting, 40, L"%d",xOffer.dwPointsPrice);
// 		wstrTemp=wstring(formatting);
// 		m_PriceTag.SetText(wstrTemp.c_str());
		m_PriceTag.SetText(xOffer.wszCurrencyPrice);

		XuiElementSetShow(m_List,TRUE);
		XuiElementSetFocus(m_List);

		UpdateTooltips(xOffer);
	}
	else if(bNoDLCToDisplay)
	{
		// set the default text

		wchar_t formatting[40];
		wstring wstrTemp = app.GetString(IDS_NO_DLCOFFERS);
		swprintf(formatting, 40, L"<font size=\"%d\">", m_bIsSD?12:14);
		wstrTemp = formatting + wstrTemp;

		m_SellText.SetText(wstrTemp.c_str());
		m_SellText.SetShow(TRUE);		
	}
	return S_OK;
}

HRESULT CScene_DLCOffers::OnDestroy()
{
	// 4J-PB - don't cancel the DLC anymore
	//StorageManager.CancelGetDLCOffers();

	// clear out any TMS images loaded from the XZP
	app.FreeLocalTMSFiles(eTMSFileType_MinecraftStore);
	return S_OK;
}


//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_DLCOffers::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnorePress) return S_OK;

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(hObjPressed==m_List)
	{
		CXuiControl pItem;
		int iIndex;
		CXuiCtrl4JList::LIST_ITEM_INFO ItemInfo;
		// get the selected item
		iIndex=m_List.GetCurSel(&pItem);
		ItemInfo=m_pOffersList->GetData(iIndex);

		ULONGLONG ullIndexA[1];

		// check if it's already installed
		// 		if(StorageManager.GetOffer(iIndex).fUserHasPurchased)
		// 		{
		// 
		// 		}
		// 		else
		// if it's already been purchased, we need to let the user download it anyway
		{
			ullIndexA[0]=StorageManager.GetOffer(ItemInfo.iData).qwOfferID;
			StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);
		}	
	}
	
	return S_OK;
}

HRESULT CScene_DLCOffers::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	// Fix for Compliance fail -
	// On a functional console, the game must not enter an extended unresponsive state, cause unintentional loss of player data, crash, or cause an unintended reboot of the machine.

	//if(m_bIgnorePress) return S_OK;

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_ESCAPE:
		app.NavigateBack(XUSER_INDEX_ANY);
		rfHandled = TRUE;

		break;

	case VK_PAD_RTHUMB_DOWN:
		{
			XUIHtmlScrollInfo ScrollInfo;

			XuiHtmlControlGetVScrollInfo(m_SellText.m_hObj,&ScrollInfo);
			if(!ScrollInfo.bScrolling)
			{
				XuiHtmlControlVScrollBy(m_SellText.m_hObj,1);
			}
		}
		break;
	case VK_PAD_RTHUMB_UP:
		{
			XUIHtmlScrollInfo ScrollInfo;

			XuiHtmlControlGetVScrollInfo(m_SellText.m_hObj,&ScrollInfo);
			if(!ScrollInfo.bScrolling)
			{
				XuiHtmlControlVScrollBy(m_SellText.m_hObj,-1);
			}
		}		
		break;
	}

	return S_OK;
}


HRESULT CScene_DLCOffers::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	// re-enable button presses
	m_bIgnorePress=false;

	return S_OK;
}

//void CScene_DLCOffers::UpdateTooltips(XMARKETPLACE_CONTENTOFFER_INFO& xOffer)
void CScene_DLCOffers::UpdateTooltips(XMARKETPLACE_CURRENCY_CONTENTOFFER_INFO& xOffer)
{
	// if the current offer hasn't been purchased already, check if there's a trial version available
	if(xOffer.fUserHasPurchased==false )
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_INSTALL,IDS_TOOLTIPS_BACK);
	}
	else
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_REINSTALL,IDS_TOOLTIPS_BACK);
	}
}


HRESULT CScene_DLCOffers::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled)
{
	if( pGetSourceImageData->bItemData )
	{
		pGetSourceImageData->hBrush = m_hXuiBrush;	

		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CScene_DLCOffers::OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled)
{
	//int index = pNotifySelChangedData->iItem;
	// reset the image monitor, but not for the first selection
	if(pNotifySelChangedData->iOldItem!=-1)
	{
		m_pNoImageFor_DLC=nullptr;
	}

	if (m_List.TreeHasFocus())// && offerIndexes.size() > index)
	{
		CXuiControl pItem;
		int iIndex;
		CXuiCtrl4JList::LIST_ITEM_INFO ItemInfo;
		// get the selected item
		iIndex=m_List.GetCurSel(&pItem);
		ItemInfo=m_pOffersList->GetData(iIndex);

// 		XMARKETPLACE_CONTENTOFFER_INFO xOffer= 
// 			StorageManager.GetOffer(ItemInfo.iData);
		XMARKETPLACE_CURRENCY_CONTENTOFFER_INFO xOffer= 
			StorageManager.GetOffer(ItemInfo.iData);


		wchar_t formatting[40];
		wstring wstrTemp=xOffer.wszSellText;
		swprintf(formatting, 40, L"<font size=\"%d\">",m_bIsSD?12:14);
		wstrTemp = wstring(formatting) + wstrTemp;

		m_SellText.SetText(wstrTemp.c_str());

		// set the price info	
 		m_PriceTag.SetShow(TRUE);
// 		swprintf(formatting, 40, L"%d",xOffer.dwPointsPrice);
// 		wstrTemp=wstring(formatting);
// 		m_PriceTag.SetText(wstrTemp.c_str());
		m_PriceTag.SetText(xOffer.wszCurrencyPrice);

		DLC_INFO *dlc = app.GetDLCInfoForTrialOfferID(xOffer.qwOfferID);
		if(dlc==nullptr)
		{
			dlc = app.GetDLCInfoForFullOfferID(xOffer.qwOfferID);
		}

		if (dlc != nullptr)
		{
			BYTE *pImage=nullptr;
			UINT uiSize=0;
			DWORD dwSize=0;

			WCHAR *cString = dlc->wchBanner;

			int iIndex = app.GetLocalTMSFileIndex(cString,true);

			if(iIndex!=-1)
			{
				// it's in the xzp
				if(m_hXuiBrush!=nullptr)
				{
					XuiDestroyBrush(m_hXuiBrush);
					// clear the TMS XZP vector memory
					//app.FreeLocalTMSFiles();
				}
				app.LoadLocalTMSFile(cString);
				XuiCreateTextureBrushFromMemory(app.TMSFileA[iIndex].pbData,app.TMSFileA[iIndex].uiSize,&m_hXuiBrush);
			}
			else
			{
				bool bPresent = app.IsFileInMemoryTextures(cString);
				if (!bPresent)
				{
					// Image has not come in yet
					// Set the item monitored in the timer, so we can set the image when it comes in
					m_pNoImageFor_DLC=dlc;

					// promote it to the top of the queue of images to be retrieved
					// We can't trust the dwContentCategory from partnernet - it has avatar items as content instead of avatars
					app.AddTMSPPFileTypeRequest(dlc->eDLCType,true);
				}
				else
				{
					if(m_hXuiBrush!=nullptr)
					{
						XuiDestroyBrush(m_hXuiBrush);
						// clear the TMS XZP vector memory
						//app.FreeLocalTMSFiles();
					}
					app.GetMemFileDetails(cString,&pImage,&dwSize);
					XuiCreateTextureBrushFromMemory(pImage,dwSize,&m_hXuiBrush);
				}			
			}
		}	
		else
		{
			if(m_hXuiBrush!=nullptr)
			{
				XuiDestroyBrush(m_hXuiBrush);
				// clear the TMS XZP vector memory
				//app.FreeLocalTMSFiles();

				m_hXuiBrush=nullptr;
			}
		}

		UpdateTooltips(xOffer);
	}
	else
	{
		m_SellText.SetText(L"");
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_ACCEPT ,IDS_TOOLTIPS_BACK);
	}
	return S_OK;
}


HRESULT CScene_DLCOffers::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	// check the ethernet status - if it's disconnected, exit the xui

	if(ProfileManager.GetLiveConnectionStatus()!=XONLINE_S_LOGON_CONNECTION_ESTABLISHED)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;

		m_pOffersList->RemoveAllData();
		m_iOfferC=0;
		StorageManager.ClearDLCOffers();
		app.ClearAndResetDLCDownloadQueue();

		StorageManager.RequestMessageBox(IDS_CONNECTION_LOST, IDS_CONNECTION_LOST_LIVE, uiIDA, 1, ProfileManager.GetPrimaryPad(),&CConsoleMinecraftApp::EthernetDisconnectReturned,this, app.GetStringTable());
	}

	// Is the DLC we're looking for available?
	if(!m_bDLCRequiredIsRetrieved)
	{
		if(app.DLCContentRetrieved(static_cast<eDLCMarketplaceType>(m_iType)))
		{	
			m_bDLCRequiredIsRetrieved=true;
		
			// Retrieve the info
			GetDLCInfo(app.GetDLCOffersCount(), false);
			m_bIgnorePress=false;
		}
	}

	// Check for any TMS image we're waiting for
	if(m_pNoImageFor_DLC!=nullptr)
	{
		// Is it present now?
		WCHAR *cString = m_pNoImageFor_DLC->wchBanner;

		bool bPresent = app.IsFileInMemoryTextures(cString);

		if(bPresent)
		{
			BYTE *pImage=nullptr;
			DWORD dwSize=0;

			if(m_hXuiBrush!=nullptr)
			{
				XuiDestroyBrush(m_hXuiBrush);
				// clear the TMS XZP vector memory
				//app.FreeLocalTMSFiles();
			}
			app.GetMemFileDetails(cString,&pImage,&dwSize);
			XuiCreateTextureBrushFromMemory(pImage,dwSize,&m_hXuiBrush);
			m_pNoImageFor_DLC=nullptr;
		}
	}

	return S_OK;
}

// int CScene_DLCOffers::EthernetDisconnectReturned(void *pParam,int iPad,const C4JStorage::EMessageResult)
// {
// 	CConsoleMinecraftApp* pApp = (CConsoleMinecraftApp*)pParam;
// 	
// 	pApp->NavigateBack(XUSER_INDEX_ANY);
// 
// 	return 0;
// }

HRESULT CScene_DLCOffers::OnCustomMessage_DLCInstalled()
{
	// mounted DLC may have changed - need to re-run the GetDLC
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1, IDS_TOOLTIPS_BACK);
	m_bIgnorePress=true;
	m_pOffersList->RemoveAllData();
	m_iOfferC=0;

	// Update the dlc info
	StorageManager.ClearDLCOffers();
	app.ClearAndResetDLCDownloadQueue();

	// order these requests so the current DLC comes in first
	switch(m_iType)
	{
	case e_DLC_Gamerpics:
		app.AddDLCRequest(e_Marketplace_Gamerpics);
		app.AddDLCRequest(e_Marketplace_Content);
		app.AddDLCRequest(e_Marketplace_Themes);
		app.AddDLCRequest(e_Marketplace_AvatarItems);
		break;
	case e_DLC_Themes:
		app.AddDLCRequest(e_Marketplace_Themes);
		app.AddDLCRequest(e_Marketplace_Content);
		app.AddDLCRequest(e_Marketplace_Gamerpics);
		app.AddDLCRequest(e_Marketplace_AvatarItems);
		break;
	case e_DLC_AvatarItems:
		app.AddDLCRequest(e_Marketplace_AvatarItems);
		app.AddDLCRequest(e_Marketplace_Content);
		app.AddDLCRequest(e_Marketplace_Themes);
		app.AddDLCRequest(e_Marketplace_Gamerpics);
		break;
	default:
		app.AddDLCRequest(e_Marketplace_Content);
		app.AddDLCRequest(e_Marketplace_Gamerpics);
		app.AddDLCRequest(e_Marketplace_Themes);
		app.AddDLCRequest(e_Marketplace_AvatarItems);
		break;
	}

	m_Timer.SetShow(TRUE);
	m_bDLCRequiredIsRetrieved=false;

	return S_OK;
}
