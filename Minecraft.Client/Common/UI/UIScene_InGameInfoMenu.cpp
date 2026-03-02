#include "stdafx.h"
#include "UI.h"
#include "UIScene_InGameInfoMenu.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\ClientConnection.h"

UIScene_InGameInfoMenu::UIScene_InGameInfoMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_buttonGameOptions.init(app.GetString(IDS_HOST_OPTIONS),eControl_GameOptions);
	m_labelTitle.init(app.GetString(IDS_PLAYERS_INVITE));
	m_playerList.init(eControl_GamePlayers);

	for(unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
	{
		m_playerNames[i] = L"";
	}

	DWORD playerCount = g_NetworkManager.GetPlayerCount();

	m_playersCount = 0;
	for(DWORD i = 0; i < playerCount; ++i)
	{
		INetworkPlayer *player = g_NetworkManager.GetPlayerByIndex( i );

		if( player != NULL )
		{
			m_players[i] = player->GetSmallId();
			++m_playersCount;

			wstring playerName = L"";
#ifndef _CONTENT_PACKAGE
			if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
			{
				playerName =  L"WWWWWWWWWWWWWWWW";
			}
			else
#endif
			{
				playerName = player->GetDisplayName();
			}

			int voiceStatus = 0;
			if(player != NULL && player->HasVoice() )
			{
				if( player->IsMutedByLocalUser(m_iPad) )
				{
					// Muted image
					voiceStatus = 3;
				}
				else if( player->IsTalking() )
				{
					// Talking image
					voiceStatus = 2;
				}
				else
				{
					// Not talking image
					voiceStatus = 1;
				}
			}

			m_playersVoiceState[i] = voiceStatus;
			m_playersColourState[i] = app.GetPlayerColour( m_players[i] );
			m_playerNames[i] = playerName;
			m_playerList.addItem( playerName, app.GetPlayerColour( m_players[i] ), voiceStatus); 
		}
	}

	g_NetworkManager.RegisterPlayerChangedCallback(m_iPad, &UIScene_InGameInfoMenu::OnPlayerChanged, this);

	INetworkPlayer *thisPlayer = g_NetworkManager.GetLocalPlayerByUserIndex( m_iPad );
	m_isHostPlayer = false;
	if(thisPlayer != NULL) m_isHostPlayer = thisPlayer->IsHost() == TRUE;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[m_iPad];
	if(!m_isHostPlayer && !localPlayer->isModerator() )
	{
		removeControl( &m_buttonGameOptions, false );
	}

	updateTooltips();

#if TO_BE_IMPLEMENTED
	SetTimer( TOOLTIP_TIMERID , INGAME_INFO_TOOLTIP_TIMER );
#endif

	// get rid of the quadrant display if it's on
	ui.HidePressStart();

#if TO_BE_IMPLEMENTED
	SetTimer(IGNORE_KEYPRESS_TIMERID,IGNORE_KEYPRESS_TIME);
#endif
}

wstring UIScene_InGameInfoMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"InGameInfoMenuSplit";
	}
	else
	{
		return L"InGameInfoMenu";
	}
}

void UIScene_InGameInfoMenu::updateTooltips()
{
	int keyX = IDS_TOOLTIPS_INVITE_FRIENDS;
	int ikeyY = -1;

	XPARTY_USER_LIST partyList;
	if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY ) && (partyList.dwUserCount>1))
	{
		keyX = IDS_TOOLTIPS_INVITE_PARTY;
	}

	if(g_NetworkManager.IsLocalGame()) keyX = -1;
#ifdef __PSVITA__
	if(CGameNetworkManager::usingAdhocMode()) keyX = -1;
#endif


	INetworkPlayer *selectedPlayer = g_NetworkManager.GetPlayerBySmallId( m_players[ m_playerList.getCurrentSelection() ] );

	int keyA = -1;
	Minecraft *pMinecraft = Minecraft::GetInstance();
	shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[m_iPad];

	bool isOp = m_isHostPlayer || localPlayer->isModerator();
	bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
	bool trust = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;

	if( isOp )
	{
		if(m_buttonGameOptions.hasFocus())
		{
			keyA = IDS_TOOLTIPS_SELECT;
		}
		else if( selectedPlayer != NULL)
		{
			bool editingHost = selectedPlayer->IsHost();
			if( (cheats && (m_isHostPlayer || !editingHost ) ) || (!trust && (m_isHostPlayer || !editingHost))
#if (!defined(_CONTENT_PACKAGE) && !defined(_FINAL_BUILD) && defined(_DEBUG_MENUS_ENABLED))
				|| (m_isHostPlayer && editingHost)
#endif
				)
			{
				keyA = IDS_TOOLTIPS_PRIVILEGES;
			}
			else if(selectedPlayer->IsLocal() != TRUE && selectedPlayer->IsSameSystem(g_NetworkManager.GetHostPlayer()) != TRUE)
			{
				// Only ops will hit this, can kick anyone not local and not local to the host
				keyA = IDS_TOOLTIPS_KICK;
			}
		}
	}
	
