// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "..\XUI\XUI_HelpAndOptions.h"


//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_HelpAndOptions::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad = *static_cast<int *>(pInitData->pvInitData);
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);

	MapChildControls();
	XuiControlSetText(m_Buttons[BUTTON_HAO_CHANGESKIN],app.GetString(IDS_CHANGE_SKIN));
	XuiControlSetText(m_Buttons[BUTTON_HAO_HOWTOPLAY],app.GetString(IDS_HOW_TO_PLAY));
	XuiControlSetText(m_Buttons[BUTTON_HAO_CONTROLS],app.GetString(IDS_CONTROLS));
	XuiControlSetText(m_Buttons[BUTTON_HAO_SETTINGS],app.GetString(IDS_SETTINGS));
	XuiControlSetText(m_Buttons[BUTTON_HAO_CREDITS],app.GetString(IDS_CREDITS));
	XuiControlSetText(m_Buttons[BUTTON_HAO_REINSTALL],app.GetString(IDS_REINSTALL_CONTENT));
	XuiControlSetText(m_Buttons[BUTTON_HAO_DEBUG],app.GetString(IDS_DEBUG_SETTINGS));

	//if(app.GetTMSDLCInfoRead())
	{
		m_Timer.SetShow(FALSE);
		m_bIgnoreInput=false;

	#ifndef _FINAL_BUILD
		if(!app.DebugSettingsOn())
		{
			m_Buttons[BUTTON_HAO_DEBUG].SetEnable(FALSE);
			m_Buttons[BUTTON_HAO_DEBUG].SetShow(FALSE);
		}
		else
		{
			m_Buttons[BUTTON_HAO_DEBUG].SetEnable(TRUE);
			m_Buttons[BUTTON_HAO_DEBUG].SetShow(TRUE);

		}
	#endif

		// 4J-PB - do not need a storage device to see this menu - just need one when you choose to re-install them

		// any content to be re-installed? 
		if(m_iPad==ProfileManager.GetPrimaryPad() && bNotInGame)
		{
			// We should show the reinstall menu
			app.DebugPrintf("Reinstall Menu required...\n");
			m_Buttons[BUTTON_HAO_REINSTALL].SetEnable(TRUE);
			m_Buttons[BUTTON_HAO_REINSTALL].SetShow(TRUE);
		}
		else
		{
			m_Buttons[BUTTON_HAO_REINSTALL].SetEnable(FALSE);
			m_Buttons[BUTTON_HAO_REINSTALL].SetShow(FALSE);
		}

		if(app.GetLocalPlayerCount()>1)
		{
			// no credits in splitscreen
			D3DXVECTOR3 vec;

			m_Buttons[BUTTON_HAO_CREDITS].GetPosition(&vec);

			m_Buttons[BUTTON_HAO_CREDITS].SetEnable(FALSE);
			m_Buttons[BUTTON_HAO_CREDITS].SetShow(FALSE);

			m_Buttons[BUTTON_HAO_REINSTALL].SetPosition(&vec);

			app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,false);
			CXuiSceneBase::ShowLogo( m_iPad, FALSE );

			if(ProfileManager.GetPrimaryPad()!=m_iPad)
			{
				m_Buttons[BUTTON_HAO_DEBUG].SetEnable(FALSE);
				m_Buttons[BUTTON_HAO_DEBUG].SetShow(FALSE);
			}
		}
		else
		{
			if(bNotInGame)
			{
				CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );
			}
			else
			{
				CXuiSceneBase::ShowLogo( m_iPad, TRUE );
			}
		}
		// Display the tooltips

		// if we're not in the game, we need to use basescene 0 
		if(bNotInGame)
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		}
		else
		{
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		}

	
		if(!ProfileManager.IsFullVersion() )//|| ProfileManager.IsGuest(m_iPad))
		{
			// disable save button
			m_Buttons[BUTTON_HAO_CHANGESKIN].SetEnable(FALSE);
			m_Buttons[BUTTON_HAO_CHANGESKIN].EnableInput(FALSE);
			// set the focus to the second button

			XuiElementSetUserFocus(m_Buttons[BUTTON_HAO_HOWTOPLAY].m_hObj, m_iPad);
		}
	}
	/*else
	{
		m_bIgnoreInput=true;
		m_Timer.SetShow(TRUE);

		// hide all the buttons
		for(int i=0;i<BUTTONS_HAO_MAX;i++)
		{
			m_Buttons[i].SetEnable(FALSE);
			m_Buttons[i].SetShow(FALSE);
		}

		if(bNotInGame)
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1,-1);
		}
		else
		{
			ui.SetTooltips( m_iPad, -1,-1);
		}
	}*/
	return S_OK;
}

