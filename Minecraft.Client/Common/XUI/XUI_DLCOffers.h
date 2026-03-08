#pragma once

#include "../media/xuiscene_DLCOffers.h"
#include "../media/xuiscene_DLCMain.h"


#include "XUI_CustomMessages.h"
#include "XUI_Ctrl_4JList.h"
#include "XUI_Ctrl_4JIcon.h"
//#include "XUI_Ctrl_DLCPrice.h"

class CXuiCtrl4JList;
class CScene_DLCOffers;
class CXuiCtrlDLCPrice;

class CScene_DLCMain : public CXuiSceneImpl
{
	// Xui Elements
	CXuiList xList;
	CXuiCtrl4JList *list;
	CXuiControl m_Timer;

	// Misc
	int iPad, iOfferC;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_DESTROY(OnDestroy)
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiOffersList, xList)
		MAP_CONTROL(IDC_Timer, m_Timer)	
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );

public:
	XUI_IMPLEMENT_CLASS( CScene_DLCMain, L"CScene_DLCMain", XUI_CLASS_SCENE )

private:
	bool m_bAllDLCContentRetrieved;
	bool m_bIgnoreInput;
};

class CScene_DLCOffers : public CXuiSceneImpl
{
protected:
	//static const wstring DEFAULT_BANNER;

	// Control and Element wrapper objects.
	CXuiList	m_List;
	CXuiCtrl4JList *m_pOffersList;
	CXuiImageElement m_Banner;
	CXuiCtrl4JIcon m_TMSImage;
	CXuiHtmlControl m_SellText;
	CXuiControl m_PriceTag;
	CXuiControl m_Timer;
	HXUIBRUSH m_hXuiBrush;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_DESTROY(OnDestroy)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_DLCINSTALLED_MESSAGE(OnCustomMessage_DLCInstalled)
		XUI_ON_XM_GET_SOURCE_IMAGE( OnGetSourceDataImage )
	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiOffersList, m_List)
		MAP_CONTROL(IDC_XuiHTMLSellText, m_SellText)
		MAP_CONTROL(IDC_XuiDLCPriceTag, m_PriceTag)
		MAP_CONTROL(IDC_XuiDLCBanner, m_TMSImage)
		MAP_CONTROL(IDC_Timer, m_Timer)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnDestroy();
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnCustomMessage_DLCInstalled();
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled);

	HRESULT GetDLCInfo(  int iOfferC, bool bUpdateOnly=false );

	//static int EthernetDisconnectReturned(void *pParam,int iPad,const C4JStorage::EMessageResult);
	static int TMSReadCallback(void *pParam,int iPad,bool bResult);

	//void UpdateTooltips(XMARKETPLACE_CONTENTOFFER_INFO& xOffer);
	void UpdateTooltips(XMARKETPLACE_CURRENCY_CONTENTOFFER_INFO& xOffer);
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_DLCOffers, L"CScene_DLCOffers", XUI_CLASS_SCENE )

	typedef struct _DLCOffer
	{
		int iOfferC;
	} 
	DLCOffer;

private:
	
	//vector<int> offerIndexes;
	CScene_DLCMain *pMain;
	bool m_bIgnorePress;
	int m_iPad;
	int m_iOfferC;
	int m_iType;
	bool m_bIsSD;
	bool m_bAllDLCContentRetrieved;
	bool m_bDLCRequiredIsRetrieved;
	bool m_bIsFemale; // to only show the correct gender type offers for avatars
	DLC_INFO *m_pNoImageFor_DLC;
	bool bNoDLCToDisplay; // to display a default "No DLC available at this time"

};