#if defined(__PS3__) || defined(__ORBIS__)
	if(m_iPad == ProfileManager.GetPrimaryPad() ) ikeyY = IDS_TOOLTIPS_GAME_INVITES;
#else
	if(!m_buttonGameOptions.hasFocus())
	{
		// if the player is me, then view gamer profile
		if(selectedPlayer != NULL && selectedPlayer->IsLocal() && selectedPlayer->GetUserIndex()==m_iPad)
		{
			ikeyY = IDS_TOOLTIPS_VIEW_GAMERPROFILE;
		}
		else
		{
			ikeyY = IDS_TOOLTIPS_VIEW_GAMERCARD;
		}
	}
#endif
	ui.SetTooltips( m_iPad, keyA,IDS_TOOLTIPS_BACK,keyX,ikeyY);
}

void UIScene_InGameInfoMenu::handleDestroy()
{
	g_NetworkManager.UnRegisterPlayerChangedCallback(m_iPad, &UIScene_InGameInfoMenu::OnPlayerChanged, this);

	m_parentLayer->removeComponent(eUIComponent_MenuBackground);
}

void UIScene_InGameInfoMenu::handleGainFocus(bool navBack)
{
	UIScene::handleGainFocus(navBack);
	if( navBack ) g_NetworkManager.RegisterPlayerChangedCallback(m_iPad, &UIScene_InGameInfoMenu::OnPlayerChanged, this);
}

void UIScene_InGameInfoMenu::handleReload()
{
	DWORD playerCount = g_NetworkManager.GetPlayerCount();

	m_playersCount = 0;
	for(DWORD i = 0; i < playerCount; ++i)
	{
		INetworkPlayer *player = g_NetworkManager.GetPlayerByIndex( i );

		if( player != NULL )
		{
			m_players[i] = player->GetSmallId();
			++m_playersCount;

			wstring playerName = L"";
#ifndef _CONTENT_PACKAGE
			if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
			{
				playerName =  L"WWWWWWWWWWWWWWWW";
			}
			else
#endif
			{
				playerName = player->GetDisplayName();
			}

			int voiceStatus = 0;
			if(player != NULL && player->HasVoice() )
			{
				if( player->IsMutedByLocalUser(m_iPad) )
				{
					// Muted image
					voiceStatus = 3;
				}
				else if( player->IsTalking() )
				{
					// Talking image
					voiceStatus = 2;
				}
				else
				{
					// Not talking image
					voiceStatus = 1;
				}
			}

			m_playersVoiceState[i] = voiceStatus;
			m_playersColourState[i] = app.GetPlayerColour( m_players[i] );
			m_playerNames[i] = playerName;
			m_playerList.addItem( playerName, app.GetPlayerColour( m_players[i] ), voiceStatus); 
		}
	}

	INetworkPlayer *thisPlayer = g_NetworkManager.GetLocalPlayerByUserIndex( m_iPad );
	m_isHostPlayer = false;
	if(thisPlayer != NULL) m_isHostPlayer = thisPlayer->IsHost() == TRUE;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[m_iPad];
	if(!m_isHostPlayer && !localPlayer->isModerator() )
	{
		removeControl( &m_buttonGameOptions, false );
	}

	updateTooltips();

	if(controlHasFocus(eControl_GamePlayers))
	{
		m_playerList.setCurrentSelection(getControlChildFocus());
	}
}

void UIScene_InGameInfoMenu::tick()
{
	UIScene::tick();

	for(DWORD i = 0; i < m_playersCount; ++i)
	{
		INetworkPlayer *player = g_NetworkManager.GetPlayerByIndex( i );

		if( player != NULL )
		{
			m_players[i] = player->GetSmallId();
			int voiceStatus = 0;
			if(player != NULL && player->HasVoice() )
			{
				if( player->IsMutedByLocalUser(m_iPad) )
				{
					// Muted image
					voiceStatus = 3;
				}
				else if( player->IsTalking() )
				{
					// Talking image
					voiceStatus = 2;
				}
				else
				{
					// Not talking image
					voiceStatus = 1;
				}
			}

			if(voiceStatus != m_playersVoiceState[i])
			{
				m_playersVoiceState[i] = voiceStatus;
				m_playerList.setVOIPIcon( i, voiceStatus );
			}

			short icon = app.GetPlayerColour( m_players[i] );

			if(icon != m_playersColourState[i])
			{
				m_playersColourState[i] = icon;
				m_playerList.setPlayerIcon( i, (int)app.GetPlayerColour( m_players[i] ) );
			}

			wstring playerName = L"";
#ifndef _CONTENT_PACKAGE
			if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
			{
				playerName =  L"WWWWWWWWWWWWWWWW";
			}
			else
#endif
			{
				playerName = player->GetDisplayName();
			}
			if(playerName.compare( m_playerNames[i] ) != 0 )
			{
				m_playerList.setButtonLabel(i, playerName);
				m_playerNames[i] = playerName;
			}
		}
	}
}

