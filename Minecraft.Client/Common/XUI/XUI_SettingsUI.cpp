// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../XUI/XUI_SettingsUI.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsUI::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	WCHAR TempString[256];
	m_iPad=*static_cast<int *>(pInitData->pvInitData);
	// if we're not in the game, we need to use basescene 0 
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);
	bool bPrimaryPlayer = ProfileManager.GetPrimaryPad()==m_iPad;

	MapChildControls();

	m_SplitScreen.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_SplitScreenVertical)!=0)?TRUE:FALSE);
	m_SplitScreen.SetText(app.GetString( IDS_CHECKBOX_VERTICAL_SPLIT_SCREEN ) );

	m_SplitScreenGamertags.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_DisplaySplitscreenGamertags)!=0)?TRUE:FALSE);
	m_SplitScreenGamertags.SetText(app.GetString( IDS_CHECKBOX_DISPLAY_SPLITSCREENGAMERTAGS ));

	m_SliderA[SLIDER_SETTINGS_UISIZE].SetValue(app.GetGameSettings(m_iPad,eGameSetting_UISize)+1);
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZE ),app.GetGameSettings(m_iPad,eGameSetting_UISize)+1);	
	m_SliderA[SLIDER_SETTINGS_UISIZE].SetText(TempString);

	m_SliderA[SLIDER_SETTINGS_UISIZESPLITSCREEN].SetValue(app.GetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen)+1);
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZESPLITSCREEN ),app.GetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen)+1);	
	m_SliderA[SLIDER_SETTINGS_UISIZESPLITSCREEN].SetText(TempString);

	m_DisplayHUD.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_DisplayHUD)!=0)?TRUE:FALSE);
	m_DisplayHUD.SetText(app.GetString( IDS_CHECKBOX_DISPLAY_HUD ));

	m_DisplayHand.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_DisplayHand)!=0)?TRUE:FALSE);
	m_DisplayHand.SetText(app.GetString( IDS_CHECKBOX_DISPLAY_HAND ));

	m_DeathMessages.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_DeathMessages)!=0)?TRUE:FALSE);
	m_DeathMessages.SetText(app.GetString( IDS_CHECKBOX_DEATH_MESSAGES ));

	m_AnimatedCharacter.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_AnimatedCharacter)!=0)?TRUE:FALSE);
	m_AnimatedCharacter.SetText(app.GetString( IDS_CHECKBOX_ANIMATED_CHARACTER ));

	// if we're not in the game, we need to use basescene 0 
	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
		CXuiSceneBase::ShowBackground( DEFAULT_XUI_MENU_USER, TRUE );
	}
	else
	{	
		// If the game has started, then you need to be the host to change the in-game gamertags
		if(!bPrimaryPlayer)
		{	
			// hide things we don't want the splitscreen player changing
			m_SplitScreen.SetShow(FALSE);
			m_SplitScreen.SetEnable(FALSE);
			m_SplitScreenGamertags.SetShow(FALSE);
			m_SplitScreenGamertags.SetEnable(FALSE);

			// move the sliders up, and resize the scene
			float fRemoveHeight=0.0f, fHeight, fWidth;
			D3DXVECTOR3 vec;

			m_SplitScreen.GetBounds(&fWidth, &fHeight);
			fRemoveHeight+=fHeight+4.0f; // add padding
			m_SplitScreenGamertags.GetBounds(&fWidth, &fHeight);
			fRemoveHeight+=fHeight+4.0f; // add padding

			m_SliderA[SLIDER_SETTINGS_UISIZE].GetPosition(&vec);
			vec.y-=fRemoveHeight;
			m_SliderA[SLIDER_SETTINGS_UISIZE].SetPosition(&vec);
			m_SliderA[SLIDER_SETTINGS_UISIZESPLITSCREEN].GetPosition(&vec);
			vec.y-=fRemoveHeight;
			m_SliderA[SLIDER_SETTINGS_UISIZESPLITSCREEN].SetPosition(&vec);

			GetBounds(&fWidth, &fHeight);
			fHeight-=fRemoveHeight;
			SetBounds(fWidth, fHeight);
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

HRESULT CScene_SettingsUI::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled )
{
	WCHAR TempString[256];

	if(hObjSource==m_SliderA[SLIDER_SETTINGS_UISIZE].GetSlider() )
	{
		// slider is 1 to 3

		// is this different from the current value?
		swprintf( static_cast<WCHAR *>(TempString), 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZE ),pNotifyValueChanged->nValue);		
		m_SliderA[SLIDER_SETTINGS_UISIZE].SetText(TempString);
		if(pNotifyValueChanged->nValue != app.GetGameSettings(m_iPad,eGameSetting_UISize)+1)
		{
			app.SetGameSettings(m_iPad,eGameSetting_UISize,pNotifyValueChanged->nValue-1);
			// Apply the changes to the selected text position
			CXuiSceneBase::UpdateSelectedItemPos(m_iPad);
		}
	}
	else if(hObjSource==m_SliderA[SLIDER_SETTINGS_UISIZESPLITSCREEN].GetSlider() )
	{
		swprintf( static_cast<WCHAR *>(TempString), 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZESPLITSCREEN ),pNotifyValueChanged->nValue);		
		m_SliderA[SLIDER_SETTINGS_UISIZESPLITSCREEN].SetText(TempString);

		if(pNotifyValueChanged->nValue != app.GetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen)+1)
		{
			// slider is 1 to 3
			app.SetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen,pNotifyValueChanged->nValue-1);
			// Apply the changes to the selected text position
			CXuiSceneBase::UpdateSelectedItemPos(m_iPad);
		}
	}

	return S_OK;
}


