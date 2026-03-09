#include "stdafx.h"
#include "UI.h"
#include "UIScene_PauseMenu.h"
#include "..\..\MinecraftServer.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\TexturePack.h"
#include "..\..\DLCTexturePack.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#ifdef __ORBIS__
#include <error_dialog.h>
#endif

#ifdef _DURANGO
#include "..\..\Durango\Leaderboards\DurangoStatsDebugger.h"
#endif

#ifdef __PSVITA__
#include "PSVita\Network\SonyCommerce_Vita.h"
#endif

#if defined __PS3__ || defined __ORBIS__
#define USE_SONY_REMOTE_STORAGE
#endif 

UIScene_PauseMenu::UIScene_PauseMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	m_bIgnoreInput=false;
	m_eAction=eAction_None;

	m_buttons[BUTTON_PAUSE_RESUMEGAME].init(app.GetString(IDS_RESUME_GAME),BUTTON_PAUSE_RESUMEGAME);
	m_buttons[BUTTON_PAUSE_HELPANDOPTIONS].init(app.GetString(IDS_HELP_AND_OPTIONS),BUTTON_PAUSE_HELPANDOPTIONS);
	m_buttons[BUTTON_PAUSE_LEADERBOARDS].init(app.GetString(IDS_LEADERBOARDS),BUTTON_PAUSE_LEADERBOARDS);
#ifdef _DURANGO
	m_buttons[BUTTON_PAUSE_XBOXHELP].init(app.GetString(IDS_XBOX_HELP_APP), BUTTON_PAUSE_XBOXHELP);
#else
	m_buttons[BUTTON_PAUSE_ACHIEVEMENTS].init(app.GetString(IDS_ACHIEVEMENTS),BUTTON_PAUSE_ACHIEVEMENTS);
#endif
#if defined(_XBOX_ONE) || defined(__ORBIS__)
	m_bTrialTexturePack = false;
	if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
	{
		TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
		DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

		m_pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();

		if(!m_pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
		{
			m_bTrialTexturePack = true;
		}
	}

	// 4J-TomK - check for all possible labels being fed into BUTTON_PAUSE_SAVEGAME (Bug 163775)
	// this has to be done before button initialisation!
	wchar_t saveButtonLabels[2][256];
	swprintf( saveButtonLabels[0], 256, L"%ls", app.GetString( IDS_SAVE_GAME ));
	swprintf( saveButtonLabels[1], 256, L"%ls", app.GetString( IDS_DISABLE_AUTOSAVE ));
	m_buttons[BUTTON_PAUSE_SAVEGAME].setAllPossibleLabels(2,saveButtonLabels);

	if(app.GetGameHostOption(eGameHostOption_DisableSaving) || m_bTrialTexturePack)
	{
		m_savesDisabled = true;
		m_buttons[BUTTON_PAUSE_SAVEGAME].init(app.GetString(IDS_SAVE_GAME),BUTTON_PAUSE_SAVEGAME);
	}
	else
	{
		m_savesDisabled = false;
		m_buttons[BUTTON_PAUSE_SAVEGAME].init(app.GetString(IDS_DISABLE_AUTOSAVE),BUTTON_PAUSE_SAVEGAME);
	}
#else
	m_buttons[BUTTON_PAUSE_SAVEGAME].init(app.GetString(IDS_SAVE_GAME),BUTTON_PAUSE_SAVEGAME);
#endif
	m_buttons[BUTTON_PAUSE_EXITGAME].init(app.GetString(IDS_EXIT_GAME),BUTTON_PAUSE_EXITGAME);

	if(!ProfileManager.IsFullVersion())
	{
		// hide the trial timer
		ui.ShowTrialTimer(false);
	}

	updateControlsVisibility();

	doHorizontalResizeCheck();

	// get rid of the quadrant display if it's on
	ui.HidePressStart();

#if TO_BE_IMPLEMENTED
	XuiSetTimer(m_hObj,IGNORE_KEYPRESS_TIMERID,IGNORE_KEYPRESS_TIME);
#endif

	if( g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 )
	{
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)TRUE);
	}

	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_PauseMenu, 0);
	TelemetryManager->RecordPauseOrInactive(m_iPad);

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft != nullptr && pMinecraft->localgameModes[iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[iPad]);

		// This just allows it to be shown
		gameMode->getTutorial()->showTutorialPopup(false);
	}
	m_bErrorDialogRunning = false;
}

UIScene_PauseMenu::~UIScene_PauseMenu()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft != nullptr && pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);

		// This just allows it to be shown
		gameMode->getTutorial()->showTutorialPopup(true);
	}

	m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);
	m_parentLayer->showComponent(m_iPad,eUIComponent_MenuBackground,false);
	m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
}

wstring UIScene_PauseMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"PauseMenuSplit";
	}
	else
	{
		return L"PauseMenu";
	}
}

void UIScene_PauseMenu::tick()
{
	UIScene::tick();

#ifdef __PSVITA__
	// 4J-MGH - Need to check for installed DLC here, as we delay the installation of the key file on Vita
	if(!app.DLCInstallProcessCompleted()) app.StartInstallDLCProcess(0);
#endif


#if defined _XBOX_ONE || defined __ORBIS__
	if(!m_bTrialTexturePack && m_savesDisabled != (app.GetGameHostOption(eGameHostOption_DisableSaving) != 0) && ProfileManager.GetPrimaryPad() == m_iPad )
	{
		// We show the save button if saves are disabled as this lets us show a prompt to enable them (via purchasing a texture pack)
		if( app.GetGameHostOption(eGameHostOption_DisableSaving) )
		{
			m_savesDisabled = true;
			m_buttons[BUTTON_PAUSE_SAVEGAME].setLabel( app.GetString(IDS_SAVE_GAME) );
		}
		else
		{
			m_savesDisabled = false;
			m_buttons[BUTTON_PAUSE_SAVEGAME].setLabel( app.GetString(IDS_DISABLE_AUTOSAVE) );
		}
	}
#endif

#ifdef __ORBIS__
	// Process the error dialog (for a patch being available)
	if(m_bErrorDialogRunning)
	{	
		SceErrorDialogStatus stat = sceErrorDialogUpdateStatus();
		if( stat == SCE_ERROR_DIALOG_STATUS_FINISHED ) 
		{
			sceErrorDialogTerminate();
			m_bErrorDialogRunning=false;
		}
	}
#endif
}

