#include "stdafx.h"
#include "SlideButton.h"

SlideButton::SlideButton(int id, int x, int y, const Options::Option *option, const wstring& msg, float value) : Button(id, x, y, 150, 20, msg)
{
	this->sliding = false;	//4J added
	this->option = option;
	this->value = value;
}

int SlideButton::getYImage(bool hovered)
{
	return 0;
}

void SlideButton::renderBg(Minecraft *minecraft, int xm, int ym)
{
    if (!visible) return;
    if (sliding)
	{
        value = (xm - (x + 4)) / static_cast<float>(w - 8);
        if (value < 0) value = 0;
        if (value > 1) value = 1;
        minecraft->options->set(option, value);
        msg = minecraft->options->getMessage(option);
    }
    glColor4f(1, 1, 1, 1);
    blit(x + static_cast<int>(value * (w - 8)), y, 0, 46 + 1 * 20, 4, 20);
    blit(x + static_cast<int>(value * (w - 8)) + 4, y, 196, 46 + 1 * 20, 4, 20);
}

bool SlideButton::clicked(Minecraft *minecraft, int mx, int my)
{
    if (Button::clicked(minecraft, mx, my))
	{
        value = (mx - (x + 4)) / static_cast<float>(w - 8);
        if (value < 0) value = 0;
        if (value > 1) value = 1;
        minecraft->options->set(option, value);
        msg = minecraft->options->getMessage(option);
        sliding = true;
        return true;
    }

    return false;
}

void SlideButton::released(int mx, int my)
{
	sliding = false;
}