#include "stdafx.h"
#include <assert.h>
#include "../XUI/XUI_SaveMessage.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_SaveMessage::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	CXuiSceneBase::ShowBackground( DEFAULT_XUI_MENU_USER, TRUE );
	CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );

	m_button.SetText(app.GetString(IDS_CONFIRM_OK));

	m_SaveMessage.SetText(app.GetString(IDS_SAVE_ICON_MESSAGE));

	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT );

	// 4J-PB - If we have a signed in user connected, let's get the DLC now
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if( (InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i)) ) 
		{
			if(!app.DLCInstallProcessCompleted() && !app.DLCInstallPending()) 
			{
				app.StartInstallDLCProcess(i);
				break;
			}
		}
	}

	// set a timer on the saving message screen, so we continue after 8 seconds
	XuiSetTimer( m_hObj,0,8000);
	m_bIgnoreInput=false;
	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_SaveMessage::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	XuiKillTimer(m_hObj,0);
	app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_MainMenu);
	rfHandled = TRUE;
	return S_OK;
}

HRESULT CScene_SaveMessage::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	return S_OK;
}

// 4J-PB  - added for Compliance fail - 
// Games must enter an interactive state that accepts player input within 20 seconds after the initial start-up sequence. 
// If an animation or cinematic shown during the start-up sequence runs longer than 20 seconds, it must be skippable using the START button or natural input.
HRESULT CScene_SaveMessage::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	m_bIgnoreInput=true;
	XuiKillTimer(m_hObj,0);
	app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_MainMenu);

	return S_OK;
}