void UIScene_PauseMenu::updateTooltips()
{
	bool bUserisClientSide = ProfileManager.IsSignedInLive(m_iPad);
	bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==m_iPad);

#ifdef _XBOX_ONE
	bool bDisplayBanTip = !g_NetworkManager.IsLocalGame() && !bIsisPrimaryHost && !ProfileManager.IsGuest(m_iPad);
#endif

	int iY = -1;
#if defined __PS3__ || defined __ORBIS__
	if(m_iPad == ProfileManager.GetPrimaryPad() ) iY = IDS_TOOLTIPS_GAME_INVITES;
#endif
	int iRB = -1;
	int iX = -1;

	if(ProfileManager.IsFullVersion())
	{
		if(StorageManager.GetSaveDisabled())
		{
			iX = bIsisPrimaryHost?IDS_TOOLTIPS_SELECTDEVICE:-1;
#ifdef _XBOX_ONE
			iRB = bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1;
#endif
			if( CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide )
			{
#ifndef __PS3__
				iY = IDS_TOOLTIPS_SHARE;
#endif
			}		
		}
		else
		{
			iX = bIsisPrimaryHost?IDS_TOOLTIPS_CHANGEDEVICE:-1;
#ifdef _XBOX_ONE
			iRB = bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1;
#endif
			if( CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide)
			{
#ifndef __PS3__
				iY = IDS_TOOLTIPS_SHARE;
#endif
			}	
		}
	}
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,iX,iY, -1,-1,-1,iRB);
}

void UIScene_PauseMenu::updateComponents()
{
	m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);
	m_parentLayer->showComponent(m_iPad,eUIComponent_MenuBackground,true);

	if( app.GetLocalPlayerCount() == 1 ) m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
	else m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
}

void UIScene_PauseMenu::handlePreReload()
{
#if defined _XBOX_ONE || defined __ORBIS__
	if(ProfileManager.GetPrimaryPad() == m_iPad)
	{
		// 4J-TomK - check for all possible labels being fed into BUTTON_PAUSE_SAVEGAME (Bug 163775)
		// this has to be done before button initialisation!
		wchar_t saveButtonLabels[2][256];
		swprintf( saveButtonLabels[0], 256, L"%ls", app.GetString( IDS_SAVE_GAME ));
		swprintf( saveButtonLabels[1], 256, L"%ls", app.GetString( IDS_DISABLE_AUTOSAVE ));
		m_buttons[BUTTON_PAUSE_SAVEGAME].setAllPossibleLabels(2,saveButtonLabels);
	}
#endif
}

void UIScene_PauseMenu::handleReload()
{
	updateTooltips();
	updateControlsVisibility();	

#if defined _XBOX_ONE || defined __ORBIS__
	if(ProfileManager.GetPrimaryPad() == m_iPad)
	{
		// We show the save button if saves are disabled as this lets us show a prompt to enable them (via purchasing a texture pack)
		if( app.GetGameHostOption(eGameHostOption_DisableSaving) || m_bTrialTexturePack )
		{
			m_savesDisabled = true;
			m_buttons[BUTTON_PAUSE_SAVEGAME].setLabel( app.GetString(IDS_SAVE_GAME) );
		}
		else
		{
			m_savesDisabled = false;
			m_buttons[BUTTON_PAUSE_SAVEGAME].setLabel( app.GetString(IDS_DISABLE_AUTOSAVE) );
		}
	}
#endif

	doHorizontalResizeCheck();
}

void UIScene_PauseMenu::updateControlsVisibility()
{
	// are we the primary player?
	// 4J-PB - fix for 7844 & 7845 - 
	// TCR # 128:  XLA Pause Menu:   When in a multiplayer game as a client the Pause Menu does not have a Leaderboards option.
	// TCR # 128:  XLA Pause Menu:   When in a multiplayer game as a client the Pause Menu does not have an Achievements option.
	if(ProfileManager.GetPrimaryPad()==m_iPad) // && g_NetworkManager.IsHost()) 
	{
		// are we in splitscreen?
		// how many local players do we have?
		if( app.GetLocalPlayerCount()>1 )
		{
			// Hide the BUTTON_PAUSE_LEADERBOARDS and BUTTON_PAUSE_ACHIEVEMENTS
			removeControl( &m_buttons[BUTTON_PAUSE_LEADERBOARDS], false );
#ifndef _XBOX_ONE
			removeControl( &m_buttons[BUTTON_PAUSE_ACHIEVEMENTS], false );
#endif
		}
#ifdef __PSVITA__
		// MGH added - remove leaderboards in adhoc
		if(CGameNetworkManager::usingAdhocMode())
		{
			removeControl( &m_buttons[BUTTON_PAUSE_LEADERBOARDS], false );
		}
#endif

		if( !g_NetworkManager.IsHost() )
		{
			// Hide the BUTTON_PAUSE_SAVEGAME
			removeControl( &m_buttons[BUTTON_PAUSE_SAVEGAME], false );
		}
	}
	else
	{
		// Hide the BUTTON_PAUSE_LEADERBOARDS, BUTTON_PAUSE_ACHIEVEMENTS and BUTTON_PAUSE_SAVEGAME
		removeControl( &m_buttons[BUTTON_PAUSE_LEADERBOARDS], false );
#ifndef _XBOX_ONE
		removeControl( &m_buttons[BUTTON_PAUSE_ACHIEVEMENTS], false );
#endif
		removeControl( &m_buttons[BUTTON_PAUSE_SAVEGAME], false );
	}

	// is saving disabled?
	if(StorageManager.GetSaveDisabled())
	{
#ifdef _XBOX
		// disable save button
		m_buttons[BUTTON_PAUSE_SAVEGAME].setEnable(false);
#endif
	}

#if defined(__PS3__)  || defined (__PSVITA__) || defined(__ORBIS__) 
	// We don't have a way to display trophies/achievements, so remove the button, and we're allowed to not have it on Xbox One
	removeControl( &m_buttons[BUTTON_PAUSE_ACHIEVEMENTS], false );
#endif

}