/*HRESULT CScene_HelpAndOptions::OnTMSDLCFileRetrieved( )
{
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);
	m_Timer.SetShow(FALSE);
	m_bIgnoreInput=false;

	// show all the buttons - except the debug settings and reinstall content
	m_Buttons[BUTTON_HAO_CHANGESKIN].SetEnable(TRUE);
	m_Buttons[BUTTON_HAO_CHANGESKIN].SetShow(TRUE);
	m_Buttons[BUTTON_HAO_HOWTOPLAY].SetEnable(TRUE);
	m_Buttons[BUTTON_HAO_HOWTOPLAY].SetShow(TRUE);
	m_Buttons[BUTTON_HAO_CONTROLS].SetEnable(TRUE);
	m_Buttons[BUTTON_HAO_CONTROLS].SetShow(TRUE);
	m_Buttons[BUTTON_HAO_SETTINGS].SetEnable(TRUE);
	m_Buttons[BUTTON_HAO_SETTINGS].SetShow(TRUE);
	m_Buttons[BUTTON_HAO_CREDITS].SetEnable(TRUE);
	m_Buttons[BUTTON_HAO_CREDITS].SetShow(TRUE);



#ifndef _FINAL_BUILD
	if(!app.DebugSettingsOn())
	{
		m_Buttons[BUTTON_HAO_DEBUG].SetEnable(FALSE);
		m_Buttons[BUTTON_HAO_DEBUG].SetShow(FALSE);
	}
	else
	{
		m_Buttons[BUTTON_HAO_DEBUG].SetEnable(TRUE);
		m_Buttons[BUTTON_HAO_DEBUG].SetShow(TRUE);

	}
#endif

	// 4J-PB - do not need a storage device to see this menu - just need one when you choose to re-install them

	// any content to be re-installed? 
	if(m_iPad==ProfileManager.GetPrimaryPad() && bNotInGame)
	{
		// We should show the reinstall menu
		app.DebugPrintf("Reinstall Menu required...\n");
		m_Buttons[BUTTON_HAO_REINSTALL].SetEnable(TRUE);
		m_Buttons[BUTTON_HAO_REINSTALL].SetShow(TRUE);
	}
	else
	{
		m_Buttons[BUTTON_HAO_REINSTALL].SetEnable(FALSE);
		m_Buttons[BUTTON_HAO_REINSTALL].SetShow(FALSE);
	}

	if(app.GetLocalPlayerCount()>1)
	{
		// no credits in splitscreen
		D3DXVECTOR3 vec;

		m_Buttons[BUTTON_HAO_CREDITS].GetPosition(&vec);

		m_Buttons[BUTTON_HAO_CREDITS].SetEnable(FALSE);
		m_Buttons[BUTTON_HAO_CREDITS].SetShow(FALSE);

		m_Buttons[BUTTON_HAO_REINSTALL].SetPosition(&vec);

		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,false);
		CXuiSceneBase::ShowLogo( m_iPad, FALSE );

		if(ProfileManager.GetPrimaryPad()!=m_iPad)
		{
			m_Buttons[BUTTON_HAO_DEBUG].SetEnable(FALSE);
			m_Buttons[BUTTON_HAO_DEBUG].SetShow(FALSE);
		}
	}
	else
	{
		if(bNotInGame)
		{
			CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );
		}
		else
		{
			CXuiSceneBase::ShowLogo( m_iPad, TRUE );
		}
	}
	// Display the tooltips

	// if we're not in the game, we need to use basescene 0 
	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
	}
	else
	{
		ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
	}


	if(!ProfileManager.IsFullVersion() )//|| ProfileManager.IsGuest(m_iPad))
	{
		// disable save button
		m_Buttons[BUTTON_HAO_CHANGESKIN].SetEnable(FALSE);
		m_Buttons[BUTTON_HAO_CHANGESKIN].EnableInput(FALSE);
		// set the focus to the second button

		XuiElementSetUserFocus(m_Buttons[BUTTON_HAO_HOWTOPLAY].m_hObj, m_iPad);
	}
	else
	{
		XuiElementSetUserFocus(m_Buttons[BUTTON_HAO_CHANGESKIN].m_hObj, m_iPad);
	}
	return S_OK;
}*/

HRESULT CScene_HelpAndOptions::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest!=nullptr)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_HelpAndOptions::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	unsigned int uiButtonCounter=0;

	while((uiButtonCounter<BUTTONS_HAO_MAX) && (m_Buttons[uiButtonCounter]!=hObjPressed)) uiButtonCounter++;

	// Determine which button was pressed,
	// and call the appropriate function.	

	switch(uiButtonCounter)
	{
	case BUTTON_HAO_HOWTOPLAY:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_HowToPlayMenu);
		break;
 	case BUTTON_HAO_CONTROLS:
 		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_ControlsMenu);
 		break;
	case BUTTON_HAO_SETTINGS:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_SettingsMenu);
		break;
	case BUTTON_HAO_CHANGESKIN:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_SkinSelectMenu);
		break;
	case BUTTON_HAO_CREDITS:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_Credits);
		break;

	case BUTTON_HAO_REINSTALL:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_ReinstallMenu);
		break;

	case BUTTON_HAO_DEBUG:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_DebugOptions);
		break;

	default:
		break;
	}
	
	return S_OK;
}

