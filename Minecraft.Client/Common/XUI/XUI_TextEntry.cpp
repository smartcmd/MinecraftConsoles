#include "stdafx.h"
#include "XUI_TextEntry.h"
#include "..\..\MultiplayerLocalPlayer.h"


CScene_TextEntry::CommandParams CScene_TextEntry::CommandA[CScene_TextEntry::eCommand_MAX]=
{
	{ L"goto", L"%s%c%d%c%d" },
	{ L"give", L"%s%c%s" }
};

HRESULT CScene_TextEntry::Init_Commands()
{
	for(int i=0;i<eCommand_MAX;i++)
	{
		m_CommandSet[CommandA[i].wchCommand]=i;
	}
	return S_OK;
}

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_TextEntry::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	XuiTextInputParams *params = static_cast<XuiTextInputParams *>(pInitData->pvInitData);
	m_iPad=params->iPad;
	m_wchInitialChar=params->wch;
	delete params;

	WCHAR wchEditText[40];

	Init_Commands();

	ZeroMemory(wchEditText,sizeof(WCHAR)*40);
	wchEditText[0]=tolower(m_wchInitialChar);

	m_EditText.SetTextLimit(40);
	m_EditText.SetText(wchEditText);
	// set the caret to the end of the default text
	m_EditText.SetCaretPosition(1);

	m_EditText.SetTitleAndText(IDS_NAME_WORLD,IDS_NAME_WORLD_TEXT);

	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_EXECUTE_COMMAND, IDS_TOOLTIPS_BACK);
	return S_OK;
}

HRESULT CScene_TextEntry::OnNotifyValueChanged (HXUIOBJ hObjSource, XUINotifyValueChanged* pValueChangedData, BOOL& rfHandled)
{
	// If the user presses return, interpret the string, and exit
	if(pValueChangedData->nValue==10)
	{
		LPCWSTR pText = m_EditText.GetText();

		if(pText)
		{
			wstring wText = pText;
			InterpretString(wText);
		}

		app.NavigateBack(m_iPad);
		rfHandled = TRUE;
	}

	return S_OK;
}

HRESULT CScene_TextEntry::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_A:
		{
			LPCWSTR pText = m_EditText.GetText();

			if(pText)
			{
				wstring wText = pText;
				InterpretString(wText);
			}

			app.NavigateBack(m_iPad);
			rfHandled = TRUE;
		}
		break;

	case VK_PAD_B:
	case VK_ESCAPE:

		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	}
	return S_OK;
}

HRESULT CScene_TextEntry::InterpretString(wstring &wsText)
{
	wstring wsFormat;
	WCHAR wchCommand[40];
	int iCommand=-1;
	WCHAR wchSep[2];

#ifdef __PS3__
	// 4J Stu - The Xbox version uses swscanf_s which isn't available in GCC.
	swscanf(wsText.c_str(), L"%40s", wchCommand);
#else
	swscanf_s(wsText.c_str(), L"%s", wchCommand,40);
#endif

    auto it = m_CommandSet.find(wchCommand);
    if(it != m_CommandSet.end())
	{
		// found it

		iCommand=it->second;

		switch(iCommand)
		{
		case eCommand_Teleport:
			{
				int x,z;

#ifdef __PS3__
				// 4J Stu - The Xbox version uses swscanf_s which isn't available in GCC.
				swscanf(wsText.c_str(), L"%40s%c%d%c%d", wchCommand,wchSep,&x,wchSep,&z);
#else
				swscanf_s(wsText.c_str(), L"%s%c%d%c%d", wchCommand,40,wchSep,2,&x,wchSep,2, &z);
#endif

				app.DebugPrintf("eCommand_Teleport x=%d z=%d\n",x,z);

				// check the bounds
				int iBound=54*16;
				if( (x>-iBound) && (x<iBound) && (z>-iBound) && (z<iBound) )
				{
					// valid numbers
					Minecraft *pMinecraft=Minecraft::GetInstance();
					pMinecraft->localplayers[m_iPad]->teleportTo(x,pMinecraft->localplayers[m_iPad]->y,z);
				}
			}
			break;
		case eCommand_Give:
			{
				int iItem,iCount;

#ifdef __PS3__
				// 4J Stu - The Xbox version uses swscanf_s which isn't available in GCC.
				swscanf(wsText.c_str(), L"%40s%c%d%c%d", wchCommand,wchSep,&iItem,wchSep,&iCount);
#else
				swscanf_s(wsText.c_str(), L"%s%c%d%c%d", wchCommand,40,wchSep,2,&iItem,wchSep,2, &iCount);
#endif
				app.DebugPrintf("eCommand_Give, item=%d count=%d\n",iItem,iCount);
				Minecraft *pMinecraft=Minecraft::GetInstance();
				for(int i=0;i<iCount;i++)
				pMinecraft->localplayers[m_iPad]->drop(); // shared_ptr<ItemInstance>(new ItemInstance( iItem, 1, 0 )) );
			}

			break;
		default:
			app.DebugPrintf("Unknown command\n");
			break;
		}
	}

	return S_OK;
}