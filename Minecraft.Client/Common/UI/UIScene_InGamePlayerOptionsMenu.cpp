#include "stdafx.h"
#include "UI.h"
#include "UIScene_InGamePlayerOptionsMenu.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\ClientConnection.h"
#include "..\..\..\Minecraft.World\net.minecraft.network.packet.h"

#define CHECKBOXES_TIMER_ID 0
#define CHECKBOXES_TIMER_TIME 100

UIScene_InGamePlayerOptionsMenu::UIScene_InGamePlayerOptionsMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_bShouldNavBack = false;

	InGamePlayerOptionsInitData *initData = static_cast<InGamePlayerOptionsInitData *>(_initData);
	m_networkSmallId = initData->networkSmallId;
	m_playerPrivileges = initData->playerPrivileges;

	INetworkPlayer *localPlayer = g_NetworkManager.GetLocalPlayerByUserIndex( m_iPad );
	INetworkPlayer *editingPlayer = g_NetworkManager.GetPlayerBySmallId(m_networkSmallId);

	if(editingPlayer != nullptr)
	{
		m_labelGamertag.init(editingPlayer->GetDisplayName());
	}

	bool trustPlayers = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
	bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
	m_editingSelf = (localPlayer != nullptr && localPlayer == editingPlayer);

	if( m_editingSelf || trustPlayers || editingPlayer->IsHost())
	{
		removeControl( &m_checkboxes[eControl_BuildAndMine], true );
		removeControl( &m_checkboxes[eControl_UseDoorsAndSwitches], true );
		removeControl( &m_checkboxes[eControl_UseContainers], true );
		removeControl( &m_checkboxes[eControl_AttackPlayers], true );
		removeControl( &m_checkboxes[eControl_AttackAnimals], true );
	}
	else
	{
		bool checked = (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CannotMine)==0 && Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CannotBuild)==0);
		m_checkboxes[eControl_BuildAndMine].init( app.GetString(IDS_CAN_BUILD_AND_MINE), eControl_BuildAndMine, checked);

		checked = (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches)!=0);
		m_checkboxes[eControl_UseDoorsAndSwitches].init( app.GetString(IDS_CAN_USE_DOORS_AND_SWITCHES), eControl_UseDoorsAndSwitches, checked);

		checked = (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanUseContainers)!=0);
		m_checkboxes[eControl_UseContainers].init( app.GetString(IDS_CAN_OPEN_CONTAINERS), eControl_UseContainers, checked);

		checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CannotAttackPlayers)==0;
		m_checkboxes[eControl_AttackPlayers].init( app.GetString(IDS_CAN_ATTACK_PLAYERS), eControl_AttackPlayers, checked);

		checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CannotAttackAnimals)==0;
		m_checkboxes[eControl_AttackAnimals].init( app.GetString(IDS_CAN_ATTACK_ANIMALS), eControl_AttackAnimals, checked);
	}

	if(m_editingSelf)
	{
#ifndef _DEBUG // (defined(_CONTENT_PACKAGE) || defined(_FINAL_BUILD) && !defined(_DEBUG_MENUS_ENABLED))
		removeControl( &m_checkboxes[eControl_Op], true );
#else
		m_checkboxes[eControl_Op].init(L"DEBUG: Creative",eControl_Op,Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode));
