// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "..\XUI\XUI_SettingsGraphics.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsGraphics::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	WCHAR TempString[256];
	m_iPad=*(int *)pInitData->pvInitData;
	// if we're not in the game, we need to use basescene 0 
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);
	bool bIsPrimaryPad=(ProfileManager.GetPrimaryPad()==m_iPad);

	MapChildControls();

	// Display the tooltips

	m_Clouds.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_Clouds)!=0)?TRUE:FALSE);
	m_Clouds.SetText(app.GetString( IDS_CHECKBOX_RENDER_CLOUDS ));

	m_SliderA[SLIDER_SETTINGS_GAMMA].SetValue(app.GetGameSettings(m_iPad,eGameSetting_Gamma));
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_GAMMA ),app.GetGameSettings(m_iPad,eGameSetting_Gamma));	
	m_SliderA[SLIDER_SETTINGS_GAMMA].SetText(TempString);

	m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY].SetValue(app.GetGameSettings(m_iPad,eGameSetting_InterfaceOpacity));
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_INTERFACEOPACITY ),app.GetGameSettings(m_iPad,eGameSetting_InterfaceOpacity));	
	m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY].SetText(TempString);

	m_BedrockFog.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_BedrockFog)!=0)?TRUE:FALSE);
	m_BedrockFog.SetText(app.GetString( IDS_CHECKBOX_RENDER_BEDROCKFOG ));

	m_CustomSkinAnim.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_CustomSkinAnim)!=0)?TRUE:FALSE);
	m_CustomSkinAnim.SetText(app.GetString( IDS_CHECKBOX_CUSTOM_SKIN_ANIM ));

	// if we're not in the game, we need to use basescene 0 
	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		CXuiSceneBase::ShowBackground( DEFAULT_XUI_MENU_USER, TRUE );
	}
	else
	{	
		// If the game has started, then you need to be the host to change the in-game gamertags
		if(bIsPrimaryPad)
		{	
			// are we the game host? If not, we need to remove the bedrockfog setting
			if(!g_NetworkManager.IsHost())
			{
				// we are the primary player on this machine, but not the game host
				D3DXVECTOR3 vec;
				// hide the in-game bedrock fog setting
				m_BedrockFog.SetShow(FALSE);

				// m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY] -> m_SliderA[SLIDER_SETTINGS_GAMMA]
				// m_SliderA[SLIDER_SETTINGS_GAMMA] -> m_CustomSkinAnim
				// m_CustomSkinAnim -> m_BedrockFog

				XuiElementGetPosition(m_SliderA[SLIDER_SETTINGS_GAMMA],&vec);
				m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY].SetPosition(&vec);

				XuiElementGetPosition(m_CustomSkinAnim,&vec);
				m_SliderA[SLIDER_SETTINGS_GAMMA].SetPosition(&vec);

				XuiElementGetPosition(m_BedrockFog,&vec);
				m_CustomSkinAnim.SetPosition(&vec);

				// reduce the size of the background
				float fWidth, fHeight, fbgnWidth, fBgnHeight;
				m_BedrockFog.GetBounds(&fWidth, &fHeight);
				GetBounds(&fbgnWidth, &fBgnHeight);
				fBgnHeight-=fHeight;
				SetBounds(fbgnWidth, fBgnHeight);
			}
		}
		else
		{
			// We shouldn't have the bedrock fog option, or the m_CustomSkinAnim option
			m_BedrockFog.SetShow(FALSE);
			m_CustomSkinAnim.SetShow(FALSE);

			D3DXVECTOR3 vec,vecGamma,vecOpacity;
			float fSliderYDiff;

			// m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY] -> m_BedrockFog
			// m_SliderA[SLIDER_SETTINGS_GAMMA] -> m_BedrockFog

			m_SliderA[SLIDER_SETTINGS_GAMMA].GetPosition(&vecGamma);
			m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY].GetPosition(&vecOpacity);
			fSliderYDiff=vecOpacity.y-vecGamma.y;

			XuiElementGetPosition(m_BedrockFog,&vec);
			m_SliderA[SLIDER_SETTINGS_GAMMA].SetPosition(&vec);
			vec.y+=fSliderYDiff;
			m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY].SetPosition(&vec);
			
			// reduce the size of the background
			float fWidth, fHeight, fbgnWidth, fBgnHeight;
			GetBounds(&fbgnWidth, &fBgnHeight);
			m_BedrockFog.GetBounds(&fWidth, &fHeight);
			fBgnHeight-=fHeight;
			m_CustomSkinAnim.GetBounds(&fWidth, &fHeight);
			fBgnHeight-=fHeight;
			SetBounds(fbgnWidth, fBgnHeight);
		}

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

