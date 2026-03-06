#include "stdafx.h"
#include "UI.h"
#include "UIScene_SettingsOptionsMenu.h"

#if defined(_XBOX_ONE)
#define _ENABLE_LANGUAGE_SELECT
#endif

int UIScene_SettingsOptionsMenu::m_iDifficultySettingA[4]=
{
	IDS_DIFFICULTY_PEACEFUL,
	IDS_DIFFICULTY_EASY,
	IDS_DIFFICULTY_NORMAL,
	IDS_DIFFICULTY_HARD
};

int UIScene_SettingsOptionsMenu::m_iDifficultyTitleSettingA[4]=
{
	IDS_DIFFICULTY_TITLE_PEACEFUL,
	IDS_DIFFICULTY_TITLE_EASY,
	IDS_DIFFICULTY_TITLE_NORMAL,
	IDS_DIFFICULTY_TITLE_HARD
};

UIScene_SettingsOptionsMenu::UIScene_SettingsOptionsMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	m_bNavigateToLanguageSelector = false;

	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	
	m_bNotInGame=(Minecraft::GetInstance()->level==NULL);

	m_checkboxViewBob.init(IDS_VIEW_BOBBING,eControl_ViewBob,(app.GetGameSettings(m_iPad,eGameSetting_ViewBob)!=0));
	m_checkboxShowHints.init(IDS_HINTS,eControl_ShowHints,(app.GetGameSettings(m_iPad,eGameSetting_Hints)!=0));
	m_checkboxShowTooltips.init(IDS_IN_GAME_TOOLTIPS,eControl_ShowTooltips,(app.GetGameSettings(m_iPad,eGameSetting_Tooltips)!=0));
	m_checkboxInGameGamertags.init(IDS_IN_GAME_GAMERTAGS,eControl_InGameGamertags,(app.GetGameSettings(m_iPad,eGameSetting_GamertagsVisible)!=0));

	// check if we should display the mash-up option
	if(m_bNotInGame && app.GetMashupPackWorlds(m_iPad)!=0xFFFFFFFF)
	{
		// the mash-up option is needed
		m_bMashUpWorldsUnhideOption=true;
		m_checkboxMashupWorlds.init(IDS_UNHIDE_MASHUP_WORLDS,eControl_ShowMashUpWorlds,false);
	}
	else
	{
		//m_checkboxMashupWorlds.init(L"",eControl_ShowMashUpWorlds,false);
		removeControl(&m_checkboxMashupWorlds, true);
		m_bMashUpWorldsUnhideOption=false;
	}

	unsigned char ucValue=app.GetGameSettings(m_iPad,eGameSetting_Autosave);

	wchar_t autosaveLabels[9][256];
	for(unsigned int i = 0; i < 9; ++i)
	{
		if(i==0)
		{
			swprintf( autosaveLabels[i], 256, L"%ls", app.GetString( IDS_SLIDER_AUTOSAVE_OFF ));		
		}
		else
		{
			swprintf( autosaveLabels[i], 256, L"%ls: %d %ls", app.GetString( IDS_SLIDER_AUTOSAVE ),i*15, app.GetString( IDS_MINUTES ));		
		}

	}
	m_sliderAutosave.setAllPossibleLabels(9,autosaveLabels);
	m_sliderAutosave.init(autosaveLabels[ucValue],eControl_Autosave,0,8,ucValue);

#if defined(_XBOX_ONE) || defined(__ORBIS__)
	removeControl(&m_sliderAutosave,true);
#endif

	ucValue = app.GetGameSettings(m_iPad,eGameSetting_Difficulty);
	wchar_t difficultyLabels[4][256];
	for(unsigned int i = 0; i < 4; ++i)
	{
		swprintf( difficultyLabels[i], 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[i]));	
	}
	m_sliderDifficulty.setAllPossibleLabels(4,difficultyLabels);
	m_sliderDifficulty.init(difficultyLabels[ucValue],eControl_Difficulty,0,3,ucValue);

 	wstring wsText=app.GetString(m_iDifficultySettingA[app.GetGameSettings(m_iPad,eGameSetting_Difficulty)]);
	EHTMLFontSize size = eHTMLSize_Normal;
	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		size = eHTMLSize_Splitscreen;
	}
	wchar_t startTags[64];
	swprintf(startTags,64,L"<font color=\"#%08x\">",app.GetHTMLColour(eHTMLColor_White));
 	wsText= startTags + wsText;

	m_labelDifficultyText.init(wsText);

	// If you are in-game, only the game host can change in-game gamertags, and you can't change difficulty
	// only the primary player gets to change the autosave and difficulty settings
	bool bRemoveDifficulty=false;
	bool bRemoveAutosave=false;
	bool bRemoveInGameGamertags=false;
	
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);
	bool bPrimaryPlayer = ProfileManager.GetPrimaryPad()==m_iPad;
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
	if(bRemoveDifficulty)
	{
		m_labelDifficultyText.setVisible( false );
		removeControl(&m_sliderDifficulty, true);
	}

	if(bRemoveAutosave)
	{
		removeControl(&m_sliderAutosave, true);
	}

	if(bRemoveInGameGamertags)
	{
		removeControl(&m_checkboxInGameGamertags, true);
	}

	// 4J-JEV: Changing languages in-game will produce many a bug.
	// MGH - disabled the language select for the patch build, we'll re-enable afterwards
	// 4J Stu - Removed it with a preprocessor def as we turn this off in various places
