#include "stdafx.h"
#include "UI.h"
#include "UIScene_TeleportMenu.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\ClientConnection.h"
#include "TeleportCommand.h"

UIScene_TeleportMenu::UIScene_TeleportMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	
	TeleportMenuInitData *initParam = static_cast<TeleportMenuInitData *>(initData);

	m_teleportToPlayer = initParam->teleportToPlayer;

	delete initParam;
	
	if(m_teleportToPlayer)
	{
		m_labelTitle.init(app.GetString(IDS_TELEPORT_TO_PLAYER));
	}
	else
	{
		m_labelTitle.init(app.GetString(IDS_TELEPORT_TO_ME));
	}

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

		if( player != nullptr && !(player->IsLocal() && player->GetUserIndex() == m_iPad) )
		{
			m_players[m_playersCount] = player->GetSmallId();
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
			if(player != nullptr && player->HasVoice() )
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

			m_playersVoiceState[m_playersCount] = voiceStatus;
			m_playersColourState[m_playersCount] = app.GetPlayerColour( m_players[m_playersCount] );
			m_playerNames[m_playersCount] = playerName;
			m_playerList.addItem( playerName, app.GetPlayerColour( m_players[m_playersCount] ), voiceStatus); 
		}
	}

	g_NetworkManager.RegisterPlayerChangedCallback(m_iPad, &UIScene_TeleportMenu::OnPlayerChanged, this);

	parentLayer->addComponent(iPad,eUIComponent_MenuBackground);

	// get rid of the quadrant display if it's on
	ui.HidePressStart();
}

wstring UIScene_TeleportMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"InGameTeleportMenuSplit";
	}
	else
	{
		return L"InGameTeleportMenu";
	}
}

void UIScene_TeleportMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_TeleportMenu::handleDestroy()
{
	g_NetworkManager.UnRegisterPlayerChangedCallback(m_iPad, &UIScene_TeleportMenu::OnPlayerChanged, this);

	m_parentLayer->removeComponent(eUIComponent_MenuBackground);
}

void UIScene_TeleportMenu::handleGainFocus(bool navBack)
{
	if( navBack ) g_NetworkManager.RegisterPlayerChangedCallback(m_iPad, &UIScene_TeleportMenu::OnPlayerChanged, this);
}

void UIScene_TeleportMenu::handleReload()
{
	DWORD playerCount = g_NetworkManager.GetPlayerCount();

	m_playersCount = 0;
	for(DWORD i = 0; i < playerCount; ++i)
	{
		INetworkPlayer *player = g_NetworkManager.GetPlayerByIndex( i );

		if( player != nullptr && !(player->IsLocal() && player->GetUserIndex() == m_iPad)  )
		{
			m_players[m_playersCount] = player->GetSmallId();
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
			if(player != nullptr && player->HasVoice() )
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

			m_playersVoiceState[m_playersCount] = voiceStatus;
			m_playersColourState[m_playersCount] = app.GetPlayerColour( m_players[m_playersCount] );
			m_playerNames[m_playersCount] = playerName;
			m_playerList.addItem( playerName, app.GetPlayerColour( m_players[m_playersCount] ), voiceStatus); 
		}
	}

	if(controlHasFocus(eControl_GamePlayers))
	{
		m_playerList.setCurrentSelection(getControlChildFocus());
	}
}

void UIScene_TeleportMenu::tick()
{
	UIScene::tick();

	for(DWORD i = 0; i < m_playersCount; ++i)
	{
		INetworkPlayer *player = g_NetworkManager.GetPlayerBySmallId( m_players[i] );

		if( player != nullptr )
		{
			m_players[i] = player->GetSmallId();

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

void UIScene_TeleportMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
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

void UIScene_TeleportMenu::handlePress(F64 controlId, F64 childId)
{
	app.DebugPrintf("Pressed = %d, %d\n", static_cast<int>(controlId), static_cast<int>(childId));
	switch(static_cast<int>(controlId))
	{
	case eControl_GamePlayers:
		int currentSelection = static_cast<int>(childId);
		INetworkPlayer *selectedPlayer = g_NetworkManager.GetPlayerBySmallId( m_players[ currentSelection ] );
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
		break;
	}
}

void UIScene_TeleportMenu::OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving)
{
	UIScene_TeleportMenu *scene = static_cast<UIScene_TeleportMenu *>(callbackParam);
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
		if(pPlayer != nullptr && pPlayer->HasVoice() )
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
