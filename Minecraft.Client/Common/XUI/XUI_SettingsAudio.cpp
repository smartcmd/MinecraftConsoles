#include "stdafx.h"
#include "..\XUI\XUI_SettingsAudio.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsAudio::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	WCHAR TempString[256];
	m_iPad=*(int *)pInitData->pvInitData;
	// if we're not in the game, we need to use basescene 0 
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);

	MapChildControls();

	// Display the tooltips
	HRESULT hr = S_OK; 
	HXUIOBJ hSlider;

	m_SliderA[SLIDER_SETTINGS_MUSIC].SetValue(app.GetGameSettings(m_iPad,eGameSetting_MusicVolume));
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_MUSIC ),app.GetGameSettings(m_iPad,eGameSetting_MusicVolume));	
	m_SliderA[SLIDER_SETTINGS_MUSIC].SetText(TempString);

	m_SliderA[SLIDER_SETTINGS_SOUND].SetValue(app.GetGameSettings(m_iPad,eGameSetting_SoundFXVolume));
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SOUND ),app.GetGameSettings(m_iPad,eGameSetting_SoundFXVolume));	
	m_SliderA[SLIDER_SETTINGS_SOUND].SetText(TempString);
	
	// only the primary player gets to change the music and sound settings
	// only the primary player gets to change the gamma and splitscreen settings
	// only the primary player gets to change the difficulty settings	
	if(ProfileManager.GetPrimaryPad()!=m_iPad)
	{
		D3DXVECTOR3 vec;

		m_SliderA[SLIDER_SETTINGS_MUSIC].GetPosition(&vec);
		m_SliderA[SLIDER_SETTINGS_MUSIC].SetEnable(FALSE);
		m_SliderA[SLIDER_SETTINGS_MUSIC].SetShow(FALSE);
		hr=XuiElementGetChildById(m_SliderA[SLIDER_SETTINGS_MUSIC].m_hObj,L"XuiSlider",&hSlider);
		XuiElementSetShow(hSlider,FALSE);
		XuiControlSetEnable(hSlider,FALSE);
		m_SliderA[SLIDER_SETTINGS_SOUND].SetEnable(FALSE);
		m_SliderA[SLIDER_SETTINGS_SOUND].SetShow(FALSE);
		hr=XuiElementGetChildById(m_SliderA[SLIDER_SETTINGS_SOUND].m_hObj,L"XuiSlider",&hSlider);
		XuiElementSetShow(hSlider,FALSE);
		XuiControlSetEnable(hSlider,FALSE);
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
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
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

HRESULT CScene_SettingsAudio::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled )
{
	WCHAR TempString[256];

	if(hObjSource==m_SliderA[SLIDER_SETTINGS_MUSIC].GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_MusicVolume,pNotifyValueChanged->nValue);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_MUSIC ),pNotifyValueChanged->nValue);		
		m_SliderA[SLIDER_SETTINGS_MUSIC].SetText(TempString);
	}
	else if(hObjSource==m_SliderA[SLIDER_SETTINGS_SOUND].GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_SoundFXVolume,pNotifyValueChanged->nValue);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SOUND ),pNotifyValueChanged->nValue);		
		m_SliderA[SLIDER_SETTINGS_SOUND].SetText(TempString);
	}
	

	return S_OK;
}


HRESULT CScene_SettingsAudio::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
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

		// Not in this scene - app.CheckGameSettingsChanged(true,pInputData->UserIndex);

		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	}

	return hr;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsAudio::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	return S_OK;
}

HRESULT CScene_SettingsAudio::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	// added so we can skip greyed out items
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest!=NULL)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

HRESULT CScene_SettingsAudio::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
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
		for(int i=0;i<SLIDER_SETTINGS_AUDIO_MAX;i++)
		{
			m_SliderA[i].SetValueDisplay(FALSE);
		}

		hr=XuiElementGetChildById(m_SliderA[SLIDER_SETTINGS_MUSIC].m_hObj,L"XuiSlider",&hSlider);
		hr=XuiControlGetVisual(hSlider,&hVisual);
		hr=XuiElementGetChildById(hVisual,L"text_Label",&hText);
		hr=XuiElementGetPosition(m_SliderA[SLIDER_SETTINGS_MUSIC].m_hObj,&vSlider);

		for(int i=0;i<SLIDER_SETTINGS_AUDIO_MAX;i++)
		{
			switch(i)
			{
			case SLIDER_SETTINGS_MUSIC: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_MUSIC ),999);	
				break;
			case SLIDER_SETTINGS_SOUND: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SOUND ),999);	
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
			// Need to refresh the scenes visual since the object size has now changed
			XuiControlAttachVisual(m_hObj);

			// centre is vec.x+(fWidth/2)
			for(int i=0;i<SLIDER_SETTINGS_AUDIO_MAX;i++)
			{
				hr=XuiElementGetChildById(m_SliderA[i].m_hObj,L"XuiSlider",&hSlider);
				XuiElementGetPosition(hSlider,&vec);
				XuiElementGetBounds(hSlider,&fTemp,&fHeight);
				XuiElementSetBounds(hSlider,fMaxTextLen,fHeight);
			}
		}
	}

	return S_OK;
}

HRESULT CScene_SettingsAudio::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
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

HRESULT CScene_SettingsAudio::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