#endif
		
		removeControl( &m_buttonKick, true );
		removeControl( &m_checkboxes[eControl_CheatTeleport], true );

		if(cheats)
		{
			bool canBeInvisible = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
			m_checkboxes[eControl_HostInvisible].SetEnable(canBeInvisible);
			bool checked = canBeInvisible && (Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_Invisible)!=0 && Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_Invulnerable)!=0);
			m_checkboxes[eControl_HostInvisible].init( app.GetString(IDS_INVISIBLE), eControl_HostInvisible, checked);

			bool inCreativeMode = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode) != 0;
			if(inCreativeMode)
			{
				removeControl( &m_checkboxes[eControl_HostFly], true );
				removeControl( &m_checkboxes[eControl_HostHunger], true );
			}
			else
			{
				bool canFly = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleFly);
				bool canChangeHunger = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleClassicHunger);

				m_checkboxes[eControl_HostFly].SetEnable(canFly);
				checked = canFly && Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanFly)!=0;
				m_checkboxes[eControl_HostFly].init( app.GetString(IDS_CAN_FLY), eControl_HostFly, checked);

				m_checkboxes[eControl_HostHunger].SetEnable(canChangeHunger);
				checked = canChangeHunger && Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_ClassicHunger)!=0;
				m_checkboxes[eControl_HostHunger].init( app.GetString(IDS_DISABLE_EXHAUSTION), eControl_HostHunger, checked);
			}
		}
		else
		{
			removeControl( &m_checkboxes[eControl_HostInvisible], true );
			removeControl( &m_checkboxes[eControl_HostFly], true );
			removeControl( &m_checkboxes[eControl_HostHunger], true );
		}
	}
	else
	{
		if(localPlayer->IsHost())
		{
			// Only host can make people moderators, or enable teleporting for them
			m_checkboxes[eControl_Op].init( app.GetString(IDS_MODERATOR), eControl_Op, Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_Op)!=0);
		}
		else
		{
			removeControl( &m_checkboxes[eControl_Op], true );
		}

		/*if(localPlayer->IsHost() && cheats )
		{
			m_checkboxes[eControl_HostInvisible].SetEnable(true);
			bool checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleInvisible)!=0;
			m_checkboxes[eControl_HostInvisible].init( app.GetString(IDS_CAN_INVISIBLE), eControl_HostInvisible, checked);

			m_checkboxes[eControl_HostFly].SetEnable(true);
			checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleFly)!=0;
			m_checkboxes[eControl_HostFly].init( app.GetString(IDS_CAN_FLY), eControl_HostFly, checked);

			m_checkboxes[eControl_HostHunger].SetEnable(true);
			checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleClassicHunger)!=0;
			m_checkboxes[eControl_HostHunger].init( app.GetString(IDS_CAN_DISABLE_EXHAUSTION), eControl_HostHunger, checked);
			
			checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanTeleport)!=0;
			m_checkboxes[eControl_CheatTeleport].init(app.GetString(IDS_ENABLE_TELEPORT),eControl_CheatTeleport,checked);
		}
		else
		{
			removeControl( &m_checkboxes[eControl_HostInvisible], true );
			removeControl( &m_checkboxes[eControl_HostFly], true );
			removeControl( &m_checkboxes[eControl_HostHunger], true );
			removeControl( &m_checkboxes[eControl_CheatTeleport], true );
		}*/

		if(localPlayer->IsHost() && cheats )
		{
			m_checkboxes[eControl_HostInvisible].SetEnable(true);
			bool checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleInvisible)!=0;
			m_checkboxes[eControl_HostInvisible].init( app.GetString(IDS_CAN_INVISIBLE), eControl_HostInvisible, checked);


			bool inCreativeMode = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode) != 0;
			if(inCreativeMode)
			{
				removeControl( &m_checkboxes[eControl_HostFly], true );
				removeControl( &m_checkboxes[eControl_HostHunger], true );
			}
			else
			{
				m_checkboxes[eControl_HostFly].SetEnable(true);
				checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleFly)!=0;
				m_checkboxes[eControl_HostFly].init( app.GetString(IDS_CAN_FLY), eControl_HostFly, checked);

				m_checkboxes[eControl_HostHunger].SetEnable(true);
				checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanToggleClassicHunger)!=0;
				m_checkboxes[eControl_HostHunger].init( app.GetString(IDS_CAN_DISABLE_EXHAUSTION), eControl_HostHunger, checked);
			}

			checked = Player::getPlayerGamePrivilege(m_playerPrivileges, Player::ePlayerGamePrivilege_CanTeleport)!=0;
			m_checkboxes[eControl_CheatTeleport].init(app.GetString(IDS_ENABLE_TELEPORT),eControl_CheatTeleport,checked);
		}
		else
		{
			removeControl( &m_checkboxes[eControl_HostInvisible], true );
			removeControl( &m_checkboxes[eControl_HostFly], true );
			removeControl( &m_checkboxes[eControl_HostHunger], true );
			removeControl( &m_checkboxes[eControl_CheatTeleport], true );
		}


		// Can only kick people if they are not local, and not local to the host
		if(editingPlayer->IsLocal() != TRUE && editingPlayer->IsSameSystem(g_NetworkManager.GetHostPlayer()) != TRUE)
		{
			m_buttonKick.init( app.GetString(IDS_KICK_PLAYER), eControl_Kick);
		}
		else
		{
			removeControl( &m_buttonKick, true );
		}
	}

	short colourIndex = app.GetPlayerColour( m_networkSmallId );
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = colourIndex;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetPlayerIcon , 1 , value );

