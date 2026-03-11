#include "stdafx.h"
#include "UI.h"
#include "UIScene_JoinMenu.h"
#include "..\..\Minecraft.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\Options.h"
#include "..\..\MinecraftServer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.h"

#define UPDATE_PLAYERS_TIMER_ID 0
#define UPDATE_PLAYERS_TIMER_TIME 30000

UIScene_JoinMenu::UIScene_JoinMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	JoinMenuInitData *initData = static_cast<JoinMenuInitData *>(_initData);
	m_selectedSession = initData->selectedSession;
	m_friendInfoUpdatedOK = false;
	m_friendInfoUpdatedERROR = false;
	m_friendInfoRequestIssued = false;
#ifdef _WINDOWS64
	m_serverIndex = initData->serverIndex;
	m_editServerPhase = eEditServer_Idle;
	m_editServerButtonIndex = -1;
	m_deleteServerButtonIndex = -1;
#endif
}

void UIScene_JoinMenu::updateTooltips()
{
	int iA = -1;
	int iY = -1;
	int iX = -1;
	if (getControlFocus() == eControl_GamePlayers)
	{
#ifdef _DURANGO
		iY = IDS_TOOLTIPS_VIEW_GAMERCARD;
#endif
	}
	else
	{
		iA = IDS_TOOLTIPS_SELECT;
	}

#ifdef _WINDOWS64
	if (m_serverIndex >= 0)
	{
		iX = IDS_TOOLTIPS_DELETE;
		iY = IDS_TITLE_RENAME;
	}
#endif

	ui.SetTooltips( DEFAULT_XUI_MENU_USER, iA, IDS_TOOLTIPS_BACK, iX, iY );

}