#ifdef _ENABLE_LANGUAGE_SELECT
	if (app.GetGameStarted())	
	{
		removeControl( &m_buttonLanguageSelect, false );
	}
	else						
	{
		m_buttonLanguageSelect.init(IDS_LANGUAGE_SELECTOR, eControl_Languages);
	}
#else
	removeControl( &m_buttonLanguageSelect, false );
#endif

	doHorizontalResizeCheck();

	if(app.GetLocalPlayerCount()>1)
	{
#if TO_BE_IMPLEMENTED
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
#endif
	}

	m_labelDifficultyText.disableReinitialisation();
}

UIScene_SettingsOptionsMenu::~UIScene_SettingsOptionsMenu()
{
}

void UIScene_SettingsOptionsMenu::tick()
{
	UIScene::tick();

	if (m_bNavigateToLanguageSelector)
	{
		m_bNavigateToLanguageSelector = false;
		setGameSettings();
		ui.NavigateToScene(m_iPad, eUIScene_LanguageSelector);
	}
}

wstring UIScene_SettingsOptionsMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"SettingsOptionsMenuSplit";
	}
	else
	{
		return L"SettingsOptionsMenu";
	}
}

void UIScene_SettingsOptionsMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsOptionsMenu::updateComponents()
{
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);
	if(bNotInGame)
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,true);
		m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
	}
	else
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);

		if( app.GetLocalPlayerCount() == 1 ) m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,RenderManager.IsHiDef());
		else m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
	}
}

void UIScene_SettingsOptionsMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);
	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			setGameSettings();
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		sendInputToMovie(key, repeat, pressed, released);
		break;
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_SettingsOptionsMenu::handlePress(F64 controlId, F64 childId)
{
	//CD - Added for audio
	ui.PlayUISFX(eSFX_Press);

	switch((int)controlId)
	{
	case eControl_Languages:
		m_bNavigateToLanguageSelector = true;
		break;
	}
}