#if TO_BE_IMPLEMENTED
	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}
#endif

	m_bModeratorState = m_checkboxes[eControl_Op].IsChecked();

	resetCheatCheckboxes();

	addTimer(CHECKBOXES_TIMER_ID,CHECKBOXES_TIMER_TIME);

	g_NetworkManager.RegisterPlayerChangedCallback(m_iPad, &UIScene_InGamePlayerOptionsMenu::OnPlayerChanged, this);

#ifdef __PSVITA__
	ui.TouchBoxRebuild(this);
#endif
}

wstring UIScene_InGamePlayerOptionsMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"InGamePlayerOptionsSplit";
	}
	else
	{
		return L"InGamePlayerOptions";
	}
}

void UIScene_InGamePlayerOptionsMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_InGamePlayerOptionsMenu::handleReload()
{
	UIScene::handleReload();

	INetworkPlayer *localPlayer = g_NetworkManager.GetLocalPlayerByUserIndex( m_iPad );
	INetworkPlayer *editingPlayer = g_NetworkManager.GetPlayerBySmallId(m_networkSmallId);

	bool trustPlayers = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
	bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
	m_editingSelf = (localPlayer != nullptr && localPlayer == editingPlayer);

	if( m_editingSelf || trustPlayers || editingPlayer->IsHost())
	{
		removeControl( &m_checkboxes[eControl_BuildAndMine], true );
		removeControl( &m_checkboxes[eControl_UseDoorsAndSwitches], true );
		removeControl( &m_checkboxes[eControl_UseContainers], true );
		removeControl( &m_checkboxes[eControl_AttackPlayers], true );
		removeControl( &m_checkboxes[eControl_AttackAnimals], true );
	}

	if(m_editingSelf)
	{
#ifndef _DEBUG //(defined(_CONTENT_PACKAGE) || defined(_FINAL_BUILD) && !defined(_DEBUG_MENUS_ENABLED))
		removeControl( &m_checkboxes[eControl_Op], true );
#endif
		
		removeControl( &m_buttonKick, true );
		removeControl( &m_checkboxes[eControl_CheatTeleport], true );

		if(cheats)
		{
			bool inCreativeMode = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode) != 0;
			if(inCreativeMode)
			{
				removeControl( &m_checkboxes[eControl_HostFly], true );
				removeControl( &m_checkboxes[eControl_HostHunger], true );
			}
		}
		else
		{
			removeControl( &m_checkboxes[eControl_HostInvisible], true );
			removeControl( &m_checkboxes[eControl_HostFly], true );
			removeControl( &m_checkboxes[eControl_HostHunger], true );
		}
	}
	else
	{
		if(!localPlayer->IsHost())
		{
			removeControl( &m_checkboxes[eControl_Op], true );
		}

		if(localPlayer->IsHost() && cheats )
		{

			bool inCreativeMode = Player::getPlayerGamePrivilege(m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode) != 0;
			if(inCreativeMode)
			{
				removeControl( &m_checkboxes[eControl_HostFly], true );
				removeControl( &m_checkboxes[eControl_HostHunger], true );
			}
		}
		else
		{
			removeControl( &m_checkboxes[eControl_HostInvisible], true );
			removeControl( &m_checkboxes[eControl_HostFly], true );
			removeControl( &m_checkboxes[eControl_HostHunger], true );
			removeControl( &m_checkboxes[eControl_CheatTeleport], true );
		}


		// Can only kick people if they are not local, and not local to the host
		if(editingPlayer->IsLocal() == TRUE || editingPlayer->IsSameSystem(g_NetworkManager.GetHostPlayer()) == TRUE)
		{
			removeControl( &m_buttonKick, true );
		}
	}

	short colourIndex = app.GetPlayerColour( m_networkSmallId );
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = colourIndex;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetPlayerIcon , 1 , value );
}

