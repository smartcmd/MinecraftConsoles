#include "stdafx.h"
#include "UI.h"
#include "UIScene_InGameHostOptionsMenu.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\ClientConnection.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"

UIScene_InGameHostOptionsMenu::UIScene_InGameHostOptionsMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_checkboxFireSpreads.init(app.GetString(IDS_FIRE_SPREADS), eControl_FireSpreads, app.GetGameHostOption(eGameHostOption_FireSpreads)!=0);
	m_checkboxTNT.init(app.GetString(IDS_TNT_EXPLODES), eControl_TNT, app.GetGameHostOption(eGameHostOption_TNT)!=0);

	INetworkPlayer *localPlayer = g_NetworkManager.GetLocalPlayerByUserIndex( m_iPad );
	unsigned int privs = app.GetPlayerPrivileges(localPlayer->GetSmallId());
	if(app.GetGameHostOption(eGameHostOption_CheatsEnabled)
		&& Player::getPlayerGamePrivilege(privs,Player::ePlayerGamePrivilege_CanTeleport)
		&& g_NetworkManager.GetPlayerCount() > 1)
	{
		m_buttonTeleportToPlayer.init(app.GetString(IDS_TELEPORT_TO_PLAYER), eControl_TeleportToPlayer);
		m_buttonTeleportToMe.init(app.GetString(IDS_TELEPORT_TO_ME), eControl_TeleportToMe);
	}
	else
	{
		removeControl(&m_buttonTeleportToPlayer, true);
		removeControl(&m_buttonTeleportToMe, true);
	}
}

wstring UIScene_InGameHostOptionsMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"InGameHostOptionsSplit";
	}
	else
	{
		return L"InGameHostOptions";
	}
}

void UIScene_InGameHostOptionsMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_InGameHostOptionsMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");

	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);
	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			unsigned int hostOptions = app.GetGameHostOption(eGameHostOption_All);
			app.SetGameHostOption(hostOptions,eGameHostOption_FireSpreads,m_checkboxFireSpreads.IsChecked());
			app.SetGameHostOption(hostOptions,eGameHostOption_TNT,m_checkboxTNT.IsChecked());

			// Send update settings packet to server
			if(hostOptions != app.GetGameHostOption(eGameHostOption_All) )
			{
				Minecraft *pMinecraft = Minecraft::GetInstance();				
				shared_ptr<MultiplayerLocalPlayer> player = pMinecraft->localplayers[m_iPad];
				if(player->connection)
				{
					player->connection->send( shared_ptr<ServerSettingsChangedPacket>( new ServerSettingsChangedPacket( ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS, hostOptions) ) );
				}
			}

			navigateBack();

			handled = true;
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		sendInputToMovie(key, repeat, pressed, released);
		break;
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_InGameHostOptionsMenu::handlePress(F64 controlId, F64 childId)
{
	TeleportMenuInitData *initData = new TeleportMenuInitData();
	initData->iPad = m_iPad;
	initData->teleportToPlayer = false;
	if( (int)controlId == eControl_TeleportToPlayer )
	{
		initData->teleportToPlayer = true;
	}
	ui.NavigateToScene(m_iPad,eUIScene_TeleportMenu,(void*)initData);
}
