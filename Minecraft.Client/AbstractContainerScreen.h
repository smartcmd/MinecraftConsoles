#pragma once
#include "Screen.h"
class ItemRenderer;
class AbstractContainerMenu;
class Slot;
class Container;

class AbstractContainerScreen : public Screen
{
private:
	static ItemRenderer *itemRenderer;
protected:
	int imageWidth;
    int imageHeight;
	//int leftPos, topPos;
public:
	AbstractContainerMenu *menu;

    AbstractContainerScreen(AbstractContainerMenu *menu);
    virtual void init();
    virtual void render(int xm, int ym, float a);
protected:
	virtual void renderLabels();
    virtual void renderBg(float a) = 0;
private:
	virtual void renderSlot(Slot *slot);
    virtual Slot *findSlot(int x, int y);
    virtual bool isHovering(Slot *slot, int xm, int ym);
protected:
	virtual void mouseClicked(int x, int y, int buttonNum);
    virtual void mouseReleased(int x, int y, int buttonNum);
    virtual void keyPressed(wchar_t eventCharacter, int eventKey);
public:
	virtual void removed();
    virtual void slotsChanged(shared_ptr<Container> container);
    virtual bool isPauseScreen();
    virtual void tick();
};