// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../XUI/XUI_SettingsControl.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsControl::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	WCHAR TempString[256];
	m_iPad=*(int *)pInitData->pvInitData;
	// if we're not in the game, we need to use basescene 0 
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);

	MapChildControls();

	// Display the tooltips

	m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INGAME].SetValue(app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InGame));
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INGAME ),app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InGame));	
	m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INGAME].SetText(TempString);

	m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INMENU].SetValue(app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InMenu));
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INMENU ),app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InMenu));	
	m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INMENU].SetText(TempString);

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

HRESULT CScene_SettingsControl::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled )
{
	WCHAR TempString[256];

	if(hObjSource==m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INGAME].GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_Sensitivity_InGame,pNotifyValueChanged->nValue);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INGAME ),pNotifyValueChanged->nValue);		
		m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INGAME].SetText(TempString);
	}
	else if(hObjSource==m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INMENU].GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_Sensitivity_InMenu,pNotifyValueChanged->nValue);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INMENU ),pNotifyValueChanged->nValue);		
		m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INMENU].SetText(TempString);
	}

	return S_OK;
}


HRESULT CScene_SettingsControl::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
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

		// not in this scene - app.CheckGameSettingsChanged(true,pInputData->UserIndex);

		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	}

	return hr;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsControl::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	return S_OK;
}

HRESULT CScene_SettingsControl::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	// added so we can skip greyed out items
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest!=NULL)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

HRESULT CScene_SettingsControl::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	HRESULT hr;
	WCHAR TempString[256];
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{		
		// 4J-PB - Going to resize buttons if the text is too big to fit on any of them (Br-pt problem with the length of Unlock Full Game)

		float fMaxTextLen=0.0f;
		float fMaxLen=0.0f;

		// sliders first
		HXUIOBJ hSlider,hVisual,hText;//,hCheckboxText;
		XUIRect xuiRect;
		float fWidth,fHeight,fTemp;
		D3DXVECTOR3 vec,vecCheckboxText,vSlider,vecCheckbox;

		// don't display values on these - we handle that
		for(int i=0;i<SLIDER_SETTINGS_CONTROL_MAX;i++)
		{
			m_SliderA[i].SetValueDisplay(FALSE);
		}

		hr=XuiElementGetChildById(m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INGAME].m_hObj,L"XuiSlider",&hSlider);
		hr=XuiControlGetVisual(hSlider,&hVisual);
		hr=XuiElementGetChildById(hVisual,L"text_Label",&hText);
		hr=XuiElementGetPosition(m_SliderA[SLIDER_SETTINGS_SENSITIVITY_INGAME].m_hObj,&vSlider);

		for(int i=0;i<SLIDER_SETTINGS_CONTROL_MAX;i++)
		{
			switch(i)
			{
			case SLIDER_SETTINGS_SENSITIVITY_INGAME: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INGAME ),999);	
				break;
			case SLIDER_SETTINGS_SENSITIVITY_INMENU: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INMENU ),999);	
				break;
			}
			hr=XuiTextPresenterMeasureText(hText, TempString, &xuiRect);
			// 4J-PB - the text measuring doesn't seem to be long enough - add a fudge
			xuiRect.right+=25.0f;
			m_SliderA[i].GetBounds(&fWidth,&fHeight);
			if(xuiRect.right>fMaxTextLen) fMaxTextLen=xuiRect.right;
			if(fWidth>fMaxLen) fMaxLen=fWidth;
		}
	
		if(fMaxLen<fMaxTextLen)
		{		
			float fWidth;
			XuiElementGetPosition(m_hObj,&vec);
			XuiElementGetBounds(m_hObj,&fWidth,&fHeight);
			
			// need to centre the scene now the size has changed
			if((!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()) || app.GetLocalPlayerCount()>1)
			{
				// scene width needs to be more that the text width on buttons
				fWidth=vSlider.x;
				vec.x=floorf((640.0f-(fMaxTextLen+fWidth))/2.0f);
				XuiElementSetPosition(m_hObj,&vec);
				XuiElementSetBounds(m_hObj,fMaxTextLen+(fWidth*2.0f),fHeight);
			}
			else
			{
				fWidth=vSlider.x;
				vec.x=floorf((1280.0f-(fMaxTextLen+fWidth))/2.0f);
				XuiElementSetPosition(m_hObj,&vec);
				XuiElementSetBounds(m_hObj,fMaxTextLen+(fWidth*2.0f),fHeight);
			}

			// centre is vec.x+(fWidth/2)
			for(int i=0;i<SLIDER_SETTINGS_CONTROL_MAX;i++)
			{
				hr=XuiElementGetChildById(m_SliderA[i].m_hObj,L"XuiSlider",&hSlider);
				XuiElementGetPosition(hSlider,&vec);
				XuiElementGetBounds(hSlider,&fTemp,&fHeight);
				XuiElementSetBounds(hSlider,fMaxTextLen,fHeight);
			}
			// Need to refresh the scenes visual since the object size has now changed
			XuiControlAttachVisual(m_hObj);
		}
	}

	return S_OK;
}

HRESULT CScene_SettingsControl::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);

	// if we're not in the game, we need to use basescene 0 
	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
	}
	else
	{		
		ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
	}

	return S_OK;
}

HRESULT CScene_SettingsControl::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

