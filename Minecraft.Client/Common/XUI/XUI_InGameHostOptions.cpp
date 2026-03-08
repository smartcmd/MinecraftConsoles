#include "stdafx.h"
#include "XUI_MultiGameCreate.h"
#include "XUI_InGameHostOptions.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../ClientConnection.h"
#include "../../../Minecraft.World/net.minecraft.network.h"
#include "../../../Minecraft.World/net.minecraft.network.packet.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_InGameHostOptions::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad = *(int *)pInitData->pvInitData;

	MapChildControls();

	m_focusElement = m_CheckboxFireSpreads.m_hObj;

	XuiControlSetText(m_CheckboxFireSpreads,app.GetString(IDS_FIRE_SPREADS));
	XuiControlSetText(m_CheckboxTNTExplodes,app.GetString(IDS_TNT_EXPLODES));

	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}

	m_CheckboxFireSpreads.SetEnable(TRUE);
	m_CheckboxTNTExplodes.SetEnable(TRUE);
	m_CheckboxFireSpreads.SetCheck((app.GetGameHostOption(eGameHostOption_FireSpreads)!=0)?TRUE:FALSE);
	m_CheckboxTNTExplodes.SetCheck((app.GetGameHostOption(eGameHostOption_TNT)!=0)?TRUE:FALSE);

	INetworkPlayer *localPlayer = g_NetworkManager.GetLocalPlayerByUserIndex( m_iPad );
	unsigned int privs = app.GetPlayerPrivileges(localPlayer->GetSmallId());
	if   ( app.GetGameHostOption(eGameHostOption_CheatsEnabled)
		&& Player::getPlayerGamePrivilege(privs,Player::ePlayerGamePrivilege_CanTeleport)
		&& (g_NetworkManager.GetPlayerCount() > 1) )
	{
		m_buttonTeleportToPlayer.SetText(app.GetString(IDS_TELEPORT_TO_PLAYER));
		m_buttonTeleportToMe.SetText(app.GetString(IDS_TELEPORT_TO_ME));
	}
	else
	{
		removeControl(m_buttonTeleportToPlayer, true);
		removeControl(m_buttonTeleportToMe, true);
	}

	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);

	CXuiSceneBase::ShowLogo( m_iPad, FALSE );

	
	//SentientManager.RecordMenuShown(m_iPad, eUIScene_CreateWorldMenu, 0);

	return S_OK;
}


HRESULT CScene_InGameHostOptions::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		{
			unsigned int hostOptions = app.GetGameHostOption(eGameHostOption_All);
			app.SetGameHostOption(hostOptions,eGameHostOption_FireSpreads,m_CheckboxFireSpreads.IsChecked());
			app.SetGameHostOption(hostOptions,eGameHostOption_TNT,m_CheckboxTNTExplodes.IsChecked());

			// Send update settings packet to server
			if(hostOptions != app.GetGameHostOption(eGameHostOption_All) )
			{
				Minecraft *pMinecraft = Minecraft::GetInstance();				
				shared_ptr<MultiplayerLocalPlayer> player = pMinecraft->localplayers[m_iPad];
				if(player != NULL && player->connection)
				{
					player->connection->send( shared_ptr<ServerSettingsChangedPacket>( new ServerSettingsChangedPacket( ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS, hostOptions) ) );
				}
			}

			app.NavigateBack(pInputData->UserIndex);
			rfHandled = TRUE;
		}
		break;
	}
	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_InGameHostOptions::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(hObjPressed == m_buttonTeleportToPlayer || hObjPressed == m_buttonTeleportToMe)
	{
		TeleportMenuInitData *initData = new TeleportMenuInitData();
		initData->iPad = m_iPad;
		initData->teleportToPlayer = false;
		if( hObjPressed == m_buttonTeleportToPlayer )
		{
			initData->teleportToPlayer = true;
		}
		ui.NavigateToScene(m_iPad,eUIScene_TeleportMenu,(void*)initData);
	}
	return S_OK;
}

void CScene_InGameHostOptions::removeControl(HXUIOBJ hObjToRemove, bool center)
{
	D3DXVECTOR3 pos;
	float fControlHeight, fTempHeight, fWidth;

	bool changeFocus = m_focusElement == hObjToRemove;

	XuiElementGetBounds(hObjToRemove,&fWidth,&fControlHeight);

	// Hide this control
	XuiControlSetEnable(hObjToRemove, FALSE);
	XuiElementSetShow(hObjToRemove, FALSE);

	// Move future downwards nav up
	HXUIOBJ controlToMove = hObjToRemove;
	while(controlToMove = XuiControlGetNavigation(controlToMove, XUI_CONTROL_NAVIGATE_DOWN, FALSE, TRUE) )
	{
		if(changeFocus && XuiElementIsShown(controlToMove))
		{
			m_focusElement = controlToMove;
			XuiElementSetUserFocus( controlToMove, m_iPad );
			changeFocus = FALSE;
		}
		XuiElementGetPosition(controlToMove, &pos);
		pos.y -= fControlHeight;
		XuiElementSetPosition(controlToMove, &pos);
	}

	// Resize and move scene
	GetBounds(&fWidth, &fTempHeight);
	SetBounds(fWidth, fTempHeight - fControlHeight);

	GetPosition(&pos);
	pos.y += fControlHeight/2;
	SetPosition(&pos);
}