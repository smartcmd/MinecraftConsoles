// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "..\..\Minecraft.h"
#include "..\..\ProgressRenderer.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\Common\Tutorial\TutorialMode.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_FullscreenProgress::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();

	m_buttonConfirm.SetText( app.GetString( IDS_CONFIRM_OK ) );

	LoadingInputParams *params = (LoadingInputParams *)pInitData->pvInitData;

	m_CompletionData = params->completionData;
	m_iPad=params->completionData->iPad;
	m_cancelFunc = params->cancelFunc;
	m_cancelFuncParam = params->m_cancelFuncParam;
	m_completeFunc = params->completeFunc;
	m_completeFuncParam = params->m_completeFuncParam;
	m_bWasCancelled=false;

	thread = new C4JThread(params->func, params->lpParam, "FullscreenProgress");
	thread->SetProcessor(3); // TODO 4J Stu - Make sure this is a good thread/core to use

	m_threadCompleted = false;
	threadStarted = false;
	//ResumeThread( thread );
	if( CXuiSceneBase::GetPlayerBasePosition(m_iPad) != CXuiSceneBase::e_BaseScene_Fullscreen && CXuiSceneBase::GetPlayerBasePosition(m_iPad) != CXuiSceneBase::e_BaseScene_NotSet)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,false);
		CXuiSceneBase::ShowLogo( m_iPad, FALSE );
	}
	else
	{
		CXuiSceneBase::ShowLogo( m_iPad, params->completionData->bShowLogo );
	}

	CXuiSceneBase::ShowBackground( m_iPad, params->completionData->bShowBackground );
	ui.SetTooltips( m_iPad, -1, params->cancelText, -1, -1 );

	// Clear the progress text
	Minecraft *pMinecraft=Minecraft::GetInstance();
	pMinecraft->progressRenderer->progressStart(-1);
	pMinecraft->progressRenderer->progressStage(-1);

	// set the tip
	wstring wsText=app.FormatHTMLString(m_iPad,app.GetString(app.GetNextTip()));
	wchar_t startTags[64];
	swprintf(startTags,64,L"<font color=\"#%08x\" size=14><DIV ALIGN=CENTER>",app.GetHTMLColour(eHTMLColor_White));
	wsText= startTags + wsText + L"</DIV>";
	XuiControlSetText(m_tip, wsText.c_str());

	m_tip.SetShow( m_CompletionData->bShowTips );

	return S_OK;
}

// The framework calls this handler when the object is to be destroyed. 
HRESULT CScene_FullscreenProgress::OnDestroy()
{
	if( thread != NULL && thread != INVALID_HANDLE_VALUE )
		delete thread;

	if( m_CompletionData != NULL )
		delete m_CompletionData;

	return S_OK;
}

HRESULT CScene_FullscreenProgress::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_ESCAPE:		
		// 4J-JEV: Fix for Xbox360 #162749 - TU17: Save Upload: Content: UI: Player is presented with non-functional Tooltips after the Upload Save For Xbox One is completed.
		if( m_cancelFunc != NULL && !m_threadCompleted ) 
		{
			m_cancelFunc( m_cancelFuncParam );
			m_bWasCancelled=true;
		}
		break;
	}

	return S_OK;
}

//----------------------------------------------------------------------------------
// Updates the UI when the list selection changes.
//----------------------------------------------------------------------------------
HRESULT CScene_FullscreenProgress::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{

	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_FullscreenProgress::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(m_threadCompleted == true && hObjPressed == m_buttonConfirm)
	{
		// if there's a complete function, call it
		if(m_completeFunc)
		{
			m_completeFunc(m_completeFuncParam);
		}
		switch(m_CompletionData->type)
		{
		case e_ProgressCompletion_NavigateBack:
			CXuiSceneBase::ShowBackground( m_CompletionData->iPad, FALSE );
			CXuiSceneBase::ShowBackground( m_CompletionData->iPad, TRUE );
			app.NavigateBack(m_CompletionData->iPad);
			// Show the other players scenes
			CXuiSceneBase::ShowOtherPlayersBaseScene(m_CompletionData->iPad, true);
			ui.UpdatePlayerBasePositions();
			break;
		case e_ProgressCompletion_NavigateBackToScene:
			CXuiSceneBase::ShowBackground( m_CompletionData->iPad, FALSE );
			CXuiSceneBase::ShowBackground( m_CompletionData->iPad, TRUE );
			CXuiSceneBase::ShowOtherPlayersBaseScene(m_CompletionData->iPad, true);
			// If the pause menu is still active, then navigate back
			// Otherwise close everything then navigate forwads to the pause menu
			if(app.IsSceneInStack(m_CompletionData->iPad, m_CompletionData->scene))
			{
				app.NavigateBack(m_CompletionData->iPad,false, m_CompletionData->scene);
			}
			else
			{
				app.CloseXuiScenesAndNavigateToScene(m_CompletionData->iPad,m_CompletionData->scene);
			}
			ui.UpdatePlayerBasePositions();
			break;
		case e_ProgressCompletion_CloseUIScenes:
			app.CloseXuiScenes(m_CompletionData->iPad);
			ui.UpdatePlayerBasePositions();
			break;
		case e_ProgressCompletion_CloseAllPlayersUIScenes:
			app.CloseAllPlayersXuiScenes();
			ui.UpdatePlayerBasePositions();
			break;
		case e_ProgressCompletion_NavigateToHomeMenu:
			app.NavigateToHomeMenu();
			ui.UpdatePlayerBasePositions();
			break;
		}
	}

	return S_OK;
}

