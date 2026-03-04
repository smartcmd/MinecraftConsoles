#include "stdafx.h"
#include "UI.h"
#include "UIScene_ControlRemapMenu.h"
#include "..\..\Minecraft.h"
#include "..\..\Options.h"
#include "..\..\KeyMapping.h"

static const int MAX_ACTIONS = Options::keyMappings_length; // 14

static wstring getControllerButtonName(unsigned int button)
{
	if (button == _360_JOY_BUTTON_A)          return L"A";
	if (button == _360_JOY_BUTTON_B)          return L"B";
	if (button == _360_JOY_BUTTON_X)          return L"X";
	if (button == _360_JOY_BUTTON_Y)          return L"Y";
	if (button == _360_JOY_BUTTON_RB)         return L"RB";
	if (button == _360_JOY_BUTTON_LB)         return L"LB";
	if (button == _360_JOY_BUTTON_RT)         return L"RT";
	if (button == _360_JOY_BUTTON_LT)         return L"LT";
	if (button == _360_JOY_BUTTON_START)      return L"START";
	if (button == _360_JOY_BUTTON_BACK)       return L"BACK";
	if (button == _360_JOY_BUTTON_RTHUMB)     return L"R.STICK";
	if (button == _360_JOY_BUTTON_LTHUMB)     return L"L.STICK";
	if (button == _360_JOY_BUTTON_DPAD_UP)    return L"D-UP";
	if (button == _360_JOY_BUTTON_DPAD_DOWN)  return L"D-DOWN";
	if (button == _360_JOY_BUTTON_DPAD_LEFT)  return L"D-LEFT";
	if (button == _360_JOY_BUTTON_DPAD_RIGHT) return L"D-RIGHT";
	return L"NONE";
}

wstring UIScene_ControlRemapMenu::getActionFriendlyName(int action)
{
	switch (action)
	{
	case 0:  return L"Attack";
	case 1:  return L"Use";
	case 2:  return L"Forward";
	case 3:  return L"Left";
	case 4:  return L"Back";
	case 5:  return L"Right";
	case 6:  return L"Jump";
	case 7:  return L"Sneak";
	case 8:  return L"Drop";
	case 9:  return L"Inventory";
	case 10: return L"Chat";
	case 11: return L"Player List";
	case 12: return L"Pick Item";
	case 13: return L"Toggle Fog";
	default: return L"Unknown";
	}
}

wstring UIScene_ControlRemapMenu::getKeyDisplayName(Options *options, int action)
{
	int key = options->keyMappings[action]->key;
	if (key < 0)
	{
		int mouseButton = key + 101;
		switch (mouseButton)
		{
		case 1: return L"Mouse Left";
		case 2: return L"Mouse Right";
		case 3: return L"Mouse Middle";
		default:
			{
				WCHAR buf[32];
				swprintf(buf, 32, L"Mouse %d", mouseButton);
				return buf;
			}
		}
	}
	return Keyboard::getKeyName(key);
}

void UIScene_ControlRemapMenu::updateActionDisplay()
{
	wstring actionName = getActionFriendlyName(m_currentAction);
	wstring keyName = getKeyDisplayName(m_options, m_currentAction);

	wstring padName = L"N/A";
	if (m_currentAction < Options::CONTROLLER_ACTIONS)
		padName = getControllerButtonName(m_options->controllerMappings[m_currentAction]);

	// Update the info box labels/values
	m_labelLabels[eLabel_Action].init(L"Action:");
	m_labelValues[eLabel_Action].init(actionName);

	m_labelLabels[eLabel_Keyboard].init(L"Keyboard:");
	m_labelValues[eLabel_Keyboard].init(keyName);

	m_labelLabels[eLabel_Controller].init(L"Controller:");
	m_labelValues[eLabel_Controller].init(padName);

	// Blank out unused labels (Label3-8)
	for (int i = eLabel_COUNT; i < 9; i++)
	{
		m_labelLabels[i].init(L"");
		m_labelValues[i].init(L"");
	}
}

UIScene_ControlRemapMenu::UIScene_ControlRemapMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	initialiseMovie();

	m_options = Minecraft::GetInstance()->options;
	m_currentAction = 0;

	// Setup the Reset button (mapped to JoinGame button in SWF)
	m_buttonReset.init(L"Reset All Defaults", eControl_ResetDefaults);

	// Setup the action buttons list (mapped to GamePlayers list in SWF)
	m_buttonListActions.init(eControl_ActionButtons);
	m_buttonListActions.addItem(L"< Previous Action");
	m_buttonListActions.addItem(L"Next Action >");
	m_buttonListActions.addItem(L"Set Keyboard Key");
	m_buttonListActions.addItem(L"Set Controller Button");

	// Populate the info box display
	updateActionDisplay();

	doHorizontalResizeCheck();
}

wstring UIScene_ControlRemapMenu::getMoviePath()
{
	if (app.GetLocalPlayerCount() > 1)
		return L"JoinMenuSplit";
	else
		return L"JoinMenu";
}

void UIScene_ControlRemapMenu::updateTooltips()
{
	ui.SetTooltips(m_iPad, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK);
}

