#pragma once
#include "GuiComponent.h"
class Button;
class GuiParticles;
class Minecraft;
using namespace std;

class Screen : public GuiComponent
{
protected:
	Minecraft *minecraft;
public:
	int width;
    int height;
protected:
	vector<Button *> buttons;
public:
	bool passEvents;
protected:
	Font *font;
public:
	GuiParticles *particles;

	Screen();		// 4J added
    virtual void render(int xm, int ym, float a);
protected:
	virtual void keyPressed(wchar_t eventCharacter, int eventKey);
public:
	static wstring getClipboard();
    static void setClipboard(const wstring& str);
private:
	Button *clickedButton;

protected:
	virtual void mouseClicked(int x, int y, int buttonNum);
    virtual void mouseReleased(int x, int y, int buttonNum);
    virtual void buttonClicked(Button *button);
public:
    virtual void init(Minecraft *minecraft, int width, int height);
    virtual void setSize(int width, int height);
    virtual void init();
	virtual void handlePasteRequest() {}
	virtual void handleHistoryUp() {}
	virtual void handleHistoryDown() {}
    virtual void updateEvents();
    virtual void mouseEvent();
    virtual void keyboardEvent();
    virtual void tick();
    virtual void removed();
    virtual void renderBackground();
    virtual void renderBackground(int vo);
    virtual void renderDirtBackground(int vo);
    virtual bool isPauseScreen();
    virtual void confirmResult(bool result, int id);
    virtual void tabPressed();
};