HRESULT CScene_FullscreenProgress::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	// This gets called every frame, so use it to update our two text boxes
	
	Minecraft *pMinecraft=Minecraft::GetInstance();

	int title = pMinecraft->progressRenderer->getCurrentTitle();
	if(title >= 0)
		m_title.SetText( app.GetString( title ) );
	else
		m_title.SetText( L"" );

	int status = pMinecraft->progressRenderer->getCurrentStatus();
	if(status >= 0)
		m_status.SetText( app.GetString( status ) );
	else		
		m_status.SetText( L"" );

	return S_OK;
}

HRESULT CScene_FullscreenProgress::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(!threadStarted)
	{
		thread->Run();
		threadStarted = true;
		XuiSetTimer(m_hObj,TIMER_FULLSCREEN_PROGRESS,TIMER_FULLSCREEN_PROGRESS_TIME);
		XuiSetTimer(m_hObj,TIMER_FULLSCREEN_TIPS,TIMER_FULLSCREEN_TIPS_TIME);
	}
	return S_OK;
}

HRESULT CScene_FullscreenProgress::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	int code = thread->GetExitCode();
    DWORD exitcode = *((DWORD *)&code);

	//app.DebugPrintf("CScene_FullscreenProgress Timer %d\n",pTimer->nId);

	if( exitcode != STILL_ACTIVE )
	{
		// 4J-PB - need to kill the timers whatever happens
		XuiKillTimer(m_hObj,TIMER_FULLSCREEN_PROGRESS);
		XuiKillTimer(m_hObj,TIMER_FULLSCREEN_TIPS);
		XuiControlSetText(m_tip,L"");

		// hide the tips bar in cause we're waiting for the user to press ok
		m_tip.SetShow( FALSE );

		// If we failed (currently used by network connection thread), navigate back
		if( exitcode != S_OK )
		{
			if( exitcode == ERROR_CANCELLED )
			{
				// Current thread cancelled for whatever reason
				// Currently used only for the CConsoleMinecraftApp::RemoteSaveThreadProc thread
				// Assume to just ignore this thread as something else is now running that will
				// cause another action
			}
			else
			{
				/*m_threadCompleted = true;
				m_buttonConfirm.SetShow( TRUE );
				m_buttonConfirm.SetFocus( m_CompletionData->iPad );
				m_CompletionData->type = e_ProgressCompletion_NavigateToHomeMenu;

				int exitReasonStringId;
				switch( app.GetDisconnectReason() )
				{
				default:
					exitReasonStringId = IDS_CONNECTION_FAILED;
				}
				Minecraft *pMinecraft=Minecraft::GetInstance();
				pMinecraft->progressRenderer->progressStartNoAbort( exitReasonStringId );*/
				//app.NavigateBack(m_CompletionData->iPad);

				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				StorageManager.RequestMessageBox( IDS_CONNECTION_FAILED, IDS_CONNECTION_LOST_SERVER, uiIDA,1,ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable());
			
				app.NavigateToHomeMenu();
				ui.UpdatePlayerBasePositions();
			}
		}
		else
		{
			if(( m_CompletionData->bRequiresUserAction == TRUE ) && (!m_bWasCancelled))
			{				
				m_threadCompleted = true;
				m_buttonConfirm.SetShow( TRUE );
				m_buttonConfirm.SetFocus( ProfileManager.GetPrimaryPad() );
				ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT, -1, -1, -1 );
			}
			else
			{
				if(m_bWasCancelled)
				{
					m_threadCompleted = true;
				}
				app.DebugPrintf("FullScreenProgress complete with action: ");
				switch(m_CompletionData->type)
				{
				case e_ProgressCompletion_AutosaveNavigateBack:
					app.DebugPrintf("e_ProgressCompletion_AutosaveNavigateBack\n");
					{	
						// store these - they get wiped by the destroy caused by navigateback
						int iPad=m_CompletionData->iPad;
						//bool bAutosaveWasMenuDisplayed=m_CompletionData->bAutosaveWasMenuDisplayed;
						CXuiSceneBase::ShowBackground( iPad, FALSE );
						CXuiSceneBase::ShowLogo(iPad, FALSE );
						app.NavigateBack(iPad);

						// 4J Stu - Fix for #65437 - Customer Encountered: Code: Settings: Autosave option doesn't work when the Host goes into idle state during gameplay.
						// Autosave obviously cannot occur if an ignore autosave menu is displayed, so even if we navigate back to a scene and not empty
						// then we still want to reset this flag which was set true by the navigate to the fullscreen progress
						app.SetIgnoreAutosaveMenuDisplayed(iPad, false);

						// the navigate back leaves SetMenuDisplayed as true, but there may not have been a menu up when autosave was kicked off
// 						if(bAutosaveWasMenuDisplayed==false)
// 						{
// 							app.SetMenuDisplayed(iPad,false);
// 						}
						// Show the other players scenes
						CXuiSceneBase::ShowOtherPlayersBaseScene(iPad, true);
						// This just allows it to be shown
						Minecraft *pMinecraft = Minecraft::GetInstance();
						if(pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()] != NULL) pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()]->getTutorial()->showTutorialPopup(true);
						ui.UpdatePlayerBasePositions();
					}
					break;

				case e_ProgressCompletion_NavigateBack:
					app.DebugPrintf("e_ProgressCompletion_NavigateBack\n");
					{
						// store these - they get wiped by the destroy caused by navigateback
						int iPad=m_CompletionData->iPad;
					
						CXuiSceneBase::ShowBackground( iPad, FALSE );
						CXuiSceneBase::ShowBackground( iPad, TRUE );
						app.NavigateBack(iPad);
						// Show the other players scenes
						CXuiSceneBase::ShowOtherPlayersBaseScene(iPad, true);
						ui.UpdatePlayerBasePositions();
					}
					break;
				case e_ProgressCompletion_NavigateBackToScene:
					app.DebugPrintf("e_ProgressCompletion_NavigateBackToScene\n");
					CXuiSceneBase::ShowBackground( m_CompletionData->iPad, FALSE );
					CXuiSceneBase::ShowBackground( m_CompletionData->iPad, TRUE );
					CXuiSceneBase::ShowOtherPlayersBaseScene(m_CompletionData->iPad, true);
					// If the pause menu is still active, then navigate back
					// Otherwise close everything then navigate forwads to the pause menu
					if(app.IsSceneInStack(m_CompletionData->iPad, m_CompletionData->scene))
					{
						app.NavigateBack(m_CompletionData->iPad,false, m_CompletionData->scene);
					}
					else
					{
						app.CloseXuiScenesAndNavigateToScene(m_CompletionData->iPad,m_CompletionData->scene);
					}
					ui.UpdatePlayerBasePositions();
					break;
				case e_ProgressCompletion_CloseUIScenes:
					app.DebugPrintf("e_ProgressCompletion_CloseUIScenes\n");
					app.CloseXuiScenes(m_CompletionData->iPad);
					ui.UpdatePlayerBasePositions();
					break;
				case e_ProgressCompletion_CloseAllPlayersUIScenes:
					app.DebugPrintf("e_ProgressCompletion_CloseAllPlayersUIScenes\n");
					app.CloseAllPlayersXuiScenes();
					ui.UpdatePlayerBasePositions();
					break;
				case e_ProgressCompletion_NavigateToHomeMenu:
					app.DebugPrintf("e_ProgressCompletion_NavigateToHomeMenu\n");
					app.NavigateToHomeMenu();
					//ui.UpdatePlayerBasePositions();
					break;
				default:
					app.DebugPrintf("Default\n");
					break;
				}
			}								
		}
	}
	else
	{
		switch(pTimer->nId)
		{
		case TIMER_FULLSCREEN_PROGRESS:	
			break;
		case TIMER_FULLSCREEN_TIPS:
			{		
				// display the next tip
				wstring wsText=app.FormatHTMLString(m_iPad,app.GetString(app.GetNextTip()));
				wchar_t startTags[64];
				swprintf(startTags,64,L"<font color=\"#%08x\" size=14><DIV ALIGN=CENTER>",app.GetHTMLColour(eHTMLColor_White));
				wsText= startTags + wsText + L"</DIV>";
					XuiControlSetText(m_tip,wsText.c_str());
			}
			break;
		}			
	}
    
    bHandled = TRUE;
    
    return( S_OK );
}