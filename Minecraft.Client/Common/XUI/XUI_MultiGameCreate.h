#pragma once
#include "..\Media\xuiscene_multi_create.h"
#include "XUI_Ctrl_4JEdit.h"
#include "XUI_Ctrl_SliderWrapper.h"
#include "XUI_MultiGameLaunchMoreOptions.h"

class CXuiCtrl4JList;
class CXuiCtrl4JIcon;

class CScene_MultiGameCreate : public CXuiSceneImpl
{
protected:
	CXuiScene m_MainScene;
	CXuiScene m_TexturePackDetails;
	CXuiControl m_NewWorld;
	CXuiControl m_labelWorldName;
	CXuiControl m_labelSeed;
	CXuiControl m_labelRandomSeed;
	CXuiControl m_MoreOptions;
	CXuiCtrl4JEdit m_EditSeed;
	CXuiCtrl4JEdit m_EditWorldName;
	CXuiControl m_ButtonGameMode;
	CXuiCtrlSliderWrapper	m_SliderDifficulty;
	CXuiCtrl4JList *m_pTexturePacksList;
	CXuiControl m_texturePackTitle, m_texturePackDescription;
	CXuiCtrl4JIcon *m_texturePackIcon, *m_texturePackComparison;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_NOTIFY_VALUE_CHANGED(OnNotifyValueChanged)
		XUI_ON_XM_CONTROL_NAVIGATE(OnControlNavigate)
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_TRANSITION_END(OnTransitionEnd)
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_NOTIFY_KILL_FOCUS( OnNotifyKillFocus )
		XUI_ON_XM_DLCINSTALLED_MESSAGE(OnCustomMessage_DLCInstalled)
		XUI_ON_XM_DLCLOADED_MESSAGE(OnCustomMessage_DLCMountingComplete)
		XUI_ON_XM_DESTROY( OnDestroy )
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_MainScene, m_MainScene)
		BEGIN_MAP_CHILD_CONTROLS(m_MainScene)
			MAP_CONTROL(IDC_XuiLabelWorldName, m_labelWorldName)
			MAP_CONTROL(IDC_XuiLabelSeed, m_labelSeed)
			MAP_CONTROL(IDC_XuiLabelRandomSeed, m_labelRandomSeed)
			MAP_CONTROL(IDC_XuiGameModeToggle, m_ButtonGameMode)
			MAP_CONTROL(IDC_XuiNewWorld, m_NewWorld)
			MAP_CONTROL(IDC_XuiMoreOptions, m_MoreOptions)
			MAP_CONTROL(IDC_XuiEditSeed, m_EditSeed)
			MAP_CONTROL(IDC_XuiEditWorldName, m_EditWorldName)
			MAP_CONTROL(IDC_XuiSliderDifficulty,	m_SliderDifficulty)
			MAP_OVERRIDE(IDC_TexturePacksList,	m_pTexturePacksList)
		END_MAP_CHILD_CONTROLS()
		MAP_CONTROL(IDC_TexturePackDetails, m_TexturePackDetails)
		BEGIN_MAP_CHILD_CONTROLS(m_TexturePackDetails)
			MAP_CONTROL(IDC_TexturePackName, m_texturePackTitle)
			MAP_CONTROL(IDC_TexturePackDescription,	m_texturePackDescription)
			MAP_OVERRIDE(IDC_Icon,	m_texturePackIcon)
			MAP_OVERRIDE(IDC_ComparisonPic,	m_texturePackComparison)
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyValueChanged (HXUIOBJ hObjSource, XUINotifyValueChanged* pValueChangedData, BOOL& rfHandled);
	HRESULT OnControlNavigate( XUIMessageControlNavigate *pControlNavigateData, BOOL &bHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnTransitionEnd( XUIMessageTransition *pTransition, BOOL& bHandled );
	HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
	HRESULT OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
	HRESULT OnCustomMessage_DLCInstalled();
	HRESULT OnCustomMessage_DLCMountingComplete();
	HRESULT	OnDestroy();

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_MultiGameCreate, L"CScene_MultiGameCreate", XUI_CLASS_SCENE )

private:
	static int LoadSaveDataReturned(void *pParam,bool bContinue);
	static int StartGame_SignInReturned(void *pParam,bool bContinue, int iPad);
	static void CreateGame(CScene_MultiGameCreate* pClass, DWORD dwLocalUsersMask);
	static int ConfirmCreateReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int UnlockTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int WarningTrialTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);


	void ToggleShowSaveList();
	void UpdateTexturePackDescription(int index);
	void ClearTexturePackDescription();
	void UpdateCurrentTexturePack();

	bool m_bMultiplayerAllowed;
	int m_iPad;
	int m_CurrentDifficulty;
	static int m_iDifficultyTitleSettingA[4];
	LaunchMoreOptionsMenuInitData m_MoreOptionsParams;
	bool m_bGameModeSurvival;
	bool m_bIgnoreInput;
	unsigned int m_currentTexturePackIndex;
	DLCPack * m_pDLCPack;
	bool m_bSetup;
	bool m_texturePackDescDisplayed;
	HXUIBRUSH m_hTexturePackIconBrush;
	HXUIBRUSH m_hTexturePackComparisonBrush;
	int *m_iConfigA; // track the texture packs that we don't have installed
	int m_iTexturePacksNotInstalled;
};