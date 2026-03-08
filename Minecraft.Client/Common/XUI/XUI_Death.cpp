// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../XUI/XUI_Death.h"
#include <assert.h>
#include "../../../Minecraft.World/AABB.h"
#include "../../../Minecraft.World/Vec3.h"
#include "../../../Minecraft.World/net.minecraft.stats.h"
#include "../../../Minecraft.Client/StatsCounter.h"
#include "../../../Minecraft.World/Entity.h"
#include "../../../Minecraft.Client/MultiPlayerLocalPlayer.h"
#include "../../../Minecraft.World/Level.h"
#include "../../../Minecraft.World/ChunkSource.h"
#include "../../../Minecraft.Client/ProgressRenderer.h"
#include "../../../Minecraft.Client/GameRenderer.h"
#include "../../../Minecraft.Client/LevelRenderer.h"
#include "../../../Minecraft.World/Pos.h"
#include "../../../Minecraft.World/Dimension.h"
#include "../../Minecraft.h"
#include "../../Options.h"
#include "../../LocalPlayer.h"
#include "../../../Minecraft.World/compression.h"
//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_Death::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	m_iPad = *(int *)pInitData->pvInitData;

	m_bIgnoreInput = false;

	MapChildControls();
	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}
	
	XuiControlSetText(m_Title,app.GetString(IDS_YOU_DIED));
	XuiControlSetText(m_Buttons[BUTTON_DEATH_RESPAWN],app.GetString(IDS_RESPAWN));
	XuiControlSetText(m_Buttons[BUTTON_DEATH_EXITGAME],app.GetString(IDS_EXIT_GAME));

	// Display the tooltips
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT);

	return S_OK;
}

//----------------------------------------------------------------------------------
// Updates the UI when the list selection changes.
//----------------------------------------------------------------------------------
HRESULT CScene_Death::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if( hObjSource == m_Scene )
	{
		/*int curSel = m_List.GetCurSel();

		// Set the locale with the current language.
		XuiSetLocale( Languages[curSel].pszLanguagePath );

		// Apply the locale to the main scene.
		XuiApplyLocale( m_hObj, NULL );

		// Update the text for the current value.
		m_Value.SetText( m_List.GetText( curSel ) );*/



		bHandled = TRUE;
	}

	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_Death::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	unsigned int uiButtonCounter=0;

	while((uiButtonCounter<BUTTONS_DEATH_MAX) && (m_Buttons[uiButtonCounter]!=hObjPressed)) uiButtonCounter++;

	Minecraft *pMinecraft=Minecraft::GetInstance();

	// Determine which button was pressed,
	// and call the appropriate function.	
	switch(uiButtonCounter)
	{
	case BUTTON_DEATH_EXITGAME:		
		{
			// 4J-PB - fix for #8333 - BLOCKER: If player decides to exit game, then cancels the exit player becomes stuck at game over screen
			//m_bIgnoreInput = true;
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
					TelemetryManager->RecordLevelExit(pNotifyPressData->UserIndex, eSen_LevelExitStatus_Failed);
					
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

							StorageManager.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, pNotifyPressData->UserIndex,&UIScene_PauseMenu::ExitGameSaveDialogReturned,this, app.GetStringTable());
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
					TelemetryManager->RecordLevelExit(pNotifyPressData->UserIndex, eSen_LevelExitStatus_Failed);
					
					// just exit the player
					app.SetAction(pNotifyPressData->UserIndex,eAppAction_ExitPlayer);
				}		
			}
			else
			{
				// is it the primary player exiting?
				if(pNotifyPressData->UserIndex==ProfileManager.GetPrimaryPad())
				{
					TelemetryManager->RecordLevelExit(pNotifyPressData->UserIndex, eSen_LevelExitStatus_Failed);
					
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
					TelemetryManager->RecordLevelExit(pNotifyPressData->UserIndex, eSen_LevelExitStatus_Failed);

					// just exit the player
					app.SetAction(pNotifyPressData->UserIndex,eAppAction_ExitPlayer);
				}
			}
		}
		break;
	case BUTTON_DEATH_RESPAWN:
		{
			m_bIgnoreInput = true;
			app.SetAction(pNotifyPressData->UserIndex,eAppAction_Respawn);
		}

		break;
	default:
		break;
	}



	return S_OK;
}

HRESULT CScene_Death::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);


	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_PAD_START:
	case VK_ESCAPE:

		// kill the crafting xui
		// 4J Stu - No back out, must choose
		//app.CloseXuiScenes();

		rfHandled = TRUE;

		break;
	}

	return S_OK;
}

int CScene_Death::RespawnThreadProc( void* lpParameter )
{
	AABB::UseDefaultThreadStorage();
	Vec3::UseDefaultThreadStorage();
	Compression::UseDefaultThreadStorage();
	size_t iPad=(size_t)lpParameter;

	Minecraft *pMinecraft=Minecraft::GetInstance();

	pMinecraft->localplayers[iPad]->respawn();

	app.SetGameStarted(true);
	pMinecraft->gameRenderer->EnableUpdateThread();
	
	// If we are online, then we should wait here until the respawn is done
	// If we are offline, this should release straight away
	//WaitForSingleObject( pMinecraft->m_hPlayerRespawned, INFINITE );
	while(pMinecraft->localplayers[iPad]->GetPlayerRespawned()==false)
	{
		Sleep(50);
	}

	return S_OK;
}

HRESULT CScene_Death::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}