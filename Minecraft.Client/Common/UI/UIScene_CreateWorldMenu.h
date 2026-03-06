#pragma once

#include "IUIScene_StartGame.h"

class UIScene_CreateWorldMenu : public IUIScene_StartGame
{
private:
	enum EControls
	{
		eControl_EditWorldName,
		eControl_TexturePackList,
		eControl_GameModeToggle,
		eControl_Difficulty,
		eControl_MoreOptions,
		eControl_NewWorld,
		eControl_OnlineGame,
	};

	static int m_iDifficultyTitleSettingA[4];


	wstring m_worldName;
	wstring m_seed;

	UIControl m_controlMainPanel;
	UIControl_Label m_labelWorldName;
	UIControl_Button m_buttonGamemode, m_buttonMoreOptions, m_buttonCreateWorld;
	UIControl_TextInput m_editWorldName;
	UIControl_Slider m_sliderDifficulty;
	UIControl_CheckBox m_checkboxOnline;

	UIControl_BitmapIcon m_bitmapIcon, m_bitmapComparison;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(IUIScene_StartGame)
		UI_MAP_ELEMENT( m_controlMainPanel, "MainPanel" )
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_labelWorldName, "WorldName")
			UI_MAP_ELEMENT( m_editWorldName, "EditWorldName")
			UI_MAP_ELEMENT( m_texturePackList, "TexturePackSelector")
			UI_MAP_ELEMENT( m_buttonGamemode, "GameModeToggle")
			UI_MAP_ELEMENT( m_checkboxOnline, "CheckboxOnline")
			UI_MAP_ELEMENT( m_buttonMoreOptions, "MoreOptions")
			UI_MAP_ELEMENT( m_buttonCreateWorld, "NewWorld")
			UI_MAP_ELEMENT( m_sliderDifficulty, "Difficulty")
		UI_END_MAP_CHILD_ELEMENTS()
	UI_END_MAP_ELEMENTS_AND_NAMES()

	bool m_bGameModeCreative;
	int m_iGameModeId;
	bool m_bMultiplayerAllowed;
	DLCPack * m_pDLCPack;
	bool m_bRebuildTouchBoxes;

#ifdef _WINDOWS64
	bool m_bDirectEditing;
	wstring m_worldNameBeforeEdit;
	int m_iDirectEditCooldown;
#endif

public:
	UIScene_CreateWorldMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_CreateWorldMenu();

	virtual void updateTooltips();
	virtual void updateComponents();

	virtual EUIScene getSceneType() { return eUIScene_CreateWorldMenu;}

	virtual void handleDestroy();
	virtual void tick();

	virtual UIControl* GetMainPanel();

	virtual void handleTouchBoxRebuild();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();
	
	virtual void handleTimerComplete(int id);
	virtual void handleGainFocus(bool navBack);

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

private:
	void StartSharedLaunchFlow();
	bool IsLocalMultiplayerAvailable();

#ifdef _DURANGO
	static void checkPrivilegeCallback(LPVOID lpParam, bool hasPrivilege, int iPad);
#endif

protected:
	static int KeyboardCompleteWorldNameCallback(LPVOID lpParam,const bool bRes);
	void handlePress(F64 controlId, F64 childId);
	void handleSliderMove(F64 sliderId, F64 currentValue);
	
	static void CreateGame(UIScene_CreateWorldMenu* pClass, DWORD dwLocalUsersMask);
	static int ConfirmCreateReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int StartGame_SignInReturned(void *pParam,bool bContinue, int iPad);
	static int MustSignInReturnedPSN(void *pParam,int iPad,C4JStorage::EMessageResult result);

#ifdef __ORBIS__
	//static int PSPlusReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int ContinueOffline(void *pParam,int iPad,C4JStorage::EMessageResult result);
#endif

	virtual void checkStateAndStartGame();
};