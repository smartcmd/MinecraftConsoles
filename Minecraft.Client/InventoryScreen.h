#pragma once
#include "AbstractContainerScreen.h"
class Player;
class Button;

class InventoryScreen : public AbstractContainerScreen
{
public:
	InventoryScreen(shared_ptr<Player> player);
    virtual void init();
protected:
	virtual void renderLabels();
private:
	float xMouse, yMouse;
public:
	virtual void render(int xm, int ym, float a);
protected:
	virtual void renderBg(float a);
	virtual void buttonClicked(Button *button);
};