#include "stdafx.h"
#include "XUI_MultiGameCreate.h"
#include "XUI_InGamePlayerOptions.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../ClientConnection.h"
#include "../../../Minecraft.World/net.minecraft.network.h"
#include "../../../Minecraft.World/net.minecraft.network.packet.h"
#include "../../../Minecraft.World/net.minecraft.world.entity.player.h"
#include "XUI_InGameInfo.h"



//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_InGamePlayerOptions::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad = *static_cast<int *>(pInitData->pvInitData);

	InGamePlayerOptionsInitData *initData = static_cast<InGamePlayerOptionsInitData *>(pInitData->pvInitData);
	m_iPad = initData->iPad;
	m_networkSmallId = initData->networkSmallId;
	m_playerPrivileges = initData->playerPrivileges;

	MapChildControls();

	m_focusElement = m_checkboxes[eControl_BuildAndMine].m_hObj;

	m_TeleportGroup.SetShow(false);

	INetworkPlayer *localPlayer = g_NetworkManager.GetLocalPlayerByUserIndex( m_iPad );
	INetworkPlayer *editingPlayer = g_NetworkManager.GetPlayerBySmallId(m_networkSmallId);

	if(editingPlayer != nullptr)
	{
		m_Gamertag.SetText(editingPlayer->GetOnlineName());
	}

	bool trustPlayers = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
	bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
	m_editingSelf = (localPlayer != nullptr && localPlayer == editingPlayer);

	if( m_editingSelf || trustPlayers || editingPlayer->IsHost())
	{
		removeControl( m_checkboxes[eControl_BuildAndMine], true );
		removeControl( m_checkboxes[eControl_UseDoorsAndSwitches], true );
		removeControl( m_checkboxes[eControl_UseContainers], true );
		removeControl( m_checkboxes[eControl_AttackPlayers], true );
		removeControl( m_checkboxes[eControl_AttackAnimals], true );
	}
	else
	{
		bool checked = (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CannotMine)==0 && Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CannotBuild)==0);
		m_checkboxes[eControl_BuildAndMine].SetText( app.GetString(IDS_CAN_BUILD_AND_MINE) );
		m_checkboxes[eControl_BuildAndMine].SetCheck(checked);

		checked = (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches)!=0);
		m_checkboxes[eControl_UseDoorsAndSwitches].SetText( app.GetString(IDS_CAN_USE_DOORS_AND_SWITCHES) );
		m_checkboxes[eControl_UseDoorsAndSwitches].SetCheck(checked);

		checked = (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanUseContainers)!=0);
		m_checkboxes[eControl_UseContainers].SetText( app.GetString(IDS_CAN_OPEN_CONTAINERS) );
		m_checkboxes[eControl_UseContainers].SetCheck(checked);

		checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CannotAttackPlayers)==0;
		m_checkboxes[eControl_AttackPlayers].SetText( app.GetString(IDS_CAN_ATTACK_PLAYERS) );
		m_checkboxes[eControl_AttackPlayers].SetCheck(checked);

		checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CannotAttackAnimals)==0;
		m_checkboxes[eControl_AttackAnimals].SetText( app.GetString(IDS_CAN_ATTACK_ANIMALS) );
		m_checkboxes[eControl_AttackAnimals].SetCheck(checked);
	}

	if(m_editingSelf)
	{
#if (defined(_CONTENT_PACKAGE) || defined(_FINAL_BUILD) && !defined(_DEBUG_MENUS_ENABLED))
		removeControl( m_checkboxes[eControl_Op], true );
#else
		m_checkboxes[eControl_Op].SetText(L"DEBUG: Creative");
		m_checkboxes[eControl_Op].SetCheck(Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode));