void UIScene_PauseMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bIgnoreInput)
	{
		return;
	}

	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

#ifdef _XBOX_ONE
	bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==iPad);
	bool bDisplayBanTip = !g_NetworkManager.IsLocalGame() && !bIsisPrimaryHost && !ProfileManager.IsGuest(iPad);
#endif

	switch(key)
	{
#ifdef _DURANGO
	case ACTION_MENU_GTC_RESUME:
#endif
#if defined(__PS3__) // not for Orbis - we want to use the pause menu (touchpad press) to select a menu item
	case ACTION_MENU_PAUSEMENU:
#endif
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
#ifdef _DURANGO
			//DurangoStatsDebugger::PrintStats(iPad);
#endif

			if( iPad == ProfileManager.GetPrimaryPad() && g_NetworkManager.IsLocalGame() )
			{
				app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)FALSE);
			}

			ui.PlayUISFX(eSFX_Back);
			navigateBack();
			if(!ProfileManager.IsFullVersion())
			{
				ui.ShowTrialTimer(true);
			}
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		if(pressed)
		{
			sendInputToMovie(key, repeat, pressed, released);
		}
		break;

#if TO_BE_IMPLEMENTED
	case VK_PAD_X:
		// Change device
		if(bIsisPrimaryHost)
		{	
			// we need a function to deal with the return from this - if it changes, we need to update the pause menu and tooltips
			// Fix for #12531 - TCR 001: BAS Game Stability: When a player selects to change a storage 
			// device, and repeatedly backs out of the SD screen, disconnects from LIVE, and then selects a SD, the title crashes.
			m_bIgnoreInput=true;

			StorageManager.SetSaveDevice(&UIScene_PauseMenu::DeviceSelectReturned,this,true);
		}
		rfHandled = TRUE;
		break;
#endif

	case ACTION_MENU_Y:
		{
			
#if defined(__PS3__) || defined(__ORBIS__)
		if(pressed && iPad == ProfileManager.GetPrimaryPad())
		{
#ifdef __ORBIS__
			// If a patch is available, can't view invites
			if (CheckForPatch()) break;
#endif

			// Are we offline?
			if(!ProfileManager.IsSignedInLive(iPad))
			{
				m_eAction=eAction_ViewInvitesPSN;
#ifdef __ORBIS__
				int npAvailability = ProfileManager.getNPAvailability(iPad);
				if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
				{
					// 4J Stu - This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive is the npAvailability isn't SCE_OK
					UINT uiIDA[1];
					uiIDA[0]=IDS_OK;
					ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad);
				}
				else
					// Determine why they're not "signed in live"
					if (ProfileManager.isSignedInPSN(iPad))
					{
						// Signed in to PSN but not connected (no internet access)
						assert(!ProfileManager.isConnectedToPSN(iPad));

						UINT uiIDA[1];
						uiIDA[0] = IDS_OK;
						ui.RequestErrorMessage( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, iPad);
					}
					else
					{		
						// Not signed in to PSN
						UINT uiIDA[1];
						uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
						ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, (LPVOID)GetCallbackUniqueId() );
					}				
#else // __PS3__
					// get them to sign in to online
				UINT uiIDA[1];
					uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
				ui.RequestAlertMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, (LPVOID)GetCallbackUniqueId() );
#endif
				}
				else
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
#if TO_BE_IMPLEMENTED
			if(bUserisClientSide)
			{			
				// 4J Stu - Added check in 1.8.2 bug fix (TU6) to stop repeat key presses
				bool bCanScreenshot = true;
				for(int j=0; j < XUSER_MAX_COUNT;++j)
				{
					if(app.GetXuiAction(j) == eAppAction_SocialPostScreenshot)
					{
						bCanScreenshot = false;
						break;
					}
				}
				if(bCanScreenshot) app.SetAction(pInputData->UserIndex,eAppAction_SocialPost);
			}
			rfHandled = TRUE;
#endif
#endif // __PS3__
		}
		break;
#ifdef _XBOX_ONE
	case ACTION_MENU_RIGHT_SCROLL:
		if( bDisplayBanTip )
		{
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestAlertMessage(IDS_ACTION_BAN_LEVEL_TITLE, IDS_ACTION_BAN_LEVEL_DESCRIPTION, uiIDA, 2, iPad,&UIScene_PauseMenu::BanGameDialogReturned,(LPVOID)GetCallbackUniqueId() );

			//rfHandled = TRUE;
		}
		break;
#endif
	}
}

void UIScene_PauseMenu::handlePress(F64 controlId, F64 childId)
{
	if(m_bIgnoreInput) return;

	switch(static_cast<int>(controlId))
	{
	case BUTTON_PAUSE_RESUMEGAME:
		if( m_iPad == ProfileManager.GetPrimaryPad() && g_NetworkManager.IsLocalGame() )
		{
			app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)FALSE);
		}
		navigateBack();
		break;
	case BUTTON_PAUSE_LEADERBOARDS:
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;

			//4J Gordon: Being used for the leaderboards proper now
			// guests can't look at leaderboards
			if(ProfileManager.IsGuest(m_iPad))
			{
				ui.RequestAlertMessage(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad());
			}
			else if(!ProfileManager.IsSignedInLive(m_iPad))
			{
#ifdef __ORBIS__
				// If a patch is available, can't show leaderboard
				if (CheckForPatch()) break;

				// Check for content restricted user
				// Update error code
				int errorCode = ProfileManager.getNPAvailability(m_iPad);

				// Check if PSN is unavailable because of age restriction
				if (errorCode == SCE_NP_ERROR_AGE_RESTRICTION)
				{
					UINT uiIDA[1];
					uiIDA[0] = IDS_CONFIRM_OK;
					ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, m_iPad);

					break;;
				}

