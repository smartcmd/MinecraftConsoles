#include "stdafx.h"
#include "XUI_PartnernetPassword.h"
#include "..\XUI\XUI_Ctrl_4JList.h"

#ifdef _CONTENT_PACKAGE
#ifndef _FINAL_BUILD

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_PartnernetPassword::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT);

	m_PartnernetPassword.SetEnable(true);

	m_PartnernetPassword.SetTextLimit(XCONTENT_MAX_DISPLAYNAME_LENGTH);
	
	// set the caret to the end of the default text
	m_PartnernetPassword.SetCaretPosition(0);
	m_PartnernetPassword.SetKeyboardType(C_4JInput::EKeyboardMode_Phone);

	m_PartnernetPassword.SetTitleAndText(IDS_NAME_WORLD,IDS_NAME_WORLD_TEXT);

	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_PartnernetPassword::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(hObjPressed==m_OK)
	{
		// create the world and launch
		wstring wPassword = m_PartnernetPassword.GetText();
		if(wPassword==L"5183")
		{
			app.NavigateBack(pNotifyPressData->UserIndex);
			app.SetPartnernetPasswordRunning(false);
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1);
		}
		rfHandled = TRUE;
	}

	return S_OK;
}

HRESULT CScene_PartnernetPassword::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_A:
	case VK_PAD_B:
	case VK_ESCAPE:
		rfHandled = TRUE;
		break;
	}
	return S_OK;
}

#endif
#endif