#endif

		removeControl( m_buttonKick, true );
		removeControl( m_checkboxes[eControl_CheatTeleport], true );

		if(cheats)
		{
			bool canBeInvisible = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
			m_checkboxes[eControl_HostInvisible].SetEnable(canBeInvisible);
			bool checked = canBeInvisible && (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_Invisible)!=0 && Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_Invulnerable)!=0);
			m_checkboxes[eControl_HostInvisible].SetText( app.GetString(IDS_INVISIBLE) );
			m_checkboxes[eControl_HostInvisible].SetCheck(checked);

			bool inCreativeMode = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode) != 0;
			if(inCreativeMode)
			{
				removeControl( m_checkboxes[eControl_HostFly], true );
				removeControl( m_checkboxes[eControl_HostHunger], true );
			}
			else
			{
				bool canFly = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleFly);
				bool canChangeHunger = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleClassicHunger);

				m_checkboxes[eControl_HostFly].SetEnable(canFly);
				checked = canFly && Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanFly)!=0;
				m_checkboxes[eControl_HostFly].SetText( app.GetString(IDS_CAN_FLY) );
				m_checkboxes[eControl_HostFly].SetCheck(checked);

				m_checkboxes[eControl_HostHunger].SetEnable(canChangeHunger);
				checked = canChangeHunger && Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_ClassicHunger)!=0;
				m_checkboxes[eControl_HostHunger].SetText( app.GetString(IDS_DISABLE_EXHAUSTION) );
				m_checkboxes[eControl_HostHunger].SetCheck(checked);
			}
		}
		else
		{
			removeControl( m_checkboxes[eControl_HostInvisible], true );
			removeControl( m_checkboxes[eControl_HostFly], true );
			removeControl( m_checkboxes[eControl_HostHunger], true );
		}
	}
	else
	{
		if(localPlayer->IsHost())
		{
			// Only host can make people moderators, or enable teleporting for them
			m_checkboxes[eControl_Op].SetText( app.GetString(IDS_MODERATOR) );
			m_checkboxes[eControl_Op].SetCheck(Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_Op)!=0);
		}
		else
		{
			removeControl( m_checkboxes[eControl_Op], true );
		}

		if(localPlayer->IsHost() && cheats)
		{
			m_checkboxes[eControl_HostInvisible].SetEnable(true);
			bool checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleInvisible)!=0;
			m_checkboxes[eControl_HostInvisible].SetText( app.GetString(IDS_CAN_INVISIBLE) );
			m_checkboxes[eControl_HostInvisible].SetCheck(checked);

			
			bool inCreativeMode = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode) != 0;
			if(inCreativeMode)
			{
				removeControl( m_checkboxes[eControl_HostFly], true );
				removeControl( m_checkboxes[eControl_HostHunger], true );
			}
			else
			{
				m_checkboxes[eControl_HostFly].SetEnable(true);
				checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleFly)!=0;
				m_checkboxes[eControl_HostFly].SetText( app.GetString(IDS_CAN_FLY) );
				m_checkboxes[eControl_HostFly].SetCheck(checked);

				m_checkboxes[eControl_HostHunger].SetEnable(true);
				checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleClassicHunger)!=0;
				m_checkboxes[eControl_HostHunger].SetText( app.GetString(IDS_CAN_DISABLE_EXHAUSTION) );
				m_checkboxes[eControl_HostHunger].SetCheck(checked);
			}

			checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanTeleport)!=0;
			m_checkboxes[eControl_CheatTeleport].SetText(app.GetString(IDS_ENABLE_TELEPORT));
			m_checkboxes[eControl_CheatTeleport].SetCheck(checked);
		}
		else
		{
			removeControl( m_checkboxes[eControl_HostInvisible], true );
			removeControl( m_checkboxes[eControl_HostFly], true );
			removeControl( m_checkboxes[eControl_HostHunger], true );
			removeControl( m_checkboxes[eControl_CheatTeleport], true );
		}

		// Can only kick people if they are not local, and not local to the host
		if(editingPlayer->IsLocal() != TRUE && editingPlayer->IsSameSystem(g_NetworkManager.GetHostPlayer()) != TRUE)
		{
			m_buttonKick.SetText( app.GetString(IDS_KICK_PLAYER));
		}
		else
		{
			removeControl( m_buttonKick, true );
		}
	}

	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}

	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);

	CXuiSceneBase::ShowLogo( m_iPad, FALSE );	

	g_NetworkManager.RegisterPlayerChangedCallback(m_iPad, &CScene_InGamePlayerOptions::OnPlayerChanged, this);

	//SentientManager.RecordMenuShown(m_iPad, eUIScene_CreateWorldMenu, 0);

	resetCheatCheckboxes();

	return S_OK;
}

HRESULT CScene_InGamePlayerOptions::OnDestroy()
{
	g_NetworkManager.UnRegisterPlayerChangedCallback(m_iPad, &CScene_InGameInfo::OnPlayerChanged, this);
	return S_OK;
}


