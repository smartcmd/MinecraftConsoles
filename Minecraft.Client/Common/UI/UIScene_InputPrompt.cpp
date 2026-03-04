#include "stdafx.h"
#include "UI.h"
#include "UIScene_InputPrompt.h"

#ifdef _WINDOWS64
#include "..\..\Windows64\KeyboardMouseInput.h"
#include <Xinput.h>
#endif

UIScene_InputPrompt::UIScene_InputPrompt(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	initialiseMovie();

	InputPromptInfo *info = (InputPromptInfo *)initData;
	m_detectKeyboard = info->detectKeyboard;
	m_callback = info->callback;
	m_callbackParam = info->callbackParam;
	m_completed = false;
	m_waitForRelease = true; // Wait for the A button press that opened this to be released

	// Call the MessageBox Flash Init function: 1 button, focus on button 0
	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = 1; // 1 button
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = 0; // focus button 0
	IggyPlayerCallMethodRS(getMovie(), &result, IggyPlayerRootPath(getMovie()), m_funcInit, 2, value);

	// Set up the single Cancel button
	m_buttonCancel.init(L"Cancel", eControl_Cancel);

	// Set title and content labels directly (bypass string table)
	if (m_detectKeyboard)
	{
		m_labelTitle.init(L"Set Keyboard Key");
		m_labelContent.init(L"Press a key...");
	}
	else
	{
		m_labelTitle.init(L"Set Controller Button");
		m_labelContent.init(L"Press a button...");
	}

	IggyPlayerCallMethodRS(getMovie(), &result, IggyPlayerRootPath(getMovie()), m_funcAutoResize, 0, NULL);

	parentLayer->addComponent(iPad, eUIComponent_MenuBackground);
}

UIScene_InputPrompt::~UIScene_InputPrompt()
{
	m_parentLayer->removeComponent(eUIComponent_MenuBackground);
}

wstring UIScene_InputPrompt::getMoviePath()
{
	if (app.GetLocalPlayerCount() > 1 && !m_parentLayer->IsFullscreenGroup())
		return L"MessageBoxSplit";
	else
		return L"MessageBox";
}

void UIScene_InputPrompt::updateTooltips()
{
	ui.SetTooltips(m_parentLayer->IsFullscreenGroup() ? XUSER_INDEX_ANY : m_iPad, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_CANCEL);
}

int UIScene_InputPrompt::pollKeyboard()
{
#ifdef _WINDOWS64
	// Check mouse buttons first
	if (Mouse::isButtonDown(0)) return -100; // Left mouse = attack default
	if (Mouse::isButtonDown(1)) return -99;  // Right mouse = use default
	if (Mouse::isButtonDown(2)) return -98;  // Middle mouse = pick item default

	// Check all keyboard keys
	for (int k = 0; k < Keyboard::KEY_COUNT; k++)
	{
		if (Keyboard::isKeyDown(k))
			return k;
	}
#endif
	return -1; // Nothing detected
}

int UIScene_InputPrompt::pollController()
{
#ifdef _WINDOWS64
	XINPUT_STATE state;
	memset(&state, 0, sizeof(state));
	if (XInputGetState(0, &state) == ERROR_SUCCESS)
	{
		WORD buttons = state.Gamepad.wButtons;
		// Map XINPUT_GAMEPAD_* to _360_JOY_BUTTON_*
		if (buttons & XINPUT_GAMEPAD_A)              return _360_JOY_BUTTON_A;
		if (buttons & XINPUT_GAMEPAD_B)              return _360_JOY_BUTTON_B;
		if (buttons & XINPUT_GAMEPAD_X)              return _360_JOY_BUTTON_X;
		if (buttons & XINPUT_GAMEPAD_Y)              return _360_JOY_BUTTON_Y;
		if (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) return _360_JOY_BUTTON_RB;
		if (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER)  return _360_JOY_BUTTON_LB;
		if (buttons & XINPUT_GAMEPAD_START)          return _360_JOY_BUTTON_START;
		if (buttons & XINPUT_GAMEPAD_BACK)           return _360_JOY_BUTTON_BACK;
		if (buttons & XINPUT_GAMEPAD_RIGHT_THUMB)    return _360_JOY_BUTTON_RTHUMB;
		if (buttons & XINPUT_GAMEPAD_LEFT_THUMB)     return _360_JOY_BUTTON_LTHUMB;
		if (buttons & XINPUT_GAMEPAD_DPAD_UP)        return _360_JOY_BUTTON_DPAD_UP;
		if (buttons & XINPUT_GAMEPAD_DPAD_DOWN)      return _360_JOY_BUTTON_DPAD_DOWN;
		if (buttons & XINPUT_GAMEPAD_DPAD_LEFT)      return _360_JOY_BUTTON_DPAD_LEFT;
		if (buttons & XINPUT_GAMEPAD_DPAD_RIGHT)     return _360_JOY_BUTTON_DPAD_RIGHT;

		// Check triggers (threshold > 128)
		if (state.Gamepad.bRightTrigger > 128) return _360_JOY_BUTTON_RT;
		if (state.Gamepad.bLeftTrigger > 128)  return _360_JOY_BUTTON_LT;
	}
#endif
	return -1; // Nothing detected
}

void UIScene_InputPrompt::tick()
{
	UIScene::tick();

	if (m_completed) return;

#ifdef _WINDOWS64
	if (m_waitForRelease)
	{
		// Wait until no keyboard keys, mouse buttons, or controller buttons are pressed
		bool anyPressed = false;
		for (int k = 0; k < Keyboard::KEY_COUNT; k++)
		{
			if (Keyboard::isKeyDown(k)) { anyPressed = true; break; }
		}
		if (!anyPressed)
		{
			for (int b = 0; b < 3; b++)
			{
				if (Mouse::isButtonDown(b)) { anyPressed = true; break; }
			}
		}
		if (!anyPressed)
		{
			XINPUT_STATE state;
			memset(&state, 0, sizeof(state));
			if (XInputGetState(0, &state) == ERROR_SUCCESS)
			{
				if (state.Gamepad.wButtons != 0 || state.Gamepad.bRightTrigger > 128 || state.Gamepad.bLeftTrigger > 128)
					anyPressed = true;
			}
		}
		if (!anyPressed)
			m_waitForRelease = false;
		return;
	}

	int detected = m_detectKeyboard ? pollKeyboard() : pollController();
	if (detected != -1)
	{
		m_completed = true;
		navigateBack();
		if (m_callback) m_callback(m_callbackParam, detected);
	}
#endif
}

void UIScene_InputPrompt::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch (key)
	{
	case ACTION_MENU_CANCEL:
		if (pressed)
		{
			m_completed = true;
			navigateBack();
			if (m_callback) m_callback(m_callbackParam, -1); // -1 = cancelled
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
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
	handled = true; // Consume all input (like MessageBox)
}

void UIScene_InputPrompt::handlePress(F64 controlId, F64 childId)
{
	// Cancel button pressed
	m_completed = true;
	navigateBack();
	if (m_callback) m_callback(m_callbackParam, -1); // -1 = cancelled
}
