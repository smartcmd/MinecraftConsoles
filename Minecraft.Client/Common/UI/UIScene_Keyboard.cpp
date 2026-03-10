#include "stdafx.h"
#include "UI.h"
#include "UIScene_Keyboard.h"

#ifdef _WINDOWS64
// Global buffer that stores the text entered in the native keyboard scene.
// Callbacks retrieve it via Win64_GetKeyboardText() declared in UIStructs.h.
wchar_t g_Win64KeyboardResult[256] = {};
#endif

#define KEYBOARD_DONE_TIMER_ID 0
#define KEYBOARD_DONE_TIMER_TIME 100

UIScene_Keyboard::UIScene_Keyboard(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

#ifdef _WINDOWS64
	m_win64Callback = nullptr;
	m_win64CallbackParam = nullptr;
	m_win64TextBuffer = L"";
	m_win64MaxChars = 25;

	const wchar_t* titleText = L"Enter text";
	const wchar_t* defaultText = L"";

	m_bPCMode = false;
	if (initData)
	{
		UIKeyboardInitData* kbData = static_cast<UIKeyboardInitData *>(initData);
		m_win64Callback = kbData->callback;
		m_win64CallbackParam = kbData->lpParam;
		if (kbData->title)       titleText        = kbData->title;
		if (kbData->defaultText) defaultText      = kbData->defaultText;
		m_win64MaxChars = kbData->maxChars;
		m_bPCMode = kbData->pcMode;
	}

	m_win64TextBuffer = defaultText;
	m_iCursorPos = (int)m_win64TextBuffer.length();

	m_EnterTextLabel.init(titleText);
	m_KeyboardTextInput.init(defaultText, -1);
	m_KeyboardTextInput.SetCharLimit(m_win64MaxChars);

	// Clear any leftover typed characters from a previous keyboard session
	g_KBMInput.ClearCharBuffer();
	g_Win64KeyboardResult[0] = L'\0';
#else
	m_EnterTextLabel.init(L"Enter Sign Text");

	m_KeyboardTextInput.init(L"", -1);
	m_KeyboardTextInput.SetCharLimit(15);
#endif
	
	m_ButtonSpace.init(L"Space", -1);
	m_ButtonCursorLeft.init(L"Cur L", -1);
	m_ButtonCursorRight.init(L"Cur R", -1);
	m_ButtonCaps.init(L"Caps", -1);
	m_ButtonDone.init(L"Done", 0);	// only the done button needs an id, the others will never call back!
	m_ButtonSymbols.init(L"Symbols", -1);
	m_ButtonBackspace.init(L"Backspace", -1);

	// Initialise function keyboard Buttons and set alternative symbol button string
#ifdef _WINDOWS64
	if (!m_bPCMode)
#endif
	{
		wstring label = L"Abc";
		IggyStringUTF16 stringVal;
		stringVal.string = (IggyUTF16*)label.c_str();
		stringVal.length = label.length();

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_string_UTF16;
		value[0].string16 = stringVal;

		IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcInitFunctionButtons , 1 , value );
	}

#ifdef _WINDOWS64
	if (m_bPCMode)
	{
		// PC text-input mode: hide all on-screen buttons, user types with physical keyboard

		// Hide the mapped function-row buttons
		m_ButtonSpace.setVisible(false);
		m_ButtonCursorLeft.setVisible(false);
		m_ButtonCursorRight.setVisible(false);
		m_ButtonCaps.setVisible(false);
		m_ButtonSymbols.setVisible(false);
		m_ButtonBackspace.setVisible(false);

		// Hide the letter/number key grid (Flash-baked, not mapped as UIControls)
		static const char* s_keyNames[] = {
			"Button_q", "Button_w", "Button_e", "Button_r", "Button_t",
			"Button_y", "Button_u", "Button_i", "Button_o", "Button_p",
			"Button_a", "Button_s", "Button_d", "Button_f", "Button_g",
			"Button_h", "Button_j", "Button_k", "Button_l", "Button_apostraphy",
			"Button_z", "Button_x", "Button_c", "Button_v", "Button_b",
			"Button_n", "Button_m", "Button_comma", "Button_stop", "Button_qmark",
			"Button_0", "Button_1", "Button_2", "Button_3", "Button_4",
			"Button_5", "Button_6", "Button_7", "Button_8", "Button_9"
		};
		IggyName nameVisible = registerFastName(L"visible");
		IggyValuePath* root = IggyPlayerRootPath(getMovie());
		for (int i = 0; i < static_cast<int>(sizeof(s_keyNames) / sizeof(s_keyNames[0])); ++i)
		{
			IggyValuePath keyPath;
			if (IggyValuePathMakeNameRef(&keyPath, root, s_keyNames[i]))
				IggyValueSetBooleanRS(&keyPath, nameVisible, nullptr, false);
		}

		m_KeyboardTextInput.setCaretVisible(true);
		m_KeyboardTextInput.setCaretIndex(m_iCursorPos);
	}