#endif

#if defined __PS3__ || __PSVITA__
				// get them to sign in to online
				m_eAction=eAction_ViewLeaderboardsPSN;
				UINT uiIDA[1];
				uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
				ui.RequestAlertMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::MustSignInReturnedPSN,(LPVOID)GetCallbackUniqueId() );
#elif defined(__ORBIS__)
				m_eAction=eAction_ViewLeaderboardsPSN;
				int npAvailability = ProfileManager.getNPAvailability(m_iPad);
				if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
				{
					// 4J Stu - This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive is the npAvailability isn't SCE_OK
					UINT uiIDA[1];
					uiIDA[0]=IDS_OK;
					ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, m_iPad);
				}
				else
					// Determine why they're not "signed in live"
					if (ProfileManager.isSignedInPSN(m_iPad))
					{
						// Signed in to PSN but not connected (no internet access)

						// Id
						assert(!ProfileManager.isConnectedToPSN(m_iPad));

						UINT uiIDA[1];
						uiIDA[0] = IDS_OK;
						ui.RequestErrorMessage( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, m_iPad);
					}
					else
					{		
						// Not signed in to PSN
						UINT uiIDA[1];
						uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
						ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, m_iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, (LPVOID)GetCallbackUniqueId() );
					}
#else
			UINT uiIDA[1] = { IDS_OK };
			ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1, m_iPad);
#endif
			}
			else
			{	
				bool bContentRestricted=false;
#if defined(__PS3__) || defined(__PSVITA__)
				ProfileManager.GetChatAndContentRestrictions(m_iPad,true,nullptr,&bContentRestricted,nullptr);
#endif
				if(bContentRestricted)
				{
#if !(defined(_XBOX) || defined(_WINDOWS64) || defined(_XBOX_ONE)) // 4J Stu - Temp to get the win build running, but so we check this for other platforms
					// you can't see leaderboards
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, m_iPad);
#endif
				}
				else
				{
					ui.NavigateToScene(m_iPad, eUIScene_LeaderboardsMenu);
				}
			}
		}
		break;
#ifdef _DURANGO
	case BUTTON_PAUSE_XBOXHELP:
		{
			// 4J: Launch the crummy xbox help application.
			WXS::User^ user = ProfileManager.GetUser(m_iPad);
			Windows::Xbox::ApplicationModel::Help::Show(user);
		}
		break;
#elif TO_BE_IMPLEMENTED
	case BUTTON_PAUSE_ACHIEVEMENTS:

		// guests can't look at achievements
		if(ProfileManager.IsGuest(pNotifyPressData->UserIndex))
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;
			ui.RequestAlertMessage(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad());
		}
		else
		{
			XShowAchievementsUI( pNotifyPressData->UserIndex );
		}
		break;
#endif

	case BUTTON_PAUSE_HELPANDOPTIONS:
		ui.NavigateToScene(m_iPad,eUIScene_HelpAndOptionsMenu);	
		break;
	case BUTTON_PAUSE_SAVEGAME:
		PerformActionSaveGame();
		break;
	case BUTTON_PAUSE_EXITGAME:
		{
			Minecraft *pMinecraft = Minecraft::GetInstance();
			// Check if it's the trial version
			if(ProfileManager.IsFullVersion())
			{	
				UINT uiIDA[3];

				// is it the primary player exiting?
				if(m_iPad==ProfileManager.GetPrimaryPad())
				{
					int playTime = -1;
					if( pMinecraft->localplayers[m_iPad] != nullptr )
					{
						playTime = static_cast<int>(pMinecraft->localplayers[m_iPad]->getSessionTimer());
					}

#if defined(_XBOX_ONE) || defined(__ORBIS__)
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_CONFIRM_OK;

					if(g_NetworkManager.IsHost() && StorageManager.GetSaveDisabled())
					{
						uiIDA[0]=IDS_CONFIRM_CANCEL;
						uiIDA[1]=IDS_EXIT_GAME_SAVE;
						uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

						if(g_NetworkManager.GetPlayerCount()>1)
						{
							ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3, m_iPad,&UIScene_PauseMenu::ExitGameSaveDialogReturned, (LPVOID)GetCallbackUniqueId());
						}
						else
						{
							ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, m_iPad,&UIScene_PauseMenu::ExitGameSaveDialogReturned, (LPVOID)GetCallbackUniqueId());
						}
					}
					else if(g_NetworkManager.IsHost() && g_NetworkManager.GetPlayerCount()>1)
					{
						ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned, (LPVOID)GetCallbackUniqueId());
					}
					else
					{
						ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned, (LPVOID)GetCallbackUniqueId());
					}
#else
					if(StorageManager.GetSaveDisabled())
					{
						uiIDA[0]=IDS_CONFIRM_CANCEL;
						uiIDA[1]=IDS_CONFIRM_OK;
						ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned, (LPVOID)GetCallbackUniqueId());
					}
					else
					{
						if( g_NetworkManager.IsHost() )
						{	
							uiIDA[0]=IDS_CONFIRM_CANCEL;
							uiIDA[1]=IDS_EXIT_GAME_SAVE;
							uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

							if(g_NetworkManager.GetPlayerCount()>1)
							{
								ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3, m_iPad,&UIScene_PauseMenu::ExitGameSaveDialogReturned, (LPVOID)GetCallbackUniqueId());
							}
							else
							{
								ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, m_iPad,&UIScene_PauseMenu::ExitGameSaveDialogReturned, (LPVOID)GetCallbackUniqueId());
							}
						}
						else
						{
							uiIDA[0]=IDS_CONFIRM_CANCEL;
							uiIDA[1]=IDS_CONFIRM_OK;

							ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned, (LPVOID)GetCallbackUniqueId());
						}
					}
