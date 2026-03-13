#include "stdafx.h"
#include "UI.h"
#include "UIScene_FullscreenProgress.h"
#include "../../Minecraft.h"
#include "../../ProgressRenderer.h"

UIScene_FullscreenProgress::UIScene_FullscreenProgress(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	parentLayer->addComponent(iPad,eUIComponent_Panorama);
	parentLayer->addComponent(iPad,eUIComponent_Logo);
	parentLayer->showComponent(iPad,eUIComponent_Logo,true);
	parentLayer->showComponent(iPad,eUIComponent_MenuBackground,false);

	m_controlTimer.setVisible( false );

	m_titleText = L"";
	m_statusText = L"";

	m_lastTitle = -1;
	m_lastStatus = -1;
	m_lastProgress = 0;

	m_buttonConfirm.init( app.GetString( IDS_CONFIRM_OK ), eControl_Confirm );
	m_buttonConfirm.setVisible(false);

	LoadingInputParams *params = static_cast<LoadingInputParams *>(initData);

	m_CompletionData = params->completionData;
	m_iPad=params->completionData->iPad;
	m_cancelFunc = params->cancelFunc;
	m_cancelFuncParam = params->m_cancelFuncParam;
	m_completeFunc = params->completeFunc;
	m_completeFuncParam = params->m_completeFuncParam;

	m_cancelText = params->cancelText;
	m_bWasCancelled=false;
	m_bWaitForThreadToDelete = params->waitForThreadToDelete;

	// Clear the progress text
	Minecraft *pMinecraft=Minecraft::GetInstance();
	pMinecraft->progressRenderer->progressStart(-1);
	pMinecraft->progressRenderer->progressStage(-1);
	m_progressBar.init(L"",0,0,100,0);

	// set the tip
	wstring wsText= app.FormatHTMLString(m_iPad,app.GetString(app.GetNextTip()));

	wchar_t startTags[64];
	swprintf(startTags,64,L"<font color=\"#%08x\"><p align=center>",app.GetHTMLColour(eHTMLColor_White));
	wsText= startTags + wsText + L"</p>";
	m_labelTip.init(wsText);

	addTimer(TIMER_FULLSCREEN_TIPS, TIMER_FULLSCREEN_TIPS_TIME);

	m_labelTitle.init(L"");

	m_labelTip.setVisible( m_CompletionData->bShowTips );

	thread = new C4JThread(params->func, params->lpParam, "FullscreenProgress");
	thread->SetProcessor(CPU_CORE_UI_SCENE); // TODO 4J Stu - Make sure this is a good thread/core to use

	m_threadCompleted = false;
	thread->Run();
	threadStarted = true;

#ifdef __PSVITA__
	ui.TouchBoxRebuild(this);
#endif

#ifdef _XBOX_ONE
	ui.ShowPlayerDisplayname(false);
#endif
}

UIScene_FullscreenProgress::~UIScene_FullscreenProgress()
{
	m_parentLayer->removeComponent(eUIComponent_Panorama);
	m_parentLayer->removeComponent(eUIComponent_Logo);

	delete thread;

	delete m_CompletionData;
}

wstring UIScene_FullscreenProgress::getMoviePath()
{
	return L"FullscreenProgress";
}

void UIScene_FullscreenProgress::updateTooltips()
{
	ui.SetTooltips( m_parentLayer->IsFullscreenGroup()?XUSER_INDEX_ANY:m_iPad, m_threadCompleted?IDS_TOOLTIPS_SELECT:-1, m_threadCompleted?-1:m_cancelText, -1, -1 );
}

void UIScene_FullscreenProgress::handleDestroy()
{
	int code = thread->GetExitCode();
	DWORD exitcode = *((DWORD *)&code);

	// If we're active, have a cancel func, and haven't already cancelled, call cancel func
	if( exitcode == STILL_ACTIVE && m_cancelFunc != nullptr && !m_bWasCancelled)
	{
		m_bWasCancelled = true;
		m_cancelFunc(m_cancelFuncParam);
	}
}

