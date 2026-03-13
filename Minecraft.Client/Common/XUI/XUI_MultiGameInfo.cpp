#include "stdafx.h"
#include <xuiresource.h>
#include <xuiapp.h>
#include <assert.h>
#include "../../../Minecraft.World/LevelSettings.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "XUI_MultiGameInfo.h"
#include "XUI_MultiGameJoinLoad.h"
#include "../../../Minecraft.World/LevelSettings.h"
#include "../../../Minecraft.World/Difficulty.h"

#define UPDATE_PLAYERS_TIMER_ID 0
#define UPDATE_PLAYERS_TIMER_TIME 30000

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_MultiGameInfo::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	
	XuiControlSetText(playersList,app.GetString(IDS_PLAYERS));
	XuiControlSetText(m_JoinGame,app.GetString(IDS_JOIN_GAME));
	XuiControlSetText(m_labelDifficulty,app.GetString(IDS_LABEL_DIFFICULTY));
	XuiControlSetText(m_labelGameType,app.GetString(IDS_LABEL_GAME_TYPE));
	XuiControlSetText(m_labelGamertagsOn,app.GetString(IDS_LABEL_GAMERTAGS));
	XuiControlSetText(m_labelStructuresOn,app.GetString(IDS_LABEL_STRUCTURES));
	XuiControlSetText(m_labelLevelType,app.GetString(IDS_LABEL_LEVEL_TYPE));
	XuiControlSetText(m_labelPvP,app.GetString(IDS_LABEL_PvP));
	XuiControlSetText(m_labelTrust,app.GetString(IDS_LABEL_TRUST));
	XuiControlSetText(m_labelTNTOn,app.GetString(IDS_LABEL_TNT));
	XuiControlSetText(m_labelFireOn,app.GetString(IDS_LABEL_FIRE_SPREADS));

	JoinMenuInitData *initData = static_cast<JoinMenuInitData *>(pInitData->pvInitData);
	m_selectedSession = initData->selectedSession;
	m_iPad = initData->iPad;
	// 4J-PB - don't delete this - it's part of the joinload structure
	//delete initData;

	for(unsigned int i = 0; i < MINECRAFT_NET_MAX_PLAYERS; ++i)
	{
		if( m_selectedSession->data.players[i] != nullptr )
		{
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

	unsigned int uiGameHostSettings = m_selectedSession->data.m_uiGameHostSettings;
	switch(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_Difficulty))
	{
	case Difficulty::EASY:
		m_difficulty.SetText( app.GetString(IDS_DIFFICULTY_TITLE_EASY) );
		break;
	case Difficulty::NORMAL:
		m_difficulty.SetText( app.GetString(IDS_DIFFICULTY_TITLE_NORMAL) );
		break;
	case Difficulty::HARD:
		m_difficulty.SetText( app.GetString(IDS_DIFFICULTY_TITLE_HARD) );
		break;
	case Difficulty::PEACEFUL:
	default:
		m_difficulty.SetText( app.GetString(IDS_DIFFICULTY_TITLE_PEACEFUL) );
		break;
	}
	
	unsigned int hostOption = app.GetGameHostOption(uiGameHostSettings,eGameHostOption_GameType);

	if (hostOption == GameType::CREATIVE->getId())
	{
		m_GameType.SetText( app.GetString(IDS_CREATIVE) );
	}
	else if (hostOption == GameType::SURVIVAL->getId())
	{
		m_GameType.SetText( app.GetString(IDS_SURVIVAL) );
	}
	else
	{
		m_GameType.SetText( app.GetString(IDS_SURVIVAL) );
	}

	if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_Gamertags))	m_gamertagsOn.SetText( app.GetString(IDS_ON) );
	else m_gamertagsOn.SetText( app.GetString(IDS_OFF) );

	if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_Structures)) m_structuresOn.SetText( app.GetString(IDS_ON) );
	else m_structuresOn.SetText( app.GetString(IDS_OFF) );

	if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_LevelType)) m_levelType.SetText( app.GetString(IDS_LEVELTYPE_SUPERFLAT) );
	else m_levelType.SetText( app.GetString(IDS_LEVELTYPE_NORMAL) );

	if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_PvP)) m_pvpOn.SetText( app.GetString(IDS_ON) );
	else m_pvpOn.SetText( app.GetString(IDS_OFF) );

	if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_TrustPlayers)) m_trustPlayers.SetText( app.GetString(IDS_ON) );
	else m_trustPlayers.SetText( app.GetString(IDS_OFF) );

	if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_TNT)) m_tntOn.SetText( app.GetString(IDS_ON) );
	else m_tntOn.SetText( app.GetString(IDS_OFF) );

	if(app.GetGameHostOption(uiGameHostSettings,eGameHostOption_FireSpreads)) m_fireOn.SetText( app.GetString(IDS_ON) );
	else m_fireOn.SetText( app.GetString(IDS_OFF) );

	m_bIgnoreInput = false;
	
	// Alert the app the we want to be informed of ethernet connections
	app.SetLiveLinkRequired( true );
	
	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_JoinMenu, 0);

	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK );

	XuiSetTimer(m_hObj,UPDATE_PLAYERS_TIMER_ID,UPDATE_PLAYERS_TIMER_TIME);
	
	return S_OK;
}