HRESULT CScene_SettingsGraphics::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled )
{
	WCHAR TempString[256];

	if(hObjSource==m_SliderA[SLIDER_SETTINGS_GAMMA].GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_Gamma,pNotifyValueChanged->nValue);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_GAMMA ),pNotifyValueChanged->nValue);		
		m_SliderA[SLIDER_SETTINGS_GAMMA].SetText(TempString);
	}
	else if(hObjSource==m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY].GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_InterfaceOpacity,pNotifyValueChanged->nValue);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_INTERFACEOPACITY ),pNotifyValueChanged->nValue);		
		m_SliderA[SLIDER_SETTINGS_INTERFACE_OPACITY].SetText(TempString);
	}

	return S_OK;
}


HRESULT CScene_SettingsGraphics::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
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

		// check the checkboxes
		app.SetGameSettings(m_iPad,eGameSetting_Clouds,m_Clouds.IsChecked()?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_BedrockFog,m_BedrockFog.IsChecked()?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_CustomSkinAnim,m_CustomSkinAnim.IsChecked()?1:0);

		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	}

	return hr;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsGraphics::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	return S_OK;
}

HRESULT CScene_SettingsGraphics::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	// added so we can skip greyed out items
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest!=NULL)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

HRESULT CScene_SettingsGraphics::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
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
		HXUIOBJ hSlider,hVisual,hText,hCheckboxText;
		XUIRect xuiRect;
		float fWidth,fHeight,fTemp;
		D3DXVECTOR3 vec,vecCheckboxText,vSlider,vecCheckbox;

		// don't display values on these - we handle that
		for(int i=0;i<SLIDER_SETTINGS_GRAPHICS_MAX;i++)
		{
			m_SliderA[i].SetValueDisplay(FALSE);
		}

		hr=XuiControlGetVisual(m_Clouds.m_hObj,&hVisual);
		hr=XuiElementGetChildById(hVisual,L"text_Button",&hCheckboxText);

		hr=XuiElementGetChildById(m_SliderA[SLIDER_SETTINGS_GAMMA].m_hObj,L"XuiSlider",&hSlider);
		hr=XuiControlGetVisual(hSlider,&hVisual);
		hr=XuiElementGetChildById(hVisual,L"text_Label",&hText);
		hr=XuiElementGetPosition(m_SliderA[SLIDER_SETTINGS_GAMMA].m_hObj,&vSlider);

		for(int i=0;i<SLIDER_SETTINGS_GRAPHICS_MAX;i++)
		{
			switch(i)
			{
			case SLIDER_SETTINGS_GAMMA: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_GAMMA ),999);	
				break;
			case SLIDER_SETTINGS_INTERFACE_OPACITY: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_INTERFACEOPACITY ),999);	
				break;
			}
			hr=XuiTextPresenterMeasureText(hText, TempString, &xuiRect);
			// 4J-PB - the text measuring doesn't seem to be long enough - add a fudge
			xuiRect.right+=25.0f;
			m_SliderA[i].GetBounds(&fWidth,&fHeight);
			if(xuiRect.right>fMaxTextLen) fMaxTextLen=xuiRect.right;
			if(fWidth>fMaxLen) fMaxLen=fWidth;
		}

		// now the clouds checkbox - let's just use the visual we already have...
		hr=XuiTextPresenterMeasureText(hText, m_Clouds.GetText(), &xuiRect);
		hr=XuiTextPresenterMeasureText(hCheckboxText, m_Clouds.GetText(), &xuiRect);
		m_Clouds.GetBounds(&fWidth,&fHeight);
		// need to add the size of the checkbox graphic
		if((xuiRect.right+vecCheckbox.x+vecCheckboxText.x)>fMaxTextLen) fMaxTextLen=xuiRect.right+vecCheckbox.x+vecCheckboxText.x;
		if(fWidth>fMaxLen) fMaxLen=fWidth;

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
			for(int i=0;i<SLIDER_SETTINGS_GRAPHICS_MAX;i++)
			{
				hr=XuiElementGetChildById(m_SliderA[i].m_hObj,L"XuiSlider",&hSlider);
				XuiElementGetPosition(hSlider,&vec);
				XuiElementGetBounds(hSlider,&fTemp,&fHeight);
				XuiElementSetBounds(hSlider,fMaxTextLen,fHeight);
			}

			m_Clouds.SetBounds(fMaxTextLen,fHeight);

		}
	}

	return S_OK;
}

HRESULT CScene_SettingsGraphics::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
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

HRESULT CScene_SettingsGraphics::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

