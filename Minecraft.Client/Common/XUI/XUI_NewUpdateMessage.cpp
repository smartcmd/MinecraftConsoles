

#include "stdafx.h"
 // #include "XUI_Ctrl_4JIcon.h"
#include "XUI_NewUpdateMessage.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"


HRESULT CScene_NewUpdateMessage::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad = *static_cast<int *>(pInitData->pvInitData);
	m_bIsSD=!RenderManager.IsHiDef() && !RenderManager.IsWidescreen();

	MapChildControls();

	ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1, IDS_TOOLTIPS_BACK );	 

	// set the text in the XuiHTMLMessage
	wchar_t formatting[40];
	wstring wstrTemp = app.GetString(IDS_TITLEUPDATE);
	swprintf(formatting, 40, L"<font size=\"%d\">", m_bIsSD?12:14);
	wstrTemp = formatting + wstrTemp;

	wstring wsText=app.FormatHTMLString(m_iPad,wstrTemp);
	m_HTMLText.SetText(wsText.c_str());
	m_HTMLText.SetShow(TRUE);

	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_NewUpdateMessage, 0); 

	return S_OK;
}



HRESULT CScene_NewUpdateMessage::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		{
			int iVal=app.GetGameSettings(m_iPad,eGameSetting_DisplayUpdateMessage);
			if(iVal>0) iVal--;
		
		// set the update text as seen, by clearing the flag
		app.SetGameSettings(m_iPad,eGameSetting_DisplayUpdateMessage,iVal);
		// force a profile write
		app.CheckGameSettingsChanged(true,m_iPad);
		app.NavigateBack(XUSER_INDEX_ANY);
		rfHandled = TRUE;
		}

		break;
	case VK_PAD_RTHUMB_DOWN:
	case VK_PAD_LTHUMB_DOWN:
		{
			XUIHtmlScrollInfo ScrollInfo;

			XuiHtmlControlGetVScrollInfo(m_HTMLText.m_hObj,&ScrollInfo);
			if(!ScrollInfo.bScrolling)
			{
				XuiHtmlControlVScrollBy(m_HTMLText.m_hObj,1);
			}
		}
		break;
	case VK_PAD_RTHUMB_UP:
	case VK_PAD_LTHUMB_UP:
		{
			XUIHtmlScrollInfo ScrollInfo;

			XuiHtmlControlGetVScrollInfo(m_HTMLText.m_hObj,&ScrollInfo);
			if(!ScrollInfo.bScrolling)
			{
				XuiHtmlControlVScrollBy(m_HTMLText.m_hObj,-1);
			}
		}		
		break;	}

	return S_OK;
}

HRESULT CScene_NewUpdateMessage::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK );

	return S_OK;
}

