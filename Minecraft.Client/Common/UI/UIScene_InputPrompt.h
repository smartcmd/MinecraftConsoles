#pragma once

#include "UIScene.h"

// Callback: void callback(void *param, int detectedValue, bool isKeyboard)
// For keyboard: detectedValue = Keyboard::KEY_* constant (or negative for mouse buttons: -100+btn)
// For controller: detectedValue = _360_JOY_BUTTON_* constant
// detectedValue = -1 means cancelled
typedef void (*InputPromptCallback)(void *param, int detectedValue);

struct InputPromptInfo
{
	bool detectKeyboard;       // true = keyboard/mouse mode, false = controller mode
	InputPromptCallback callback;
	void *callbackParam;
};

class UIScene_InputPrompt : public UIScene
{
private:
	enum EControls
	{
		eControl_Cancel = 0,
	};

	UIControl_Button m_buttonCancel;
	UIControl_Button m_buttonUnused[3]; // MessageBox has 4 buttons, we only use 1
	UIControl_Label m_labelTitle, m_labelContent;
	IggyName m_funcInit, m_funcAutoResize;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_buttonCancel, "Button3")
		UI_MAP_ELEMENT( m_buttonUnused[0], "Button0")
		UI_MAP_ELEMENT( m_buttonUnused[1], "Button1")
		UI_MAP_ELEMENT( m_buttonUnused[2], "Button2")
		UI_MAP_ELEMENT( m_labelTitle, "Title")
		UI_MAP_ELEMENT( m_labelContent, "Content")
		UI_MAP_NAME( m_funcInit, L"Init")
		UI_MAP_NAME( m_funcAutoResize, L"AutoResize")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	bool m_detectKeyboard;
	bool m_waitForRelease; // Wait for all buttons to be released before detecting
	InputPromptCallback m_callback;
	void *m_callbackParam;
	bool m_completed;

	int pollKeyboard();
	int pollController();

public:
	UIScene_InputPrompt(int iPad, void *initData, UILayer *parentLayer);
	~UIScene_InputPrompt();

	virtual EUIScene getSceneType() { return eUIScene_InputPrompt; }
	virtual bool hidesLowerScenes() { return false; }

	virtual void tick();

protected:
	virtual wstring getMoviePath();
	virtual void updateTooltips();

public:
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

protected:
	void handlePress(F64 controlId, F64 childId);
};