HRESULT CScene_InGamePlayerOptions::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		{
			bool trustPlayers = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
			bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
			if(m_editingSelf)
			{
#if (defined(_CONTENT_PACKAGE) || defined(_FINAL_BUILD) && !defined(_DEBUG_MENUS_ENABLED))
#else
				Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode,m_checkboxes[eControl_Op].IsChecked());
#endif
				if(cheats)
				{
					bool canBeInvisible = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
					if(canBeInvisible) Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_Invisible,m_checkboxes[eControl_HostInvisible].IsChecked());
					if(canBeInvisible) Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_Invulnerable,m_checkboxes[eControl_HostInvisible].IsChecked());

					bool inCreativeMode = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode) != 0;
					if(!inCreativeMode)
					{
						bool canFly = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleFly);
						bool canChangeHunger = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleClassicHunger);

						if(canFly) Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanFly,m_checkboxes[eControl_HostFly].IsChecked());
						if(canChangeHunger) Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_ClassicHunger,m_checkboxes[eControl_HostHunger].IsChecked());
					}
				}
			}
			else
			{
				INetworkPlayer *editingPlayer = g_NetworkManager.GetPlayerBySmallId(m_networkSmallId);
				if(!trustPlayers && (editingPlayer != nullptr && !editingPlayer->IsHost() ) )
				{
					Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CannotMine,!m_checkboxes[eControl_BuildAndMine].IsChecked());
					Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CannotBuild,!m_checkboxes[eControl_BuildAndMine].IsChecked());
					Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CannotAttackPlayers,!m_checkboxes[eControl_AttackPlayers].IsChecked());
					Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CannotAttackAnimals, !m_checkboxes[eControl_AttackAnimals].IsChecked());
					Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches, m_checkboxes[eControl_UseDoorsAndSwitches].IsChecked());
					Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanUseContainers, m_checkboxes[eControl_UseContainers].IsChecked());
				}

				INetworkPlayer *localPlayer = g_NetworkManager.GetLocalPlayerByUserIndex( m_iPad );

				if(localPlayer->IsHost())
				{
					if(cheats)
					{
						Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleInvisible,m_checkboxes[eControl_HostInvisible].IsChecked());
						Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleFly,m_checkboxes[eControl_HostFly].IsChecked());
						Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleClassicHunger,m_checkboxes[eControl_HostHunger].IsChecked());
						Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanTeleport,m_checkboxes[eControl_CheatTeleport].IsChecked());
					}

					Player::setPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_Op,m_checkboxes[eControl_Op].IsChecked());
				}
			}
			unsigned int originalPrivileges = app.GetPlayerPrivileges(m_networkSmallId);
			if(originalPrivileges != m_playerPrivileges)
			{
				// Send update settings packet to server
				Minecraft *pMinecraft = Minecraft::GetInstance();				
				shared_ptr<MultiplayerLocalPlayer> player = pMinecraft->localplayers[m_iPad];
				if(player != nullptr && player->connection)
				{
					player->connection->send(std::make_shared<PlayerInfoPacket>(m_networkSmallId, -1, m_playerPrivileges));
				}
			}

			app.NavigateBack(pInputData->UserIndex);
			rfHandled = TRUE;
		}
		break;
	}
	return S_OK;
}

HRESULT CScene_InGamePlayerOptions::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	//HRESULT hr = S_OK;
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if( hObjPressed == m_buttonKick )
	{
		BYTE *smallId = new BYTE();
		*smallId = m_networkSmallId;
		UINT uiIDA[2];
		uiIDA[0]=IDS_CONFIRM_OK;
		uiIDA[1]=IDS_CONFIRM_CANCEL;

		StorageManager.RequestMessageBox(IDS_UNLOCK_KICK_PLAYER_TITLE, IDS_UNLOCK_KICK_PLAYER, uiIDA, 2, m_iPad,&CScene_InGamePlayerOptions::KickPlayerReturned,smallId,app.GetStringTable());
	}
	else if (hObjPressed == m_checkboxes[eControl_Op] )
	{
		resetCheatCheckboxes();
	}

	return S_OK;
}

HRESULT CScene_InGamePlayerOptions::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest!=nullptr)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

int CScene_InGamePlayerOptions::KickPlayerReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	BYTE smallId = *static_cast<BYTE *>(pParam);
	delete pParam;

	if(result==C4JStorage::EMessage_ResultAccept)
	{		
		Minecraft *pMinecraft = Minecraft::GetInstance();
		shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[iPad];
		if(localPlayer != nullptr && localPlayer->connection)
		{
			localPlayer->connection->send(std::make_shared<KickPlayerPacket>(smallId));
		}

		// Fix for #61494 - [CRASH]: TU7: Code: Multiplayer: Title may crash while kicking a player from an online game.
		// We cannot do a navigate back here is this actually occurs on a thread other than the main thread. On rare occasions this can clash
		// with the XUI render and causes a crash. The OnPlayerChanged event should perform the navigate back on the main thread
		//app.NavigateBack(iPad);
	}

	return 0;
}

