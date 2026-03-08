#include "stdafx.h"

#include <assert.h>
#include "XUI_InGameInfo.h"
#include "../../ServerPlayer.h"
#include "../../PlayerConnection.h"
#include "../../PlayerList.h"
#include "../../MinecraftServer.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../PlayerRenderer.h"
#include "XUI_InGamePlayerOptions.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../ClientConnection.h"
#include "../../../Minecraft.World/net.minecraft.network.packet.h"
#include "../../Xbox/Network/NetworkPlayerXbox.h"

#define IGNORE_KEYPRESS_TIMERID 0
#define TOOLTIP_TIMERID 1
#define IGNORE_KEYPRESS_TIME 100

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_InGameInfo::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_bIgnoreKeyPresses=true;
	m_iPad = *(int *)pInitData->pvInitData;

	MapChildControls();

	XuiControlSetText(m_gameOptionsButton,app.GetString(IDS_HOST_OPTIONS));
	XuiControlSetText(m_title,app.GetString(IDS_PLAYERS_INVITE));

	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
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
		}
	}

	g_NetworkManager.RegisterPlayerChangedCallback(m_iPad, &CScene_InGameInfo::OnPlayerChanged, this);

	INetworkPlayer *thisPlayer = g_NetworkManager.GetLocalPlayerByUserIndex( m_iPad );
	m_isHostPlayer = false;
	if(thisPlayer != NULL) m_isHostPlayer = thisPlayer->IsHost() == TRUE;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[m_iPad];
	if(!m_isHostPlayer && !localPlayer->isModerator() )
	{
		m_gameOptionsButton.SetEnable(FALSE);
		m_gameOptionsButton.SetShow(FALSE);
		playersList.SetFocus(m_iPad);
	}

	int keyX = IDS_TOOLTIPS_INVITE_FRIENDS;
	XPARTY_USER_LIST partyList;
	if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY ) && (partyList.dwUserCount>1))
	{
		keyX = IDS_TOOLTIPS_INVITE_PARTY;
	}
	if(g_NetworkManager.IsLocalGame()) keyX = -1;

	int keyA = -1;
	ui.SetTooltips( m_iPad, keyA,IDS_TOOLTIPS_BACK,keyX,-1);

	
	CXuiSceneBase::ShowDarkOverlay( m_iPad, TRUE );

	SetTimer( TOOLTIP_TIMERID , INGAME_INFO_TOOLTIP_TIMER );

	// get rid of the quadrant display if it's on
	CXuiSceneBase::HidePressStart();

	SetTimer(IGNORE_KEYPRESS_TIMERID,IGNORE_KEYPRESS_TIME);

	return S_OK;
}

HRESULT CScene_InGameInfo::OnDestroy()
{
	XuiKillTimer(m_hObj,TOOLTIP_TIMERID);
	g_NetworkManager.UnRegisterPlayerChangedCallback(m_iPad, &CScene_InGameInfo::OnPlayerChanged, this);
	return S_OK;
}

//----------------------------------------------------------------------------------
// Updates the UI when the list selection changes.
//----------------------------------------------------------------------------------
HRESULT CScene_InGameInfo::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if( hObjSource == playersList)
	{
		updateTooltips();

		bHandled = TRUE;
	}

	return S_OK;
}