HRESULT CScene_SettingsUI::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
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
 		app.SetGameSettings(m_iPad,eGameSetting_DisplayHUD,m_DisplayHUD.IsChecked()?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_DisplayHand,m_DisplayHand.IsChecked()?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_DisplaySplitscreenGamertags,m_SplitScreenGamertags.IsChecked()?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_DeathMessages,m_DeathMessages.IsChecked()?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_AnimatedCharacter,m_AnimatedCharacter.IsChecked()?1:0);

		// if the splitscreen vertical/horizontal has changed, need to update the scenes
		if(app.GetGameSettings(m_iPad,eGameSetting_SplitScreenVertical)!=(m_SplitScreen.IsChecked()?1:0))
		{
			// changed
			app.SetGameSettings(m_iPad,eGameSetting_SplitScreenVertical,m_SplitScreen.IsChecked()?1:0);
			
			// close the xui scenes, so we don't have the navigate backed to menu at the wrong place
			if(app.GetLocalPlayerCount()==2)
			{
				app.CloseAllPlayersXuiScenes();
			}
			else
			{
				app.NavigateBack(pInputData->UserIndex);
			}
		}
		else
		{
			app.NavigateBack(pInputData->UserIndex);
		}


		rfHandled = TRUE;
		break;
	}

	return hr;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsUI::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	return S_OK;
}

HRESULT CScene_SettingsUI::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	// added so we can skip greyed out items
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest!=nullptr)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

