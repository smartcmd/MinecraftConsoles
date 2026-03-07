#include "stdafx.h"
#include "ConfirmScreen.h"
#include "SmallButton.h"
#include "../Minecraft.World/net.minecraft.locale.h"

ConfirmScreen::ConfirmScreen(Screen *parent, const wstring& title1, const wstring& title2, int id)
{
    this->parent = parent;
    this->title1 = title1;
    this->title2 = title2;
    this->id = id;

    Language *language = Language::getInstance();
    yesButton = language->getElement(L"gui.yes");
    noButton = language->getElement(L"gui.no");
}

ConfirmScreen::ConfirmScreen(Screen *parent, const wstring& title1, const wstring& title2, const wstring& yesButton, const wstring& noButton, int id)
{
    this->parent = parent;
    this->title1 = title1;
    this->title2 = title2;
    this->yesButton = yesButton;
    this->noButton = noButton;
    this->id = id;
}

void ConfirmScreen::init()
{
    buttons.push_back(new SmallButton(0, width / 2 - 155 + 0 % 2 * 160, height / 6 + 24 * 4, yesButton));
    buttons.push_back(new SmallButton(1, width / 2 - 155 + 1 % 2 * 160, height / 6 + 24 * 4, noButton));
}

void ConfirmScreen::buttonClicked(Button *button)
{
	parent->confirmResult(button->id == 0, id);
}

void ConfirmScreen::render(int xm, int ym, float a)
{
    renderBackground();

    drawCenteredString(font, title1, width / 2, 70, 0xffffff);
    drawCenteredString(font, title2, width / 2, 90, 0xffffff);

    Screen::render(xm, ym, a);

	// 4J - debug code - remove
	static int count = 0;
	if( count++ == 100 )
	{
		count = 0;
		buttonClicked(buttons[0]);
	}
}