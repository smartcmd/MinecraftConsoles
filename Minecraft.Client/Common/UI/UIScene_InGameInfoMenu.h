#pragma once

#include "UIScene.h"

class UIScene_InGameInfoMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_GameOptions,
		eControl_GamePlayers,
	};

	typedef struct _PlayerInfo
	{
		byte m_smallId;
		char m_voiceStatus;
		short m_colorState;
		wstring m_name;

	} PlayerInfo;
	
	bool m_isHostPlayer;
	//int m_playersCount;
	vector<PlayerInfo *> m_players; // A vector of player info structs
	//char m_playersVoiceState[MINECRAFT_NET_MAX_PLAYERS];
	//short m_playersColourState[MINECRAFT_NET_MAX_PLAYERS];
	//wstring m_playerNames[MINECRAFT_NET_MAX_PLAYERS];

	UIControl_Button m_buttonGameOptions;
	UIControl_PlayerList m_playerList;
	UIControl_Label m_labelTitle;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_buttonGameOptions, "GameOptions")
		UI_MAP_ELEMENT( m_playerList, "GamePlayers")
		UI_MAP_ELEMENT( m_labelTitle, "Title")
	UI_END_MAP_ELEMENTS_AND_NAMES()
public:
	UIScene_InGameInfoMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_InGameInfoMenu();

	virtual EUIScene getSceneType() { return eUIScene_InGameInfoMenu;}
	virtual void updateTooltips();
	
	virtual void handleReload();

	virtual void tick();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

protected:
	virtual void handleGainFocus(bool navBack);
	void handlePress(F64 controlId, F64 childId);
	virtual void handleDestroy();
	virtual void handleFocusChange(F64 controlId, F64 childId);

public:
	static int KickPlayerReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static void OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving);

private:
	PlayerInfo *BuildPlayerInfo(INetworkPlayer *player);

#if defined(__PS3__) || defined (__PSVITA__) || defined(__ORBIS__)
	static int MustSignInReturnedPSN(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int ViewInvites_SignInReturned(void *pParam,bool bContinue, int iPad);
#endif
};