HRESULT CScene_SettingsUI::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
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
		float fWidth,fHeight;//,fTemp;
		D3DXVECTOR3 vec,vecCheckboxText,vSlider,vecCheckbox;

		hr=XuiControlGetVisual(m_SplitScreen.m_hObj,&hVisual);
		hr=XuiElementGetChildById(hVisual,L"text_Button",&hCheckboxText);
		hr=XuiElementGetPosition(hCheckboxText,&vecCheckboxText);
		hr=XuiElementGetPosition(m_SplitScreen.m_hObj,&vecCheckbox);		
		hr=XuiElementGetChildById(m_SliderA[SLIDER_SETTINGS_UISIZESPLITSCREEN].m_hObj,L"XuiSlider",&hSlider);
		hr=XuiControlGetVisual(hSlider,&hVisual);
		hr=XuiElementGetChildById(hVisual,L"text_Label",&hText);
		hr=XuiElementGetPosition(m_SliderA[SLIDER_SETTINGS_UISIZE].m_hObj,&vSlider);


		// don't display values on these - we handle that
		for(int i=0;i<SLIDER_SETTINGS_UI_MAX;i++)
		{
			m_SliderA[i].SetValueDisplay(FALSE);
		}

		for(int i=0;i<SLIDER_SETTINGS_UI_MAX;i++)
		{
			switch(i)
			{
			case SLIDER_SETTINGS_UISIZE: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZE ),3);	
				break;
			case SLIDER_SETTINGS_UISIZESPLITSCREEN: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZESPLITSCREEN ),3);	
				break;
			}

			//LPCWSTR temp=m_SliderA[i].GetText();
			hr=XuiTextPresenterMeasureText(hText, TempString, &xuiRect);
			// 4J-PB - the text measuring doesn't seem to be long enough - add a fudge
			xuiRect.right+=25.0f;
			m_SliderA[i].GetBounds(&fWidth,&fHeight);
			if(xuiRect.right>fMaxTextLen) fMaxTextLen=xuiRect.right;
			if(fWidth>fMaxLen) fMaxLen=fWidth;
		}

		// now the m_SplitScreen checkbox - let's just use the visual we already have...
		
		hr=XuiTextPresenterMeasureText(hCheckboxText, m_SplitScreen.GetText(), &xuiRect);
		m_SplitScreen.GetBounds(&fWidth,&fHeight);
		// need to add the size of the checkbox graphic
		if((xuiRect.right+vecCheckbox.x+vecCheckboxText.x)>fMaxTextLen) fMaxTextLen=xuiRect.right+vecCheckbox.x+vecCheckboxText.x;
		if(fWidth>fMaxLen) fMaxLen=fWidth;

		// now the m_SplitScreenGamertags checkbox - let's just use the visual we already have...

		hr=XuiTextPresenterMeasureText(hCheckboxText, m_SplitScreenGamertags.GetText(), &xuiRect);
		m_SplitScreenGamertags.GetBounds(&fWidth,&fHeight);
		// need to add the size of the checkbox graphic
		if((xuiRect.right+vecCheckbox.x+vecCheckboxText.x)>fMaxTextLen) fMaxTextLen=xuiRect.right+vecCheckbox.x+vecCheckboxText.x;
		if(fWidth>fMaxLen) fMaxLen=fWidth;

		// now the m_DisplayHUD checkbox - let's just use the visual we already have...

		hr=XuiTextPresenterMeasureText(hCheckboxText, m_DisplayHUD.GetText(), &xuiRect);
		m_DisplayHUD.GetBounds(&fWidth,&fHeight);
		// need to add the size of the checkbox graphic
		if((xuiRect.right+vecCheckbox.x+vecCheckboxText.x)>fMaxTextLen) fMaxTextLen=xuiRect.right+vecCheckbox.x+vecCheckboxText.x;
		if(fWidth>fMaxLen) fMaxLen=fWidth;


		// now the m_DisplayHand checkbox - let's just use the visual we already have...
		hr=XuiTextPresenterMeasureText(hCheckboxText, m_DisplayHand.GetText(), &xuiRect);
		m_DisplayHand.GetBounds(&fWidth,&fHeight);
		// need to add the size of the checkbox graphic
		if((xuiRect.right+vecCheckbox.x+vecCheckboxText.x)>fMaxTextLen) fMaxTextLen=xuiRect.right+vecCheckbox.x+vecCheckboxText.x;
		if(fWidth>fMaxLen) fMaxLen=fWidth;


		// now the m_DeathMessages checkbox - let's just use the visual we already have...
		hr=XuiTextPresenterMeasureText(hCheckboxText, m_DeathMessages.GetText(), &xuiRect);
		m_DeathMessages.GetBounds(&fWidth,&fHeight);
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

			m_SplitScreen.SetBounds(fMaxTextLen,fHeight);
			m_SplitScreenGamertags.SetBounds(fMaxTextLen,fHeight);
			m_DisplayHUD.SetBounds(fMaxTextLen,fHeight);
			m_DisplayHand.SetBounds(fMaxTextLen,fHeight);
			m_DeathMessages.SetBounds(fMaxTextLen,fHeight);

			// Need to refresh the scenes visual since the object size has now changed
			XuiControlAttachVisual(m_hObj);
		}
	}

	return S_OK;
}

HRESULT CScene_SettingsUI::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);

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

HRESULT CScene_SettingsUI::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