void UIScene_InGameInfoMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed && !repeat)
		{
			ui.PlayUISFX(eSFX_Back);
			navigateBack();
		}
		break;
	case ACTION_MENU_Y:
#if defined(__PS3__) || defined(__ORBIS__)
		if(pressed && iPad == ProfileManager.GetPrimaryPad())
		{
#ifdef __PS3__
			// are we offline?
			if(!ProfileManager.IsSignedInLive(iPad))
			{
				// get them to sign in to online
				UINT uiIDA[2];
				uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
				uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
				ui.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&UIScene_InGameInfoMenu::MustSignInReturnedPSN,this, app.GetStringTable());
			}
			else
#endif
			{
#ifdef __ORBIS__
				SQRNetworkManager_Orbis::RecvInviteGUI();
#else // __PS3__
				int ret = sceNpBasicRecvMessageCustom(SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE, SCE_NP_BASIC_RECV_MESSAGE_OPTIONS_INCLUDE_BOOTABLE, SYS_MEMORY_CONTAINER_ID_INVALID);
				app.DebugPrintf("sceNpBasicRecvMessageCustom return %d ( %08x )\n", ret, ret);
#endif 
			}
		}
#else


		if(pressed && m_playerList.hasFocus() && (m_playerList.getItemCount() > 0) && (m_playerList.getCurrentSelection() < m_playersCount) )
		{
			INetworkPlayer *player = g_NetworkManager.GetPlayerBySmallId(m_players[m_playerList.getCurrentSelection()]);
			if( player != NULL )
			{
				PlayerUID uid = player->GetUID();
				if( uid != INVALID_XUID )
				{
#ifdef __PSVITA__
					PSVITA_STUBBED;
#else
					ProfileManager.ShowProfileCard(iPad,uid);
#endif
				}
			}
		}

#endif
		break;
	case ACTION_MENU_X:

		if(pressed && !repeat && !g_NetworkManager.IsLocalGame() )
		{
#ifdef __PSVITA__
			if(CGameNetworkManager::usingAdhocMode() == false) 
				g_NetworkManager.SendInviteGUI(iPad);
#else
			g_NetworkManager.SendInviteGUI(iPad);
#endif
		}

		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_PAGEUP:
	case ACTION_MENU_PAGEDOWN:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_InGameInfoMenu::handlePress(F64 controlId, F64 childId)
{
	app.DebugPrintf("Pressed = %d, %d\n", (int)controlId, (int)childId);
	switch((int)controlId)
	{
	case eControl_GameOptions:
		ui.NavigateToScene(m_iPad,eUIScene_InGameHostOptionsMenu);
		break;
	case eControl_GamePlayers:
		int currentSelection = (int)childId;
		INetworkPlayer *selectedPlayer = g_NetworkManager.GetPlayerBySmallId( m_players[ currentSelection ] );

		Minecraft *pMinecraft = Minecraft::GetInstance();
		shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[m_iPad];

		bool isOp = m_isHostPlayer || localPlayer->isModerator();
		bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
		bool trust = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;

		if( isOp && selectedPlayer != NULL)
		{
			bool editingHost = selectedPlayer->IsHost();
			if( (cheats && (m_isHostPlayer || !editingHost ) ) || (!trust && (m_isHostPlayer || !editingHost))
#if (!defined(_CONTENT_PACKAGE) && !defined(_FINAL_BUILD) && defined(_DEBUG_MENUS_ENABLED))
				|| (m_isHostPlayer && editingHost)
#endif
				)
			{
				InGamePlayerOptionsInitData *pInitData = new InGamePlayerOptionsInitData();
				pInitData->iPad = m_iPad;
				pInitData->networkSmallId = m_players[ currentSelection ];
				pInitData->playerPrivileges = app.GetPlayerPrivileges(m_players[ currentSelection ] );
				ui.NavigateToScene(m_iPad,eUIScene_InGamePlayerOptionsMenu,pInitData);
			}
			else if(selectedPlayer->IsLocal() != TRUE && selectedPlayer->IsSameSystem(g_NetworkManager.GetHostPlayer()) != TRUE)
			{
				// Only ops will hit this, can kick anyone not local and not local to the host
				BYTE *smallId = new BYTE();
				*smallId = m_players[currentSelection];
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_OK;
				uiIDA[1]=IDS_CONFIRM_CANCEL;

				ui.RequestMessageBox(IDS_UNLOCK_KICK_PLAYER_TITLE, IDS_UNLOCK_KICK_PLAYER, uiIDA, 2, m_iPad,&UIScene_InGameInfoMenu::KickPlayerReturned,smallId,app.GetStringTable(),NULL,0,false);
			}
		}
		break;
	}
}

