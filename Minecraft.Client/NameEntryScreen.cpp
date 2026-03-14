#include "stdafx.h"
#include "NameEntryScreen.h"
#include "Button.h"
#include "../Minecraft.World/StringHelpers.h"

const wstring NameEntryScreen::allowedChars = L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ,.:-_'*!\"#%/()=+?[]{}<>";

NameEntryScreen::NameEntryScreen(Screen *lastScreen, const wstring& oldName, int slot)
{
	frame = 0; // 4J added

    this->lastScreen = lastScreen;
    this->slot = slot;
    this->name = oldName;
    if (name==L"-") name = L"";
}

void NameEntryScreen::init()
{
    buttons.clear();
    Keyboard::enableRepeatEvents(true);
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 24 * 5, L"Save"));
    buttons.push_back(new Button(1, width / 2 - 100, height / 4 + 24 * 6, L"Cancel"));
    buttons[0]->active = trimString(name).length() > 1;
}

void NameEntryScreen::removed()
{
	Keyboard::enableRepeatEvents(false);
}

void NameEntryScreen::tick()
{
	frame++;
}

void NameEntryScreen::buttonClicked(Button button)
{
    if (!button.active) return;

    if (button.id == 0 && trimString(name).length() > 1)
    {
        minecraft->saveSlot(slot, trimString(name));
        minecraft->setScreen(nullptr);
//        minecraft->grabMouse();	// 4J - removed
    }
    if (button.id == 1)
    {
        minecraft->setScreen(lastScreen);
    }
}

void NameEntryScreen::keyPressed(wchar_t ch, int eventKey)
{
    if (eventKey == Keyboard::KEY_BACK && name.length() > 0) name = name.substr(0, name.length() - 1);
    if (allowedChars.find(ch) != wstring::npos && name.length()<64)
    {
        name += ch;
    }
    buttons[0]->active = trimString(name).length() > 1;
}

void NameEntryScreen::render(int xm, int ym, float a)
{
    renderBackground();

    drawCenteredString(font, title, width / 2, 40, 0xffffff);

    int bx = width / 2 - 100;
    int by = height / 2 - 10;
    int bw = 200;
    int bh = 20;
    fill(bx - 1, by - 1, bx + bw + 1, by + bh + 1, 0xffa0a0a0);
    fill(bx, by, bx + bw, by + bh, 0xff000000);
    drawString(font, name + (frame / 6 % 2 == 0 ? L"_" : L""), bx + 4, by + (bh - 8) / 2, 0xe0e0e0);

    Screen::render(xm, ym, a);
}