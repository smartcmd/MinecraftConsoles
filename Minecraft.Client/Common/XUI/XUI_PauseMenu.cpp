// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "../../../Minecraft.World/AABB.h"
#include "../../../Minecraft.World/Vec3.h"
#include "../../../Minecraft.World/net.minecraft.stats.h"
#include "../../../Minecraft.Client/StatsCounter.h"
#include "../../../Minecraft.World/Entity.h"
#include "../../../Minecraft.World/Level.h"
#include "../../../Minecraft.Client/MultiPlayerLocalPlayer.h"
#include "../../MinecraftServer.h"
#include "../../MultiPlayerLevel.h"
#include "../../ProgressRenderer.h"
#include "../../../Minecraft.World/DisconnectPacket.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "../../../Minecraft.World/compression.h"
#include "../../TexturePackRepository.h"
#include "../../TexturePack.h"
#include "../../DLCTexturePack.h"

#define IGNORE_KEYPRESS_TIMERID 0
#define IGNORE_KEYPRESS_TIME 100

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT UIScene_PauseMenu::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_bIgnoreInput=true;
	m_iPad = *(int *)pInitData->pvInitData;
	bool bUserisClientSide = ProfileManager.IsSignedInLive(m_iPad);

	app.DebugPrintf("PAUSE PRESS PROCESSING - ipad = %d, UIScene_PauseMenu::OnInit\n",m_iPad);

	bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==m_iPad);
	bool bDisplayBanTip = !g_NetworkManager.IsLocalGame() && !bIsisPrimaryHost && !ProfileManager.IsGuest(m_iPad);

	MapChildControls();

	XuiControlSetText(m_Buttons[BUTTON_PAUSE_RESUMEGAME],app.GetString(IDS_RESUME_GAME));
	XuiControlSetText(m_Buttons[BUTTON_PAUSE_HELPANDOPTIONS],app.GetString(IDS_HELP_AND_OPTIONS));
	XuiControlSetText(m_Buttons[BUTTON_PAUSE_LEADERBOARDS],app.GetString(IDS_LEADERBOARDS));
	XuiControlSetText(m_Buttons[BUTTON_PAUSE_ACHIEVEMENTS],app.GetString(IDS_ACHIEVEMENTS));
	XuiControlSetText(m_Buttons[BUTTON_PAUSE_SAVEGAME],app.GetString(IDS_SAVE_GAME));
	XuiControlSetText(m_Buttons[BUTTON_PAUSE_EXITGAME],app.GetString(IDS_EXIT_GAME));

	if(app.GetLocalPlayerCount()>1)
	{
		m_bSplitscreen = true;
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,false);
		CXuiSceneBase::ShowLogo( m_iPad, FALSE );
	}
	else
	{
		m_bSplitscreen = false;
		CXuiSceneBase::ShowLogo( m_iPad, TRUE );
	}

	// test award the theme
	//ProfileManager.Award( ProfileManager.GetPrimaryPad(), eAward_socialPost );
	// Display the tooltips, we are only allowed to display "SHARE" if we have the capability (TCR).

	if(!ProfileManager.IsFullVersion())
	{
		ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		// hide the trial timer
		CXuiSceneBase::ShowTrialTimer(FALSE);
	}
	else if(StorageManager.GetSaveDisabled())
	{
		if( CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide )
		{
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_SELECTDEVICE:-1,IDS_TOOLTIPS_SHARE, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}
		else
		{
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_SELECTDEVICE:-1,-1,-1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}		
	}
	else
	{
		if( CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide)
		{
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_CHANGEDEVICE:-1,IDS_TOOLTIPS_SHARE, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}
		else
		{
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_CHANGEDEVICE:-1,-1, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}		
	}

	CXuiSceneBase::ShowDarkOverlay( m_iPad, TRUE );

	// are we the primary player?
	// 4J-PB - fix for 7844 & 7845 - 
	// TCR # 128:  XLA Pause Menu:   When in a multiplayer game as a client the Pause Menu does not have a Leaderboards option.
	// TCR # 128:  XLA Pause Menu:   When in a multiplayer game as a client the Pause Menu does not have an Achievements option.
	if(ProfileManager.GetPrimaryPad()==m_iPad) // && g_NetworkManager.IsHost()) 
	{
		// are we in splitscreen?
		// how many local players do we have?
		
		D3DXVECTOR3 vPos;
		if( app.GetLocalPlayerCount()>1 )
		{
			m_Buttons[BUTTON_PAUSE_LEADERBOARDS].GetPosition(&vPos);
			m_Buttons[BUTTON_PAUSE_SAVEGAME].SetPosition(&vPos);
			m_Buttons[BUTTON_PAUSE_ACHIEVEMENTS].GetPosition(&vPos);
			m_Buttons[BUTTON_PAUSE_EXITGAME].SetPosition(&vPos);
			// Hide the BUTTON_PAUSE_LEADERBOARDS and BUTTON_PAUSE_ACHIEVEMENTS
			XuiElementSetShow(m_Buttons[BUTTON_PAUSE_LEADERBOARDS],FALSE);
			XuiElementSetShow(m_Buttons[BUTTON_PAUSE_ACHIEVEMENTS],FALSE);
		}

		if( !g_NetworkManager.IsHost() )
		{
			m_Buttons[BUTTON_PAUSE_SAVEGAME].GetPosition(&vPos);
			m_Buttons[BUTTON_PAUSE_EXITGAME].SetPosition(&vPos);
			// Hide the BUTTON_PAUSE_SAVEGAME
			XuiElementSetShow(m_Buttons[BUTTON_PAUSE_SAVEGAME],FALSE);
		}
	}
	else
	{
		D3DXVECTOR3 vPos;
		m_Buttons[BUTTON_PAUSE_LEADERBOARDS].GetPosition(&vPos);
		m_Buttons[BUTTON_PAUSE_EXITGAME].SetPosition(&vPos);
		// Hide the BUTTON_PAUSE_LEADERBOARDS, BUTTON_PAUSE_ACHIEVEMENTS and BUTTON_PAUSE_SAVEGAME
		XuiElementSetShow(m_Buttons[BUTTON_PAUSE_LEADERBOARDS],FALSE);
		XuiElementSetShow(m_Buttons[BUTTON_PAUSE_ACHIEVEMENTS],FALSE);
		XuiElementSetShow(m_Buttons[BUTTON_PAUSE_SAVEGAME],FALSE);
	}

	// is saving disabled?
	if(StorageManager.GetSaveDisabled())
	{
		// disable save button
		m_Buttons[BUTTON_PAUSE_SAVEGAME].SetEnable(FALSE);
		m_Buttons[BUTTON_PAUSE_SAVEGAME].EnableInput(FALSE);
	}

	m_iLastButtonPressed=0;

	// get rid of the quadrant display if it's on
	CXuiSceneBase::HidePressStart();

	XuiSetTimer(m_hObj,IGNORE_KEYPRESS_TIMERID,IGNORE_KEYPRESS_TIME);

	if( g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 )
	{
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)TRUE);
	}

	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_PauseMenu, 0);
	TelemetryManager->RecordPauseOrInactive(m_iPad);

	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT UIScene_PauseMenu::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	unsigned int uiButtonCounter=0;

	while((uiButtonCounter<BUTTONS_PAUSE_MAX) && (m_Buttons[uiButtonCounter]!=hObjPressed)) uiButtonCounter++;
	
	Minecraft *pMinecraft=Minecraft::GetInstance();

	// ignore buttons not from this user
	//if(pNotifyPressData->UserIndex!=pMinecraft->player->GetXboxPad()) return S_OK;

	// Determine which button was pressed,
	// and call the appropriate function.	

	// store the last button pressed, so on a nav back we can set the focus properly
	m_iLastButtonPressed=uiButtonCounter;
	switch(uiButtonCounter)
	{
	case BUTTON_PAUSE_RESUMEGAME:
		if( m_iPad == ProfileManager.GetPrimaryPad() && g_NetworkManager.IsLocalGame() )
		{
			app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)FALSE);
		}
		app.CloseXuiScenes(pNotifyPressData->UserIndex);
		break;

	case BUTTON_PAUSE_LEADERBOARDS:
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;
		
			//4J Gordon: Being used for the leaderboards proper now
			// guests can't look at leaderboards
			if(ProfileManager.IsGuest(pNotifyPressData->UserIndex))
			{
				StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
			}
			else if(!ProfileManager.IsSignedInLive(pNotifyPressData->UserIndex))
			{
				StorageManager.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1);
			}
			else
			{	
				app.NavigateToScene(pNotifyPressData->UserIndex, eUIScene_LeaderboardsMenu);
			}
		}
		break;
	case BUTTON_PAUSE_ACHIEVEMENTS:
		// guests can't look at achievements
		if(ProfileManager.IsGuest(pNotifyPressData->UserIndex))
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;
			StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
		}
		else
		{
			XShowAchievementsUI( pNotifyPressData->UserIndex );
		}

		break;
	case BUTTON_PAUSE_HELPANDOPTIONS:
		if(app.GetLocalPlayerCount()>1)
		{
			app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_HelpAndOptionsMenu);	
		}
		else
		{
			app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_HelpAndOptionsMenu);	
		}
		break;

	case BUTTON_PAUSE_SAVEGAME:		
		{	
			// 4J-PB - Is the player trying to save but they are using a trial texturepack ?
			if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
			{
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;
				
				m_pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();

				if(!m_pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
				{					
					// upsell
					ULONGLONG ullOfferID_Full;
					// get the dlc texture pack
					DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;
					
					app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullOfferID_Full);

					// tell sentient about the upsell of the full version of the texture pack
					TelemetryManager->RecordUpsellPresented(pNotifyPressData->UserIndex, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);

					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_OK;
					uiIDA[1]=IDS_CONFIRM_CANCEL;

					// Give the player a warning about the trial version of the texture pack
					StorageManager.RequestMessageBox(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&UIScene_PauseMenu::WarningTrialTexturePackReturned,this,app.GetStringTable());

					return S_OK;					
				}
			}

			// does the save exist?
			bool bSaveExists;
			C4JStorage::ELoadGameStatus result=StorageManager.DoesSaveExist(&bSaveExists);

			if(result == C4JStorage::ELoadGame_DeviceRemoved)
			{
				// this will be a tester trying to be clever
				UINT uiIDA[2];
				uiIDA[0]=IDS_SELECTANEWDEVICE;
				uiIDA[1]=IDS_NODEVICE_DECLINE;

				StorageManager.RequestMessageBox(IDS_STORAGEDEVICEPROBLEM_TITLE, IDS_FAILED_TO_LOADSAVE_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&UIScene_PauseMenu::DeviceRemovedDialogReturned,this);
			}
			else
			{
				// we need to ask if they are sure they want to overwrite the existing game
				if(bSaveExists)
				{
					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_CONFIRM_OK;
					StorageManager.RequestMessageBox(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2, pNotifyPressData->UserIndex,&UIScene_PauseMenu::SaveGameDialogReturned,this, app.GetStringTable());
				}
				else
				{
					// flag a app action of save game
					app.SetAction(pNotifyPressData->UserIndex,eAppAction_SaveGame);
				}
			}
		}

		break;
	case BUTTON_PAUSE_EXITGAME:
		{
			// Check if it's the trial version
			if(ProfileManager.IsFullVersion())
			{	
				UINT uiIDA[3];
			
				// is it the primary player exiting?
				if(pNotifyPressData->UserIndex==ProfileManager.GetPrimaryPad())
				{
					int playTime = -1;
					if( pMinecraft->localplayers[pNotifyPressData->UserIndex] != NULL )
					{
						playTime = (int)pMinecraft->localplayers[pNotifyPressData->UserIndex]->getSessionTimer();
					}
					
					if(StorageManager.GetSaveDisabled())
					{
						uiIDA[0]=IDS_CONFIRM_CANCEL;
						uiIDA[1]=IDS_CONFIRM_OK;
						StorageManager.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST, uiIDA, 2, pNotifyPressData->UserIndex,&UIScene_PauseMenu::ExitGameDialogReturned,this, app.GetStringTable());
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
								StorageManager.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3, pNotifyPressData->UserIndex,&UIScene_PauseMenu::ExitGameSaveDialogReturned,this, app.GetStringTable());
							}
							else
							{
								StorageManager.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, pNotifyPressData->UserIndex,&UIScene_PauseMenu::ExitGameSaveDialogReturned,this, app.GetStringTable());
							}
						}
						else
						{
							uiIDA[0]=IDS_CONFIRM_CANCEL;
							uiIDA[1]=IDS_CONFIRM_OK;

							StorageManager.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2, pNotifyPressData->UserIndex,&UIScene_PauseMenu::ExitGameDialogReturned,this, app.GetStringTable());
						}
					}
				}
				else
				{
					int playTime = -1;
					if( pMinecraft->localplayers[pNotifyPressData->UserIndex] != NULL )
					{
						playTime = (int)pMinecraft->localplayers[pNotifyPressData->UserIndex]->getSessionTimer();
					}

					TelemetryManager->RecordLevelExit(pNotifyPressData->UserIndex, eSen_LevelExitStatus_Exited);

					
					// just exit the player
					app.SetAction(pNotifyPressData->UserIndex,eAppAction_ExitPlayer);
				}		
			}
			else
			{
				// is it the primary player exiting?
				if(pNotifyPressData->UserIndex==ProfileManager.GetPrimaryPad())
				{		
					int playTime = -1;
					if( pMinecraft->localplayers[pNotifyPressData->UserIndex] != NULL )
					{
						playTime = (int)pMinecraft->localplayers[pNotifyPressData->UserIndex]->getSessionTimer();
					}	
					
					// adjust the trial time played
					CXuiSceneBase::ReduceTrialTimerValue();

					// exit the level
					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_CONFIRM_OK;
					StorageManager.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST, uiIDA, 2, pNotifyPressData->UserIndex,&UIScene_PauseMenu::ExitGameDialogReturned,this, app.GetStringTable());
				}
				else
				{
					int playTime = -1;
					if( pMinecraft->localplayers[pNotifyPressData->UserIndex] != NULL )
					{
						playTime = (int)pMinecraft->localplayers[pNotifyPressData->UserIndex]->getSessionTimer();
					}

					TelemetryManager->RecordLevelExit(pNotifyPressData->UserIndex, eSen_LevelExitStatus_Exited);

					// just exit the player
					app.SetAction(pNotifyPressData->UserIndex,eAppAction_ExitPlayer);
				}
			}
		}
		break;
	default:
		break;
	}

	return S_OK;
}

