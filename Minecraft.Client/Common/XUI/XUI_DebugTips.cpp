#include "stdafx.h"

#include <assert.h>
#include "XUI_DebugTips.h"
#include "../../../Minecraft.World/StringHelpers.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_DebugTips::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{	
	m_iPad = *(int *)pInitData->pvInitData;

	m_bIgnoreInput = false;

	MapChildControls();

	// Display the tooltips
	//ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT);
	// display the next tip
	wstring wsText=app.FormatHTMLString(m_iPad,app.GetString(app.GetNextTip()));
	wchar_t startTags[64];
	swprintf(startTags,64,L"<font color=\"#%08x\" size=14><DIV ALIGN=CENTER>",app.GetHTMLColour(eHTMLColor_White));
	wsText= startTags + wsText + L"</DIV>";
	XuiControlSetText(m_tip,wsText.c_str());

	return S_OK;
}


HRESULT CScene_DebugTips::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	//ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);


	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_A:
		{

		
		// next tip
		// display the next tip
		wstring wsText=app.FormatHTMLString(m_iPad,app.GetString(app.GetNextTip()));
		wchar_t startTags[64];
		swprintf(startTags,64,L"<font color=\"#%08x\" size=14><DIV ALIGN=CENTER>",app.GetHTMLColour(eHTMLColor_White));
		wsText= startTags + wsText + L"</DIV>";
		XuiControlSetText(m_tip,wsText.c_str());

		rfHandled = TRUE;
		}
		break;

	case VK_PAD_B:
	case VK_PAD_START:
	case VK_ESCAPE:

		app.NavigateBack(m_iPad);

		rfHandled = TRUE;

		break;
#ifndef _CONTENT_PACKAGE
	case VK_PAD_LTHUMB_PRESS:
#ifdef _XBOX
		app.OverrideFontRenderer(true);
#endif
		break;
#endif
	}

	return S_OK;
}