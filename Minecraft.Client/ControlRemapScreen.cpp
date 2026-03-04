#include "stdafx.h"
#include "ControlRemapScreen.h"
#include "Options.h"
#include "KeyMapping.h"
#include "SmallButton.h"
#include "..\Minecraft.World\net.minecraft.locale.h"

ControlRemapScreen::ControlRemapScreen(Screen *lastScreen, Options *options)
{
	title = L"Control Remapping";
	selectedKey = -1;
	controllerActionSelected = -1;
	controllerButtonCursor = 0;
	currentMode = MODE_KEYBOARD;

	this->lastScreen = lastScreen;
	this->options = options;
}

int ControlRemapScreen::getLeftScreenPosition()
{
	return width / 2 - 155;
}

unsigned int ControlRemapScreen::getControllerButtonByIndex(int index)
{
	switch (index)
	{
	case 0:  return _360_JOY_BUTTON_A;
	case 1:  return _360_JOY_BUTTON_B;
	case 2:  return _360_JOY_BUTTON_X;
	case 3:  return _360_JOY_BUTTON_Y;
	case 4:  return _360_JOY_BUTTON_RB;
	case 5:  return _360_JOY_BUTTON_LB;
	case 6:  return _360_JOY_BUTTON_RT;
	case 7:  return _360_JOY_BUTTON_LT;
	case 8:  return _360_JOY_BUTTON_START;
	case 9:  return _360_JOY_BUTTON_BACK;
	case 10: return _360_JOY_BUTTON_RTHUMB;
	case 11: return _360_JOY_BUTTON_LTHUMB;
	case 12: return _360_JOY_BUTTON_DPAD_UP;
	case 13: return _360_JOY_BUTTON_DPAD_DOWN;
	case 14: return _360_JOY_BUTTON_DPAD_LEFT;
	case 15: return _360_JOY_BUTTON_DPAD_RIGHT;
	default: return 0;
	}
}

wstring ControlRemapScreen::getControllerButtonNameByIndex(int index)
{
	switch (index)
	{
	case 0:  return L"A";
	case 1:  return L"B";
	case 2:  return L"X";
	case 3:  return L"Y";
	case 4:  return L"RB";
	case 5:  return L"LB";
	case 6:  return L"RT";
	case 7:  return L"LT";
	case 8:  return L"START";
	case 9:  return L"BACK";
	case 10: return L"R.STICK";
	case 11: return L"L.STICK";
	case 12: return L"D-UP";
	case 13: return L"D-DOWN";
	case 14: return L"D-LEFT";
	case 15: return L"D-RIGHT";
	default: return L"NONE";
	}
}

void ControlRemapScreen::rebuildButtons()
{
	buttons.clear();

	if (currentMode == MODE_KEYBOARD)
	{
		int leftPos = getLeftScreenPosition();
		for (int i = 0; i < Options::keyMappings_length; i++)
		{
			buttons.push_back(new SmallButton(i, leftPos + i % 2 * ROW_WIDTH, height / 6 + 24 * (i >> 1), BUTTON_WIDTH, 20, options->getKeyMessage(i)));
		}
	}
	else // MODE_CONTROLLER
	{
		int leftPos = getLeftScreenPosition();
		for (int i = 0; i < Options::CONTROLLER_ACTIONS; i++)
		{
			wstring buttonName = Options::getControllerButtonName(options->controllerMappings[i]);
			buttons.push_back(new SmallButton(i, leftPos + i % 2 * ROW_WIDTH, height / 6 + 24 * (i >> 1), BUTTON_WIDTH, 20, buttonName));
		}
	}

	// Mode toggle button
	wstring modeLabel = (currentMode == MODE_KEYBOARD) ? L"Mode: Keyboard" : L"Mode: Controller";
	buttons.push_back(new Button(MODE_TOGGLE_BUTTON_ID, width / 2 - 155, height / 6 + 24 * 8, 150, 20, modeLabel));

	// Reset defaults button
	buttons.push_back(new Button(RESET_DEFAULTS_BUTTON_ID, width / 2 + 5, height / 6 + 24 * 8, 150, 20, L"Reset Defaults"));

	// Done button
	Language *language = Language::getInstance();
	buttons.push_back(new Button(DONE_BUTTON_ID, width / 2 - 100, height / 6 + 24 * 9 + 6, language->getElement(L"gui.done")));
}

void ControlRemapScreen::init()
{
	Language *language = Language::getInstance();
	title = L"Control Remapping";
	rebuildButtons();
}

