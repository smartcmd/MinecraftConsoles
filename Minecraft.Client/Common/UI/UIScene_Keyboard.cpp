#include "stdafx.h"
#include "UI.h"
#include "UIScene_Keyboard.h"
#include "..\..\Tesselator.h"

#define KEYBOARD_DONE_TIMER_ID 0
#define KEYBOARD_DONE_TIMER_TIME 100

extern char chGlobalText[256];

UIScene_Keyboard::UIScene_Keyboard(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	if (initData != NULL)
	{
		m_initData = *((KeyboardInitData *)initData);
	}

	wstring labelText = m_initData.title.empty() ? L"Enter Text" : m_initData.title;
	m_EnterTextLabel.init(labelText);

	m_KeyboardTextInput.init(m_initData.initialText.empty() ? L"" : m_initData.initialText, -1);
	m_KeyboardTextInput.SetCharLimit(m_initData.charLimit > 0 ? m_initData.charLimit : 15);
	
	m_ButtonSpace.init(L"Space", -1);
	m_ButtonCursorLeft.init(L"Cursor Left", -1);
	m_ButtonCursorRight.init(L"Cursor Right", -1);
	m_ButtonCaps.init(L"Caps", -1);
	m_ButtonDone.init(L"Done", 0);	// only the done button needs an id, the others will never call back!
	m_ButtonSymbols.init(L"Symbols", -1);
	m_ButtonBackspace.init(L"Backspace", -1);

	// Initialise function keyboard Buttons and set alternative symbol button string
	wstring label = L"Abc";
	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcInitFunctionButtons , 1 , value );

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
			if(m_initData.Func != NULL)
			{
				m_initData.Func(m_initData.lpParam, L"", false);
			}
#endif
			navigateBack();
			handled = true;
			break;
		case ACTION_MENU_X:					// X
			out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcBackspaceButtonPressed, 0 , NULL );
			handled = true;
			break;
		case ACTION_MENU_PAGEUP:			// LT
			out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSymbolButtonPressed, 0 , NULL );
			handled = true;
			break;
		case ACTION_MENU_Y:					// Y
			out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSpaceButtonPressed, 0 , NULL );
			handled = true;
			break;
		case ACTION_MENU_STICK_PRESS:		// LS
			out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcCapsButtonPressed, 0 , NULL );
			handled = true;
			break;
		case ACTION_MENU_LEFT_SCROLL:		// LB
			out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcCursorLeftButtonPressed, 0 , NULL );
			handled = true;
			break;
		case ACTION_MENU_RIGHT_SCROLL:		// RB
			out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcCursorRightButtonPressed, 0 , NULL );
			handled = true;
			break;
		case ACTION_MENU_PAUSEMENU:			// Start
			if(!m_bKeyboardDonePressed)
			{
				out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcDoneButtonPressed, 0 , NULL );
				
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
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
		handled = true;
		break;
	}
}

void UIScene_Keyboard::handlePress(F64 controlId, F64 childId)
{
	if((int)controlId == 0)
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
	// Debug
	app.DebugPrintf("UI Keyboard - DONE - [%ls]\n", m_KeyboardTextInput.getLabel());

	// Capture the final string value from the keyboard text input
	m_capturedText = m_KeyboardTextInput.getLabel();

#ifdef _WINDOWS64
	// Invoke the callback if one was provided
	if(m_initData.Func != NULL)
	{
		m_initData.Func(m_initData.lpParam, m_capturedText, true);
	}


#endif
	navigateBack();
}

void UIScene_Keyboard::render(S32 width, S32 height, C4JRender::eViewportType viewport)
{
	ui.setupRenderPosition(viewport);

	// Draw a semi transparent dark overlay behind the keyboard
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

	Tesselator *t = Tesselator::getInstance();
	t->begin();
	t->vertex(0.0f,          (float)height, 0.0f);
	t->vertex((float)width,  (float)height, 0.0f);
	t->vertex((float)width,  0.0f,          0.0f);
	t->vertex(0.0f,          0.0f,          0.0f);
	t->end();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	UIScene::render(width, height, viewport);
}