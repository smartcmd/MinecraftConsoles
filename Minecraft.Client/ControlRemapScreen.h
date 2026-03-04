#pragma once
#include "Screen.h"
using namespace std;
class Options;

class ControlRemapScreen : public Screen
{
public:
	enum InputMode
	{
		MODE_KEYBOARD = 0,
		MODE_CONTROLLER = 1
	};

private:
	Screen *lastScreen;
	Options *options;
	InputMode currentMode;
	int selectedKey;

	static const int DONE_BUTTON_ID = 200;
	static const int MODE_TOGGLE_BUTTON_ID = 201;
	static const int RESET_DEFAULTS_BUTTON_ID = 202;

	static const int BUTTON_WIDTH = 75;
	static const int ROW_WIDTH = 165;

protected:
	wstring title;

public:
	ControlRemapScreen(Screen *lastScreen, Options *options);

private:
	int getLeftScreenPosition();
	void rebuildButtons();

	// Controller button selection helpers
	static const int CONTROLLER_BUTTON_COUNT = 16;
	static unsigned int getControllerButtonByIndex(int index);
	static wstring getControllerButtonNameByIndex(int index);
	int controllerActionSelected; // which controller action is being remapped
	int controllerButtonCursor;   // cursor position in button picker

public:
	virtual void init();

protected:
	virtual void buttonClicked(Button *button);
	virtual void keyPressed(wchar_t eventCharacter, int eventKey);

public:
	virtual void render(int xm, int ym, float a);
};
