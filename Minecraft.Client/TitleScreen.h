#pragma once
#include "Screen.h"
class Random;
class Button;
using namespace std;

class TitleScreen : public Screen
{
private:
	static Random *random;

    float vo;

    wstring splash;
    Button *multiplayerButton;

public:
	TitleScreen();
    virtual void tick();
protected:
	virtual void keyPressed(wchar_t eventCharacter, int eventKey);
public:
	virtual void init();
protected:
	virtual void buttonClicked(Button *button);
protected:
	virtual void renderPanoramas(int xm, int ym, float a);
	virtual void renderGaussianBlur(float a);
	virtual void renderBackground(int xm, int ym, float a);
public:
	virtual void render(int xm, int ym, float a);
};