#endif
				}
				else
				{
					int playTime = -1;
					if( pMinecraft->localplayers[m_iPad] != nullptr )
					{
						playTime = static_cast<int>(pMinecraft->localplayers[m_iPad]->getSessionTimer());
					}

					TelemetryManager->RecordLevelExit(m_iPad, eSen_LevelExitStatus_Exited);


					// just exit the player
					app.SetAction(m_iPad,eAppAction_ExitPlayer);
				}		
			}
			else
			{
				// is it the primary player exiting?
				if(m_iPad==ProfileManager.GetPrimaryPad())
				{
					int playTime = -1;
					if( pMinecraft->localplayers[m_iPad] != nullptr )
					{
						playTime = static_cast<int>(pMinecraft->localplayers[m_iPad]->getSessionTimer());
					}	

					// adjust the trial time played
					ui.ReduceTrialTimerValue();

					// exit the level
					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_CONFIRM_OK;
					ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned, (LPVOID)GetCallbackUniqueId());

				}
				else
				{
					int playTime = -1;
					if( pMinecraft->localplayers[m_iPad] != nullptr )
					{
						playTime = static_cast<int>(pMinecraft->localplayers[m_iPad]->getSessionTimer());
					}

					TelemetryManager->RecordLevelExit(m_iPad, eSen_LevelExitStatus_Exited);

					// just exit the player
					app.SetAction(m_iPad,eAppAction_ExitPlayer);
				}
			}
		}
		break;
	}
}

void UIScene_PauseMenu::PerformActionSaveGame()
{
	// is the player trying to save in the trial version?
	if(!ProfileManager.IsFullVersion())
	{
#ifdef __ORBIS__
		// If a patch is available, can't buy full game
		if (CheckForPatch()) return;
#endif

		// Unlock the full version?
		if(!ProfileManager.IsSignedInLive(m_iPad))
		{
#if defined(__PS3__) || defined (__PSVITA__)
			m_eAction=eAction_SaveGamePSN;
			UINT uiIDA[2];
			uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestAlertMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 2, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::MustSignInReturnedPSN,(LPVOID)GetCallbackUniqueId());
#elif defined(__ORBIS__)
			m_eAction=eAction_SaveGamePSN;
			int npAvailability = ProfileManager.getNPAvailability(m_iPad);
			if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
			{
				// 4J Stu - This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive is the npAvailability isn't SCE_OK
				UINT uiIDA[1];
				uiIDA[0]=IDS_OK;
				ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, m_iPad);
			}
			else
				// Determine why they're not "signed in live"
				if (ProfileManager.isSignedInPSN(m_iPad))
				{
					// Signed in to PSN but not connected (no internet access)
					assert(!ProfileManager.isConnectedToPSN(m_iPad));

					UINT uiIDA[1];
					uiIDA[0] = IDS_OK;
					ui.RequestErrorMessage( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, m_iPad);
				}
				else
				{		
					// Not signed in to PSN
					UINT uiIDA[1];
					uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
					ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, m_iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, (LPVOID)GetCallbackUniqueId());
				}
#endif
		}
		else
		{
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			ui.RequestAlertMessage(IDS_UNLOCK_TITLE, IDS_UNLOCK_TOSAVE_TEXT, uiIDA, 2,m_iPad,&UIScene_PauseMenu::UnlockFullSaveReturned,(LPVOID)GetCallbackUniqueId());
		}

		return;
	}

	// 4J-PB - Is the player trying to save but they are using a trial texturepack ?
	if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
	{
		TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
		DLCTexturePack *pDLCTexPack=static_cast<DLCTexturePack *>(tPack);

		m_pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();

		if(!m_pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
		{					
			// upsell
#ifdef _XBOX
			ULONGLONG ullOfferID_Full;
			// get the dlc texture pack
			DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

			app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullOfferID_Full);

			// tell sentient about the upsell of the full version of the texture pack
			TelemetryManager->RecordUpsellPresented(m_iPad, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;

			// Give the player a warning about the trial version of the texture pack
#ifdef __PSVITA__
			if(app.DLCInstallProcessCompleted() && !SonyCommerce_Vita::getDLCUpgradePending())  // MGH - devtrack #5861 On vita it can take a bit after the install has finished to register the purchase, so make sure we don't end up asking to purchase again
#endif
			{
				ui.RequestAlertMessage(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, m_iPad,&UIScene_PauseMenu::WarningTrialTexturePackReturned,(LPVOID)GetCallbackUniqueId());
			}

			return;					
		}
		else
		{
			m_bTrialTexturePack = false;
		}
	}

	// does the save exist?
	bool bSaveExists;
	C4JStorage::ESaveGameState result=StorageManager.DoesSaveExist(&bSaveExists);

#ifdef _XBOX
	if(result == C4JStorage::ELoadGame_DeviceRemoved)
	{
		// this will be a tester trying to be clever
		UINT uiIDA[2];
		uiIDA[0]=IDS_SELECTANEWDEVICE;
		uiIDA[1]=IDS_NODEVICE_DECLINE;

		ui.RequestAlertMessage(IDS_STORAGEDEVICEPROBLEM_TITLE, IDS_FAILED_TO_LOADSAVE_TEXT, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::DeviceRemovedDialogReturned,(LPVOID)GetCallbackUniqueId());
	}
	else
#endif
	{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
		if(!m_savesDisabled)
		{
			UINT uiIDA[2];
			uiIDA[0]=IDS_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestAlertMessage(IDS_TITLE_DISABLE_AUTOSAVE, IDS_CONFIRM_DISABLE_AUTOSAVE, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::DisableAutosaveDialogReturned,(LPVOID)GetCallbackUniqueId());
		}
		else
#endif
			// we need to ask if they are sure they want to overwrite the existing game
			if(bSaveExists)
			{
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::SaveGameDialogReturned,(LPVOID)GetCallbackUniqueId());
			}
			else
			{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_TITLE_ENABLE_AUTOSAVE, IDS_CONFIRM_ENABLE_AUTOSAVE, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::EnableAutosaveDialogReturned,(LPVOID)GetCallbackUniqueId());
#else
				// flag a app action of save game
				app.SetAction(m_iPad,eAppAction_SaveGame);
#endif
			}
	}
}

