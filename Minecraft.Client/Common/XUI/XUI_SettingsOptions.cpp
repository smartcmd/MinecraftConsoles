// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../XUI/XUI_SettingsOptions.h"

int CScene_SettingsOptions::m_iDifficultySettingA[4]=
{
	IDS_DIFFICULTY_PEACEFUL,
	IDS_DIFFICULTY_EASY,
	IDS_DIFFICULTY_NORMAL,
	IDS_DIFFICULTY_HARD
};

int CScene_SettingsOptions::m_iDifficultyTitleSettingA[4]=
{
	IDS_DIFFICULTY_TITLE_PEACEFUL,
	IDS_DIFFICULTY_TITLE_EASY,
	IDS_DIFFICULTY_TITLE_NORMAL,
	IDS_DIFFICULTY_TITLE_HARD
};

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsOptions::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	WCHAR TempString[256];
	m_iPad=*static_cast<int *>(pInitData->pvInitData);
	// if we're not in the game, we need to use basescene 0 
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);
	bool bPrimaryPlayer = ProfileManager.GetPrimaryPad()==m_iPad;

	MapChildControls();
	
	XuiControlSetText(m_ViewBob,app.GetString(IDS_VIEW_BOBBING));
	XuiControlSetText(m_Hints,app.GetString(IDS_HINTS));
	XuiControlSetText(m_Tooltips,app.GetString(IDS_IN_GAME_TOOLTIPS));
	XuiControlSetText(m_InGameGamertags,app.GetString(IDS_IN_GAME_GAMERTAGS));
	XuiControlSetText(m_MashUpWorlds,app.GetString(IDS_UNHIDE_MASHUP_WORLDS));

	// check if we should display the mash-up option
	if(bNotInGame && app.GetMashupPackWorlds(m_iPad)!=0xFFFFFFFF)
	{
		// the mash-up option is needed
		m_bMashUpWorldsUnhideOption=true;
		m_MashUpWorlds.SetShow(TRUE);
	}
	else
	{
		m_bMashUpWorldsUnhideOption=false;
		m_MashUpWorlds.SetShow(FALSE);
	}

	// Display the tooltips
	HRESULT hr = S_OK; 
	HXUIOBJ hSlider;

	unsigned char ucValue=app.GetGameSettings(m_iPad,eGameSetting_Autosave);
 	m_SliderA[SLIDER_SETTINGS_AUTOSAVE].SetValue(ucValue);
	if(ucValue==0)
	{
		swprintf( (WCHAR *)TempString, 256, L"%ls", app.GetString( IDS_SLIDER_AUTOSAVE_OFF ));		
	}
	else
	{
		swprintf( (WCHAR *)TempString, 256, L"%ls: %d %ls", app.GetString( IDS_SLIDER_AUTOSAVE ),ucValue*15, app.GetString( IDS_MINUTES ));		
	}

 	m_SliderA[SLIDER_SETTINGS_AUTOSAVE].SetText(TempString);


	m_ViewBob.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_ViewBob)!=0)?TRUE:FALSE);
	m_InGameGamertags.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_GamertagsVisible)!=0)?TRUE:FALSE);
	m_Hints.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_Hints)!=0)?TRUE:FALSE);
	m_Tooltips.SetCheck( (app.GetGameSettings(m_iPad,eGameSetting_Tooltips)!=0)?TRUE:FALSE);

	m_SliderA[SLIDER_SETTINGS_DIFFICULTY].SetValue(app.GetGameSettings(m_iPad,eGameSetting_Difficulty));
	swprintf( (WCHAR *)TempString, 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[app.GetGameSettings(m_iPad,eGameSetting_Difficulty)]));	
	m_SliderA[SLIDER_SETTINGS_DIFFICULTY].SetText(TempString);


 	wstring wsText=app.GetString(m_iDifficultySettingA[app.GetGameSettings(m_iPad,eGameSetting_Difficulty)]);
	int size = 14;
	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		size = 12;
	}
	wchar_t startTags[64];
	swprintf(startTags,64,L"<font color=\"#%08x\" size=%d>",app.GetHTMLColour(eHTMLColor_White),size);
 	wsText= startTags + wsText;

	m_DifficultyText.SetText(wsText.c_str());
	

	// If you are in-game, only the game host can change in-game gamertags, and you can't change difficulty
	// only the primary player gets to change the autosave and difficulty settings

	bool bRemoveDifficulty=false;
	bool bRemoveAutosave=false;
	bool bRemoveInGameGamertags=false;
	float fRemoveHeight=0.0f,fWidth,fHeight;
	float fSlidersMoveUp=0.0f;

	if(!bPrimaryPlayer)
	{
		bRemoveDifficulty=true;
		bRemoveAutosave=true;
		bRemoveInGameGamertags=true;
	}

	if(!bNotInGame) // in the game
	{ 
		bRemoveDifficulty=true;
		if(!g_NetworkManager.IsHost())
		{
			bRemoveAutosave=true;
			bRemoveInGameGamertags=true;	
		}
	}

	D3DXVECTOR3 vec1,vec2;
	XuiElementGetPosition(m_ViewBob,&vec1);
	XuiElementGetPosition(m_Hints,&vec2);

	float fCheckboxHeight=vec2.y-vec1.y;

	if(bRemoveDifficulty)
	{
		m_SliderA[SLIDER_SETTINGS_DIFFICULTY].SetEnable(FALSE);
		m_SliderA[SLIDER_SETTINGS_DIFFICULTY].SetShow(FALSE);
		m_DifficultyText.SetShow(FALSE);
		hr=XuiElementGetChildById(m_SliderA[SLIDER_SETTINGS_DIFFICULTY].m_hObj,L"XuiSlider",&hSlider);
		XuiElementSetShow(hSlider,FALSE);
		XuiControlSetEnable(hSlider,FALSE);
		m_SliderA[SLIDER_SETTINGS_DIFFICULTY].GetBounds(&fWidth,&fHeight);
		fRemoveHeight+=fHeight+4.0f; // add padding
	}
	else
	{
		wstring wsText=app.GetString(m_iDifficultySettingA[app.GetGameSettings(m_iPad,eGameSetting_Difficulty)]);
		int size = 14;
		if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
		{
			size = 12;
		}
		wchar_t startTags[64];
		swprintf(startTags,64,L"<font color=\"#%08x\" size=%d>",app.GetHTMLColour(eHTMLColor_White),size);
		wsText= startTags + wsText;

		m_DifficultyText.SetText(wsText.c_str());
	}

	if(bRemoveAutosave)
	{
		m_SliderA[SLIDER_SETTINGS_AUTOSAVE].SetEnable(FALSE);
		m_SliderA[SLIDER_SETTINGS_AUTOSAVE].SetShow(FALSE);
		hr=XuiElementGetChildById(m_SliderA[SLIDER_SETTINGS_AUTOSAVE].m_hObj,L"XuiSlider",&hSlider);
		XuiElementSetShow(hSlider,FALSE);
		XuiControlSetEnable(hSlider,FALSE);
		m_SliderA[SLIDER_SETTINGS_AUTOSAVE].GetBounds(&fWidth,&fHeight);
		fRemoveHeight+=fHeight+4.0f; // add padding
	}

	if(bRemoveInGameGamertags)
	{
		m_InGameGamertags.SetShow(FALSE);
		m_InGameGamertags.SetEnable(FALSE);
		m_InGameGamertags.GetBounds(&fWidth, &fHeight);

		// move the mash-up worlds option up 
		if(m_bMashUpWorldsUnhideOption)
		{
			D3DXVECTOR3 vec;
			XuiElementGetPosition(m_InGameGamertags,&vec);
			m_MashUpWorlds.SetPosition(&vec);
		}
		fRemoveHeight+=fCheckboxHeight;//fHeight+4.0f; // add padding
		fSlidersMoveUp+=fCheckboxHeight;
	}

	if(!m_bMashUpWorldsUnhideOption)
	{
		m_MashUpWorlds.SetShow(FALSE);
		m_MashUpWorlds.SetEnable(FALSE);
		m_MashUpWorlds.GetBounds(&fWidth, &fHeight);
		fRemoveHeight+=fCheckboxHeight; 
		fSlidersMoveUp+=fCheckboxHeight;
	}


	if(fRemoveHeight>0.0f)
	{
		D3DXVECTOR3 vec;
		// autosave should move up by the height of the number of checkboxes hidden
		m_SliderA[SLIDER_SETTINGS_AUTOSAVE].GetPosition(&vec);
		vec.y-=fSlidersMoveUp;
		m_SliderA[SLIDER_SETTINGS_AUTOSAVE].SetPosition(&vec);
		m_SliderA[SLIDER_SETTINGS_DIFFICULTY].GetPosition(&vec);
		vec.y-=fSlidersMoveUp;
		m_SliderA[SLIDER_SETTINGS_DIFFICULTY].SetPosition(&vec);
		m_DifficultyText.GetPosition(&vec);
		vec.y-=fSlidersMoveUp;
		m_DifficultyText.SetPosition(&vec);

		float fbgnWidth, fBgnHeight;
		GetBounds(&fbgnWidth, &fBgnHeight);
		fBgnHeight-=fRemoveHeight;
		SetBounds(fbgnWidth, fBgnHeight);
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
			// can't show the logo for 480 mode 
			if(!RenderManager.IsHiDef())
			{
				CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, FALSE );
			}
			else
			{
				CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );
			}
		}
		else
		{
			// can't show the logo for 480 mode 
			if(!RenderManager.IsHiDef())
			{
				CXuiSceneBase::ShowLogo( m_iPad, FALSE );
			}
			else
			{
				CXuiSceneBase::ShowLogo( m_iPad, TRUE );
			}
		}
	}

	return S_OK;
}