void UIScene_JoinMenu::tick()
{
	if( !m_friendInfoRequestIssued )
	{
		ui.NavigateToScene(m_iPad, eUIScene_Timer);
		g_NetworkManager.GetFullFriendSessionInfo(m_selectedSession, &friendSessionUpdated, this);
		m_friendInfoRequestIssued = true;
	}

	if( m_friendInfoUpdatedOK )
	{
		m_friendInfoUpdatedOK = false;

		m_buttonJoinGame.init(app.GetString(IDS_JOIN_GAME),eControl_JoinGame);

		m_buttonListPlayers.init(eControl_GamePlayers);

#if defined(__PS3__) || defined(__ORBIS__) || defined __PSVITA__
		for( int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; i++ )
		{
			if( m_selectedSession->data.players[i] != nullptr )
			{
	#ifndef _CONTENT_PACKAGE
				if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
				{
					m_buttonListPlayers.addItem(L"WWWWWWWWWWWWWWWW");
				}
				else
	#endif
				{
					string playerName(m_selectedSession->data.players[i].getOnlineID());

	#ifndef __PSVITA__
					// Append guest number (any players in an online game not signed into PSN are guests)
					if( m_selectedSession->data.players[i].isSignedIntoPSN() == false )
					{
						char suffix[5];
						sprintf(suffix, " (%d)", m_selectedSession->data.players[i].getQuadrant() + 1);
						playerName.append(suffix);
					}
	#endif
					m_buttonListPlayers.addItem(playerName);
				}
			}
			else
			{
				// Leave the loop when we hit the first nullptr player
				break;
			}
		}
#elif defined(_DURANGO)
		for( int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; i++ )
		{
			if ( m_selectedSession->searchResult.m_playerNames[i].size() )
			{
				m_buttonListPlayers.addItem(m_selectedSession->searchResult.m_playerNames[i]);
			}
			else
			{
				// Leave the loop when we hit the first empty player name
				break;
			}
		}
#endif

#ifdef _WINDOWS64
		if (m_serverIndex >= 0)
		{
			m_editServerButtonIndex = m_buttonListPlayers.getItemCount();
			m_buttonListPlayers.addItem(L"Edit Server");
			m_deleteServerButtonIndex = m_buttonListPlayers.getItemCount();
			m_buttonListPlayers.addItem(L"Delete Server");
		}
#endif

		m_labelLabels[eLabel_Difficulty].init(app.GetString(IDS_LABEL_DIFFICULTY));
		m_labelLabels[eLabel_GameType].init(app.GetString(IDS_LABEL_GAME_TYPE));
		m_labelLabels[eLabel_GamertagsOn].init(app.GetString(IDS_LABEL_GAMERTAGS));
		m_labelLabels[eLabel_Structures].init(app.GetString(IDS_LABEL_STRUCTURES));
		m_labelLabels[eLabel_LevelType].init(app.GetString(IDS_LABEL_LEVEL_TYPE));
		m_labelLabels[eLabel_PVP].init(app.GetString(IDS_LABEL_PvP));
		m_labelLabels[eLabel_Trust].init(app.GetString(IDS_LABEL_TRUST));
		m_labelLabels[eLabel_TNTOn].init(app.GetString(IDS_LABEL_TNT));
		m_labelLabels[eLabel_FireOn].init(app.GetString(IDS_LABEL_FIRE_SPREADS));

		unsigned int uiGameHostSettings = m_selectedSession->data.m_uiGameHostSettings;
		switch(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_Difficulty))
		{
		case Difficulty::EASY:
			m_labelValues[eLabel_Difficulty].init( app.GetString(IDS_DIFFICULTY_TITLE_EASY) );
			break;
		case Difficulty::NORMAL:
			m_labelValues[eLabel_Difficulty].init( app.GetString(IDS_DIFFICULTY_TITLE_NORMAL) );
			break;
		case Difficulty::HARD:
			m_labelValues[eLabel_Difficulty].init( app.GetString(IDS_DIFFICULTY_TITLE_HARD) );
			break;
		case Difficulty::PEACEFUL:
		default:
			m_labelValues[eLabel_Difficulty].init( app.GetString(IDS_DIFFICULTY_TITLE_PEACEFUL) );
			break;
		}

		int option = app.GetGameHostOption(uiGameHostSettings,eGameHostOption_GameType);
		if(option == GameType::CREATIVE->getId())
		{
			m_labelValues[eLabel_GameType].init( app.GetString(IDS_CREATIVE) );
		}
		else if(option == GameType::ADVENTURE->getId())
		{
			m_labelValues[eLabel_GameType].init( app.GetString(IDS_ADVENTURE) );
		}
		else
		{
			m_labelValues[eLabel_GameType].init( app.GetString(IDS_SURVIVAL) );
		}

		if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_Gamertags))	m_labelValues[eLabel_GamertagsOn].init( app.GetString(IDS_ON) );
		else m_labelValues[eLabel_GamertagsOn].init( app.GetString(IDS_OFF) );

		if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_Structures)) m_labelValues[eLabel_Structures].init( app.GetString(IDS_ON) );
		else m_labelValues[eLabel_Structures].init( app.GetString(IDS_OFF) );

		if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_LevelType)) m_labelValues[eLabel_LevelType].init( app.GetString(IDS_LEVELTYPE_SUPERFLAT) );
		else m_labelValues[eLabel_LevelType].init( app.GetString(IDS_LEVELTYPE_NORMAL) );

		if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_PvP))m_labelValues[eLabel_PVP].init( app.GetString(IDS_ON) );
		else m_labelValues[eLabel_PVP].init( app.GetString(IDS_OFF) );

		if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_TrustPlayers)) m_labelValues[eLabel_Trust].init( app.GetString(IDS_ON) );
		else m_labelValues[eLabel_Trust].init( app.GetString(IDS_OFF) );

		if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_TNT)) m_labelValues[eLabel_TNTOn].init( app.GetString(IDS_ON) );
		else m_labelValues[eLabel_TNTOn].init( app.GetString(IDS_OFF) );

		if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_FireSpreads)) m_labelValues[eLabel_FireOn].init( app.GetString(IDS_ON) );
		else m_labelValues[eLabel_FireOn].init( app.GetString(IDS_OFF) );

		m_bIgnoreInput = false;

		// Alert the app the we want to be informed of ethernet connections
		app.SetLiveLinkRequired( true );

		TelemetryManager->RecordMenuShown(m_iPad, eUIScene_JoinMenu, 0);

		addTimer(UPDATE_PLAYERS_TIMER_ID,UPDATE_PLAYERS_TIMER_TIME);
	}

	if( m_friendInfoUpdatedERROR )
	{
		m_buttonJoinGame.init(app.GetString(IDS_JOIN_GAME),eControl_JoinGame);

		m_buttonListPlayers.init(eControl_GamePlayers);

		m_labelLabels[eLabel_Difficulty].init(app.GetString(IDS_LABEL_DIFFICULTY));
		m_labelLabels[eLabel_GameType].init(app.GetString(IDS_LABEL_GAME_TYPE));
		m_labelLabels[eLabel_GamertagsOn].init(app.GetString(IDS_LABEL_GAMERTAGS));
		m_labelLabels[eLabel_Structures].init(app.GetString(IDS_LABEL_STRUCTURES));
		m_labelLabels[eLabel_LevelType].init(app.GetString(IDS_LABEL_LEVEL_TYPE));
		m_labelLabels[eLabel_PVP].init(app.GetString(IDS_LABEL_PvP));
		m_labelLabels[eLabel_Trust].init(app.GetString(IDS_LABEL_TRUST));
		m_labelLabels[eLabel_TNTOn].init(app.GetString(IDS_LABEL_TNT));
		m_labelLabels[eLabel_FireOn].init(app.GetString(IDS_LABEL_FIRE_SPREADS));

		m_labelValues[eLabel_Difficulty].init(app.GetString(IDS_DIFFICULTY_TITLE_PEACEFUL));
		m_labelValues[eLabel_GameType].init( app.GetString(IDS_CREATIVE) );
		m_labelValues[eLabel_GamertagsOn].init( app.GetString(IDS_OFF) );
		m_labelValues[eLabel_Structures].init( app.GetString(IDS_OFF) );
		m_labelValues[eLabel_LevelType].init( app.GetString(IDS_LEVELTYPE_NORMAL) );
		m_labelValues[eLabel_PVP].init( app.GetString(IDS_OFF) );
		m_labelValues[eLabel_Trust].init( app.GetString(IDS_OFF) );
		m_labelValues[eLabel_TNTOn].init( app.GetString(IDS_OFF) );
		m_labelValues[eLabel_FireOn].init( app.GetString(IDS_OFF) );

		m_friendInfoUpdatedERROR = false;

		// Show a generic network error message, not always safe to assume the error was host quitting
		// without bubbling more info up from the network manager so this is the best we can do
		UINT uiIDA[1];
		uiIDA[0] = IDS_CONFIRM_OK;
#ifdef _XBOX_ONE
		ui.RequestErrorMessage( IDS_CONNECTION_FAILED, IDS_DISCONNECTED_SERVER_QUIT, uiIDA,1,m_iPad,ErrorDialogReturned,this);
#else
		ui.RequestErrorMessage( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA,1,m_iPad,ErrorDialogReturned,this);
#endif
	}

	UIScene::tick();
}