void UIScene_PauseMenu::ShowScene(bool show)
{
	app.DebugPrintf("UIScene_PauseMenu::ShowScene is not implemented\n");
}

void UIScene_PauseMenu::HandleDLCInstalled()
{
	// mounted DLC may have changed
	if(app.StartInstallDLCProcess(m_iPad)==false)
	{
		// not doing a mount, so re-enable input
		//m_bIgnoreInput=false;
		app.DebugPrintf("UIScene_PauseMenu::HandleDLCInstalled - m_bIgnoreInput false\n");
	}
	else
	{
		// 4J-PB - Somehow, on th edisc build, we get in here, but don't call HandleDLCMountingComplete, so input locks up
		//m_bIgnoreInput=true;
		app.DebugPrintf("UIScene_PauseMenu::HandleDLCInstalled - m_bIgnoreInput true\n");
	}
	// this will send a CustomMessage_DLCMountingComplete when done
}


void UIScene_PauseMenu::HandleDLCMountingComplete()
{	
	// check if we should display the save option

	//m_bIgnoreInput=false;
	app.DebugPrintf("UIScene_PauseMenu::HandleDLCMountingComplete - m_bIgnoreInput false \n");

	// 	if(ProfileManager.IsFullVersion())
	// 	{
	// 		bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==m_iPad);
	// 
	// 		if(bIsisPrimaryHost)
	// 		{
	// 			m_buttons[BUTTON_PAUSE_SAVEGAME].setEnable(true);
	// 		}
	// 	}
}

int UIScene_PauseMenu::UnlockFullSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedInLive(pMinecraft->player->GetXboxPad()))
		{
			// 4J-PB - need to check this user can access the store
#if defined(__PS3__) || defined(__PSVITA__)
			bool bContentRestricted;
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad());
			}
			else
#endif
			{
				ProfileManager.DisplayFullVersionPurchase(false,pMinecraft->player->GetXboxPad(),eSen_UpsellID_Full_Version_Of_Game);
			}
		}
	}
	else
	{
		//SentientManager.RecordUpsellResponded(iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID, eSen_UpsellOutcome_Declined);
	}

	return 0;
}

int UIScene_PauseMenu::SaveGame_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	UIScene_PauseMenu* pClass = static_cast<UIScene_PauseMenu *>(ui.GetSceneFromCallbackId((size_t)pParam));
	if(pClass) pClass->SetIgnoreInput(false);

	if(bContinue==true)
	{
		if(pClass) pClass->PerformActionSaveGame();
	}

	return 0;
}

#ifdef _XBOX_ONE
int UIScene_PauseMenu::BanGameDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		app.SetAction(iPad,eAppAction_BanLevel);
	}
	return 0;
}
#endif

#if defined(__PS3__)  || defined (__PSVITA__) || defined(__ORBIS__)
int UIScene_PauseMenu::MustSignInReturnedPSN(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)ui.GetSceneFromCallbackId((size_t)pParam);
	if(result==C4JStorage::EMessage_ResultAccept && pClass) 
	{
#ifdef __PS3__
		switch(pClass->m_eAction)
		{
		case eAction_ViewLeaderboardsPSN:
			SQRNetworkManager_PS3::AttemptPSNSignIn(&UIScene_PauseMenu::ViewLeaderboards_SignInReturned, pParam);
			break;
		case eAction_ViewInvitesPSN:
			SQRNetworkManager_PS3::AttemptPSNSignIn(&UIScene_PauseMenu::ViewInvites_SignInReturned, pParam);
			break;
		case eAction_SaveGamePSN:
			SQRNetworkManager_PS3::AttemptPSNSignIn(&UIScene_PauseMenu::SaveGame_SignInReturned, pParam);
			break;
		case eAction_BuyTexturePackPSN:
			SQRNetworkManager_PS3::AttemptPSNSignIn(&UIScene_PauseMenu::BuyTexturePack_SignInReturned, pParam);
			break;
		}
#elif defined __PSVITA__
		switch(pClass->m_eAction)
		{
		case eAction_ViewLeaderboardsPSN:
			//CD - Must force Ad-Hoc off if they want leaderboard PSN sign-in
			//Save settings change
			app.SetGameSettings(0, eGameSetting_PSVita_NetworkModeAdhoc, 0);
			//Force off
			CGameNetworkManager::setAdhocMode(false);
			//Now Sign-in
			SQRNetworkManager_Vita::AttemptPSNSignIn(&UIScene_PauseMenu::ViewLeaderboards_SignInReturned, pParam);
			break;
		case eAction_ViewInvitesPSN:
			SQRNetworkManager_Vita::AttemptPSNSignIn(&UIScene_PauseMenu::ViewInvites_SignInReturned, pParam);
			break;
		case eAction_SaveGamePSN:
			pClass->SetIgnoreInput(true);
			SQRNetworkManager_Vita::AttemptPSNSignIn(&UIScene_PauseMenu::SaveGame_SignInReturned, pParam, true);
			break;
		case eAction_BuyTexturePackPSN:
			SQRNetworkManager_Vita::AttemptPSNSignIn(&UIScene_PauseMenu::BuyTexturePack_SignInReturned, pParam);
			break;
		}
#else
		switch(pClass->m_eAction)
		{
		case eAction_ViewLeaderboardsPSN:
			SQRNetworkManager_Orbis::AttemptPSNSignIn(&UIScene_PauseMenu::ViewLeaderboards_SignInReturned, pClass, false, iPad);
			break;
		case eAction_ViewInvitesPSN:
			SQRNetworkManager_Orbis::AttemptPSNSignIn(&UIScene_PauseMenu::ViewInvites_SignInReturned, pClass, false, iPad);
			break;
		case eAction_SaveGamePSN:
			SQRNetworkManager_Orbis::AttemptPSNSignIn(&UIScene_PauseMenu::SaveGame_SignInReturned, pClass, false, iPad);
			break;
		case eAction_BuyTexturePackPSN:
			SQRNetworkManager_Orbis::AttemptPSNSignIn(&UIScene_PauseMenu::BuyTexturePack_SignInReturned, pClass, false, iPad);
			break;
		}
#endif
	}

	return 0;
}

