#pragma once

#include "UIScene.h"

class UIScene_MainMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_PlayGame,
		eControl_Leaderboards,
		eControl_Achievements,
		eControl_HelpAndOptions,
		eControl_UnlockOrDLC,
#ifndef _DURANGO
		eControl_Exit,
#else
		eControl_XboxHelp,
#endif
		eControl_Count,
	};

// #ifdef __ORBIS__
// 	enum EPatchCheck
// 	{
// 		ePatchCheck_Idle,
// 		ePatchCheck_Init,
// 		ePatchCheck_Running,
// 	};
// #endif

	UIControl_Button m_buttons[eControl_Count];
	UIControl m_controlTimer;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_buttons[(int)eControl_PlayGame], "Button1")
		UI_MAP_ELEMENT( m_buttons[(int)eControl_Leaderboards], "Button2")
		UI_MAP_ELEMENT( m_buttons[(int)eControl_Achievements], "Button3")
		UI_MAP_ELEMENT( m_buttons[(int)eControl_HelpAndOptions], "Button4")
		UI_MAP_ELEMENT( m_buttons[(int)eControl_UnlockOrDLC], "Button5")
#ifndef _DURANGO
		UI_MAP_ELEMENT( m_buttons[(int)eControl_Exit], "Button6")
#else
		UI_MAP_ELEMENT( m_buttons[(int)eControl_XboxHelp], "Button6")
#endif
		UI_MAP_ELEMENT( m_controlTimer, "Timer")
	UI_END_MAP_ELEMENTS_AND_NAMES()
	
	static Random *random;
	bool m_bIgnorePress;
	bool m_bTrialVersion;
	bool m_bLoadTrialOnNetworkManagerReady;
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	bool m_bLaunchFullVersionPurchase;
#endif

#ifdef _XBOX_ONE
	bool m_bWaitingForDLCInfo;
#endif
	
	float m_fScreenWidth,m_fScreenHeight;
	float m_fRawWidth,m_fRawHeight;
	vector<wstring> m_splashes;
	wstring m_splash;
	enum eSplashIndexes
	{
		eSplashHappyBirthdayEx = 0,
		eSplashHappyBirthdayNotch,
		eSplashMerryXmas,
		eSplashHappyNewYear,

		// The start index in the splashes vector from which we can select a random splash
		eSplashRandomStart,
	};

	enum eActions
	{
		eAction_None=0,
		eAction_RunGame,
		eAction_RunLeaderboards,
		eAction_RunAchievements,
		eAction_RunHelpAndOptions,
		eAction_RunUnlockOrDLC,
#if defined(__PS3__)|| defined(__PSVITA__) || defined(__ORBIS__)
		eAction_RunLeaderboardsPSN,
		eAction_RunGamePSN,
		eAction_RunUnlockOrDLCPSN,
#elif defined _DURANGO
		eAction_RunXboxHelp,
#endif

	};
	eActions m_eAction;

private:
	// 4J-JEV: Delay navigation until font changes.
	static EUIScene eNavigateWhenReady;

	static void proceedToScene(int iPad, EUIScene eScene)
	{
		eNavigateWhenReady = eScene;
	}

public:
	UIScene_MainMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_MainMenu();
	
	// Returns true if this scene has focus for the pad passed in
#ifndef __PS3__
	virtual bool hasFocus(int iPad) { return bHasFocus; }
#endif
	
	virtual void updateTooltips();
	virtual void updateComponents();

	virtual EUIScene getSceneType() { return eUIScene_MainMenu;}

	virtual void customDraw(IggyCustomDrawCallbackRegion *region);
protected:
	void customDrawSplash(IggyCustomDrawCallbackRegion *region);


	virtual wstring getMoviePath();

public:
	virtual void tick();
	virtual void handleReload();
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void handleUnlockFullVersion();

protected:
	void handlePress(F64 controlId, F64 childId);

	void handleGainFocus(bool navBack);

	virtual long long getDefaultGtcButtons() { return 0; }

private:
	void RunPlayGame(int iPad);
	void RunLeaderboards(int iPad);
	void RunUnlockOrDLC(int iPad);
	void RunAchievements(int iPad);
	void RunHelpAndOptions(int iPad);

	void RunAction(int iPad);
	
	static void LoadTrial();

#ifdef _XBOX_ONE
	static int ChooseUser_SignInReturned(void *pParam,bool bContinue, int iPad, int iController);
	static int CreateLoad_SignInReturned(void *pParam,bool bContinue, int iPad, int iController);
	static int HelpAndOptions_SignInReturned(void *pParam,bool bContinue,int iPad, int iController);
	static int Achievements_SignInReturned(void *pParam,bool bContinue,int iPad, int iController);
	static int MustSignInReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

	static int Leaderboards_SignInReturned(void* pParam, bool bContinue, int iPad, int iController);
	static int UnlockFullGame_SignInReturned(void *pParam,bool bContinue,int iPad, int iController);
	static int ExitGameReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);


	static int XboxHelp_SignInReturned(void *pParam, bool bContinue, int iPad, int iController);
#else

	static int CreateLoad_SignInReturned(void *pParam,bool bContinue, int iPad);
	static int HelpAndOptions_SignInReturned(void *pParam,bool bContinue,int iPad);
	static int Achievements_SignInReturned(void *pParam,bool bContinue,int iPad);
	static int MustSignInReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

#if defined(__PS3__) || defined(__PSVITA__) || defined(__ORBIS__)
	static int MustSignInReturnedPSN(void *pParam,int iPad,C4JStorage::EMessageResult result);
#endif
	static int Leaderboards_SignInReturned(void* pParam, bool bContinue, int iPad);
	static int UnlockFullGame_SignInReturned(void *pParam,bool bContinue,int iPad);
	static int ExitGameReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);

#ifdef __ORBIS__
	static void RefreshChatAndContentRestrictionsReturned_PlayGame(void *pParam);
	static void RefreshChatAndContentRestrictionsReturned_Leaderboards(void *pParam);

	static int PlayOfflineReturned(void *pParam, int iPad, C4JStorage::EMessageResult result);
#endif
#endif

#ifdef __PSVITA__
	static int SelectNetworkModeReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
#endif

#ifdef __ORBIS__
	//EPatchCheck m_ePatchCheckState;
	bool m_bRunGameChosen;
	int32_t m_errorCode;
	bool m_bErrorDialogRunning;
#endif
};
