#include "stdafx.h"

#include <assert.h>
#include "XUI_Teleport.h"
#include "..\..\ServerPlayer.h"
#include "..\..\PlayerConnection.h"
#include "..\..\PlayerList.h"
#include "..\..\MinecraftServer.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\PlayerRenderer.h"
#include "XUI_InGamePlayerOptions.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\ClientConnection.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\..\Xbox\Network\NetworkPlayerXbox.h"
#include "TeleportCommand.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_Teleport::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	TeleportMenuInitData *initParam = (TeleportMenuInitData *)pInitData->pvInitData;

	m_iPad = initParam->iPad;
	m_teleportToPlayer = initParam->teleportToPlayer;

	delete initParam;

	MapChildControls();

	if(m_teleportToPlayer)
	{
		m_title.SetText(app.GetString(IDS_TELEPORT_TO_PLAYER));
	}
	else
	{
		m_title.SetText(app.GetString(IDS_TELEPORT_TO_ME));
	}

	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}

	DWORD playerCount = g_NetworkManager.GetPlayerCount();

	m_playersCount = 0;
	for(DWORD i = 0; i < playerCount; ++i)
	{
		INetworkPlayer *player = g_NetworkManager.GetPlayerByIndex( i );

		if( player != NULL && !(player->IsLocal() && player->GetUserIndex() == m_iPad) )
		{
			m_players[m_playersCount] = player->GetSmallId();
			++m_playersCount;
		}
	}

	g_NetworkManager.RegisterPlayerChangedCallback(m_iPad, &CScene_Teleport::OnPlayerChanged, this);

	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
	
	CXuiSceneBase::ShowDarkOverlay( m_iPad, TRUE );

	return S_OK;
}

HRESULT CScene_Teleport::OnDestroy()
{
	g_NetworkManager.UnRegisterPlayerChangedCallback(m_iPad, &CScene_Teleport::OnPlayerChanged, this);
	return S_OK;
}

HRESULT CScene_Teleport::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr = S_OK;
	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_PAD_BACK:
	case VK_ESCAPE:
		CXuiSceneBase::PlayUISFX(eSFX_Back);
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;

		break;
	}


	return hr;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_Teleport::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if( hObjPressed == playersList )
	{	
		INetworkPlayer *selectedPlayer = g_NetworkManager.GetPlayerBySmallId( m_players[ playersList.GetCurSel() ] );
		INetworkPlayer *thisPlayer = g_NetworkManager.GetLocalPlayerByUserIndex(m_iPad);
		
		shared_ptr<GameCommandPacket> packet;
		if(m_teleportToPlayer)
		{
			packet = TeleportCommand::preparePacket(thisPlayer->GetUID(),selectedPlayer->GetUID());
		}
		else
		{
			packet = TeleportCommand::preparePacket(selectedPlayer->GetUID(),thisPlayer->GetUID());
		}
		ClientConnection *conn = Minecraft::GetInstance()->getConnection(m_iPad);
		conn->send( packet );
	}
	return S_OK;
}

void CScene_Teleport::OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving)
{
	CScene_Teleport *scene = (CScene_Teleport *)callbackParam;
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

HRESULT CScene_Teleport::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
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

HRESULT CScene_Teleport::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled)
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

HRESULT CScene_Teleport::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = m_playersCount;
	bHandled = TRUE;
	return S_OK;
}

HRESULT CScene_Teleport::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}