void UIScene_JoinMenu::friendSessionUpdated(bool success, void *pParam)
{
	UIScene_JoinMenu *scene = static_cast<UIScene_JoinMenu *>(pParam);
	ui.NavigateBack(scene->m_iPad);
	if( success )
	{
		scene->m_friendInfoUpdatedOK = true;
	}
	else
	{
		scene->m_friendInfoUpdatedERROR = true;
	}
}

int UIScene_JoinMenu::ErrorDialogReturned(void *pParam, int iPad, const C4JStorage::EMessageResult)
{
	UIScene_JoinMenu *scene = static_cast<UIScene_JoinMenu *>(pParam);
	ui.NavigateBack(scene->m_iPad);

	return 0;
}

void UIScene_JoinMenu::updateComponents()
{
	m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,true);
	m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
}

wstring UIScene_JoinMenu::getMoviePath()
{
	return L"JoinMenu";
}

void UIScene_JoinMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bIgnoreInput) return;

	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			navigateBack();
			handled = true;
		}
		break;
#ifdef _DURANGO
	case ACTION_MENU_Y:
		if(m_selectedSession != nullptr && getControlFocus() == eControl_GamePlayers && m_buttonListPlayers.getItemCount() > 0)
		{
			PlayerUID uid = m_selectedSession->searchResult.m_playerXuids[m_buttonListPlayers.getCurrentSelection()];
			if( uid != INVALID_XUID ) ProfileManager.ShowProfileCard(ProfileManager.GetLockedProfile(),uid);
		}
		break;
#endif
#ifdef _WINDOWS64
	case ACTION_MENU_X:
		if(pressed && m_serverIndex >= 0)
		{
			BeginDeleteServer();
			handled = true;
		}
		break;
	case ACTION_MENU_Y:
		if(pressed && m_serverIndex >= 0)
		{
			BeginEditServer();
			handled = true;
		}
		break;
#endif
	case ACTION_MENU_OK:
		if (getControlFocus() != eControl_GamePlayers)
		{
			sendInputToMovie(key, repeat, pressed, released);
		}