HRESULT CScene_HelpAndOptions::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// ignore a held down key to avoid skipping this scene if the user changes a setting and presses B for slightly too long
	if((pInputData->dwFlags&XUI_INPUT_FLAG_REPEAT)&&(pInputData->dwKeyCode==VK_PAD_B))
	{
		return S_OK;
	}

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_ESCAPE:
		BYTE userIndex = pInputData->UserIndex;
		if( !app.IsPauseMenuDisplayed(userIndex) )
		{
			// If we are not from a pause menu, then we are from the main menu
			userIndex = XUSER_INDEX_ANY;
		}

		app.NavigateBack(userIndex);
		rfHandled = TRUE;

		break;
	}

	return S_OK;
}

HRESULT CScene_HelpAndOptions::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);

	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );

		if(m_iPad==ProfileManager.GetPrimaryPad() )//&&
			//		StorageManager.GetSaveDeviceSelected(m_iPad) && 
			// !StorageManager.GetSaveDisabled() &&
// 			(	ProfileManager.IsAwardsFlagSet(m_iPad,eAward_mine100Blocks) || 
// 			ProfileManager.IsAwardsFlagSet(m_iPad,eAward_kill10Creepers) || 
// 			ProfileManager.IsAwardsFlagSet(m_iPad,eAward_eatPorkChop) || 
// 			ProfileManager.IsAwardsFlagSet(m_iPad,eAward_play100Days) || 
// 			ProfileManager.IsAwardsFlagSet(m_iPad,eAward_arrowKillCreeper) || 
// 			ProfileManager.IsAwardsFlagSet(m_iPad,eAward_socialPost)) )
		{
			// We should show the reinstall menu
			app.DebugPrintf("Reinstall Menu required...\n");
			m_Buttons[BUTTON_HAO_REINSTALL].SetEnable(TRUE);
			m_Buttons[BUTTON_HAO_REINSTALL].SetShow(TRUE);
		}
		else
		{
			m_Buttons[BUTTON_HAO_REINSTALL].SetEnable(FALSE);
			m_Buttons[BUTTON_HAO_REINSTALL].SetShow(FALSE);
		}

	}
	else
	{
		ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		if(app.GetLocalPlayerCount()>1)
		{
			CXuiSceneBase::ShowLogo( m_iPad, FALSE );
		}
		else
		{
			CXuiSceneBase::ShowLogo( m_iPad, TRUE );
		}
	}

	return S_OK;
}


// int CScene_HelpAndOptions::ResetDefaultsDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
// {
// 	CScene_HelpAndOptions* pClass = (CScene_HelpAndOptions*)pParam;
// 
// 	// results switched for this dialog
// 	if(result==C4JStorage::EMessage_ResultDecline) 
// 	{
// 		app.SetDefaultOptions(ProfileManager.GetDashboardProfileSettings(pClass->m_iPad),pClass->m_iPad);
// 		// if the profile data has been changed, then force a profile write
// 		// It seems we're allowed to break the 5 minute rule if it's the result of a user action
// 		app.CheckGameSettingsChanged(true,iPad);
// 	}
// 	return 0;
// }

HRESULT CScene_HelpAndOptions::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{

		// 4J-PB - Going to resize buttons if the text is too big to fit on any of them (Br-pt problem with the length of Unlock Full Game)
		XUIRect xuiRect;
		HXUIOBJ visual=nullptr;
		HXUIOBJ text;
		float fMaxTextLen=0.0f;
		float fTextVisualLen;
		float fMaxButton;
		float fWidth,fHeight;

		HRESULT hr=XuiControlGetVisual(m_Buttons[0].m_hObj,&visual);
		hr=XuiElementGetChildById(visual,L"text_Label",&text);
		hr=XuiElementGetBounds(text,&fTextVisualLen,&fHeight);
		m_Buttons[0].GetBounds(&fMaxButton,&fHeight);


		for(int i=0;i<BUTTONS_HAO_MAX;i++)
		{
			hr=XuiTextPresenterMeasureText(text, m_Buttons[i].GetText(), &xuiRect);
			if(xuiRect.right>fMaxTextLen) fMaxTextLen=xuiRect.right;
		}

		if(fTextVisualLen<fMaxTextLen)
		{
			D3DXVECTOR3 vec;

			// centre is vec.x+(fWidth/2)
			for(int i=0;i<BUTTONS_HAO_MAX;i++)
			{
				// need to resize and reposition the buttons
				m_Buttons[i].GetPosition(&vec);
				m_Buttons[i].GetBounds(&fWidth,&fHeight);
				vec.x= vec.x+(fWidth/2.0f)-(fMaxTextLen/2.0f); 

				m_Buttons[i].SetPosition(&vec);
				m_Buttons[i].SetBounds(fMaxButton+fMaxTextLen-fTextVisualLen,fHeight);
			}
		}
	}

	return S_OK;
}


HRESULT CScene_HelpAndOptions::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining,false);
}