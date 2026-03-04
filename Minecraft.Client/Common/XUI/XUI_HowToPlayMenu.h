#pragma once

#include "../media/xuiscene_howtoplay_menu.h"
#include "XUI_CustomMessages.h"

class CScene_HowToPlayMenu : public CXuiSceneImpl
{
protected:

	enum eHTPButton
	{
		eHTPButton_WhatsNew = 0,
		eHTPButton_Basics,
		eHTPButton_Multiplayer,
		eHTPButton_Hud,
		eHTPButton_Creative,
		eHTPButton_Inventory,
		eHTPButton_Chest,
		eHTPButton_Crafting,
		eHTPButton_Furnace,
		eHTPButton_Dispenser,
		eHTPButton_Brewing,
		eHTPButton_Enchantment,
		eHTPButton_Anvil,
		eHTPButton_FarmingAnimals,
		eHTPButton_Breeding,
		eHTPButton_Trading,
		eHTPButton_NetherPortal,
		eHTPButton_TheEnd,
		eHTPButton_SocialMedia,
		eHTPButton_BanningLevels,
		eHTPButton_HostOptions,
		eHTPButton_Max,
	};

	// Control and Element wrapper objects.
	CXuiScene	m_Scene;
	CXuiElement m_Background;
	CXuiList m_ButtonList;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_NOTIFY_SELCHANGED(OnNotifySelChanged)

	XUI_END_MSG_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled);

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_HowToPlayMenu, L"CScene_HowToPlayMenu", XUI_CLASS_SCENE )

private:

	int m_iPad;
	D3DXVECTOR3 m_OriginalPosition;
	static unsigned int m_uiHTPButtonNameA[eHTPButton_Max];
	static unsigned int m_uiHTPSceneA[eHTPButton_Max];
	int m_iButtons;
};