#endif

	m_bKeyboardDonePressed = false;

	parentLayer->addComponent(iPad,eUIComponent_MenuBackground);
}

UIScene_Keyboard::~UIScene_Keyboard()
{
	m_parentLayer->removeComponent(eUIComponent_MenuBackground);
}

wstring UIScene_Keyboard::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1 && !m_parentLayer->IsFullscreenGroup())
	{
		return L"KeyboardSplit";
	}
	else
	{
		return L"Keyboard";
	}
}

void UIScene_Keyboard::updateTooltips()
{
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK, -1, -1);
}

bool UIScene_Keyboard::allowRepeat(int key)
{
	// 4J - TomK - we want to allow X and Y repeats!
	switch(key)
	{
	case ACTION_MENU_OK:
	case ACTION_MENU_CANCEL:
	case ACTION_MENU_A:
	case ACTION_MENU_B:
	case ACTION_MENU_PAUSEMENU:
	//case ACTION_MENU_X:
	//case ACTION_MENU_Y:
		return false;
	}
	return true;
}

#ifdef _WINDOWS64
void UIScene_Keyboard::tick()
{
	UIScene::tick();

	// Sync our buffer from Flash so we pick up changes made via controller/on-screen buttons.
	// Without this, switching between controller and keyboard would use stale text.
	// In PC mode we own the buffer — skip sync to preserve cursor position.
	if (!m_bPCMode)
	{
		const wchar_t* flashText = m_KeyboardTextInput.getLabel();
		if (flashText)
			m_win64TextBuffer = flashText;
	}

	// Accumulate physical keyboard chars into our own buffer, then push to Flash via setLabel.
	// This bypasses Iggy's focus system (char events only route to the focused element).
	// The char buffer is cleared on open so Enter/clicks from the triggering action don't leak in.
	wchar_t ch;
	bool changed = false;
	while (g_KBMInput.ConsumeChar(ch))
	{
		if (ch == 0x08) // backspace
		{
			if (m_bPCMode)
			{
				if (m_iCursorPos > 0)
				{
					m_win64TextBuffer.erase(m_iCursorPos - 1, 1);
					m_iCursorPos--;
					changed = true;
				}
			}
			else if (!m_win64TextBuffer.empty())
			{
				m_win64TextBuffer.pop_back();
				changed = true;
			}
		}
		else if (ch == 0x0D) // enter - confirm
		{
			if (!m_bKeyboardDonePressed)
			{
				addTimer(KEYBOARD_DONE_TIMER_ID, KEYBOARD_DONE_TIMER_TIME);
				m_bKeyboardDonePressed = true;
			}
		}
		else if (static_cast<int>(m_win64TextBuffer.length()) < m_win64MaxChars)
		{
			if (m_bPCMode)
			{
				m_win64TextBuffer.insert(m_iCursorPos, 1, ch);
				m_iCursorPos++;
			}
			else
			{
				m_win64TextBuffer += ch;
			}
			changed = true;
		}
	}

	if (m_bPCMode)
	{
		// Arrow keys, Home, End, Delete for cursor movement
		if (g_KBMInput.IsKeyPressed(VK_LEFT) && m_iCursorPos > 0)
			m_iCursorPos--;
		if (g_KBMInput.IsKeyPressed(VK_RIGHT) && m_iCursorPos < (int)m_win64TextBuffer.length())
			m_iCursorPos++;
		if (g_KBMInput.IsKeyPressed(VK_HOME))
			m_iCursorPos = 0;
		if (g_KBMInput.IsKeyPressed(VK_END))
			m_iCursorPos = (int)m_win64TextBuffer.length();
		if (g_KBMInput.IsKeyPressed(VK_DELETE) && m_iCursorPos < (int)m_win64TextBuffer.length())
		{
			m_win64TextBuffer.erase(m_iCursorPos, 1);
			changed = true;
		}
	}

	if (changed)
		m_KeyboardTextInput.setLabel(m_win64TextBuffer.c_str(), true /*instant*/);

	if (m_bPCMode)
	{
		m_KeyboardTextInput.setCaretVisible(true);
		m_KeyboardTextInput.setCaretIndex(m_iCursorPos);
	}
}
#endif