HRESULT CScene_SettingsOptions::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled )
{
	WCHAR TempString[256];
		
	if(hObjSource==m_SliderA[SLIDER_SETTINGS_AUTOSAVE].GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_Autosave,pNotifyValueChanged->nValue);
		// Update the autosave timer
		app.SetAutosaveTimerTime();

		if(pNotifyValueChanged->nValue==0)
		{
			swprintf( (WCHAR *)TempString, 256, L"%ls", app.GetString( IDS_SLIDER_AUTOSAVE_OFF ));		
		}
		else
		{
			app.SetAutosaveTimerTime();
			swprintf( static_cast<WCHAR *>(TempString), 256, L"%ls: %d %ls", app.GetString( IDS_SLIDER_AUTOSAVE ),pNotifyValueChanged->nValue*15, app.GetString( IDS_MINUTES ));		
		}
		m_SliderA[SLIDER_SETTINGS_AUTOSAVE].SetText(TempString);
	}
	else if(hObjSource==m_SliderA[SLIDER_SETTINGS_DIFFICULTY].GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_Difficulty,pNotifyValueChanged->nValue);
		swprintf( static_cast<WCHAR *>(TempString), 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[pNotifyValueChanged->nValue]));		
		m_SliderA[SLIDER_SETTINGS_DIFFICULTY].SetText(TempString);
		
		wstring wsText=app.GetString(m_iDifficultySettingA[pNotifyValueChanged->nValue]);
		int size = 14;
		if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
		{
			size = 12;
		}
		wchar_t startTags[64];
		swprintf(startTags,64,L"<font color=\"#%08x\" size=%d>",app.GetHTMLColour(eHTMLColor_White),size);
		wsText= startTags + wsText;
		m_DifficultyText.SetText(wsText.c_str());
	}
	return S_OK;
}