int UIScene_PauseMenu::ViewLeaderboards_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)ui.GetSceneFromCallbackId((size_t)pParam);
	if(!pClass) return 0;

	if(bContinue==true)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;

		// guests can't look at leaderboards
		if(ProfileManager.IsGuest(pClass->m_iPad))
		{
			ui.RequestAlertMessage(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad());
		}
		else if(ProfileManager.IsSignedInLive(iPad))
		{
#ifndef __ORBIS__
			bool bContentRestricted=false;
			ProfileManager.GetChatAndContentRestrictions(pClass->m_iPad,true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				// you can't see leaderboards
				ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad());
			}
			else
#endif
			{
				ui.NavigateToScene(pClass->m_iPad, eUIScene_LeaderboardsMenu);
			}
		}
	}

	return 0;
}

int UIScene_PauseMenu::WarningTrialTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)ui.GetSceneFromCallbackId((size_t)pParam);

#ifdef __ORBIS__
	// If a patch is available, can't proceed 
	if (!pClass || pClass->CheckForPatch()) return 0;
#endif

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(!ProfileManager.IsSignedInLive(iPad))
		{
			if(pClass) pClass->m_eAction=eAction_SaveGamePSN;
#ifdef __ORBIS__// Check if PSN is unavailable because of age restriction
			int npAvailability = ProfileManager.getNPAvailability(iPad);
			if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
			{
				// 4J Stu - This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive is the npAvailability isn't SCE_OK
				UINT uiIDA[1];
				uiIDA[0]=IDS_OK;
				ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad);
			}
			else
				// Determine why they're not "signed in live"
				if (ProfileManager.isSignedInPSN(iPad))
				{
					// Signed in to PSN but not connected (no internet access)
					assert(!ProfileManager.isConnectedToPSN(iPad));

					UINT uiIDA[1];
					uiIDA[0] = IDS_OK;
					ui.RequestErrorMessage( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, iPad);
				}
				else
				{		
					UINT uiIDA[1];
					uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
					ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, pParam);
				}
#else // __PS3__
			// You're not signed in to PSN!
			UINT uiIDA[2];
			uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestAlertMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 2, iPad,&UIScene_PauseMenu::MustSignInReturnedPSN,pParam);
#endif
		}
		else
		{
#ifndef __ORBIS__
			// 4J-PB - need to check this user can access the store
			bool bContentRestricted=false;
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad);
			}
			else
#endif
			{
				// need to get info on the pack to see if the user has already downloaded it
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				// retrieve the store name for the skin pack
				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();
				const char *pchPackName=wstringtofilename(pDLCPack->getName());
				app.DebugPrintf("Texture Pack - %s\n",pchPackName);
				SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfo((char *)pchPackName);		

				if(pSONYDLCInfo!=nullptr)
				{
					char chName[42];
					char chKeyName[20];
					char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

					memset(chSkuID,0,SCE_NP_COMMERCE2_SKU_ID_LEN);
					// find the info on the skin pack
					// we have to retrieve the skuid from the store info, it can't be hardcoded since Sony may change it.
					// So we assume the first sku for the product is the one we want

					// MGH -  keyname in the DLC file is 16 chars long, but there's no space for a nullptr terminating char
					memset(chKeyName, 0, sizeof(chKeyName));
					strncpy(chKeyName, pSONYDLCInfo->chDLCKeyname, 16);

#ifdef __ORBIS__
					strcpy(chName, chKeyName);
#else
					sprintf(chName,"%s-%s",app.GetCommerceCategory(),chKeyName);
#endif
					app.GetDLCSkuIDFromProductList(chName,chSkuID);

					// 4J-PB - need to check for an empty store
#if defined __ORBIS__ || defined __PSVITA__ || defined __PS3__
					if(app.CheckForEmptyStore(iPad)==false)
#endif
					{
						if(app.DLCAlreadyPurchased(chSkuID))
						{
							app.DownloadAlreadyPurchased(chSkuID);
						}
						else
						{
							app.Checkout(chSkuID);	
						}
					}
				}
			}
		}
	}
#endif		//	

	return 0;
}

int UIScene_PauseMenu::BuyTexturePack_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	if(bContinue==true)
	{
		// Check if we're signed in to LIVE
		if(ProfileManager.IsSignedInLive(iPad))
		{
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)

#ifndef __ORBIS__
			// 4J-PB - need to check this user can access the store
			bool bContentRestricted=false;
			ProfileManager.GetChatAndContentRestrictions(iPad,true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad);
			}
			else