void UIScene_InGameInfoMenu::handleFocusChange(F64 controlId, F64 childId)
{
	switch((int)controlId)
	{
	case eControl_GamePlayers:
		m_playerList.updateChildFocus( (int) childId );
	};
	updateTooltips();
}

void UIScene_InGameInfoMenu::OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving)
{
	UIScene_InGameInfoMenu *scene = (UIScene_InGameInfoMenu *)callbackParam;
	bool playerFound = false;
	int foundIndex = 0;
	for(int i = 0; i < scene->m_playersCount; ++i)
	{
		if(!playerFound && scene->m_players[i] == pPlayer->GetSmallId() )
		{
			if( scene->m_playerList.getCurrentSelection() == scene->m_playerList.getItemCount() - 1 )
			{
				scene->m_playerList.setCurrentSelection( scene->m_playerList.getItemCount() - 2 );
			}
			// Player removed
			playerFound = true;
			foundIndex = i;
		}
	}

	if( playerFound )
	{
		--scene->m_playersCount;
		scene->m_playersVoiceState[scene->m_playersCount] = 0;
		scene->m_playersColourState[scene->m_playersCount] = 0;
		scene->m_playerNames[scene->m_playersCount] = L"";
		scene->m_playerList.removeItem(scene->m_playersCount);
	}

	if( !playerFound )
	{
		// Player added
		scene->m_players[scene->m_playersCount] = pPlayer->GetSmallId();
		++scene->m_playersCount;

		wstring playerName = L"";
#ifndef _CONTENT_PACKAGE
		if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
		{
			playerName =  L"WWWWWWWWWWWWWWWW";
		}
		else
#endif
		{
			playerName = pPlayer->GetDisplayName();
		}

		int voiceStatus = 0;
		if(pPlayer != NULL && pPlayer->HasVoice() )
		{
			if( pPlayer->IsMutedByLocalUser(scene->m_iPad) )
			{
				// Muted image
				voiceStatus = 3;
			}
			else if( pPlayer->IsTalking() )
			{
				// Talking image
				voiceStatus = 2;
			}
			else
			{
				// Not talking image
				voiceStatus = 1;
			}
		}

		scene->m_playerList.addItem( playerName, app.GetPlayerColour( scene->m_players[scene->m_playersCount - 1] ), voiceStatus); 
	}
}

int UIScene_InGameInfoMenu::KickPlayerReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	BYTE smallId = *(BYTE *)pParam;
	delete pParam;

	if(result==C4JStorage::EMessage_ResultAccept)
	{		
		Minecraft *pMinecraft = Minecraft::GetInstance();
		shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[iPad];
		if(localPlayer->connection)
		{
			localPlayer->connection->send( shared_ptr<KickPlayerPacket>( new KickPlayerPacket(smallId) ) );
		}
	}

	return 0;
}

#if defined __PS3__ || defined __PSVITA__
int UIScene_InGameInfoMenu::MustSignInReturnedPSN(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_InGameInfoMenu* pClass = (UIScene_InGameInfoMenu*)pParam;

	if(result==C4JStorage::EMessage_ResultAccept) 
	{
#ifdef __PS3__
		SQRNetworkManager_PS3::AttemptPSNSignIn(&UIScene_InGameInfoMenu::ViewInvites_SignInReturned, pClass);
#else // __PSVITA__
		SQRNetworkManager_Vita::AttemptPSNSignIn(&UIScene_InGameInfoMenu::ViewInvites_SignInReturned, pClass);
#endif
	}

	return 0;
}

int UIScene_InGameInfoMenu::ViewInvites_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	if(bContinue==true)
	{
		// Check if we're signed in to LIVE
		if(ProfileManager.IsSignedInLive(iPad))
		{
#ifdef __ORBIS__
			SQRNetworkManager_Orbis::RecvInviteGUI();
#elif defined(__PS3__)
			int ret = sceNpBasicRecvMessageCustom(SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE, SCE_NP_BASIC_RECV_MESSAGE_OPTIONS_INCLUDE_BOOTABLE, SYS_MEMORY_CONTAINER_ID_INVALID);
			app.DebugPrintf("sceNpBasicRecvMessageCustom return %d ( %08x )\n", ret, ret);
#else // __PSVITA__
			PSVITA_STUBBED;
#endif
		}
	}
	return 0;
}
#endif