HRESULT CScene_InGameInfo::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreKeyPresses) return S_OK;

	// 4J-PB - ignore repeats to stop the scene displaying and quitting right away if you hold the back button down
	if((pInputData->dwKeyCode==VK_PAD_BACK) &&(pInputData->dwFlags&XUI_INPUT_FLAG_REPEAT ))
	{
		rfHandled = TRUE;
		return S_OK;
	}
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr = S_OK;
	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_PAD_BACK:
	case VK_ESCAPE:
		CXuiSceneBase::PlayUISFX(eSFX_Back);
		app.CloseXuiScenes(pInputData->UserIndex);
		rfHandled = TRUE;

		break;
	case VK_PAD_Y:
		if(playersList.TreeHasFocus() && (playersList.GetItemCount() > 0) && (playersList.GetCurSel() < m_playersCount) )
		{
			INetworkPlayer *player = g_NetworkManager.GetPlayerBySmallId(m_players[playersList.GetCurSel()]);
			if( player != NULL )
			{
				PlayerUID xuid = ((NetworkPlayerXbox *)player)->GetUID();
				if( xuid != INVALID_XUID )
					hr = XShowGamerCardUI(pInputData->UserIndex, xuid);
			}
		}
		break;
	case VK_PAD_X:
		{
			if(!g_NetworkManager.IsLocalGame())
			{
				XPARTY_USER_LIST partyList;
				if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY) && (partyList.dwUserCount>1))
				{
					hr = XShowPartyUI( pInputData->UserIndex );
				}
				else
				{
					hr = XShowFriendsUI( pInputData->UserIndex );
				}
			}
		}
		break;
	}


	return hr;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_InGameInfo::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if( hObjPressed == playersList )
	{	
		INetworkPlayer *selectedPlayer = g_NetworkManager.GetPlayerBySmallId( m_players[ playersList.GetCurSel() ] );
		
		Minecraft *pMinecraft = Minecraft::GetInstance();
		shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[m_iPad];

		bool isOp = m_isHostPlayer || localPlayer->isModerator();
		bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
		bool trust = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;

		if( isOp && selectedPlayer != NULL && playersList.TreeHasFocus() && (playersList.GetItemCount() > 0) && (playersList.GetCurSel() < m_playersCount) )
		{
			bool editingHost = selectedPlayer->IsHost();
			if( (cheats && (m_isHostPlayer || !editingHost ) )
				|| (!trust && (m_isHostPlayer || !editingHost))
				|| (m_isHostPlayer && !editingHost)
#if (!defined(_CONTENT_PACKAGE) && !defined(_FINAL_BUILD) && defined(_DEBUG_MENUS_ENABLED))
				|| (m_isHostPlayer && editingHost)
#endif
				)
			{
				InGamePlayerOptionsInitData *pInitData = new InGamePlayerOptionsInitData();
				pInitData->iPad = m_iPad;
				pInitData->networkSmallId = m_players[ playersList.GetCurSel() ];
				pInitData->playerPrivileges = app.GetPlayerPrivileges(m_players[ playersList.GetCurSel() ] );
				app.NavigateToScene(m_iPad,eUIScene_InGamePlayerOptionsMenu,pInitData);
			}
			else if(selectedPlayer->IsLocal() != TRUE && selectedPlayer->IsSameSystem(g_NetworkManager.GetHostPlayer()) != TRUE)
			{
				// Only ops will hit this, can kick anyone not local and not local to the host
				BYTE *smallId = new BYTE();
				*smallId = m_players[playersList.GetCurSel()];
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_OK;
				uiIDA[1]=IDS_CONFIRM_CANCEL;

				StorageManager.RequestMessageBox(IDS_UNLOCK_KICK_PLAYER_TITLE, IDS_UNLOCK_KICK_PLAYER, uiIDA, 2, m_iPad,&CScene_InGameInfo::KickPlayerReturned,smallId,app.GetStringTable());
			}
		}
	}
	else if( hObjPressed == m_gameOptionsButton )
	{		
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_InGameHostOptionsMenu);
	}
	return S_OK;
}

HRESULT CScene_InGameInfo::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	if(pData->nId==IGNORE_KEYPRESS_TIMERID)
	{
		XuiKillTimer(m_hObj,IGNORE_KEYPRESS_TIMERID);
		m_bIgnoreKeyPresses=false;
	}
	else
	{
		updateTooltips();
	}

	return S_OK;
}

HRESULT CScene_InGameInfo::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransType == XUI_TRANSITION_FROM)
	{
		KillTimer( TOOLTIP_TIMERID );
	}
	return S_OK;
}