HRESULT CScene_SettingsOptions::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
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

		// check the checkboxes
		app.SetGameSettings(m_iPad,eGameSetting_ViewBob,m_ViewBob.IsChecked()?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_GamertagsVisible,m_InGameGamertags.IsChecked()?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_Hints,m_Hints.IsChecked()?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_Tooltips,m_Tooltips.IsChecked()?1:0);

		// the mashup option will only be shown if some worlds have been previously hidden
		if(m_bMashUpWorldsUnhideOption && m_MashUpWorlds.IsChecked())
		{
			// unhide all worlds
			app.EnableMashupPackWorlds(m_iPad);
		}

		// 4J-PB - don't action changes here or we might write to the profile on backing out here and then get a change in the settings all, and write again on backing out there
		//app.CheckGameSettingsChanged(true,pInputData->UserIndex);

		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	}

	return hr;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_SettingsOptions::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	return S_OK;
}

HRESULT CScene_SettingsOptions::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	// added so we can skip greyed out items
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest!=nullptr)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

HRESULT CScene_SettingsOptions::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
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
		D3DXVECTOR3 vec,vSlider,vecCheckboxText,vecCheckbox;

		// don't display values on these - we handle that
		for(int i=0;i<OPTIONS_SLIDER_SETTINGS_MAX;i++)
		{
			m_SliderA[i].SetValueDisplay(FALSE);
		}


		hr=XuiElementGetChildById(m_SliderA[SLIDER_SETTINGS_AUTOSAVE].m_hObj,L"XuiSlider",&hSlider);
		hr=XuiControlGetVisual(hSlider,&hVisual);
		hr=XuiElementGetChildById(hVisual,L"text_Label",&hText);
		hr=XuiControlGetVisual(m_InGameGamertags.m_hObj,&hVisual);
		hr=XuiElementGetChildById(hVisual,L"text_Button",&hCheckboxText);
		hr=XuiElementGetPosition(hCheckboxText,&vecCheckboxText);
		hr=XuiElementGetPosition(m_InGameGamertags.m_hObj,&vecCheckbox);		
		hr=XuiElementGetPosition(m_SliderA[SLIDER_SETTINGS_AUTOSAVE].m_hObj,&vSlider);


		for(int i=0;i<OPTIONS_SLIDER_SETTINGS_MAX;i++)
		{
			switch(i)
			{
			case SLIDER_SETTINGS_AUTOSAVE: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %d %ls", app.GetString( IDS_SLIDER_AUTOSAVE ),999, app.GetString( IDS_MINUTES ));	
				break;
			case SLIDER_SETTINGS_DIFFICULTY: // 3 digits
				swprintf( (WCHAR *)TempString, 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(IDS_DIFFICULTY_TITLE_PEACEFUL));	
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

		// now the VisibleOnMaps checkbox - let's just use the visual we already have...
		hr=XuiTextPresenterMeasureText(hCheckboxText, m_InGameGamertags.GetText(), &xuiRect);
		m_InGameGamertags.GetBounds(&fWidth,&fHeight);
		// need to add the size of the checkbox graphic
		if((xuiRect.right+vecCheckbox.x+vecCheckboxText.x)>fMaxTextLen) fMaxTextLen=xuiRect.right+vecCheckbox.x+vecCheckboxText.x;
		if(fWidth>fMaxLen) fMaxLen=fWidth;
	
		// now the m_Hints checkbox - let's just use the visual we already have...
		hr=XuiTextPresenterMeasureText(hCheckboxText, m_Hints.GetText(), &xuiRect);
		m_Hints.GetBounds(&fWidth,&fHeight);
		// need to add the size of the checkbox graphic
		if((xuiRect.right+vecCheckbox.x+vecCheckboxText.x)>fMaxTextLen) fMaxTextLen=xuiRect.right+vecCheckbox.x+vecCheckboxText.x;
		if(fWidth>fMaxLen) fMaxLen=fWidth;

		// now the m_Tooltips checkbox - let's just use the visual we already have...
		hr=XuiTextPresenterMeasureText(hCheckboxText, m_Tooltips.GetText(), &xuiRect);
		m_Tooltips.GetBounds(&fWidth,&fHeight);
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

			bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);

			if(bNotInGame)
			{
				float fDiffTextW;
				// reposition the difficulty text - this is positioned from the scene, so will be a negative value
				m_DifficultyText.GetPosition(&vec);
				m_DifficultyText.GetBounds(&fDiffTextW,&fHeight);
				vec.x=floor((fMaxTextLen+(fWidth*2.0f)-fDiffTextW)/2.0f);
				m_DifficultyText.SetPosition(&vec);
			}

			// centre is vec.x+(fWidth/2)
			//for(int i=0;i<OPTIONS_SLIDER_SETTINGS_MAX;i++)
			//{
			//	hr=XuiElementGetChildById(m_SliderA[i].m_hObj,L"XuiSlider",&hSlider);
			//	XuiElementGetPosition(hSlider,&vec);
			//	XuiElementGetBounds(hSlider,&fTemp,&fHeight);
			//	XuiElementSetBounds(hSlider,fMaxTextLen,fHeight);
			//}

			// the checkboxes are left aligned - put them at the same position as the sliders
			m_ViewBob.SetBounds(fMaxTextLen,fHeight);
			m_InGameGamertags.SetBounds(fMaxTextLen,fHeight);
			m_Hints.SetBounds(fMaxTextLen,fHeight);
			m_Tooltips.SetBounds(fMaxTextLen,fHeight);

		}
	}

	return S_OK;
}


HRESULT CScene_SettingsOptions::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}