#endif
			{
				// need to get info on the pack to see if the user has already downloaded it
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				// retrieve the store name for the skin pack
				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();
				const char *pchPackName=wstringtofilename(pDLCPack->getName());
				app.DebugPrintf("Texture Pack - %s\n",pchPackName);
				SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfo((char *)pchPackName);		

				if(pSONYDLCInfo!=nullptr)
				{
					char chName[42];
					char chKeyName[20];
					char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

					memset(chSkuID,0,SCE_NP_COMMERCE2_SKU_ID_LEN);
					// find the info on the skin pack
					// we have to retrieve the skuid from the store info, it can't be hardcoded since Sony may change it.
					// So we assume the first sku for the product is the one we want

					// MGH -  keyname in the DLC file is 16 chars long, but there's no space for a nullptr terminating char
					memset(chKeyName, 0, sizeof(chKeyName));
					strncpy(chKeyName, pSONYDLCInfo->chDLCKeyname, 16);

#ifdef __ORBIS__
					strcpy(chName, chKeyName);
#else
					sprintf(chName,"%s-%s",app.GetCommerceCategory(),chKeyName);
#endif
					app.GetDLCSkuIDFromProductList(chName,chSkuID);

					// 4J-PB - need to check for an empty store
#if defined __ORBIS__ || defined __PSVITA__ || defined __PS3__
					if(app.CheckForEmptyStore(iPad)==false)
#endif
					{	
						if(app.DLCAlreadyPurchased(chSkuID))
						{
							app.DownloadAlreadyPurchased(chSkuID);
						}
						else
						{
							app.Checkout(chSkuID);	
						}
					}
				}
			}
#else
			// TO BE IMPEMENTED FOR ORBIS
#endif
		}
	}
	return 0;
}

int UIScene_PauseMenu::ViewInvites_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	if(bContinue==true)
	{
		// Check if we're signed in to LIVE
		if(ProfileManager.IsSignedInLive(iPad))
		{
#ifdef __ORBIS__
			SQRNetworkManager_Orbis::RecvInviteGUI();
#elif defined __PS3__
			int ret = sceNpBasicRecvMessageCustom(SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE, SCE_NP_BASIC_RECV_MESSAGE_OPTIONS_INCLUDE_BOOTABLE, SYS_MEMORY_CONTAINER_ID_INVALID);
			app.DebugPrintf("sceNpBasicRecvMessageCustom return %d ( %08x )\n", ret, ret);
#else // __PSVITA__
			PSVITA_STUBBED;
#endif
		}
	}
	return 0;
}


int UIScene_PauseMenu::ExitGameSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// Exit with or without saving
	// Decline means save in this dialog
	if(result==C4JStorage::EMessage_ResultDecline || result==C4JStorage::EMessage_ResultThirdOption) 
	{
		if( result==C4JStorage::EMessage_ResultDecline ) // Save
		{
			// 4J-PB - Is the player trying to save but they are using a trial texturepack ?
			if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
			{
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();
				if(!pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
				{					
#ifdef _XBOX
					// upsell
					ULONGLONG ullOfferID_Full;
					// get the dlc texture pack
					DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

					app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullOfferID_Full);

					// tell sentient about the upsell of the full version of the skin pack
					TelemetryManager->RecordUpsellPresented(iPad, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif

					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_OK;
					uiIDA[1]=IDS_CONFIRM_CANCEL;

					// Give the player a warning about the trial version of the texture pack
					ui.RequestAlertMessage(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad() ,&UIScene_PauseMenu::WarningTrialTexturePackReturned, pParam);

					return S_OK;					
				}
			}

			// does the save exist?
			bool bSaveExists;
			StorageManager.DoesSaveExist(&bSaveExists);
			// 4J-PB - we check if the save exists inside the libs
			// we need to ask if they are sure they want to overwrite the existing game
			if(bSaveExists)
			{
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(),&IUIScene_PauseMenu::ExitGameAndSaveReturned, pParam);
				return 0;
			}
			else
			{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
				StorageManager.SetSaveDisabled(false);
#endif
				MinecraftServer::getInstance()->setSaveOnExit( true );
			}
		}
		else
		{
			// been a few requests for a confirm on exit without saving
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestAlertMessage(IDS_TITLE_DECLINE_SAVE_GAME, IDS_CONFIRM_DECLINE_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(),&IUIScene_PauseMenu::ExitGameDeclineSaveReturned, pParam);
			return 0;
		}

		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	return 0;
}

#endif

void UIScene_PauseMenu::SetIgnoreInput(bool ignoreInput)
{
	m_bIgnoreInput = ignoreInput;
}

#ifdef _XBOX_ONE
void UIScene_PauseMenu::HandleDLCLicenseChange()
{	
}
#endif

#ifdef __ORBIS__
bool UIScene_PauseMenu::CheckForPatch()
{
	int npAvailability = ProfileManager.getNPAvailability(ProfileManager.GetPrimaryPad());

	bool bPatchAvailable;
	switch(npAvailability)
	{
	case SCE_NP_ERROR_LATEST_PATCH_PKG_EXIST:
	case SCE_NP_ERROR_LATEST_PATCH_PKG_DOWNLOADED:
		bPatchAvailable=true;
		break;
	default:
		bPatchAvailable=false;
		break;
	}

	if(bPatchAvailable)
	{
		int32_t ret = sceErrorDialogInitialize();
		if (  ret==SCE_OK ) 
		{
			m_bErrorDialogRunning = true;

			SceErrorDialogParam param;
			sceErrorDialogParamInitialize( &param );
			// 4J-PB - We want to display the option to get the patch now
			param.errorCode = SCE_NP_ERROR_LATEST_PATCH_PKG_DOWNLOADED;//pClass->m_errorCode;
			ret = sceUserServiceGetInitialUser( &param.userId );
			if ( ret == SCE_OK ) 
			{
				ret = sceErrorDialogOpen( &param );
			}
			else 
			{
				sceErrorDialogTerminate();
			}
		}
	}

	return bPatchAvailable;
}
#endif