HRESULT CScene_InGameInfo::OnTransitionEnd( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{
		SetTimer( TOOLTIP_TIMERID , INGAME_INFO_TOOLTIP_TIMER );
		g_NetworkManager.RegisterPlayerChangedCallback(m_iPad, &CScene_InGameInfo::OnPlayerChanged, this);
	}
	return S_OK;
}

HRESULT CScene_InGameInfo::OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled )
{
	updateTooltips();
	return S_OK;
}

void CScene_InGameInfo::OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving)
{
	CScene_InGameInfo *scene = (CScene_InGameInfo *)callbackParam;
	bool playerFound = false;

	for(int i = 0; i < scene->m_playersCount; ++i)
	{
		if(playerFound)
		{
			scene->m_players[i-1] = scene->m_players[i];
		}
		else if( scene->m_players[i] == pPlayer->GetSmallId() )
		{
			if( scene->playersList.GetCurSel() == scene->playersList.GetItemCount() - 1 )
			{
				scene->playersList.SetCurSel( scene->playersList.GetItemCount() - 2 );
			}
			// Player removed
			playerFound = true;
		}
	}

	if( playerFound )
	{
		--scene->m_playersCount;
		scene->playersList.DeleteItems( scene->playersList.GetItemCount() - 1, 1 );
	}

	if( !playerFound )
	{
		// Player added
		scene->m_players[scene->m_playersCount] = pPlayer->GetSmallId();
		++scene->m_playersCount;
		scene->playersList.InsertItems( scene->playersList.GetItemCount(), 1 );
	}
}


HRESULT CScene_InGameInfo::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	if( pGetSourceTextData->bItemData )
	{
		if( pGetSourceTextData->iItem < m_playersCount )
		{
			INetworkPlayer *player = g_NetworkManager.GetPlayerBySmallId( m_players[pGetSourceTextData->iItem] );
			if( player != NULL )
			{
#ifndef _CONTENT_PACKAGE
				if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
				{
					pGetSourceTextData->szText =  L"WWWWWWWWWWWWWWWW";
				}
				else
#endif
				{
					pGetSourceTextData->szText = player->GetOnlineName();
				}
			}
			else
			{
				pGetSourceTextData->szText = L"";
			}

			HRESULT hr;
			HXUIOBJ hButton, hVisual, hPlayerIcon, hVoiceIcon;			
			hButton = playersList.GetItemControl(pGetSourceTextData->iItem);
			hr=XuiControlGetVisual(hButton,&hVisual);

			// Set the players icon
			hr=XuiElementGetChildById(hVisual,L"IconGroup",&hPlayerIcon);
			short colourIndex = app.GetPlayerColour( m_players[pGetSourceTextData->iItem] );
			int playFrame = 0;
			switch(colourIndex)
			{
			case 1:
				XuiElementFindNamedFrame(hPlayerIcon, L"P1", &playFrame);
				break;
			case 2:
				XuiElementFindNamedFrame(hPlayerIcon, L"P2", &playFrame);
				break;
			case 3:
				XuiElementFindNamedFrame(hPlayerIcon, L"P3", &playFrame);
				break;
			case 4:
				XuiElementFindNamedFrame(hPlayerIcon, L"P4", &playFrame);
				break;
			case 5:
				XuiElementFindNamedFrame(hPlayerIcon, L"P5", &playFrame);
				break;
			case 6:
				XuiElementFindNamedFrame(hPlayerIcon, L"P6", &playFrame);
				break;
			case 7:
				XuiElementFindNamedFrame(hPlayerIcon, L"P7", &playFrame);
				break;
			case 8:
				XuiElementFindNamedFrame(hPlayerIcon, L"P8", &playFrame);
				break;
			case 9:
				XuiElementFindNamedFrame(hPlayerIcon, L"P9", &playFrame);
				break;
			case 10:
				XuiElementFindNamedFrame(hPlayerIcon, L"P10", &playFrame);
				break;
			case 11:
				XuiElementFindNamedFrame(hPlayerIcon, L"P11", &playFrame);
				break;
			case 12:
				XuiElementFindNamedFrame(hPlayerIcon, L"P12", &playFrame);
				break;
			case 13:
				XuiElementFindNamedFrame(hPlayerIcon, L"P13", &playFrame);
				break;
			case 14:
				XuiElementFindNamedFrame(hPlayerIcon, L"P14", &playFrame);
				break;
			case 15:
				XuiElementFindNamedFrame(hPlayerIcon, L"P15", &playFrame);
				break;
			case 0:
			default:
				XuiElementFindNamedFrame(hPlayerIcon, L"P0", &playFrame);
				break;
			};
			if(playFrame < 0) playFrame = 0;
			XuiElementPlayTimeline(hPlayerIcon,playFrame,playFrame,playFrame,FALSE,FALSE);

			// Set the voice icon
			hr=XuiElementGetChildById(hVisual,L"VoiceGroup",&hVoiceIcon);

			playFrame = -1;
			if(player != NULL && player->HasVoice() )
			{
				if( player->IsMutedByLocalUser(m_iPad) )
				{
					// Muted image
					XuiElementFindNamedFrame(hVoiceIcon, L"Muted", &playFrame);
				}
				else if( player->IsTalking() )
				{
					// Talking image
					XuiElementFindNamedFrame(hVoiceIcon, L"Speaking", &playFrame);
				}
				else
				{
					// Not talking image
					XuiElementFindNamedFrame(hVoiceIcon, L"NotSpeaking", &playFrame);
				}
			}
			
			if(playFrame < 0)
			{
				XuiElementFindNamedFrame(hVoiceIcon, L"Normal", &playFrame);
			}
			XuiElementPlayTimeline(hVoiceIcon,playFrame,playFrame,playFrame,FALSE,FALSE);

			pGetSourceTextData->bDisplay = TRUE;

			bHandled = TRUE;
		}
	}
	return S_OK;
}