HRESULT UIScene_PauseMenu::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;
	// ignore repeated start presses to avoid the scene closing before it's opened
	if((pInputData->dwKeyCode==VK_PAD_START) &&(pInputData->dwFlags&XUI_INPUT_FLAG_REPEAT ))
	{
		rfHandled = TRUE;
		return S_OK;
	}	

	bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==m_iPad);	
	bool bDisplayBanTip = !g_NetworkManager.IsLocalGame() && !bIsisPrimaryHost && !ProfileManager.IsGuest(m_iPad);
	bool bUserisClientSide = ProfileManager.IsSignedInLive(m_iPad);

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	app.DebugPrintf("PAUSE- Keydown in the xui %d flags = %d\n",pInputData->dwKeyCode,pInputData->dwFlags);


	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_PAD_START:
	case VK_ESCAPE:
		app.DebugPrintf("PAUSE PRESS PROCESSING - ipad = %d, UIScene_PauseMenu::OnKeyDown - LEAVING PAUSE MENU\n",m_iPad);
		
		if( m_iPad == ProfileManager.GetPrimaryPad() && g_NetworkManager.IsLocalGame() )
		{
			app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)FALSE);
		}

		CXuiSceneBase::PlayUISFX(eSFX_Back);
		app.CloseXuiScenes(pInputData->UserIndex);
		if(!ProfileManager.IsFullVersion())
		{
			CXuiSceneBase::ShowTrialTimer(TRUE);
		}
		rfHandled = TRUE;

		break;

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


	case VK_PAD_Y:
		{
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
		}
		break;

	case VK_PAD_RSHOULDER:
		if( bDisplayBanTip )
		{
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			StorageManager.RequestMessageBox(IDS_ACTION_BAN_LEVEL_TITLE, IDS_ACTION_BAN_LEVEL_DESCRIPTION, uiIDA, 2, pInputData->UserIndex,&UIScene_PauseMenu::BanGameDialogReturned,this, app.GetStringTable());

			rfHandled = TRUE;
		}
		break;

		// handle a keyboard Return specifically, because we've turned off the VK_A_OR_START for the pause menu, since this should exit the menu, rather than cause the button to be activated
	case VK_RETURN:
		// call OnNotifyPressEx directly to trigger the button press action

		HXUIOBJ hObjPressed=TreeGetFocus(); 
		XUINotifyPress NotifyPressData; 
		BOOL rfHandled=FALSE;

		NotifyPressData.UserIndex=pInputData->UserIndex;
		OnNotifyPressEx(hObjPressed,&NotifyPressData,rfHandled);

		break;
	}

	return S_OK;
}

