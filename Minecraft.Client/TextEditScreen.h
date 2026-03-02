#pragma once
#include "Screen.h"
class SignTileEntity;
using namespace std;

class TextEditScreen : public Screen
{
protected:
	wstring title;
private:
	shared_ptr<SignTileEntity> sign;
    int frame;
    int line;

public:
	TextEditScreen(shared_ptr<SignTileEntity> sign);
    virtual void init();
    virtual void removed();
    virtual void tick();
protected:
	virtual void buttonClicked(Button *button);
private:
	static const wstring allowedChars;
protected:
	virtual void keyPressed(wchar_t ch, int eventKey);
public:
	virtual void render(int xm, int ym, float a);
};