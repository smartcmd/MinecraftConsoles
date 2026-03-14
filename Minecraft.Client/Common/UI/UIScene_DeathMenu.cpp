#include "stdafx.h"
#include "UI.h"
#include "UIScene_DeathMenu.h"
#include "IUIScene_PauseMenu.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"

UIScene_DeathMenu::UIScene_DeathMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_buttonRespawn.init(app.GetString(IDS_RESPAWN),eControl_Respawn);
	m_buttonExitGame.init(app.GetString(IDS_EXIT_GAME),eControl_ExitGame);

	m_labelTitle.setLabel(app.GetString(IDS_YOU_DIED));

	m_bIgnoreInput = false;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft != nullptr && pMinecraft->localgameModes[iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[iPad]);

		// This just allows it to be shown
		gameMode->getTutorial()->showTutorialPopup(false);
	}
}

UIScene_DeathMenu::~UIScene_DeathMenu()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft != nullptr && pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);

		// This just allows it to be shown
		gameMode->getTutorial()->showTutorialPopup(true);
	}
}

wstring UIScene_DeathMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"DeathMenuSplit";
	}
	else
	{
		return L"DeathMenu";
	}
}

void UIScene_DeathMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT);
}

void UIScene_DeathMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bIgnoreInput) return;

	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		handled = true;
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
		handled = true;
		break;
	}
}

void UIScene_DeathMenu::handlePress(F64 controlId, F64 childId)
{
	switch(static_cast<int>(controlId))
	{
	case eControl_Respawn:
		m_bIgnoreInput = true;
		app.SetAction(m_iPad,eAppAction_Respawn);
#ifdef _DURANGO
		//InputManager.SetEnabledGtcButtons(_360_GTC_MENU|_360_GTC_PAUSE|_360_GTC_VIEW);
#endif
		break;
	case eControl_ExitGame:
		{
			Minecraft *pMinecraft=Minecraft::GetInstance();
			// 4J-PB - fix for #8333 - BLOCKER: If player decides to exit game, then cancels the exit player becomes stuck at game over screen
			//m_bIgnoreInput = true;
			// Check if it's the trial version
			if(ProfileManager.IsFullVersion())
			{	
			
				// is it the primary player exiting?
				if(m_iPad==ProfileManager.GetPrimaryPad())
				{
					UINT uiIDA[3];
					int playTime = -1;
					if( pMinecraft->localplayers[m_iPad] != nullptr )
					{
						playTime = static_cast<int>(pMinecraft->localplayers[m_iPad]->getSessionTimer());
					}
					TelemetryManager->RecordLevelExit(m_iPad, eSen_LevelExitStatus_Failed);
					
#if defined (_XBOX_ONE) || defined(__ORBIS__)
					if(g_NetworkManager.IsHost() && StorageManager.GetSaveDisabled())
					{
						uiIDA[0]=IDS_CONFIRM_CANCEL;
						uiIDA[1]=IDS_EXIT_GAME_SAVE;
						uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

						ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, m_iPad,&IUIScene_PauseMenu::ExitGameSaveDialogReturned,(LPVOID)GetCallbackUniqueId());
					}
					else
					{
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_CONFIRM_OK;
						ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned,(LPVOID)GetCallbackUniqueId());
					}

#else
					if(StorageManager.GetSaveDisabled())
					{
						uiIDA[0]=IDS_CONFIRM_CANCEL;
						uiIDA[1]=IDS_CONFIRM_OK;
						ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned,(LPVOID)GetCallbackUniqueId());
					}
					else
					{
						if( g_NetworkManager.IsHost() )
						{	
							uiIDA[0]=IDS_CONFIRM_CANCEL;
							uiIDA[1]=IDS_EXIT_GAME_SAVE;
							uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

							ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, m_iPad,&IUIScene_PauseMenu::ExitGameSaveDialogReturned,(LPVOID)GetCallbackUniqueId());
						}
						else
						{
							uiIDA[0]=IDS_CONFIRM_CANCEL;
							uiIDA[1]=IDS_CONFIRM_OK;

							ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned,(LPVOID)GetCallbackUniqueId());
						}
					}
#endif
				}
				else
				{
					TelemetryManager->RecordLevelExit(m_iPad, eSen_LevelExitStatus_Failed);
					
					// just exit the player
					app.SetAction(m_iPad,eAppAction_ExitPlayer);
				}		
			}
			else
			{
				// is it the primary player exiting?
				if(m_iPad==ProfileManager.GetPrimaryPad())
				{
					TelemetryManager->RecordLevelExit(m_iPad, eSen_LevelExitStatus_Failed);
					
					// adjust the trial time played
					ui.ReduceTrialTimerValue();

					// exit the level
					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_CONFIRM_OK;
					ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned,(LPVOID)GetCallbackUniqueId());
				}
				else
				{
					TelemetryManager->RecordLevelExit(m_iPad, eSen_LevelExitStatus_Failed);

					// just exit the player
					app.SetAction(m_iPad,eAppAction_ExitPlayer);
				}
			}
		}
		break;
	}
}