HRESULT UIScene_PauseMenu::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==m_iPad);
	bool bDisplayBanTip = !g_NetworkManager.IsLocalGame() && !bIsisPrimaryHost && !ProfileManager.IsGuest(m_iPad);
	bool bUserisClientSide = ProfileManager.IsSignedInLive(m_iPad);

	// Display the tooltips, we are only allowed to display "SHARE" if we have the capability (TCR).
	if(StorageManager.GetSaveDisabled())
	{
		if( ProfileManager.IsFullVersion() && CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide)
		{
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_SELECTDEVICE:-1,IDS_TOOLTIPS_SHARE, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}
		else
		{
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,-1,-1, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}	
	}
	else
	{
		if( CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide)
		{
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_CHANGEDEVICE:-1,IDS_TOOLTIPS_SHARE, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);

		}
		else
		{
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_CHANGEDEVICE:-1,-1, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}			
	}
	
	// set the focus to the last button we were on
	XuiElementSetUserFocus(m_Buttons[m_iLastButtonPressed],m_iPad);

	CXuiSceneBase::ShowBackground( m_iPad, FALSE );
	CXuiSceneBase::ShowDarkOverlay( m_iPad, TRUE );

	bool isWrongSize = false;
	if(app.GetLocalPlayerCount()==1)
	{
		// If we were created as a splitscreen scene, then it's now going to be in the wrong place. Get rid of this scene.
		if(m_bSplitscreen)
		{
			CXuiSceneBase::ShowLogo( m_iPad, FALSE );
			isWrongSize = true;
		}
		else
		{
			CXuiSceneBase::ShowLogo( m_iPad, TRUE );
		}
	}
	else
	{
		CXuiSceneBase::ShowLogo( m_iPad, FALSE );
		if(!m_bSplitscreen) isWrongSize = true;
	}

	if(isWrongSize)
	{
		if( m_iPad == ProfileManager.GetPrimaryPad() && g_NetworkManager.IsLocalGame() )
		{
			app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)FALSE);
		}

		app.CloseXuiScenes(m_iPad);
		if(!ProfileManager.IsFullVersion())
		{
			CXuiSceneBase::ShowTrialTimer(TRUE);
		}
	}

	return S_OK;
}