void UIScene_ControlRemapMenu::updateComponents()
{
	bool bNotInGame = (Minecraft::GetInstance()->level == NULL);
	if (bNotInGame)
	{
		m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, true);
		m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
	}
	else
	{
		m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, false);
		if (app.GetLocalPlayerCount() == 1) m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
		else m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, false);
	}
}

void UIScene_ControlRemapMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch (key)
	{
	case ACTION_MENU_CANCEL:
		if (pressed)
		{
			m_options->save();
			app.CheckGameSettingsChanged(true, iPad);
			navigateBack();
			handled = true;
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		if (pressed)
		{
			if (getControlFocus() == eControl_ActionButtons)
			{
				// ButtonList items don't fire handlePress in this SWF,
				// so handle the selection directly here
				int selection = m_buttonListActions.getCurrentSelection();
				if (selection == 0)
				{
					// Previous Action
					m_currentAction--;
					if (m_currentAction < 0) m_currentAction = MAX_ACTIONS - 1;
					updateActionDisplay();
				}
				else if (selection == 1)
				{
					// Next Action
					m_currentAction++;
					if (m_currentAction >= MAX_ACTIONS) m_currentAction = 0;
					updateActionDisplay();
				}
				else if (selection == 2)
				{
					// Set Keyboard Key
					m_promptInfo.detectKeyboard = true;
					m_promptInfo.callback = &UIScene_ControlRemapMenu::KeyboardPromptCallback;
					m_promptInfo.callbackParam = this;
					ui.NavigateToScene(m_iPad, eUIScene_InputPrompt, &m_promptInfo, eUILayer_Alert);
				}
				else if (selection == 3)
				{
					// Set Controller Button
					if (m_currentAction < Options::CONTROLLER_ACTIONS)
					{
						m_promptInfo.detectKeyboard = false;
						m_promptInfo.callback = &UIScene_ControlRemapMenu::ControllerPromptCallback;
						m_promptInfo.callbackParam = this;
						ui.NavigateToScene(m_iPad, eUIScene_InputPrompt, &m_promptInfo, eUILayer_Alert);
					}
				}
			}
			else
			{
				// Reset button (JoinGame) — let Flash handle the press callback
				sendInputToMovie(key, repeat, pressed, released);
			}
		}
		handled = true;
		break;
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
		handled = true;
		break;
	}
}

void UIScene_ControlRemapMenu::handleFocusChange(F64 controlId, F64 childId)
{
	switch ((int)controlId)
	{
	case eControl_ActionButtons:
		m_buttonListActions.updateChildFocus((int)childId);
		break;
	}
	updateTooltips();
}

void UIScene_ControlRemapMenu::handlePress(F64 controlId, F64 childId)
{
	ui.PlayUISFX(eSFX_Press);

	switch ((int)controlId)
	{
	case eControl_ResetDefaults:
		{
			UINT uiIDA[2];
			uiIDA[0] = IDS_CONFIRM_CANCEL;
			uiIDA[1] = IDS_CONFIRM_OK;
			// The 100+ commits before this broke this code, so I'm not sure if the callback system is still working correctly. If it is, this should show a confirmation dialog and reset to defaults if the user confirms.
			// ui.RequestMessageBox(IDS_DEFAULTS_TITLE, IDS_DEFAULTS_TEXT, uiIDA, 2, m_iPad, &UIScene_ControlRemapMenu::ResetDefaultsDialogReturned, this, app.GetStringTable(), NULL, 0, false);
		}
		break;
	}
}

void UIScene_ControlRemapMenu::KeyboardPromptCallback(void *param, int detectedValue)
{
	UIScene_ControlRemapMenu *pThis = (UIScene_ControlRemapMenu *)param;
	if (detectedValue == -1) return; // Cancelled

	if (pThis->m_currentAction < Options::keyMappings_length)
	{
		pThis->m_options->setKey(pThis->m_currentAction, detectedValue);
		pThis->m_options->save();
		pThis->updateActionDisplay();
	}
}

void UIScene_ControlRemapMenu::ControllerPromptCallback(void *param, int detectedValue)
{
	UIScene_ControlRemapMenu *pThis = (UIScene_ControlRemapMenu *)param;
	if (detectedValue == -1) return; // Cancelled

	if (pThis->m_currentAction < Options::CONTROLLER_ACTIONS)
	{
		pThis->m_options->setControllerMapping(pThis->m_currentAction, (unsigned int)detectedValue);
		pThis->m_options->save();
		pThis->updateActionDisplay();
	}
}

int UIScene_ControlRemapMenu::ResetDefaultsDialogReturned(void *pParam, int iPad, C4JStorage::EMessageResult result)
{
	UIScene_ControlRemapMenu *pClass = (UIScene_ControlRemapMenu *)pParam;
	if (result == C4JStorage::EMessage_ResultDecline)
	{
		pClass->m_options->resetKeyboardDefaults();
		pClass->m_options->resetControllerDefaults();
		pClass->m_options->save();
		pClass->updateActionDisplay();
	}
	return 0;
}