void UIScene_FullscreenProgress::tick()
{
	UIScene::tick();

	Minecraft *pMinecraft=Minecraft::GetInstance();

	int currentProgress = pMinecraft->progressRenderer->getCurrentPercent();
	if(currentProgress < 0) currentProgress = 0;
	if(currentProgress != m_lastProgress)
	{
		m_lastProgress = currentProgress;
		m_progressBar.setProgress(currentProgress);
		//app.DebugPrintf("Updated progress value\n");
	}

	int title = pMinecraft->progressRenderer->getCurrentTitle();
	if(title >= 0 && title != m_lastTitle)
	{
		m_lastTitle = title;
		m_titleText = app.GetString( title );
		m_labelTitle.setLabel(m_titleText);
	}

	ProgressRenderer::eProgressStringType eProgressType=pMinecraft->progressRenderer->getType();

	if(eProgressType==ProgressRenderer::eProgressStringType_ID)
	{
		int status = pMinecraft->progressRenderer->getCurrentStatus();
		if(status >= 0 && status != m_lastStatus)
		{
			m_lastStatus = status;
			m_statusText = app.GetString( status );
			m_progressBar.setLabel(m_statusText.c_str());
		}
	}
	else
	{
		wstring& wstrText = pMinecraft->progressRenderer->getProgressString();
		m_progressBar.setLabel(wstrText.c_str());
	}


	int code = thread->GetExitCode();
	DWORD exitcode = *((DWORD *)&code);

	//app.DebugPrintf("CScene_FullscreenProgress Timer %d\n",pTimer->nId);

	if( exitcode != STILL_ACTIVE )
	{
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
				ui.RequestErrorMessage( g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_FAILED), g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_LOST_SERVER), uiIDA,1, XUSER_INDEX_ANY);

				ui.NavigateToHomeMenu();
				ui.UpdatePlayerBasePositions();
			}
		}
		else
		{
			if(( m_CompletionData->bRequiresUserAction == TRUE ) && (!m_bWasCancelled))
			{				
				m_threadCompleted = true;
				m_buttonConfirm.setVisible( true );
				// 4J-TomK - rebuild touch after confirm button made visible again
#ifdef __PSVITA__
				ui.TouchBoxRebuild(this);
#endif
				updateTooltips();
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
						// 4J Stu - Fix for #65437 - Customer Encountered: Code: Settings: Autosave option doesn't work when the Host goes into idle state during gameplay.
						// Autosave obviously cannot occur if an ignore autosave menu is displayed, so even if we navigate back to a scene and not empty
						// then we still want to reset this flag which was set true by the navigate to the fullscreen progress
						ui.SetIgnoreAutosaveMenuDisplayed(m_iPad, false);

						// This just allows it to be shown
						Minecraft *pMinecraft = Minecraft::GetInstance();
						if(pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()] != nullptr) pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()]->getTutorial()->showTutorialPopup(true);
						ui.UpdatePlayerBasePositions();
						navigateBack();
					}
					break;

				case e_ProgressCompletion_NavigateBack:
					app.DebugPrintf("e_ProgressCompletion_NavigateBack\n");
					{
						ui.UpdatePlayerBasePositions();
						navigateBack();
					}
					break;
				case e_ProgressCompletion_NavigateBackToScene:
					app.DebugPrintf("e_ProgressCompletion_NavigateBackToScene\n");
					ui.UpdatePlayerBasePositions();
					// 4J Stu - If used correctly this scene will not have interfered with any other scene at all, so just navigate back
					navigateBack();
					break;
				case e_ProgressCompletion_CloseUIScenes:
					app.DebugPrintf("e_ProgressCompletion_CloseUIScenes\n");
					ui.CloseUIScenes(m_CompletionData->iPad);
					ui.UpdatePlayerBasePositions();
					break;
				case e_ProgressCompletion_CloseAllPlayersUIScenes:
					app.DebugPrintf("e_ProgressCompletion_CloseAllPlayersUIScenes\n");
					ui.CloseAllPlayersScenes();
					ui.UpdatePlayerBasePositions();
					break;
				case e_ProgressCompletion_NavigateToHomeMenu:
					app.DebugPrintf("e_ProgressCompletion_NavigateToHomeMenu\n");
					ui.NavigateToHomeMenu();
					ui.UpdatePlayerBasePositions();
					break;
				default:
					app.DebugPrintf("Default\n");
					break;
				}
			}								
		}
	}
}