HRESULT UIScene_PauseMenu::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);
	
	if(pControlNavigateData->hObjDest!=NULL)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

int UIScene_PauseMenu::BanGameDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		// 4J Stu - Only do this if we are currently idle, don't want the (relatively) low priority ban task overriding something else
		if(app.GetXuiAction(iPad) == eAppAction_Idle) app.SetAction(iPad,eAppAction_BanLevel);
	}
	return 0;
}

int UIScene_PauseMenu::DeviceSelectReturned(void *pParam,bool bContinue)
{
	// Has someone pulled the ethernet cable and caused the pause menu to be closed before this callback returns?
	if(!app.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()))
	{
		return 0;
	}

	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;
	bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==pClass->m_iPad);
	bool bDisplayBanTip = !g_NetworkManager.IsLocalGame() && !bIsisPrimaryHost && !ProfileManager.IsGuest(pClass->m_iPad);
	bool bUserisClientSide = ProfileManager.IsSignedInLive(pClass->m_iPad);

	//Whatever happen, we need to update the pause menu and the tooltips
	// Display the tooltips, we are only allowed to display "SHARE" if we have the capability (TCR).
	if(StorageManager.GetSaveDisabled())
	{
		if( ProfileManager.IsFullVersion() && CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide)
		{
			ui.SetTooltips( pClass->m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_SELECTDEVICE:-1,IDS_TOOLTIPS_SHARE, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}
		else
		{
			ui.SetTooltips( pClass->m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,-1,-1, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}	
		// disable save button
		// set the focus on to another button
		pClass->m_Buttons[BUTTON_PAUSE_SAVEGAME].SetEnable(FALSE);
		pClass->m_Buttons[BUTTON_PAUSE_SAVEGAME].EnableInput(FALSE);

		pClass->m_Buttons[BUTTON_PAUSE_RESUMEGAME].InitFocus(pClass->m_iPad);
	}
	else
	{
		if( CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide)
		{
			ui.SetTooltips(pClass->m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_CHANGEDEVICE:-1,IDS_TOOLTIPS_SHARE, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}
		else
		{
			ui.SetTooltips( pClass->m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,bIsisPrimaryHost?IDS_TOOLTIPS_CHANGEDEVICE:-1,-1, -1,-1,-1,bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1);
		}
		// enable save button
		pClass->m_Buttons[BUTTON_PAUSE_SAVEGAME].SetEnable(TRUE);
		pClass->m_Buttons[BUTTON_PAUSE_SAVEGAME].EnableInput(TRUE);

	}

	pClass->m_bIgnoreInput=false;
	return 0;
}

HRESULT UIScene_PauseMenu::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining,false);
}

