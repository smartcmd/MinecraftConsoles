#include "stdafx.h"
#include "UI.h"
#include "UIScene_SettingsControlMenu.h"

UIScene_SettingsControlMenu::UIScene_SettingsControlMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	WCHAR TempString[256];
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INGAME ),app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InGame));	
	m_sliderSensitivityInGame.init(TempString,eControl_SensitivityInGame,0,200,app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InGame));
	
	swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INMENU ),app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InMenu));	
	m_sliderSensitivityInMenu.init(TempString,eControl_SensitivityInMenu,0,200,app.GetGameSettings(m_iPad,eGameSetting_Sensitivity_InMenu));

	doHorizontalResizeCheck();

	if(app.GetLocalPlayerCount()>1)
	{
#if TO_BE_IMPLEMENTED
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,false);
#endif
	}
}

UIScene_SettingsControlMenu::~UIScene_SettingsControlMenu()
{
}

wstring UIScene_SettingsControlMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"SettingsControlMenuSplit";
	}
	else
	{
		return L"SettingsControlMenu";
	}
}

void UIScene_SettingsControlMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsControlMenu::updateComponents()
{
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);
	if(bNotInGame)
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,true);
		m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
	}
	else
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);

		if( app.GetLocalPlayerCount() == 1 ) m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
		else m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
	}
}

void UIScene_SettingsControlMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			navigateBack();
			handled = true;
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

void UIScene_SettingsControlMenu::handleSliderMove(F64 sliderId, F64 currentValue)
{
	WCHAR TempString[256];
	int value = static_cast<int>(currentValue);
	switch(static_cast<int>(sliderId))
	{
	case eControl_SensitivityInGame:
		m_sliderSensitivityInGame.handleSliderMove(value);
		
		app.SetGameSettings(m_iPad,eGameSetting_Sensitivity_InGame,value);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INGAME ),value);	
		m_sliderSensitivityInGame.setLabel(TempString);

		break;
	case eControl_SensitivityInMenu:
		m_sliderSensitivityInMenu.handleSliderMove(value);
		
		app.SetGameSettings(m_iPad,eGameSetting_Sensitivity_InMenu,value);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_SENSITIVITY_INMENU ),value);	
		m_sliderSensitivityInMenu.setLabel(TempString);

		break;
	}
}
