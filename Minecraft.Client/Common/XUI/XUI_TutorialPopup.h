#pragma once
#include "../media/xuiscene_tutorialpopup.h"
#include "XUI_CustomMessages.h"

class Tutorial;
class CXuiCtrlCraftIngredientSlot;

#define TUTORIAL_POPUP_FADE_TIMER_ID 0
#define TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID 1
#define TUTORIAL_POPUP_MOVE_SCENE_TIME 500

class CScene_TutorialPopup : public CXuiSceneImpl
{
private:
	Tutorial *tutorial;

	// A scene that may be displayed behind the popup that the player is using, that will need shifted so we can see it clearly.
	CXuiScene *m_interactScene, *m_lastInteractSceneMoved;
	bool m_lastSceneMovedLeft;
	bool m_bAllowFade;

	int m_iPad;

	CXuiHtmlControl m_description;
	CXuiCtrlCraftIngredientSlot *m_pCraftingPic;
	CXuiControl m_title;
	CXuiImageElement m_image;

	CXuiControl m_fontSizeControl;

	int m_textFontSize;
	D3DXVECTOR3 m_OriginalPosition;

protected:
	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_TIMER(OnTimer)
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Title, m_title)
		MAP_CONTROL(IDC_Description, m_description)
		MAP_OVERRIDE(IDC_XuiInventoryPic, m_pCraftingPic)
		MAP_CONTROL(IDC_XuiImage, m_image)
		MAP_CONTROL(IDC_FontSize, m_fontSizeControl)

	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnTimer(XUIMessageTimer *pData,BOOL& rfHandled);
	HRESULT OnDestroy();
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_TutorialPopup, L"CScene_TutorialPopup", XUI_CLASS_SCENE )

private:
	HRESULT _SetDescription(CXuiScene *interactScene, LPCWSTR desc, LPCWSTR title, bool allowFade, bool isReminder);
	wstring _SetIcon(int icon, int iAuxVal, bool isFoil, LPCWSTR desc);
	wstring _SetImage(wstring &desc);
	HRESULT _SetVisible(bool show);
	bool _IsSceneVisible();
	void UpdateInteractScenePosition(bool visible);

public:
	static HRESULT SetDescription(int iPad, TutorialPopupInfo *info);
	static wstring ParseDescription(int iPad, wstring &text);

	static HRESULT SetSceneVisible(int iPad, bool show);
	static bool IsSceneVisible(int iPad);

};