void UIScene_SettingsOptionsMenu::handleReload()
{
	m_bNavigateToLanguageSelector = false;

	m_checkboxViewBob.init(IDS_VIEW_BOBBING,eControl_ViewBob,(app.GetGameSettings(m_iPad,eGameSetting_ViewBob)!=0));
	m_checkboxShowHints.init(IDS_HINTS,eControl_ShowHints,(app.GetGameSettings(m_iPad,eGameSetting_Hints)!=0));
	m_checkboxShowTooltips.init(IDS_IN_GAME_TOOLTIPS,eControl_ShowTooltips,(app.GetGameSettings(m_iPad,eGameSetting_Tooltips)!=0));
	m_checkboxInGameGamertags.init(IDS_IN_GAME_GAMERTAGS,eControl_InGameGamertags,(app.GetGameSettings(m_iPad,eGameSetting_GamertagsVisible)!=0));

	// check if we should display the mash-up option
	if(m_bNotInGame && app.GetMashupPackWorlds(m_iPad)!=0xFFFFFFFF)
	{
		// the mash-up option is needed
		m_bMashUpWorldsUnhideOption=true;
	}
	else
	{
		//m_checkboxMashupWorlds.init(L"",eControl_ShowMashUpWorlds,false);
		removeControl(&m_checkboxMashupWorlds, true);
		m_bMashUpWorldsUnhideOption=false;
	}

	unsigned char ucValue=app.GetGameSettings(m_iPad,eGameSetting_Autosave);

	wchar_t autosaveLabels[9][256];
	for(unsigned int i = 0; i < 9; ++i)
	{
		if(i==0)
		{
			swprintf( autosaveLabels[i], 256, L"%ls", app.GetString( IDS_SLIDER_AUTOSAVE_OFF ));		
		}
		else
		{
			swprintf( autosaveLabels[i], 256, L"%ls: %d %ls", app.GetString( IDS_SLIDER_AUTOSAVE ),i*15, app.GetString( IDS_MINUTES ));		
		}

	}
	m_sliderAutosave.setAllPossibleLabels(9,autosaveLabels);
	m_sliderAutosave.init(autosaveLabels[ucValue],eControl_Autosave,0,8,ucValue);

#if defined(_XBOX_ONE) || defined(__ORBIS__)
	removeControl(&m_sliderAutosave,true);
#endif

	ucValue = app.GetGameSettings(m_iPad,eGameSetting_Difficulty);

	wchar_t difficultyLabels[4][256];
	for(unsigned int i = 0; i < 4; ++i)
	{
		swprintf( difficultyLabels[i], 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[i]));	
	}
	m_sliderDifficulty.setAllPossibleLabels(4,difficultyLabels);
	m_sliderDifficulty.init(difficultyLabels[ucValue],eControl_Difficulty,0,3,ucValue);

	wstring wsText=app.GetString(m_iDifficultySettingA[app.GetGameSettings(m_iPad,eGameSetting_Difficulty)]);
	EHTMLFontSize size = eHTMLSize_Normal;
	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		size = eHTMLSize_Splitscreen;
	}
	wchar_t startTags[64];
	swprintf(startTags,64,L"<font color=\"#%08x\">",app.GetHTMLColour(eHTMLColor_White));
 	wsText= startTags + wsText;

	m_labelDifficultyText.init(wsText);
	

	// If you are in-game, only the game host can change in-game gamertags, and you can't change difficulty
	// only the primary player gets to change the autosave and difficulty settings
	bool bRemoveDifficulty=false;
	bool bRemoveAutosave=false;
	bool bRemoveInGameGamertags=false;
	
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);
	bool bPrimaryPlayer = ProfileManager.GetPrimaryPad()==m_iPad;
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
	if(bRemoveDifficulty)
	{
		m_labelDifficultyText.setVisible( false );
		removeControl(&m_sliderDifficulty, true);
	}

	if(bRemoveAutosave)
	{
		removeControl(&m_sliderAutosave, true);
	}

	if(bRemoveInGameGamertags)
	{
		removeControl(&m_checkboxInGameGamertags, true);
	}

	// MGH - disabled the language select for the patch build, we'll re-enable afterwards
	// 4J Stu - Removed it with a preprocessor def as we turn this off in various places
#ifdef _ENABLE_LANGUAGE_SELECT
	// 4J-JEV: Changing languages in-game will produce many a bug.
	if (app.GetGameStarted())	
	{
		removeControl( &m_buttonLanguageSelect, false );
	}
	else						
	{
	}
#else
	removeControl( &m_buttonLanguageSelect, false );
#endif

	doHorizontalResizeCheck();
}

void UIScene_SettingsOptionsMenu::handleSliderMove(F64 sliderId, F64 currentValue)
{
	int value = (int)currentValue;
	switch((int)sliderId)
	{
	case eControl_Autosave:
		m_sliderAutosave.handleSliderMove(value);

		app.SetGameSettings(m_iPad,eGameSetting_Autosave,value);
		// Update the autosave timer
		app.SetAutosaveTimerTime();

		break;
	case eControl_Difficulty:
		m_sliderDifficulty.handleSliderMove(value);

		app.SetGameSettings(m_iPad,eGameSetting_Difficulty,value);
		
		wstring wsText=app.GetString(m_iDifficultySettingA[value]);
		EHTMLFontSize size = eHTMLSize_Normal;
		if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
		{
			size = eHTMLSize_Splitscreen;
		}
		wchar_t startTags[64];
		swprintf(startTags,64,L"<font color=\"#%08x\">",app.GetHTMLColour(eHTMLColor_White));
		wsText= startTags + wsText;
		m_labelDifficultyText.setLabel(wsText.c_str());
		break;
	}
}

void UIScene_SettingsOptionsMenu::setGameSettings()
{
	// check the checkboxes
	app.SetGameSettings(m_iPad,eGameSetting_ViewBob,m_checkboxViewBob.IsChecked()?1:0);
	app.SetGameSettings(m_iPad,eGameSetting_GamertagsVisible,m_checkboxInGameGamertags.IsChecked()?1:0);
	app.SetGameSettings(m_iPad,eGameSetting_Hints,m_checkboxShowHints.IsChecked()?1:0);
	app.SetGameSettings(m_iPad,eGameSetting_Tooltips,m_checkboxShowTooltips.IsChecked()?1:0);

	// the mashup option will only be shown if some worlds have been previously hidden
	if(m_bMashUpWorldsUnhideOption && m_checkboxMashupWorlds.IsChecked())
	{
		// unhide all worlds
		app.EnableMashupPackWorlds(m_iPad);
	}

	// 4J-PB - don't action changes here or we might write to the profile on backing out here and then get a change in the settings all, and write again on backing out there
	//app.CheckGameSettingsChanged(true,pInputData->UserIndex);
}