#ifdef _WINDOWS64
		else if (pressed && m_serverIndex >= 0)
		{
			int sel = m_buttonListPlayers.getCurrentSelection();
			if (sel == m_editServerButtonIndex)
				BeginEditServer();
			else if (sel == m_deleteServerButtonIndex)
				BeginDeleteServer();
		}
#endif
		handled = true;
		break;
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_PAGEUP:
	case ACTION_MENU_PAGEDOWN:
		sendInputToMovie(key, repeat, pressed, released);
		handled = true;
		break;
	}
}

void UIScene_JoinMenu::handlePress(F64 controlId, F64 childId)
{
	switch(static_cast<int>(controlId))
	{
	case eControl_JoinGame:
		{
			m_bIgnoreInput = true;

			//CD - Added for audio
			ui.PlayUISFX(eSFX_Press);

#ifdef _DURANGO
			ProfileManager.CheckMultiplayerPrivileges(m_iPad, true, &checkPrivilegeCallback, (LPVOID)GetCallbackUniqueId());
#else
			StartSharedLaunchFlow();
#endif
		}
		break;
	case eControl_GamePlayers:
#ifdef _WINDOWS64
		if (m_serverIndex >= 0)
		{
			int sel = (int)childId;
			if (sel == m_editServerButtonIndex)
				BeginEditServer();
			else if (sel == m_deleteServerButtonIndex)
				BeginDeleteServer();
		}
#endif
		break;
	};
}

void UIScene_JoinMenu::handleFocusChange(F64 controlId, F64 childId)
{
	switch(static_cast<int>(controlId))
	{
	case eControl_GamePlayers:
		m_buttonListPlayers.updateChildFocus( static_cast<int>(childId) );
	};
	updateTooltips();
}

#ifdef _DURANGO
void UIScene_JoinMenu::checkPrivilegeCallback(LPVOID lpParam, bool hasPrivilege, int iPad)
{
	UIScene_JoinMenu* pClass = (UIScene_JoinMenu*)ui.GetSceneFromCallbackId((size_t)lpParam);

	if(pClass)
	{
		if(hasPrivilege)
		{
			pClass->StartSharedLaunchFlow();
		}
		else
		{
			pClass->m_bIgnoreInput = false;
		}
	}
}
#endif

void UIScene_JoinMenu::StartSharedLaunchFlow()
{
	if(!app.IsLocalMultiplayerAvailable())
	{
		JoinGame(this);
	}
	else
	{
		//ProfileManager.RequestSignInUI(false, false, false, true, false,&UIScene_JoinMenu::StartGame_SignInReturned, this,ProfileManager.GetPrimaryPad());
		SignInInfo info;
		info.Func = &UIScene_JoinMenu::StartGame_SignInReturned;
		info.lpParam = (LPVOID)GetCallbackUniqueId();
		info.requireOnline = true;
		ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_QuadrantSignin,&info);
	}
}

int UIScene_JoinMenu::StartGame_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	UIScene_JoinMenu* pClass = static_cast<UIScene_JoinMenu *>(ui.GetSceneFromCallbackId((size_t)pParam));

	if(pClass)
	{
		if(bContinue==true)
		{
			// It's possible that the player has not signed in - they can back out
			if(ProfileManager.IsSignedIn(iPad))
			{
				JoinGame(pClass);
			}
			else
			{
				pClass->m_bIgnoreInput=false;
			}
		}
		else
		{
			pClass->m_bIgnoreInput=false;
		}
	}
	return 0;
}

