#include "stdafx.h"
#include "UI.h"
#include "UIScene_SettingsUIMenu.h"

UIScene_SettingsUIMenu::UIScene_SettingsUIMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_bNotInGame=(Minecraft::GetInstance()->level==nullptr);

	m_checkboxDisplayHUD.init(app.GetString(IDS_CHECKBOX_DISPLAY_HUD),eControl_DisplayHUD,(app.GetGameSettings(m_iPad,eGameSetting_DisplayHUD)!=0));
	m_checkboxDisplayHand.init(app.GetString(IDS_CHECKBOX_DISPLAY_HAND),eControl_DisplayHand,(app.GetGameSettings(m_iPad,eGameSetting_DisplayHand)!=0));
	m_checkboxDisplayDeathMessages.init(app.GetString(IDS_CHECKBOX_DEATH_MESSAGES),eControl_DisplayDeathMessages,(app.GetGameSettings(m_iPad,eGameSetting_DeathMessages)!=0));
	m_checkboxDisplayAnimatedCharacter.init(app.GetString(IDS_CHECKBOX_ANIMATED_CHARACTER),eControl_DisplayAnimatedCharacter,(app.GetGameSettings(m_iPad,eGameSetting_AnimatedCharacter)!=0));
	m_checkboxSplitscreen.init(app.GetString(IDS_CHECKBOX_VERTICAL_SPLIT_SCREEN),eControl_Splitscreen,(app.GetGameSettings(m_iPad,eGameSetting_SplitScreenVertical)!=0));
	m_checkboxShowSplitscreenGamertags.init(app.GetString(IDS_CHECKBOX_DISPLAY_SPLITSCREENGAMERTAGS),eControl_ShowSplitscreenGamertags,(app.GetGameSettings(m_iPad,eGameSetting_DisplaySplitscreenGamertags)!=0));

	WCHAR TempString[256];

	swprintf( (WCHAR *)TempString, 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZE ),app.GetGameSettings(m_iPad,eGameSetting_UISize)+1);	
	m_sliderUISize.init(TempString,eControl_UISize,1,3,app.GetGameSettings(m_iPad,eGameSetting_UISize)+1);

	swprintf( (WCHAR *)TempString, 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZESPLITSCREEN ),app.GetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen)+1);	
	m_sliderUISizeSplitscreen.init(TempString,eControl_UISizeSplitscreen,1,3,app.GetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen)+1);

	doHorizontalResizeCheck();

	bool bInGame=(Minecraft::GetInstance()->level!=nullptr);
	bool bPrimaryPlayer = ProfileManager.GetPrimaryPad()==m_iPad;

	// if we're not in the game, we need to use basescene 0 
	if(bInGame)
	{
		// If the game has started, then you need to be the host to change the in-game gamertags
		if(!bPrimaryPlayer)
		{	
			// hide things we don't want the splitscreen player changing
			removeControl(&m_checkboxSplitscreen, true);
			removeControl(&m_checkboxShowSplitscreenGamertags, true);
		}
	}


	if(app.GetLocalPlayerCount()>1)
	{
#if TO_BE_IMPLEMENTED
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
#endif
	}
}

void UIScene_SettingsUIMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsUIMenu::updateComponents()
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

UIScene_SettingsUIMenu::~UIScene_SettingsUIMenu()
{
}

wstring UIScene_SettingsUIMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"SettingsUIMenuSplit";
	}
	else
	{
		return L"SettingsUIMenu";
	}
}

void UIScene_SettingsUIMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			// check the checkboxes
			app.SetGameSettings(m_iPad,eGameSetting_DisplayHUD,m_checkboxDisplayHUD.IsChecked()?1:0);
			app.SetGameSettings(m_iPad,eGameSetting_DisplayHand,m_checkboxDisplayHand.IsChecked()?1:0);
			app.SetGameSettings(m_iPad,eGameSetting_DisplaySplitscreenGamertags,m_checkboxShowSplitscreenGamertags.IsChecked()?1:0);
			app.SetGameSettings(m_iPad,eGameSetting_DeathMessages,m_checkboxDisplayDeathMessages.IsChecked()?1:0);
			app.SetGameSettings(m_iPad,eGameSetting_AnimatedCharacter,m_checkboxDisplayAnimatedCharacter.IsChecked()?1:0);

			// if the splitscreen vertical/horizontal has changed, need to update the scenes
			if(app.GetGameSettings(m_iPad,eGameSetting_SplitScreenVertical)!=(m_checkboxSplitscreen.IsChecked()?1:0))
			{
				// changed
				app.SetGameSettings(m_iPad,eGameSetting_SplitScreenVertical,m_checkboxSplitscreen.IsChecked()?1:0);

				// close the xui scenes, so we don't have the navigate backed to menu at the wrong place
				if(app.GetLocalPlayerCount()==2)
				{
					ui.CloseAllPlayersScenes();
				}
				else
				{
					navigateBack();
				}
			}
			else
			{
				navigateBack();
			}
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

void UIScene_SettingsUIMenu::handleSliderMove(F64 sliderId, F64 currentValue)
{
	WCHAR TempString[256];
	int value = static_cast<int>(currentValue);
	switch(static_cast<int>(sliderId))
	{
	case eControl_UISize:
		m_sliderUISize.handleSliderMove(value);

		swprintf( (WCHAR *)TempString, 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZE ),value);		
		m_sliderUISize.setLabel(TempString);

		// is this different from the current value?
		if(value != app.GetGameSettings(m_iPad,eGameSetting_UISize)+1)
		{
			app.SetGameSettings(m_iPad,eGameSetting_UISize,value-1);
			// Apply the changes to the selected text position
			ui.UpdateSelectedItemPos(m_iPad);
		}

		break;
	case eControl_UISizeSplitscreen:
		m_sliderUISizeSplitscreen.handleSliderMove(value);

		swprintf( (WCHAR *)TempString, 256, L"%ls: %d", app.GetString( IDS_SLIDER_UISIZESPLITSCREEN ),value);			
		m_sliderUISizeSplitscreen.setLabel(TempString);

		if(value != app.GetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen)+1)
		{
			// slider is 1 to 3
			app.SetGameSettings(m_iPad,eGameSetting_UISizeSplitscreen,value-1);
			// Apply the changes to the selected text position
			ui.UpdateSelectedItemPos(m_iPad);
		}

		break;
	}
}