HRESULT CScene_InGameInfo::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled)
{
	if( pGetSourceImageData->bItemData )
	{
		if( pGetSourceImageData->iItem < m_playersCount )
		{
			bHandled = TRUE;
		}
	}
	return S_OK;
}

HRESULT CScene_InGameInfo::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = m_playersCount;
	bHandled = TRUE;
	return S_OK;
}

void CScene_InGameInfo::updateTooltips()
{
	int keyX = IDS_TOOLTIPS_INVITE_FRIENDS;
	int ikeyY = -1;

	XPARTY_USER_LIST partyList;
	if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY ) && (partyList.dwUserCount>1))
	{
		keyX = IDS_TOOLTIPS_INVITE_PARTY;
	}
	if(g_NetworkManager.IsLocalGame()) keyX = -1;

	INetworkPlayer *selectedPlayer = g_NetworkManager.GetPlayerBySmallId( m_players[ playersList.GetCurSel() ] );

	int keyA = -1;
	Minecraft *pMinecraft = Minecraft::GetInstance();
	shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[m_iPad];
	
	bool isOp = m_isHostPlayer || localPlayer->isModerator();
	bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
	bool trust = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;

	if( isOp )
	{
		if(m_gameOptionsButton.HasFocus())
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

	if(!m_gameOptionsButton.HasFocus())
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
	ui.SetTooltips( m_iPad, keyA,IDS_TOOLTIPS_BACK,keyX,ikeyY);
}


HRESULT CScene_InGameInfo::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

int CScene_InGameInfo::KickPlayerReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	BYTE smallId = *(BYTE *)pParam;
	delete pParam;

	if(result==C4JStorage::EMessage_ResultAccept)
	{		
		Minecraft *pMinecraft = Minecraft::GetInstance();
		shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[iPad];
		if(localPlayer != NULL && localPlayer->connection)
		{
			localPlayer->connection->send( shared_ptr<KickPlayerPacket>( new KickPlayerPacket(smallId) ) );
		}
	}

	return 0;
}