#pragma once

#include "UIScene.h"
#include "UIStructs.h"

class UIScene_Keyboard : public UIScene
{
private:
	bool m_bKeyboardDonePressed;
	KeyboardInitData m_initData;
	wstring m_capturedText;

protected:
	UIControl_Label m_EnterTextLabel;
	UIControl_TextInput m_KeyboardTextInput;
	UIControl_Button m_ButtonSpace, m_ButtonCursorLeft, m_ButtonCursorRight, m_ButtonCaps, m_ButtonDone, m_ButtonSymbols, m_ButtonBackspace;

	IggyName m_funcInitFunctionButtons;
	IggyName m_funcCursorRightButtonPressed, m_funcCursorLeftButtonPressed, m_funcCapsButtonPressed, m_funcBackspaceButtonPressed;
	IggyName m_funcSpaceButtonPressed, m_funcSymbolButtonPressed, m_funcDoneButtonPressed;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT(m_EnterTextLabel, "EnterTextLabel")
		UI_MAP_ELEMENT(m_KeyboardTextInput, "KeyboardTextInput")

		UI_MAP_ELEMENT(m_ButtonSpace, "Button_space")
		UI_MAP_ELEMENT(m_ButtonCursorLeft, "Button_CursorLeft")
		UI_MAP_ELEMENT(m_ButtonCursorRight, "Button_CursorRight")
		UI_MAP_ELEMENT(m_ButtonCaps, "Button_Caps")
		UI_MAP_ELEMENT(m_ButtonDone, "Button_Done")
		UI_MAP_ELEMENT(m_ButtonSymbols, "Button_symbols")
		UI_MAP_ELEMENT(m_ButtonBackspace, "Button_bspace")

		UI_MAP_NAME(m_funcInitFunctionButtons, L"InitFunctionButtons");

		UI_MAP_NAME(m_funcCursorRightButtonPressed, L"CursorRightButtonPressed");
		UI_MAP_NAME(m_funcCursorLeftButtonPressed, L"CursorLeftButtonPressed");
		UI_MAP_NAME(m_funcCapsButtonPressed, L"CapsButtonPressed");
		UI_MAP_NAME(m_funcBackspaceButtonPressed, L"BackspaceButtonPressed");
		UI_MAP_NAME(m_funcSpaceButtonPressed, L"SpaceButtonPressed");
		UI_MAP_NAME(m_funcSymbolButtonPressed, L"SymbolButtonPressed");
		UI_MAP_NAME(m_funcDoneButtonPressed, L"DoneButtonPressed");
	UI_END_MAP_ELEMENTS_AND_NAMES()

public:
	UIScene_Keyboard(int iPad, void *initData, UILayer *parentLayer);
	~UIScene_Keyboard();

	virtual void updateTooltips();

	virtual bool allowRepeat(int key);
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void handleTimerComplete(int id);

protected:
	void handlePress(F64 controlId, F64 childId);

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

	private:
		void KeyboardDonePressed();

	public:
		virtual EUIScene getSceneType() { return eUIScene_Keyboard;}

		// RENDERING
		virtual void render(S32 width, S32 height, C4JRender::eViewportType viewport);

		// Returns true if this scene handles input
		//virtual bool stealsFocus() { return false; }

		// Returns true if this scene has focus for the pad passed in
		//virtual bool hasFocus(int iPad) { return false; }
		// Returns true if this scene has focus for the pad passed in
#ifndef __PS3__
	virtual bool hasFocus(int iPad) { return bHasFocus; }
#endif

		// Returns true if lower scenes in this scenes layer, or in any layer below this scenes layers should be hidden
		virtual bool hidesLowerScenes() { return true; }
};