// Shared function to join the game that is the same whether we used the sign-in UI or not
void UIScene_JoinMenu::JoinGame(UIScene_JoinMenu* pClass)
{
	DWORD dwSignedInUsers = 0;
	bool noPrivileges = false;
	DWORD dwLocalUsersMask = 0;
	bool isSignedInLive = true;
	int iPadNotSignedInLive = -1;

	ProfileManager.SetLockedProfile(0);		// TEMP!

	// If we're in SD mode, then only the primary player gets to play
	if (app.IsLocalMultiplayerAvailable())
	{				
		for(unsigned int index = 0; index < XUSER_MAX_COUNT; ++index)
		{
			if(ProfileManager.IsSignedIn(index))
			{
				if (isSignedInLive && !ProfileManager.IsSignedInLive(index))
				{
					// Record the first non signed in live pad
					iPadNotSignedInLive = index;
				}

				if( !ProfileManager.AllowedToPlayMultiplayer(index) ) noPrivileges = true;
				dwLocalUsersMask |= CGameNetworkManager::GetLocalPlayerMask(index);
				isSignedInLive = isSignedInLive && ProfileManager.IsSignedInLive(index);
			}
		}
	}
	else
	{
		if(ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad()))
		{
			if( !ProfileManager.AllowedToPlayMultiplayer(ProfileManager.GetPrimaryPad()) ) noPrivileges = true;
			dwLocalUsersMask |= CGameNetworkManager::GetLocalPlayerMask(ProfileManager.GetPrimaryPad());

			isSignedInLive = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad());
#ifdef __PSVITA__
			if(CGameNetworkManager::usingAdhocMode() && SQRNetworkManager_AdHoc_Vita::GetAdhocStatus())
				isSignedInLive = true;
#endif

		}
	}

	// If this is an online game but not all players are signed in to Live, stop!
	if (!isSignedInLive)
	{
#ifdef __ORBIS__
		// Check if PSN is unavailable because of age restriction
		int npAvailability = ProfileManager.getNPAvailability(iPadNotSignedInLive);
		if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
		{
			pClass->m_bIgnoreInput = false;
			// 4J Stu - This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive if the npAvailability isn't SCE_OK
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;
			ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPadNotSignedInLive);
		}
		else
#endif
		{
			pClass->m_bIgnoreInput=false;
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
			ui.RequestErrorMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA,1,ProfileManager.GetPrimaryPad());
		}
		return;
	}

	// Check if user-created content is allowed, as we cannot play multiplayer if it's not
	bool noUGC = false;
	BOOL pccAllowed = TRUE;
	BOOL pccFriendsAllowed = TRUE;

#if defined(__PS3__) || defined(__PSVITA__)
	if(isSignedInLive)
	{
		ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),false,&noUGC,nullptr,nullptr);
	}
#else
	ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
	if(!pccAllowed && !pccFriendsAllowed) noUGC = true;
#endif


#ifdef __PSVITA__
	if( CGameNetworkManager::usingAdhocMode() )
	{
		noPrivileges = false;
		noUGC = false;
	}
#endif

	if(noUGC)
	{
		pClass->setVisible( true );
		pClass->m_bIgnoreInput=false;

		int messageText = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
		if(dwSignedInUsers > 1) messageText = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;

		ui.RequestUGCMessageBox(IDS_CONNECTION_FAILED, messageText);
	}
	else if(noPrivileges)
	{
 		pClass->setVisible( true );
		pClass->m_bIgnoreInput=false;
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		ui.RequestErrorMessage( IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT, uiIDA,1,ProfileManager.GetPrimaryPad());
	}
	else
	{
#if defined(__ORBIS__) || defined(__PSVITA__)
		bool chatRestricted = false;
		ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),false,&chatRestricted,nullptr,nullptr);
		if(chatRestricted)
		{
			ProfileManager.DisplaySystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_PSN_CHAT_RESTRICTION, ProfileManager.GetPrimaryPad() );
		}
#endif
		CGameNetworkManager::eJoinGameResult result = g_NetworkManager.JoinGame( pClass->m_selectedSession, dwLocalUsersMask );

		// Alert the app the we no longer want to be informed of ethernet connections
		app.SetLiveLinkRequired( false );

		if( result != CGameNetworkManager::JOINGAME_SUCCESS )
		{
			int exitReasonStringId = -1;
			switch(result)
			{
			case CGameNetworkManager::JOINGAME_FAIL_SERVER_FULL:
				exitReasonStringId = IDS_DISCONNECTED_SERVER_FULL;
				break;
			}

			if( exitReasonStringId == -1 )
			{
				Minecraft* pMinecraft = Minecraft::GetInstance();
				int primaryPad = ProfileManager.GetPrimaryPad();
				if( pMinecraft->m_connectionFailed[primaryPad] )
				{
					switch( pMinecraft->m_connectionFailedReason[primaryPad] )
					{
					case DisconnectPacket::eDisconnect_LoginTooLong:
						exitReasonStringId = IDS_DISCONNECTED_LOGIN_TOO_LONG;
						break;
					case DisconnectPacket::eDisconnect_ServerFull:
						exitReasonStringId = IDS_DISCONNECTED_SERVER_FULL;
						break;
					case DisconnectPacket::eDisconnect_Kicked:
						exitReasonStringId = IDS_DISCONNECTED_KICKED;
						break;
					case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
						exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
						break;
					case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
						exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
						break;
					case DisconnectPacket::eDisconnect_NoFlying:
						exitReasonStringId = IDS_DISCONNECTED_FLYING;
						break;
					case DisconnectPacket::eDisconnect_Quitting:
						exitReasonStringId = IDS_DISCONNECTED_SERVER_QUIT;
						break;
					case DisconnectPacket::eDisconnect_OutdatedServer:
						exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
						break;
					case DisconnectPacket::eDisconnect_OutdatedClient:
						exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
						break;
					default:
						exitReasonStringId = IDS_CONNECTION_LOST_SERVER;
						break;
					}
				}
			}

			if( exitReasonStringId == -1 )
			{
				ui.NavigateBack(pClass->m_iPad);
			}
			else
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestErrorMessage( IDS_CONNECTION_FAILED, exitReasonStringId, uiIDA,1,ProfileManager.GetPrimaryPad());
				exitReasonStringId = -1;

				ui.NavigateToHomeMenu();
			}
		}
	}
}