//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_MultiGameInfo::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if ( hObjPressed == m_JoinGame )
	{
		// check we have the texture pack required for the game


		SetShow( FALSE );
		m_bIgnoreInput=true;

		// 4J Stu - If we only have one controller connected, then don't show the sign-in UI again
		DWORD connectedControllers = 0;
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) ) ++connectedControllers;
		}

		if(connectedControllers == 1 || !RenderManager.IsHiDef())
		{
			JoinGame( this );
		}
		else
		{
			ProfileManager.RequestSignInUI(false, false, false, true, false,&CScene_MultiGameInfo::StartGame_SignInReturned, this,ProfileManager.GetPrimaryPad());
		}
	}

	return S_OK;
}

HRESULT CScene_MultiGameInfo::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr = S_OK;

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	case VK_PAD_Y:
		if(m_selectedSession != nullptr && playersList.TreeHasFocus() && playersList.GetItemCount() > 0)
		{
			PlayerUID xuid = m_selectedSession->data.players[playersList.GetCurSel()];
			if( xuid != INVALID_XUID )
				hr = XShowGamerCardUI(ProfileManager.GetLockedProfile(), xuid);
		}
		break;
	}
	
	return hr;
}

HRESULT CScene_MultiGameInfo::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK);

	return S_OK;
}

HRESULT CScene_MultiGameInfo::OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled)
{
	if(pNotifySelChangedData->iOldItem!=-1)
	{
		CXuiSceneBase::PlayUISFX(eSFX_Focus);
	}
	bHandled = TRUE;

	return S_OK;
}

HRESULT CScene_MultiGameInfo::OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
{
	if( playersList.TreeHasFocus() && playersList.GetItemCount() > 0 )
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1, IDS_TOOLTIPS_BACK, -1, IDS_TOOLTIPS_VIEW_GAMERCARD );
		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CScene_MultiGameInfo::OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
{
	if( pNotifyFocusData->hObjOther == m_JoinGame )
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK );
		bHandled = TRUE;
	}
	return S_OK;
}

int CScene_MultiGameInfo::StartGame_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	CScene_MultiGameInfo* pClass = static_cast<CScene_MultiGameInfo *>(pParam);

	if(bContinue==true)
	{
		// It's possible that the player has not signed in - they can back out
		if(ProfileManager.IsSignedIn(iPad))
		{
			JoinGame(pClass);
		}
	}
	else
	{
		pClass->SetShow( TRUE );
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}

// Shared function to join the game that is the same whether we used the sign-in UI or not
void CScene_MultiGameInfo::JoinGame(CScene_MultiGameInfo* pClass)
{
	DWORD dwLocalUsersMask = 0;
	bool noPrivileges = false;
	DWORD dwSignedInUsers = 0;

	// if we're in SD mode, then only the primary player gets to play
	if(RenderManager.IsHiDef())
	{
		for(unsigned int index = 0; index < XUSER_MAX_COUNT; ++index)
		{
			if(ProfileManager.IsSignedIn(index) )
			{
				++dwSignedInUsers;
				if( !ProfileManager.AllowedToPlayMultiplayer(index) ) noPrivileges = true;
				dwLocalUsersMask |= CGameNetworkManager::GetLocalPlayerMask(index);
			}
		}
	}
	else
	{
		if(ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad()) )
		{
			++dwSignedInUsers;
			if( !ProfileManager.AllowedToPlayMultiplayer(ProfileManager.GetPrimaryPad()) ) noPrivileges = true;
			dwLocalUsersMask |= CGameNetworkManager::GetLocalPlayerMask(ProfileManager.GetPrimaryPad());
		}
	}

	// Check if user-created content is allowed, as we cannot play multiplayer if it's not
	bool noUGC = false;
	BOOL pccAllowed = TRUE;
	BOOL pccFriendsAllowed = TRUE;
	ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
	if(!pccAllowed && !pccFriendsAllowed) noUGC = true;

	if(noUGC)
	{
		pClass->SetShow( TRUE );
		pClass->m_bIgnoreInput=false;
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;

		int messageText = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
		if(dwSignedInUsers > 1) messageText = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;

		StorageManager.RequestMessageBox( IDS_CONNECTION_FAILED, messageText, uiIDA,1,ProfileManager.GetPrimaryPad(),nullptr,nullptr, app.GetStringTable());
	
	}
	else if(noPrivileges)
	{
		pClass->SetShow( TRUE );
		pClass->m_bIgnoreInput=false;
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		StorageManager.RequestMessageBox( IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT, uiIDA,1,ProfileManager.GetPrimaryPad(),nullptr,nullptr, app.GetStringTable());
	}
	else
	{
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
				app.NavigateBack(ProfileManager.GetPrimaryPad());
			}
			else
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				StorageManager.RequestMessageBox( IDS_CONNECTION_FAILED, exitReasonStringId, uiIDA,1,ProfileManager.GetPrimaryPad(),nullptr,nullptr, app.GetStringTable());
				exitReasonStringId = -1;

				app.NavigateToHomeMenu();
			}
		}
	}
}

HRESULT CScene_MultiGameInfo::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	if ( pTimer->nId == UPDATE_PLAYERS_TIMER_ID)
	{
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
	}

	return S_OK;
}