HRESULT UIScene_PauseMenu::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	if(pData->nId==IGNORE_KEYPRESS_TIMERID)
	{
		XuiKillTimer(m_hObj,IGNORE_KEYPRESS_TIMERID);

		// block input if we're waiting for DLC to install, and wipe the saves list. The end of dlc mounting custom message will fill the list again
		if(app.StartInstallDLCProcess(m_iPad)==true)
		{
			// not doing a mount, so enable input
			m_bIgnoreInput=true;
		}
		else
		{
			m_bIgnoreInput=false;
		}
	}

	return S_OK;
}


HRESULT UIScene_PauseMenu::OnDestroy()
{
	//XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);
	TelemetryManager->RecordUnpauseOrActive(m_iPad);

	if( m_iPad == ProfileManager.GetPrimaryPad() && g_NetworkManager.IsLocalGame() )
	{
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)FALSE);
	}

	return S_OK;
}

int UIScene_PauseMenu::DeviceRemovedDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		// continue without saving
		StorageManager.SetSaveDisabled(true);
		StorageManager.SetSaveDeviceSelected(ProfileManager.GetPrimaryPad(),false);

		// Has someone pulled the ethernet cable and caused the pause menu to be closed before this callback returns?
		if(app.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()))
		{
			UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;
		
			// use the device select returned function to wipe the saves list and change the tooltip
			pClass->DeviceSelectReturned(pClass,true);
		}
	}
	else 
	{
		// Change device
		if(app.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()))
		{
			UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;
			StorageManager.SetSaveDevice(&UIScene_PauseMenu::DeviceSelectReturned,pClass,true);
		}
	}
	return 0;
}

