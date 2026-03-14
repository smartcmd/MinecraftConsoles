// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../XUI/XUI_SettingsAll.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsAll::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	//WCHAR TempString[256];
	m_iPad=*static_cast<int *>(pInitData->pvInitData);
	// if we're not in the game, we need to use basescene 0 
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);

	MapChildControls();
	
	XuiControlSetText(m_Buttons[BUTTON_ALL_OPTIONS],app.GetString(IDS_OPTIONS));
	XuiControlSetText(m_Buttons[BUTTON_ALL_AUDIO],app.GetString(IDS_AUDIO));
	XuiControlSetText(m_Buttons[BUTTON_ALL_CONTROL],app.GetString(IDS_CONTROL));
	XuiControlSetText(m_Buttons[BUTTON_ALL_GRAPHICS],app.GetString(IDS_GRAPHICS));
	XuiControlSetText(m_Buttons[BUTTON_ALL_UI],app.GetString(IDS_USER_INTERFACE));
	XuiControlSetText(m_Buttons[BUTTON_ALL_RESETTODEFAULTS],app.GetString(IDS_RESET_TO_DEFAULTS));

	if(ProfileManager.GetPrimaryPad()!=m_iPad)
	{
		D3DXVECTOR3 vec,vecControl;
		m_Buttons[BUTTON_ALL_AUDIO].SetShow(FALSE);
		m_Buttons[BUTTON_ALL_AUDIO].SetEnable(FALSE);
		m_Buttons[BUTTON_ALL_GRAPHICS].SetShow(FALSE);
		m_Buttons[BUTTON_ALL_GRAPHICS].SetEnable(FALSE);

		float fButtonWidth, fButtonHeight;
		m_Buttons[BUTTON_ALL_GRAPHICS].GetPosition(&vecControl);	
		m_Buttons[BUTTON_ALL_RESETTODEFAULTS].GetBounds(&fButtonWidth, &fButtonHeight);
		m_Buttons[BUTTON_ALL_RESETTODEFAULTS].SetPosition(&vecControl);

		m_Buttons[BUTTON_ALL_CONTROL].GetPosition(&vec);
		m_Buttons[BUTTON_ALL_UI].SetPosition(&vec);

		m_Buttons[BUTTON_ALL_AUDIO].GetPosition(&vec);
		m_Buttons[BUTTON_ALL_CONTROL].SetPosition(&vec);
		
		// Resize the whole scene
		float fWidth, fHeight;
		GetBounds(&fWidth, &fHeight);
		SetBounds(fWidth, vecControl.y+fButtonHeight);
	}

	// if we're not in the game, we need to use basescene 0 
	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		CXuiSceneBase::ShowBackground( DEFAULT_XUI_MENU_USER, TRUE );
	}
	else
	{		
		ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		CXuiSceneBase::ShowBackground( m_iPad, FALSE );
	}

	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,false);
		CXuiSceneBase::ShowLogo( m_iPad, FALSE );
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

	return S_OK;
}




HRESULT CScene_SettingsAll::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr=S_OK;

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		// if the profile data has been changed, then force a profile write
		// It seems we're allowed to break the 5 minute rule if it's the result of a user action

		app.CheckGameSettingsChanged(true,pInputData->UserIndex);

		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	}

	return hr;
}

HRESULT CScene_SettingsAll::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	// added so we can skip greyed out items
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest!=nullptr)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

HRESULT CScene_SettingsAll::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	//HRESULT hr;
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{		
		// 4J-PB - Going to resize buttons if the text is too big to fit on any of them (Br-pt problem with the length of Unlock Full Game)
	}

	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsAll::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	unsigned int uiButtonCounter=0;

	while((uiButtonCounter<BUTTONS_ALL_MAX) && (m_Buttons[uiButtonCounter]!=hObjPressed)) uiButtonCounter++;

	switch(uiButtonCounter)
	{
	case BUTTON_ALL_OPTIONS:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_SettingsOptionsMenu);
	break;
	case BUTTON_ALL_AUDIO:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_SettingsAudioMenu);
	break;
	case BUTTON_ALL_CONTROL:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_SettingsControlMenu);
	break;
	case BUTTON_ALL_GRAPHICS:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_SettingsGraphicsMenu);
		break;
	case BUTTON_ALL_UI:
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_SettingsUIMenu);
		break;
	case BUTTON_ALL_RESETTODEFAULTS:
		{
			// check they really want to do this
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;

			StorageManager.RequestMessageBox(IDS_DEFAULTS_TITLE, IDS_DEFAULTS_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&CScene_SettingsAll::ResetDefaultsDialogReturned,this, app.GetStringTable());
		}		
	break;

	}
	
	rfHandled=TRUE;
	return S_OK;
}

HRESULT CScene_SettingsAll::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);

	// if we're not in the game, we need to use basescene 0 
	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );
	}
	else
	{		
		ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		if(!RenderManager.IsHiDef() || app.GetLocalPlayerCount()>1)
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

HRESULT CScene_SettingsAll::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

int CScene_SettingsAll::ResetDefaultsDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	//CScene_SettingsAll* pClass = (CScene_SettingsAll*)pParam;

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		app.SetDefaultOptions(ProfileManager.GetDashboardProfileSettings(iPad),iPad);
		// if the profile data has been changed, then force a profile write
		// It seems we're allowed to break the 5 minute rule if it's the result of a user action
		app.CheckGameSettingsChanged(true,iPad);
	}
	return 0;
}