void UIScene_Keyboard::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	IggyDataValue result;
	IggyResult out;

	if(repeat || pressed)
	{
		switch(key)
		{
		case ACTION_MENU_CANCEL:
#ifdef _WINDOWS64
			{
				// Cache before navigateBack() destroys this scene
				int(*cb)(LPVOID, const bool) = m_win64Callback;
				LPVOID cbParam = m_win64CallbackParam;
				navigateBack();
				if (cb)
					cb(cbParam, false);
			}
#else
			navigateBack();
#endif
			handled = true;
			break;
		case ACTION_MENU_X:					// X
		case ACTION_MENU_PAGEUP:			// LT
		case ACTION_MENU_Y:					// Y
		case ACTION_MENU_STICK_PRESS:		// LS
		case ACTION_MENU_LEFT_SCROLL:		// LB
		case ACTION_MENU_RIGHT_SCROLL:		// RB
#ifdef _WINDOWS64

			if (m_bPCMode)
			{
				handled = true;
				break;
			}
#endif
			if (key == ACTION_MENU_X)
				out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcBackspaceButtonPressed, 0 , nullptr);
			else if (key == ACTION_MENU_PAGEUP)
				out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSymbolButtonPressed, 0 , nullptr);
			else if (key == ACTION_MENU_Y)
				out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSpaceButtonPressed, 0 , nullptr);
			else if (key == ACTION_MENU_STICK_PRESS)
				out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcCapsButtonPressed, 0 , nullptr);
			else if (key == ACTION_MENU_LEFT_SCROLL)
				out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcCursorLeftButtonPressed, 0 , nullptr);
			else if (key == ACTION_MENU_RIGHT_SCROLL)
				out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcCursorRightButtonPressed, 0 , nullptr);
			handled = true;
			break;
		case ACTION_MENU_PAUSEMENU:			// Start
			if(!m_bKeyboardDonePressed)
			{
				out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcDoneButtonPressed, 0 , nullptr );
				
				// kick off done timer
				addTimer(KEYBOARD_DONE_TIMER_ID,KEYBOARD_DONE_TIMER_TIME);
				m_bKeyboardDonePressed = true;
			}
			handled = true;
			break;
		}
	}

	switch(key)
	{
	case ACTION_MENU_OK:
#ifdef _WINDOWS64
		if (m_bPCMode)
		{
			// pressing enter sometimes causes a "y" to be entered.
			handled = true;
			break;
		}
#endif
		// fall through for controller mode
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
#ifdef _WINDOWS64
		if (!m_bPCMode)
#endif
			sendInputToMovie(key, repeat, pressed, released);
		handled = true;
		break;
	}
}

void UIScene_Keyboard::handlePress(F64 controlId, F64 childId)
{
	if(static_cast<int>(controlId) == 0)
	{
		// Done has been pressed. At this point we can query for the input string and pass it on to wherever it is needed.
		// we can not query for m_KeyboardTextInput.getLabel() here because we're in an iggy callback so we need to wait a frame.
		if(!m_bKeyboardDonePressed)
		{
			// kick off done timer
			addTimer(KEYBOARD_DONE_TIMER_ID,KEYBOARD_DONE_TIMER_TIME);
			m_bKeyboardDonePressed = true;
		}
	}
}

void UIScene_Keyboard::handleTimerComplete(int id)
{
	if(id == KEYBOARD_DONE_TIMER_ID)
	{
		// remove timer
		killTimer(KEYBOARD_DONE_TIMER_ID);

		// we're done here!
		KeyboardDonePressed();
	}
}

void UIScene_Keyboard::KeyboardDonePressed()
{
#ifdef _WINDOWS64
	// Use getLabel() here — this is a timer callback (not an Iggy callback) so it's safe.
	// getLabel() reflects both physical keyboard input (pushed via setLabel) and
	// on-screen button input (set directly by Flash ActionScript).
	const wchar_t* finalText = m_KeyboardTextInput.getLabel();
	app.DebugPrintf("UI Keyboard - DONE - [%ls]\n", finalText);

	// Store the typed text so callbacks can retrieve it via Win64_GetKeyboardText()
	wcsncpy_s(g_Win64KeyboardResult, 256, finalText, _TRUNCATE);

	// Cache callback and param before navigateBack() which destroys this scene
	int(*cb)(LPVOID, const bool) = m_win64Callback;
	LPVOID cbParam = m_win64CallbackParam;

	// Navigate back so the scene stack is restored before the callback runs
	navigateBack();

	// Fire callback: bRes=true means confirmed
	if (cb)
		cb(cbParam, true);
#else
	app.DebugPrintf("UI Keyboard - DONE - [%ls]\n", m_KeyboardTextInput.getLabel());
	navigateBack();
#endif
}