HRESULT UIScene_PauseMenu::OnCustomMessage_DLCInstalled()
{
	// mounted DLC may have changed
	if(app.StartInstallDLCProcess(m_iPad)==false)
	{
		// not doing a mount, so re-enable input
		m_bIgnoreInput=false;
	}
	else
	{
		m_bIgnoreInput=true;
	}
	// this will send a CustomMessage_DLCMountingComplete when done
	return S_OK;
}

HRESULT UIScene_PauseMenu::OnCustomMessage_DLCMountingComplete()
{
	m_bIgnoreInput=false;
	app.m_dlcManager.checkForCorruptDLCAndAlert();
	XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);
	return S_OK;
}

void UIScene_PauseMenu::ShowScene(bool show)
{
	SetShow(show?TRUE:FALSE);
}

int UIScene_PauseMenu::WarningTrialTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_PauseMenu* pScene = (UIScene_PauseMenu*)pParam;

	//pScene->m_bIgnoreInput = false;
	pScene->ShowScene( true );
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{	
			ULONGLONG ullIndexA[1];

			TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
			// get the dlc texture pack
			DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

			// Need to get the parent packs id, since this may be one of many child packs with their own ids
			app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullIndexA[0]);

			// need to allow downloads here, or the player would need to quit the game to let the download of a texture pack happen. This might affect the network traffic, since the download could take all the bandwidth...
			XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);

			StorageManager.InstallOffer(1,ullIndexA,NULL,NULL);
		}
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSet_UpsellID_Texture_DLC, ( pScene->m_pDLCPack->getPurchaseOfferId() & 0xFFFFFFFF ), eSen_UpsellOutcome_Declined);
	}


	return 0;
}

int UIScene_PauseMenu::ExitGameSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_PauseMenu *pClass = (UIScene_PauseMenu *)pParam;
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
					ui.RequestMessageBox(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad() ,&UIScene_PauseMenu::WarningTrialTexturePackReturned,pClass,app.GetStringTable());

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
				ui.RequestMessageBox(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::ExitGameAndSaveReturned,pClass, app.GetStringTable());
				return 0;
			}
			else
			{
				MinecraftServer::getInstance()->setSaveOnExit( true );
			}
		}
		else
		{
			// been a few requests for a confirm on exit without saving
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestMessageBox(IDS_TITLE_DECLINE_SAVE_GAME, IDS_CONFIRM_DECLINE_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::ExitGameDeclineSaveReturned, dynamic_cast<IUIScene_PauseMenu*>(pClass), app.GetStringTable());
			return 0;
		}

		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	return 0;
}

int UIScene_PauseMenu::ExitGameDeclineSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		MinecraftServer::getInstance()->setSaveOnExit( false );
		// flag a app action of exit game
		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	else
	{
		// has someone disconnected the ethernet here, causing the pause menu to shut?
		if(ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()))
		{
			IUIScene_PauseMenu* pClass = (IUIScene_PauseMenu*)pParam;
			UINT uiIDA[3];
			// you cancelled the save on exit after choosing exit and save? You go back to the Exit choices then.
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_EXIT_GAME_SAVE;
			uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

			if(g_NetworkManager.GetPlayerCount()>1)
			{
				ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::ExitGameSaveDialogReturned,pClass, app.GetStringTable(), 0, 0, false);
			}
			else
			{
				ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::ExitGameSaveDialogReturned,pClass, app.GetStringTable(), 0, 0, false);
			}
		}

	}
	return 0;
}

int UIScene_PauseMenu::ExitGameAndSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// 4J-PB - we won't come in here if we have a trial texture pack

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		MinecraftServer::getInstance()->setSaveOnExit( true );
		// flag a app action of exit game
		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	else
	{
		// has someone disconnected the ethernet here, causing the pause menu to shut?
		if(ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()))
		{
			UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;
			UINT uiIDA[3];
			// you cancelled the save on exit after choosing exit and save? You go back to the Exit choices then.
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_EXIT_GAME_SAVE;
			uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

			if(g_NetworkManager.GetPlayerCount()>1)
			{
				ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::ExitGameSaveDialogReturned,pClass, app.GetStringTable(), 0, 0, false);
			}
			else
			{
				ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::ExitGameSaveDialogReturned,pClass, app.GetStringTable(), 0, 0, false);
			}
		}

	}
	return 0;
}

