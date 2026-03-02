#pragma once

#include "IUIScene_StartGame.h"

class UIScene_LoadMenu : public IUIScene_StartGame
{
private:
	enum EControls
	{
		eControl_GameMode,
		eControl_Difficulty,
		eControl_MoreOptions,
		eControl_LoadWorld,
		eControl_TexturePackList,
		eControl_OnlineGame,
	};

	static int m_iDifficultyTitleSettingA[4];
	
	UIControl m_controlMainPanel;
	UIControl_Label m_labelGameName, m_labelSeed, m_labelCreatedMode;
	UIControl_Button m_buttonGamemode, m_buttonMoreOptions, m_buttonLoadWorld;
	UIControl_Slider m_sliderDifficulty;
	UIControl_BitmapIcon m_bitmapIcon;
	
#if defined _XBOX_ONE || defined __ORBIS__ || defined _WINDOWS64
	UIControl_CheckBox m_checkboxOnline;
#endif

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(IUIScene_StartGame)
		UI_MAP_ELEMENT( m_controlMainPanel, "MainPanel" )
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_labelGameName, "GameName")
			UI_MAP_ELEMENT( m_labelCreatedMode, "CreatedMode")
			UI_MAP_ELEMENT( m_labelSeed, "Seed")
			UI_MAP_ELEMENT( m_texturePackList, "TexturePackSelector")
			UI_MAP_ELEMENT( m_buttonGamemode, "GameModeToggle")
			
#if defined _XBOX_ONE || defined __ORBIS__ || defined _WINDOWS64
			UI_MAP_ELEMENT( m_checkboxOnline, "CheckboxOnline")
#endif
			UI_MAP_ELEMENT( m_buttonMoreOptions, "MoreOptions")
			UI_MAP_ELEMENT( m_buttonLoadWorld, "LoadSettings")
			UI_MAP_ELEMENT( m_sliderDifficulty, "Difficulty")
			UI_MAP_ELEMENT( m_bitmapIcon, "LevelIcon")
		UI_END_MAP_CHILD_ELEMENTS()
	UI_END_MAP_ELEMENTS_AND_NAMES()

	LevelGenerationOptions *m_levelGen;
	DLCPack * m_pDLCPack;
	
	int m_iSaveGameInfoIndex;
	int m_CurrentDifficulty;
	bool m_bGameModeSurvival;
	bool m_bHasBeenInCreative;
	bool m_bRetrievingSaveThumbnail;
	bool m_bSaveThumbnailReady;
	bool m_bMultiplayerAllowed;
	bool m_bShowTimer;
	bool m_bAvailableTexturePacksChecked;
	bool m_bRequestQuadrantSignin;
	bool m_bIsCorrupt;
	bool m_bThumbnailGetFailed;
	__int64 m_seed;

#ifdef __PS3__
	std::vector<SonyCommerce::ProductInfo>*m_pvProductInfo;
#endif
	//int *m_iConfigA; // track the texture packs that we don't have installed

	PBYTE m_pbThumbnailData;
	unsigned int m_uiThumbnailSize;
	wstring m_thumbnailName;

	bool m_bRebuildTouchBoxes;
public:
	UIScene_LoadMenu(int iPad, void *initData, UILayer *parentLayer);
	
	virtual void updateTooltips();
	virtual void updateComponents();

	virtual EUIScene getSceneType() { return eUIScene_LoadMenu;}

	virtual void tick();

	virtual UIControl* GetMainPanel();

	virtual void handleTouchBoxRebuild();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
	virtual void handleTimerComplete(int id);

protected:
	void handlePress(F64 controlId, F64 childId);
	void handleSliderMove(F64 sliderId, F64 currentValue);
	virtual void handleGainFocus(bool navBack);

private:
	void StartSharedLaunchFlow();
	virtual void checkStateAndStartGame();
	void LoadLevelGen(LevelGenerationOptions *levelGen);
	void LaunchGame(void);

#ifdef _DURANGO
	static void checkPrivilegeCallback(LPVOID lpParam, bool hasPrivilege, int iPad);
#endif
	
	static int ConfirmLoadReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static void StartGameFromSave(UIScene_LoadMenu* pClass, DWORD dwLocalUsersMask);
	static int LoadSaveDataReturned(void *pParam,bool bIsCorrupt, bool bIsOwner);
	static int TrophyDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int LoadDataComplete(void *pParam);
	static int LoadSaveDataThumbnailReturned(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes);
	static int CheckResetNetherReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int DeleteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int DeleteSaveDataReturned(void *pParam,bool bSuccess);
	static int MustSignInReturnedPSN(void *pParam,int iPad,C4JStorage::EMessageResult result);
#ifdef __ORBIS__
	//static int PSPlusReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int ContinueOffline(void *pParam,int iPad,C4JStorage::EMessageResult result);
#endif

public:
	static int StartGame_SignInReturned(LPVOID pParam, bool, int);
};