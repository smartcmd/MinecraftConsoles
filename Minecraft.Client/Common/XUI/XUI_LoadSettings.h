#pragma once
#include "../media/xuiscene_load_settings.h"
#include "XUI_Ctrl_SliderWrapper.h"
#include "XUI_Ctrl_4JIcon.h"
#include "XUI_CustomMessages.h"
#include "XUI_MultiGameLaunchMoreOptions.h"

class CScene_LoadGameSettings : public CXuiSceneImpl
{
public:
	typedef struct 
	{
		unsigned int uiLen;
		unsigned int uiCode;
	}
	PNG_CHUNK;
protected:
	// Control and Element wrapper objects.
	CXuiScene m_MainScene;
	CXuiScene m_TexturePackDetails;
	CXuiControl m_GameName;
	CXuiControl m_GameSeed;
	CXuiControl m_GameCreatedMode;
	CXuiControl m_ButtonLoad;
	CXuiControl m_ButtonGameMode;
	CXuiControl m_MoreOptions;
	CXuiCtrl4JIcon m_GameIcon;
	CXuiCtrlSliderWrapper	m_SliderDifficulty;
	CXuiCtrl4JList *m_pTexturePacksList;
	CXuiControl m_texturePackTitle, m_texturePackDescription;
	CXuiCtrl4JIcon *m_texturePackIcon, *m_texturePackComparison;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_VALUE_CHANGED(OnNotifyValueChanged)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_TRANSITION_END(OnTransitionEnd)
		XUI_ON_XM_FONTRENDERERCHANGE_MESSAGE(OnFontRendererChange)
		XUI_ON_XM_NOTIFY_SELCHANGED( OnNotifySelChanged )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_NOTIFY_KILL_FOCUS( OnNotifyKillFocus )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_DLCINSTALLED_MESSAGE(OnCustomMessage_DLCInstalled)
		XUI_ON_XM_DLCLOADED_MESSAGE(OnCustomMessage_DLCMountingComplete)
		XUI_ON_XM_NAV_RETURN(OnNavReturn)
	XUI_END_MSG_MAP()

		// Control mapping to objects
		BEGIN_CONTROL_MAP()
			MAP_CONTROL(IDC_MainScene, m_MainScene)
			BEGIN_MAP_CHILD_CONTROLS(m_MainScene)
				MAP_CONTROL(IDC_XuiLoadSettings, m_ButtonLoad)
				MAP_CONTROL(IDC_XuiGameModeToggle, m_ButtonGameMode)
				MAP_CONTROL(IDC_XuiMoreOptions, m_MoreOptions)
				MAP_CONTROL(IDC_XuiGameIcon, m_GameIcon);
				MAP_CONTROL(IDC_XuiGameName, m_GameName)
				MAP_CONTROL(IDC_XuiGameSeed, m_GameSeed)
				MAP_CONTROL(IDC_XuiCreatedMode, m_GameCreatedMode)
				MAP_CONTROL(IDC_XuiSliderDifficulty,	m_SliderDifficulty)
				MAP_OVERRIDE(IDC_TexturePacksList,	m_pTexturePacksList)
				//MAP_CONTROL(IDC_XuiGameMode,	m_GameMode)
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
		HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
		HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
		HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled);
		HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled );
		HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
		HRESULT OnTransitionEnd( XUIMessageTransition *pTransition, BOOL& bHandled );
		HRESULT OnFontRendererChange();
		HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
		HRESULT OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled );
		HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
		HRESULT OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
		HRESULT	OnDestroy();
		HRESULT OnCustomMessage_DLCInstalled();
		HRESULT OnCustomMessage_DLCMountingComplete();
		HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);


		static int LoadSaveDataReturned(void *pParam,bool bContinue);
		static int LoadSaveDataForNameChangeReturned(void *pParam,bool bContinue);
		static int DeviceRemovedDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
		static void StartGameFromSave(CScene_LoadGameSettings* pClass, DWORD dwLocalUsersMask);
		static int StartGame_SignInReturned(void *pParam,bool bContinue, int iPad);
		static int DeviceSelectReturned(void *pParam,bool bContinue);
		static int ConfirmLoadReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
		static int DeleteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
		static int DeleteSaveDataReturned(void *pParam,bool bSuccess);
		static int CheckResetNetherReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
		static int TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

		static int Progress(void *pParam,float fProgress);

		void LoadLevelGen(LevelGenerationOptions *levelGen);

		HRESULT LaunchGame(void);
public:
	static unsigned char szPNG[8];

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_LoadGameSettings, L"CScene_LoadGameSettings", XUI_CLASS_SCENE )

private:
	static int UnlockTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	void UpdateTexturePackDescription(int index);
	void ClearTexturePackDescription();
	void UpdateCurrentTexturePack();

	bool m_bIgnoreInput;
	HXUIBRUSH m_hXuiBrush;
	HXUIBRUSH m_hTexturePackIconBrush;
	HXUIBRUSH m_hTexturePackComparisonBrush;

	int m_iPad;
	int m_iSaveGameInfoIndex;
	bool m_bMultiplayerAllowed;
	static int m_iDifficultyTitleSettingA[4];
	int m_CurrentDifficulty;
	bool m_bHasBeenInCreative;
	bool m_bSetup;
	bool m_texturePackDescDisplayed;

	DWORD m_dwSaveFileC;
#ifdef _XBOX
	C4JStorage::CACHEINFOSTRUCT *m_InfoA;
#endif
	unsigned char m_szSeed[50];
	XCONTENT_DATA m_XContentData;
	LaunchMoreOptionsMenuInitData m_MoreOptionsParams;
	bool m_bGameModeSurvival;
	unsigned int m_currentTexturePackIndex;
	DLCPack * m_pDLCPack;
	LevelGenerationOptions *m_levelGen;

	int m_iTexturePacksNotInstalled;
	int *m_iConfigA; // track the texture packs that we don't have installed
	LoadMenuInitData *m_params;
};