void UIScene_JoinMenu::handleTimerComplete(int id)
{
	switch(id)
	{
	case UPDATE_PLAYERS_TIMER_ID:
		{
#if TO_BE_IMPLEMENTED
			PlayerUID selectedPlayerXUID = m_selectedSession->data.players[playersList.GetCurSel()];

			bool success = g_NetworkManager.GetGameSessionInfo(m_iPad, m_selectedSession->sessionId,m_selectedSession);

			if( success )
			{
				playersList.DeleteItems(0, playersList.GetItemCount());
				int selectedIndex = 0;
				for(unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
				{
					if( m_selectedSession->data.players[i] != nullptr )
					{
						if(m_selectedSession->data.players[i] == selectedPlayerXUID) selectedIndex = i;
						playersList.InsertItems(i,1);
#ifndef _CONTENT_PACKAGE
						if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
						{
							playersList.SetText(i, L"WWWWWWWWWWWWWWWW" );
						}
						else
#endif
						{
							playersList.SetText(i, convStringToWstring( m_selectedSession->data.szPlayers[i] ).c_str() );
						}
					}
					else
					{
						// Leave the loop when we hit the first nullptr player
						break;
					}
				}
				playersList.SetCurSel(selectedIndex);
			}
#endif
		}
		break;
	};
}

#ifdef _WINDOWS64
void UIScene_JoinMenu::BeginDeleteServer()
{
	m_bIgnoreInput = true;
	UINT uiIDA[2];
	uiIDA[0] = IDS_CONFIRM_CANCEL;
	uiIDA[1] = IDS_CONFIRM_OK;
	ui.RequestAlertMessage(IDS_TOOLTIPS_DELETE, IDS_TEXT_DELETE_SAVE, uiIDA, 2, m_iPad, &UIScene_JoinMenu::DeleteServerDialogReturned, this);
}

int UIScene_JoinMenu::DeleteServerDialogReturned(void *pParam, int iPad, C4JStorage::EMessageResult result)
{
	UIScene_JoinMenu* pClass = (UIScene_JoinMenu*)pParam;

	if (result == C4JStorage::EMessage_ResultDecline)
	{
		pClass->RemoveServerFromFile();
		g_NetworkManager.ForceFriendsSessionRefresh();
		pClass->navigateBack();
	}
	else
	{
		pClass->m_bIgnoreInput = false;
	}

	return 0;
}

void UIScene_JoinMenu::BeginEditServer()
{
	m_bIgnoreInput = true;
	m_editServerPhase = eEditServer_IP;
	m_editServerIP.clear();
	m_editServerPort.clear();

	wchar_t wDefaultIP[64] = {};
	mbstowcs(wDefaultIP, m_selectedSession->data.hostIP, 63);

	UIKeyboardInitData kbData;
	kbData.title       = L"Server Address";
	kbData.defaultText = wDefaultIP;
	kbData.maxChars    = 128;
	kbData.callback    = &UIScene_JoinMenu::EditServerKeyboardCallback;
	kbData.lpParam     = this;
	kbData.pcMode      = g_KBMInput.IsKBMActive();
	ui.NavigateToScene(m_iPad, eUIScene_Keyboard, &kbData);
}

int UIScene_JoinMenu::EditServerKeyboardCallback(LPVOID lpParam, bool bRes)
{
	UIScene_JoinMenu *pClass = (UIScene_JoinMenu *)lpParam;

	if (!bRes)
	{
		pClass->m_editServerPhase = eEditServer_Idle;
		pClass->m_bIgnoreInput = false;
		return 0;
	}

	uint16_t ui16Text[256];
	ZeroMemory(ui16Text, sizeof(ui16Text));
	Win64_GetKeyboardText(ui16Text, 256);

	wchar_t wBuf[256] = {};
	for (int k = 0; k < 255 && ui16Text[k]; k++)
		wBuf[k] = (wchar_t)ui16Text[k];

	if (wBuf[0] == 0)
	{
		pClass->m_editServerPhase = eEditServer_Idle;
		pClass->m_bIgnoreInput = false;
		return 0;
	}

	switch (pClass->m_editServerPhase)
	{
	case eEditServer_IP:
	{
		pClass->m_editServerIP = wBuf;
		pClass->m_editServerPhase = eEditServer_Port;

		wchar_t wDefaultPort[16] = {};
		swprintf(wDefaultPort, 16, L"%d", pClass->m_selectedSession->data.hostPort);

		UIKeyboardInitData kbData;
		kbData.title       = L"Server Port";
		kbData.defaultText = wDefaultPort;
		kbData.maxChars    = 6;
		kbData.callback    = &UIScene_JoinMenu::EditServerKeyboardCallback;
		kbData.lpParam     = pClass;
		kbData.pcMode      = g_KBMInput.IsKBMActive();
		ui.NavigateToScene(pClass->m_iPad, eUIScene_Keyboard, &kbData);
		break;
	}
	case eEditServer_Port:
	{
		pClass->m_editServerPort = wBuf;
		pClass->m_editServerPhase = eEditServer_Name;

		wchar_t wDefaultName[64] = {};
		if (pClass->m_selectedSession->displayLabel)
			wcsncpy(wDefaultName, pClass->m_selectedSession->displayLabel, 63);

		UIKeyboardInitData kbData;
		kbData.title       = L"Server Name";
		kbData.defaultText = wDefaultName;
		kbData.maxChars    = 64;
		kbData.callback    = &UIScene_JoinMenu::EditServerKeyboardCallback;
		kbData.lpParam     = pClass;
		kbData.pcMode      = g_KBMInput.IsKBMActive();
		ui.NavigateToScene(pClass->m_iPad, eUIScene_Keyboard, &kbData);
		break;
	}
	case eEditServer_Name:
	{
		wstring newName = wBuf;
		pClass->UpdateServerInFile(pClass->m_editServerIP, pClass->m_editServerPort, newName);
		pClass->m_editServerPhase = eEditServer_Idle;
		pClass->m_bIgnoreInput = false;

		g_NetworkManager.ForceFriendsSessionRefresh();
		pClass->navigateBack();
		break;
	}
	default:
		pClass->m_editServerPhase = eEditServer_Idle;
		pClass->m_bIgnoreInput = false;
		break;
	}

	return 0;
}

void UIScene_JoinMenu::UpdateServerInFile(const wstring& newIP, const wstring& newPort, const wstring& newName)
{
	char narrowNewIP[256] = {};
	char narrowNewPort[16] = {};
	char narrowNewName[256] = {};
	wcstombs(narrowNewIP, newIP.c_str(), sizeof(narrowNewIP) - 1);
	wcstombs(narrowNewPort, newPort.c_str(), sizeof(narrowNewPort) - 1);
	wcstombs(narrowNewName, newName.c_str(), sizeof(narrowNewName) - 1);

	uint16_t newPortNum = (uint16_t)atoi(narrowNewPort);

	struct ServerEntry { std::string ip; uint16_t port; std::string name; };
	std::vector<ServerEntry> entries;

	FILE* file = fopen("servers.db", "rb");
	if (file)
	{
		char magic[4] = {};
		if (fread(magic, 1, 4, file) == 4 && memcmp(magic, "MCSV", 4) == 0)
		{
			uint32_t version = 0, count = 0;
			fread(&version, sizeof(uint32_t), 1, file);
			fread(&count, sizeof(uint32_t), 1, file);
			if (version == 1)
			{
				for (uint32_t s = 0; s < count; s++)
				{
					uint16_t ipLen = 0, p = 0, nameLen = 0;
					if (fread(&ipLen, sizeof(uint16_t), 1, file) != 1) break;
					if (ipLen == 0 || ipLen > 256) break;
					char ipBuf[257] = {};
					if (fread(ipBuf, 1, ipLen, file) != ipLen) break;
					if (fread(&p, sizeof(uint16_t), 1, file) != 1) break;
					if (fread(&nameLen, sizeof(uint16_t), 1, file) != 1) break;
					if (nameLen > 256) break;
					char nameBuf[257] = {};
					if (nameLen > 0 && fread(nameBuf, 1, nameLen, file) != nameLen) break;
					entries.push_back({std::string(ipBuf), p, std::string(nameBuf)});
				}
			}
		}
		fclose(file);
	}

	// Find and update the matching entry by original IP and port
	int idx = m_serverIndex;
	if (idx >= 0 && idx < (int)entries.size())
	{
		entries[idx].ip = std::string(narrowNewIP);
		entries[idx].port = newPortNum;
		entries[idx].name = std::string(narrowNewName);
	}

	file = fopen("servers.db", "wb");
	if (file)
	{
		fwrite("MCSV", 1, 4, file);
		uint32_t version = 1;
		uint32_t count = (uint32_t)entries.size();
		fwrite(&version, sizeof(uint32_t), 1, file);
		fwrite(&count, sizeof(uint32_t), 1, file);

		for (size_t i = 0; i < entries.size(); i++)
		{
			uint16_t ipLen = (uint16_t)entries[i].ip.length();
			fwrite(&ipLen, sizeof(uint16_t), 1, file);
			fwrite(entries[i].ip.c_str(), 1, ipLen, file);
			fwrite(&entries[i].port, sizeof(uint16_t), 1, file);
			uint16_t nameLen = (uint16_t)entries[i].name.length();
			fwrite(&nameLen, sizeof(uint16_t), 1, file);
			fwrite(entries[i].name.c_str(), 1, nameLen, file);
		}
		fclose(file);
	}
}

void UIScene_JoinMenu::RemoveServerFromFile()
{
	struct ServerEntry { std::string ip; uint16_t port; std::string name; };
	std::vector<ServerEntry> entries;

	FILE* file = fopen("servers.db", "rb");
	if (file)
	{
		char magic[4] = {};
		if (fread(magic, 1, 4, file) == 4 && memcmp(magic, "MCSV", 4) == 0)
		{
			uint32_t version = 0, count = 0;
			fread(&version, sizeof(uint32_t), 1, file);
			fread(&count, sizeof(uint32_t), 1, file);
			if (version == 1)
			{
				for (uint32_t s = 0; s < count; s++)
				{
					uint16_t ipLen = 0, p = 0, nameLen = 0;
					if (fread(&ipLen, sizeof(uint16_t), 1, file) != 1) break;
					if (ipLen == 0 || ipLen > 256) break;
					char ipBuf[257] = {};
					if (fread(ipBuf, 1, ipLen, file) != ipLen) break;
					if (fread(&p, sizeof(uint16_t), 1, file) != 1) break;
					if (fread(&nameLen, sizeof(uint16_t), 1, file) != 1) break;
					if (nameLen > 256) break;
					char nameBuf[257] = {};
					if (nameLen > 0 && fread(nameBuf, 1, nameLen, file) != nameLen) break;
					entries.push_back({std::string(ipBuf), p, std::string(nameBuf)});
				}
			}
		}
		fclose(file);
	}

	// Remove the entry at m_serverIndex
	int idx = m_serverIndex;
	if (idx >= 0 && idx < (int)entries.size())
	{
		entries.erase(entries.begin() + idx);
	}

	file = fopen("servers.db", "wb");
	if (file)
	{
		fwrite("MCSV", 1, 4, file);
		uint32_t version = 1;
		uint32_t count = (uint32_t)entries.size();
		fwrite(&version, sizeof(uint32_t), 1, file);
		fwrite(&count, sizeof(uint32_t), 1, file);

		for (size_t i = 0; i < entries.size(); i++)
		{
			uint16_t ipLen = (uint16_t)entries[i].ip.length();
			fwrite(&ipLen, sizeof(uint16_t), 1, file);
			fwrite(entries[i].ip.c_str(), 1, ipLen, file);
			fwrite(&entries[i].port, sizeof(uint16_t), 1, file);
			uint16_t nameLen = (uint16_t)entries[i].name.length();
			fwrite(&nameLen, sizeof(uint16_t), 1, file);
			fwrite(entries[i].name.c_str(), 1, nameLen, file);
		}
		fclose(file);
	}
}
#endif // _WINDOWS64