void UIScene_InGamePlayerOptionsMenu::tick()
{
	UIScene::tick();

	if(m_bShouldNavBack)
	{
		m_bShouldNavBack = false;
		ui.NavigateBack(m_iPad);
	}
}

void UIScene_InGamePlayerOptionsMenu::handleDestroy()
{
	g_NetworkManager.UnRegisterPlayerChangedCallback(m_iPad, &UIScene_InGamePlayerOptionsMenu::OnPlayerChanged, this);
}

void UIScene_InGamePlayerOptionsMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");

	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);
	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			bool trustPlayers = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
			bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
			if(m_editingSelf)
			{
#ifndef _DEBUG // (defined(_CONTENT_PACKAGE) || defined(_FINAL_BUILD) && !defined(_DEBUG_MENUS_ENABLED))
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
				if(player->connection)
				{
					player->connection->send(std::make_shared<PlayerInfoPacket>(m_networkSmallId, -1, m_playerPrivileges));
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

void UIScene_InGamePlayerOptionsMenu::handlePress(F64 controlId, F64 childId)
{
	switch(static_cast<int>(controlId))
	{
	case eControl_Kick:
		{
			BYTE *smallId = new BYTE();
			*smallId = m_networkSmallId;
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;

			ui.RequestAlertMessage(IDS_UNLOCK_KICK_PLAYER_TITLE, IDS_UNLOCK_KICK_PLAYER, uiIDA, 2, m_iPad,&UIScene_InGamePlayerOptionsMenu::KickPlayerReturned,smallId);
		}
		break;
	};
}

int UIScene_InGamePlayerOptionsMenu::KickPlayerReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	BYTE smallId = *static_cast<BYTE *>(pParam);
	delete pParam;

	if(result==C4JStorage::EMessage_ResultAccept)
	{		
		Minecraft *pMinecraft = Minecraft::GetInstance();
		shared_ptr<MultiplayerLocalPlayer> localPlayer = pMinecraft->localplayers[iPad];
		if(localPlayer->connection)
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

void UIScene_InGamePlayerOptionsMenu::OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving)
{
	app.DebugPrintf("UIScene_InGamePlayerOptionsMenu::OnPlayerChanged");
	UIScene_InGamePlayerOptionsMenu *scene = static_cast<UIScene_InGamePlayerOptionsMenu *>(callbackParam);

	UIScene_InGameInfoMenu *infoScene = static_cast<UIScene_InGameInfoMenu *>(scene->getBackScene());
	if(infoScene != nullptr) UIScene_InGameInfoMenu::OnPlayerChanged(infoScene,pPlayer,leaving);

	if(leaving && pPlayer != nullptr && pPlayer->GetSmallId() == scene->m_networkSmallId)
	{
		scene->m_bShouldNavBack = true;
	}
}

void UIScene_InGamePlayerOptionsMenu::resetCheatCheckboxes()
{
	bool isModerator = m_checkboxes[eControl_Op].IsChecked();
	//bool cheatsEnabled  = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;

	if (!m_editingSelf)
	{
		m_checkboxes[eControl_HostInvisible].SetEnable(isModerator);
		m_checkboxes[eControl_HostFly].SetEnable(isModerator);
		m_checkboxes[eControl_HostHunger].SetEnable(isModerator);
		m_checkboxes[eControl_CheatTeleport].SetEnable(isModerator);		
	}
}

void UIScene_InGamePlayerOptionsMenu::handleCheckboxToggled(F64 controlId, bool selected)
{
	switch(static_cast<int>(controlId))
	{
	case eControl_Op:
		// flag that the moderator state has changed
		//resetCheatCheckboxes();
		break;
	}
}

void UIScene_InGamePlayerOptionsMenu::handleTimerComplete(int id)
{
	switch(id)
	{
	case CHECKBOXES_TIMER_ID:
		{
			bool bIsModerator = m_checkboxes[eControl_Op].IsChecked();
			if(m_bModeratorState!=bIsModerator)
			{
				m_bModeratorState=bIsModerator;
				resetCheatCheckboxes();
			}
		}
		break;
	}
}
