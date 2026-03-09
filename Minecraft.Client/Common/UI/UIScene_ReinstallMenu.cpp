#include "stdafx.h"
#include "UI.h"
#include "UIScene_ReinstallMenu.h"

UIScene_ReinstallMenu::UIScene_ReinstallMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

#if TO_BE_IMPLEMENTED
	XuiControlSetText(m_Buttons[eControl_Theme],app.GetString(IDS_REINSTALL_THEME));
	XuiControlSetText(m_Buttons[eControl_Gamerpic1],app.GetString(IDS_REINSTALL_GAMERPIC_1));
	XuiControlSetText(m_Buttons[eControl_Gamerpic2],app.GetString(IDS_REINSTALL_GAMERPIC_2));
	XuiControlSetText(m_Buttons[eControl_Avatar1],app.GetString(IDS_REINSTALL_AVATAR_ITEM_1));
	XuiControlSetText(m_Buttons[eControl_Avatar2],app.GetString(IDS_REINSTALL_AVATAR_ITEM_2));
	XuiControlSetText(m_Buttons[eControl_Avatar3],app.GetString(IDS_REINSTALL_AVATAR_ITEM_3));
#endif
}

wstring UIScene_ReinstallMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"ReinstallSplit";
	}
	else
	{
		return L"ReinstallMenu";
	}
}

void UIScene_ReinstallMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE);
}

void UIScene_ReinstallMenu::updateComponents()
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

		// 4J Stu - Do we want to show the logo in-game?
		//if( app.GetLocalPlayerCount() == 1 ) m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
		//else m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
		m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);

	}
}

void UIScene_ReinstallMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");

	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed && !repeat)
		{
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_ReinstallMenu::handlePress(F64 controlId, F64 childId)
{
#if TO_BE_IMPLEMENTED
	switch((int)controlId)
	{
	case BUTTON_HAO_CHANGESKIN:
		ui.NavigateToScene(m_iPad, eUIScene_SkinSelectMenu);
		break;
	case BUTTON_HAO_HOWTOPLAY:
		ui.NavigateToScene(m_iPad, eUIScene_HowToPlayMenu);
		break;
	case BUTTON_HAO_CONTROLS:
		ui.NavigateToScene(m_iPad, eUIScene_ControlsMenu);
		break;
	case BUTTON_HAO_SETTINGS:
		ui.NavigateToScene(m_iPad, eUIScene_SettingsMenu);
		break;
	case BUTTON_HAO_CREDITS:
		ui.NavigateToScene(m_iPad, eUIScene_Credits);
		break;
	case BUTTON_HAO_REINSTALL:
		ui.NavigateToScene(m_iPad, eUIScene_ReinstallMenu);
		break;
	case BUTTON_HAO_DEBUG:
		ui.NavigateToScene(m_iPad, eUIScene_DebugOptions);
		break;
	}
#endif
}