int UIScene_PauseMenu::SaveGameDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		// flag a app action of save game
		app.SetAction(iPad,eAppAction_SaveGame);
	}
	return 0;
}

int UIScene_PauseMenu::ExitGameDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	return 0;
}

int UIScene_PauseMenu::SaveWorldThreadProc( LPVOID lpParameter )
{
	bool bAutosave=(bool)lpParameter;
	if(bAutosave)
	{
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_AutoSaveGame);
	}
	else
	{
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_SaveGame);
	}

	// Share AABB & Vec3 pools with default (main thread) - should be ok as long as we don't tick the main thread whilst this thread is running
	AABB::UseDefaultThreadStorage();
	Vec3::UseDefaultThreadStorage();
	Compression::UseDefaultThreadStorage();

	Minecraft *pMinecraft=Minecraft::GetInstance();

	//wprintf(L"Loading world on thread\n");

	if(ProfileManager.IsFullVersion())
	{	
		app.SetGameStarted(false);

		while( app.GetXuiServerAction(ProfileManager.GetPrimaryPad() ) != eXuiServerAction_Idle && !MinecraftServer::serverHalted() )
		{
			Sleep(10);
		}

		if(!MinecraftServer::serverHalted() && !app.GetChangingSessionType() ) app.SetGameStarted(true);
	}

	HRESULT hr = S_OK;
	if(app.GetChangingSessionType())
	{
		// 4J Stu - This causes the fullscreenprogress scene to ignore the action it was given
		hr = ERROR_CANCELLED;
	}
	return hr;
}

int UIScene_PauseMenu::ExitWorldThreadProc( void* lpParameter )
{
	// Share AABB & Vec3 pools with default (main thread) - should be ok as long as we don't tick the main thread whilst this thread is running
	AABB::UseDefaultThreadStorage();
	Vec3::UseDefaultThreadStorage();
	Compression::UseDefaultThreadStorage();

	//app.SetGameStarted(false);

	_ExitWorld(lpParameter);

	return S_OK;
}