void UIScene_FullscreenProgress::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//if( m_showTooltips )
	{
		//ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

		switch(key)
		{
		case ACTION_MENU_OK:
#ifdef __ORBIS__
		case ACTION_MENU_TOUCHPAD_PRESS:
#endif
			if(pressed && m_threadCompleted)
			{
				sendInputToMovie(key, repeat, pressed, released);
			}
			break;
		case ACTION_MENU_B:
		case ACTION_MENU_CANCEL:
			if( pressed && m_cancelFunc != nullptr && !m_bWasCancelled ) 			
			{
				m_bWasCancelled = true;
				m_cancelFunc( m_cancelFuncParam );
			}
			break;
		}
		handled = true;
	}
}

void UIScene_FullscreenProgress::handlePress(F64 controlId, F64 childId)
{
	if(m_threadCompleted && static_cast<int>(controlId) == eControl_Confirm)
	{
		// This assumes all buttons can only be pressed with the A button
		ui.AnimateKeyPress(m_iPad, ACTION_MENU_A, false, true, false);

		// if there's a complete function, call it
		if(m_completeFunc)
		{
			m_completeFunc(m_completeFuncParam);
		}

		switch(m_CompletionData->type)
		{
		case e_ProgressCompletion_NavigateBack:
			app.DebugPrintf("e_ProgressCompletion_NavigateBack\n");
			{
				ui.UpdatePlayerBasePositions();
				navigateBack();
			}
			break;
		case e_ProgressCompletion_NavigateBackToScene:
			app.DebugPrintf("e_ProgressCompletion_NavigateBackToScene\n");
			ui.UpdatePlayerBasePositions();
			// 4J Stu - If used correctly this scene will not have interfered with any other scene at all, so just navigate back
			navigateBack();
			break;
		case e_ProgressCompletion_CloseUIScenes:
			app.DebugPrintf("e_ProgressCompletion_CloseUIScenes\n");
			ui.CloseUIScenes(m_CompletionData->iPad);
			ui.UpdatePlayerBasePositions();
			break;
		case e_ProgressCompletion_CloseAllPlayersUIScenes:
			app.DebugPrintf("e_ProgressCompletion_CloseAllPlayersUIScenes\n");
			ui.CloseAllPlayersScenes();
			ui.UpdatePlayerBasePositions();
			break;
		case e_ProgressCompletion_NavigateToHomeMenu:
			app.DebugPrintf("e_ProgressCompletion_NavigateToHomeMenu\n");
			ui.NavigateToHomeMenu();
			ui.UpdatePlayerBasePositions();
			break;
		}
	}
}

void UIScene_FullscreenProgress::handleTimerComplete(int id)
{
	switch(id)
	{
	case TIMER_FULLSCREEN_TIPS:
		{		
			// display the next tip
			wstring wsText=app.FormatHTMLString(m_iPad,app.GetString(app.GetNextTip()));
			wchar_t startTags[64];
			swprintf(startTags,64,L"<font color=\"#%08x\"><p align=center>",app.GetHTMLColour(eHTMLColor_White));
			wsText= startTags + wsText + L"</p>";
			m_labelTip.setLabel(wsText);
		}
		break;
	}	
}

void UIScene_FullscreenProgress::SetWasCancelled(bool wasCancelled)
{
	m_bWasCancelled = wasCancelled;
}

bool UIScene_FullscreenProgress::isReadyToDelete()
{
	if( m_bWaitForThreadToDelete )
	{
		return !thread->isRunning();
	}
	else
	{
		return true;
	}
}