void CScene_InGamePlayerOptions::OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving)
{
	CScene_InGamePlayerOptions *scene = static_cast<CScene_InGamePlayerOptions *>(callbackParam);

	HXUIOBJ hBackScene = scene->GetBackScene();
	CScene_InGameInfo* infoScene;
	VOID *pObj;
	XuiObjectFromHandle( hBackScene, &pObj );
	infoScene = static_cast<CScene_InGameInfo *>(pObj);
	if(infoScene != nullptr) CScene_InGameInfo::OnPlayerChanged(infoScene,pPlayer,leaving);

	if(leaving && pPlayer != nullptr && pPlayer->GetSmallId() == scene->m_networkSmallId)
	{
		app.NavigateBack(scene->m_iPad);
	}
}

HRESULT CScene_InGamePlayerOptions::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{
		INetworkPlayer *editingPlayer = g_NetworkManager.GetPlayerBySmallId(m_networkSmallId);
		if(editingPlayer != nullptr)
		{
			short colourIndex = app.GetPlayerColour( m_networkSmallId );
			switch(colourIndex)
			{
			case 1:
				m_Icon.PlayVisualRange(L"P1",nullptr,L"P1");
				break;
			case 2:
				m_Icon.PlayVisualRange(L"P2",nullptr,L"P2");
				break;
			case 3:
				m_Icon.PlayVisualRange(L"P3",nullptr,L"P3");
				break;
			case 4:
				m_Icon.PlayVisualRange(L"P4",nullptr,L"P4");
				break;
			case 5:
				m_Icon.PlayVisualRange(L"P5",nullptr,L"P5");
				break;
			case 6:
				m_Icon.PlayVisualRange(L"P6",nullptr,L"P6");
				break;
			case 7:
				m_Icon.PlayVisualRange(L"P7",nullptr,L"P7");
				break;
			case 8:
				m_Icon.PlayVisualRange(L"P8",nullptr,L"P8");
				break;
			case 9:
				m_Icon.PlayVisualRange(L"P9",nullptr,L"P9");
				break;
			case 10:
				m_Icon.PlayVisualRange(L"P10",nullptr,L"P10");
				break;
			case 11:
				m_Icon.PlayVisualRange(L"P11",nullptr,L"P11");
				break;
			case 12:
				m_Icon.PlayVisualRange(L"P12",nullptr,L"P12");
				break;
			case 13:
				m_Icon.PlayVisualRange(L"P13",nullptr,L"P13");
				break;
			case 14:
				m_Icon.PlayVisualRange(L"P14",nullptr,L"P14");
				break;
			case 15:
				m_Icon.PlayVisualRange(L"P15",nullptr,L"P15");
				break;
			case 0:
			default:
				m_Icon.PlayVisualRange(L"P0",nullptr,L"P0");
				break;
			};
		}
	}
	return S_OK;
}

void CScene_InGamePlayerOptions::removeControl(HXUIOBJ hObjToRemove, bool center)
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

void CScene_InGamePlayerOptions::resetCheatCheckboxes()
{
	bool isModerator = m_checkboxes[eControl_Op].IsChecked();
	//bool cheatsEnabled  = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;

	if (!m_editingSelf)
	{
		m_checkboxes[eControl_HostInvisible].SetEnable(isModerator);
		m_checkboxes[eControl_HostInvisible].SetCheck( isModerator 
												&& (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0) );

		// NOT CREATIVE MODE.
		{
			m_checkboxes[eControl_HostFly].SetEnable(isModerator);
			m_checkboxes[eControl_HostFly].SetCheck( isModerator 
													&& (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleFly) != 0) );

			m_checkboxes[eControl_HostHunger].SetEnable(isModerator);
			m_checkboxes[eControl_HostHunger].SetCheck( isModerator 
													&& (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleClassicHunger) != 0) );
		}

		m_checkboxes[eControl_CheatTeleport].SetEnable(isModerator);
		m_checkboxes[eControl_CheatTeleport].SetCheck( isModerator 
												&& (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanTeleport) != 0) );
	}
}