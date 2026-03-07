#include "stdafx.h"
#include "RenameWorldScreen.h"
#include "EditBox.h"
#include "Button.h"
#include "../Minecraft.World/net.minecraft.locale.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.level.storage.h"

RenameWorldScreen::RenameWorldScreen(Screen *lastScreen, const wstring& levelId)
{
	nameEdit = NULL;
    this->lastScreen = lastScreen;
    this->levelId = levelId;
}

void RenameWorldScreen::tick()
{
	nameEdit->tick();
}

void RenameWorldScreen::init() 
{
	// 4J Stu - Removed this as we don't need the screen. Changed to how we pass save data around stopped this compiling
#if 0
    Language *language = Language::getInstance();

    Keyboard::enableRepeatEvents(true);
    buttons.clear();
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 24 * 4 + 12, language->getElement(L"selectWorld.renameButton")));
    buttons.push_back(new Button(1, width / 2 - 100, height / 4 + 24 * 5 + 12, language->getElement(L"gui.cancel")));

    LevelStorageSource *levelSource = minecraft->getLevelSource();
    LevelData *levelData = levelSource->getDataTagFor(levelId);
    wstring currentName = levelData->getLevelName();

    nameEdit = new EditBox(this, font, width / 2 - 100, 60, 200, 20, currentName);
    nameEdit->inFocus = true;
    nameEdit->setMaxLength(32);
#endif
}

void RenameWorldScreen::removed()
{
	Keyboard::enableRepeatEvents(false);
}

void RenameWorldScreen::buttonClicked(Button *button)
{
    if (!button->active) return;
    if (button->id == 1)
	{
        minecraft->setScreen(lastScreen);
    }
	else if (button->id == 0)
	{

        LevelStorageSource *levelSource = minecraft->getLevelSource();
        levelSource->renameLevel(levelId, trimString(nameEdit->getValue()));

        minecraft->setScreen(lastScreen);
    }
}

void RenameWorldScreen::keyPressed(wchar_t ch, int eventKey)
{
    nameEdit->keyPressed(ch, eventKey);
    buttons[0]->active = trimString(nameEdit->getValue()).length() > 0;

    if (ch == 13)
	{
        buttonClicked(buttons[0]);
    }
}

void RenameWorldScreen::mouseClicked(int x, int y, int buttonNum)
{
    Screen::mouseClicked(x, y, buttonNum);

    nameEdit->mouseClicked(x, y, buttonNum);
}

void RenameWorldScreen::render(int xm, int ym, float a)
{
    Language *language = Language::getInstance();

    // fill(0, 0, width, height, 0x40000000);
    renderBackground();

    drawCenteredString(font, language->getElement(L"selectWorld.renameTitle"), width / 2, height / 4 - 60 + 20, 0xffffff);
    drawString(font, language->getElement(L"selectWorld.enterName"), width / 2 - 100, 47, 0xa0a0a0);

    nameEdit->render();

    Screen::render(xm, ym, a);

}