// This function performs the meat of exiting from a level. It should be called from a thread other than the main thread.
void UIScene_PauseMenu::_ExitWorld(LPVOID lpParameter)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	int exitReasonStringId = pMinecraft->progressRenderer->getCurrentTitle();
	int exitReasonTitleId = IDS_CONNECTION_LOST;

	bool saveStats = true;
	if (pMinecraft->isClientSide() || g_NetworkManager.IsInSession())
	{
		if(lpParameter != NULL )
		{
			// 4J-PB - check if we have lost connection to Live
			if(ProfileManager.GetLiveConnectionStatus()!=XONLINE_S_LOGON_CONNECTION_ESTABLISHED )
			{
				exitReasonStringId = IDS_CONNECTION_LOST_LIVE;
			}
			else
			{			
				switch( app.GetDisconnectReason() )
				{
				case DisconnectPacket::eDisconnect_Kicked:
					exitReasonStringId = IDS_DISCONNECTED_KICKED;
					break;
				case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
					exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
					exitReasonTitleId = IDS_CONNECTION_FAILED;
					break;
				case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
					exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
					exitReasonTitleId = IDS_CONNECTION_FAILED;
					break;
#ifdef _XBOX
				case DisconnectPacket::eDisconnect_NoUGC_Remote:
					exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_REMOTE;
					exitReasonTitleId = IDS_CONNECTION_FAILED;
					break;
#endif
				case DisconnectPacket::eDisconnect_NoFlying:
					exitReasonStringId = IDS_DISCONNECTED_FLYING;
					break;
				case DisconnectPacket::eDisconnect_Quitting:
					exitReasonStringId = IDS_DISCONNECTED_SERVER_QUIT;
					break;
				case DisconnectPacket::eDisconnect_NoFriendsInGame:
					exitReasonStringId = IDS_DISCONNECTED_NO_FRIENDS_IN_GAME;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_Banned:
					exitReasonStringId = IDS_DISCONNECTED_BANNED;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_NotFriendsWithHost:
					exitReasonStringId = IDS_NOTALLOWED_FRIENDSOFFRIENDS;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_OutdatedServer:
					exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_OutdatedClient:
					exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_ServerFull:
					exitReasonStringId = IDS_DISCONNECTED_SERVER_FULL;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				default:
					exitReasonStringId = IDS_CONNECTION_LOST_SERVER;
				}
			}
			//pMinecraft->progressRenderer->progressStartNoAbort( exitReasonStringId );

			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
			// 4J Stu - Fix for #48669 - TU5: Code: Compliance: TCR #15: Incorrect/misleading messages after signing out a profile during online game session.
			// If the primary player is signed out, then that is most likely the cause of the disconnection so don't display a message box. This will allow the message box requested by the libraries to be brought up
			if( ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad())) ui.RequestMessageBox( exitReasonTitleId, exitReasonStringId, uiIDA,1,ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable());
			exitReasonStringId = -1;

			// 4J - Force a disconnection, this handles the situation that the server has already disconnected
			if( pMinecraft->levels[0] != NULL ) pMinecraft->levels[0]->disconnect(false);
			if( pMinecraft->levels[1] != NULL ) pMinecraft->levels[1]->disconnect(false);
			if( pMinecraft->levels[2] != NULL ) pMinecraft->levels[2]->disconnect(false);
		}
		else
		{
			exitReasonStringId = IDS_EXITING_GAME;
			pMinecraft->progressRenderer->progressStartNoAbort( IDS_EXITING_GAME );
			if( pMinecraft->levels[0] != NULL ) pMinecraft->levels[0]->disconnect();
			if( pMinecraft->levels[1] != NULL ) pMinecraft->levels[1]->disconnect();
			if( pMinecraft->levels[2] != NULL ) pMinecraft->levels[2]->disconnect();
		}

		// 4J Stu - This only does something if we actually have a server, so don't need to do any other checks
		MinecraftServer::HaltServer();

		// We need to call the stats & leaderboards save before we exit the session
		// 4J We need to do this in a QNet callback where it is safe
		//pMinecraft->forceStatsSave();
		saveStats = false;

		// 4J Stu - Leave the session once the disconnect packet has been sent
		g_NetworkManager.LeaveGame(FALSE);
	}
	else
	{
		if(lpParameter != NULL && ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad()) )
		{
			switch( app.GetDisconnectReason() )
			{
			case DisconnectPacket::eDisconnect_Kicked:
				exitReasonStringId = IDS_DISCONNECTED_KICKED;
				break;
			case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
				exitReasonTitleId = IDS_CONNECTION_FAILED;
				break;
			case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
				exitReasonTitleId = IDS_CONNECTION_FAILED;
				break;
#ifdef _XBOX
			case DisconnectPacket::eDisconnect_NoUGC_Remote:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_REMOTE;
				exitReasonTitleId = IDS_CONNECTION_FAILED;
				break;
#endif
			case DisconnectPacket::eDisconnect_Quitting:
				exitReasonStringId = IDS_DISCONNECTED_SERVER_QUIT;
				break;
			case DisconnectPacket::eDisconnect_NoMultiplayerPrivilegesJoin:
				exitReasonStringId = IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT;
				break;
			case DisconnectPacket::eDisconnect_OutdatedServer:
				exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
				exitReasonTitleId = IDS_CANTJOIN_TITLE;
				break;
			case DisconnectPacket::eDisconnect_OutdatedClient:
				exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
				exitReasonTitleId = IDS_CANTJOIN_TITLE;
				break;
			case DisconnectPacket::eDisconnect_ServerFull:
				exitReasonStringId = IDS_DISCONNECTED_SERVER_FULL;
				exitReasonTitleId = IDS_CANTJOIN_TITLE;
				break;
			default:
				exitReasonStringId = IDS_DISCONNECTED;
			}
			//pMinecraft->progressRenderer->progressStartNoAbort( exitReasonStringId );

			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
			ui.RequestMessageBox( exitReasonTitleId, exitReasonStringId, uiIDA,1,ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable());
			exitReasonStringId = -1;
		}
	}
	// Fix for #93148 - TCR 001: BAS Game Stability: Title will crash for the multiplayer client if host of the game will exit during the clients loading to created world.
	while( g_NetworkManager.IsNetworkThreadRunning() )
	{
		Sleep(1);
	}
	pMinecraft->setLevel(NULL,exitReasonStringId,nullptr,saveStats);

	TelemetryManager->Flush();

	app.m_gameRules.unloadCurrentGameRules();
	//app.m_Audio.unloadCurrentAudioDetails();

	MinecraftServer::resetFlags();

	// Fix for #48385 - BLACK OPS :TU5: Functional: Client becomes pseudo soft-locked when returned to the main menu after a remote disconnect
	// Make sure there is text explaining why the player is waiting
	pMinecraft->progressRenderer->progressStart(IDS_EXITING_GAME);

	// Fix for #13259 - CRASH: Gameplay: loading process is halted when player loads saved data
	// We can't start/join a new game until the session is destroyed, so wait for it to be idle again
	while( g_NetworkManager.IsInSession() )
	{
		Sleep(1);
	}

	app.SetChangingSessionType(false);
	app.SetReallyChangingSessionType(false);
}

void UIScene_PauseMenu::SetIgnoreInput(bool ignoreInput)
{
	m_bIgnoreInput = ignoreInput;
}