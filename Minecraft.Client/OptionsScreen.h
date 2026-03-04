#pragma once
#include "Screen.h"
class Options;
using namespace std;

class OptionsScreen : public Screen
{
private:
	static const int CONTROLS_BUTTON_ID = 100;
    static const int VIDEO_BUTTON_ID = 101;
    static const int REMAP_BUTTON_ID = 102;
    Screen *lastScreen;
protected:
	wstring title;
private:
	Options *options;
public:
	OptionsScreen(Screen *lastScreen, Options *options);
    virtual void init();
protected:
	virtual void buttonClicked(Button *button);
public:
	virtual void render(int xm, int ym, float a);
};