void ControlRemapScreen::buttonClicked(Button *button)
{
	if (button->id == DONE_BUTTON_ID)
	{
		minecraft->options->save();
		minecraft->setScreen(lastScreen);
		return;
	}

	if (button->id == MODE_TOGGLE_BUTTON_ID)
	{
		currentMode = (currentMode == MODE_KEYBOARD) ? MODE_CONTROLLER : MODE_KEYBOARD;
		selectedKey = -1;
		controllerActionSelected = -1;
		rebuildButtons();
		return;
	}

	if (button->id == RESET_DEFAULTS_BUTTON_ID)
	{
		if (currentMode == MODE_KEYBOARD)
			options->resetKeyboardDefaults();
		else
			options->resetControllerDefaults();
		rebuildButtons();
		return;
	}

	// Action button clicked - start remapping
	if (currentMode == MODE_KEYBOARD)
	{
		// Reset all keyboard button labels first
		for (int i = 0; i < Options::keyMappings_length; i++)
		{
			buttons[i]->msg = options->getKeyMessage(i);
		}
		selectedKey = button->id;
		button->msg = L"> " + options->getKeyMessage(button->id) + L" <";
	}
	else // MODE_CONTROLLER
	{
		// Reset all controller button labels first
		for (int i = 0; i < Options::CONTROLLER_ACTIONS; i++)
		{
			buttons[i]->msg = Options::getControllerButtonName(options->controllerMappings[i]);
		}
		controllerActionSelected = button->id;
		controllerButtonCursor = 0;
		button->msg = L"> " + Options::getControllerButtonName(options->controllerMappings[button->id]) + L" <";
	}
}

void ControlRemapScreen::keyPressed(wchar_t eventCharacter, int eventKey)
{
	if (currentMode == MODE_KEYBOARD && selectedKey >= 0)
	{
		// Remap the selected keyboard action to the pressed key
		options->setKey(selectedKey, eventKey);
		buttons[selectedKey]->msg = options->getKeyMessage(selectedKey);
		selectedKey = -1;
	}
	else if (currentMode == MODE_CONTROLLER && controllerActionSelected >= 0)
	{
		// Use Left/Right to cycle through controller buttons, Enter to confirm, Escape to cancel
		if (eventKey == Keyboard::KEY_RIGHT)
		{
			controllerButtonCursor = (controllerButtonCursor + 1) % CONTROLLER_BUTTON_COUNT;
			buttons[controllerActionSelected]->msg = L"[ " + getControllerButtonNameByIndex(controllerButtonCursor) + L" ]";
		}
		else if (eventKey == Keyboard::KEY_LEFT)
		{
			controllerButtonCursor = (controllerButtonCursor + CONTROLLER_BUTTON_COUNT - 1) % CONTROLLER_BUTTON_COUNT;
			buttons[controllerActionSelected]->msg = L"[ " + getControllerButtonNameByIndex(controllerButtonCursor) + L" ]";
		}
		else if (eventKey == Keyboard::KEY_RETURN || eventKey == Keyboard::KEY_SPACE)
		{
			// Confirm selection
			unsigned int btn = getControllerButtonByIndex(controllerButtonCursor);
			options->setControllerMapping(controllerActionSelected, btn);
			buttons[controllerActionSelected]->msg = Options::getControllerButtonName(options->controllerMappings[controllerActionSelected]);
			controllerActionSelected = -1;
		}
		else if (eventKey == Keyboard::KEY_ESCAPE)
		{
			// Cancel
			buttons[controllerActionSelected]->msg = Options::getControllerButtonName(options->controllerMappings[controllerActionSelected]);
			controllerActionSelected = -1;
		}
	}
	else
	{
		Screen::keyPressed(eventCharacter, eventKey);
	}
}

void ControlRemapScreen::render(int xm, int ym, float a)
{
	renderBackground();
	drawCenteredString(font, title, width / 2, 20, 0xffffff);

	int leftPos = getLeftScreenPosition();

	if (currentMode == MODE_KEYBOARD)
	{
		// Draw action names next to keyboard buttons
		for (int i = 0; i < Options::keyMappings_length; i++)
		{
			drawString(font, options->getKeyDescription(i), leftPos + i % 2 * ROW_WIDTH + BUTTON_WIDTH + 6, height / 6 + 24 * (i >> 1) + 7, 0xffffffff);
		}
	}
	else // MODE_CONTROLLER
	{
		// Draw action names next to controller buttons
		for (int i = 0; i < Options::CONTROLLER_ACTIONS; i++)
		{
			drawString(font, Options::getControllerActionName(i), leftPos + i % 2 * ROW_WIDTH + BUTTON_WIDTH + 6, height / 6 + 24 * (i >> 1) + 7, 0xffffffff);
		}

		// Draw help text when selecting a controller button
		if (controllerActionSelected >= 0)
		{
			drawCenteredString(font, L"LEFT/RIGHT to cycle, ENTER to confirm, ESC to cancel", width / 2, height / 6 + 24 * 7 + 12, 0xffff55);
		}
	}

	Screen::render(xm, ym, a);
}
