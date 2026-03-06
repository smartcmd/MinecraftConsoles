#include "stdafx.h"
#include "CreateWorldScreen.h"
#include "EditBox.h"
#include "Button.h"
#include "SurvivalMode.h"
#include "..\Minecraft.World\net.minecraft.locale.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\Minecraft.World\SharedConstants.h"
#include "..\Minecraft.World\Random.h"

CreateWorldScreen::CreateWorldScreen(Screen *lastScreen)
{
	done = false;	// 4J added
	this->lastScreen = lastScreen;
}

void CreateWorldScreen::tick()
{
    nameEdit->tick();
    seedEdit->tick();

	// 4J - debug code - to be removed
	static int count = 0;
	if(count++ == 100 ) buttonClicked(buttons[0]);
}

void CreateWorldScreen::init()
{
    Language *language = Language::getInstance();

    Keyboard::enableRepeatEvents(true);
    buttons.clear();
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 24 * 4 + 12, language->getElement(L"selectWorld.create")));
    buttons.push_back(new Button(1, width / 2 - 100, height / 4 + 24 * 5 + 12, language->getElement(L"gui.cancel")));

    nameEdit = new EditBox(this, font, width / 2 - 100, 60, 200, 20, language->getElement(L"testWorld")); // 4J - test - should be L"selectWorld.newWorld"
    nameEdit->inFocus = true;
    nameEdit->setMaxLength(32);

    seedEdit = new EditBox(this, font, width / 2 - 100, 116, 200, 20, L"");

    updateResultFolder();
}

void CreateWorldScreen::updateResultFolder()
{
    resultFolder = trimString(nameEdit->getValue());

	for( int i = 0; i < SharedConstants::ILLEGAL_FILE_CHARACTERS_LENGTH; i++ )
	{
		size_t pos;
		while( (pos = resultFolder.find(SharedConstants::ILLEGAL_FILE_CHARACTERS[i])) != wstring::npos)
		{
			resultFolder[pos] = L'_';
		}
	}

	if (resultFolder.length()==0)
	{
        resultFolder = L"World";
    }
    resultFolder = CreateWorldScreen::findAvailableFolderName(minecraft->getLevelSource(), resultFolder);

}

wstring CreateWorldScreen::findAvailableFolderName(LevelStorageSource *levelSource, const wstring& folder)
{
	wstring folder2 = folder;	// 4J - copy input as it is const

#if 0
    while (levelSource->getDataTagFor(folder2) != NULL)
	{
        folder2 = folder2 + L"-";
    }
#endif
    return folder2;
}

void CreateWorldScreen::removed()
{
	Keyboard::enableRepeatEvents(false);
}

void CreateWorldScreen::buttonClicked(Button *button)
{
    if (!button->active) return;
    if (button->id == 1)
	{
        minecraft->setScreen(lastScreen);
    }
	else if (button->id == 0)
	{
        // note: code copied from SelectWorldScreen
        minecraft->setScreen(NULL);
        if (done) return;
        done = true;

        int64_t seedValue = (new Random())->nextLong();
        wstring seedString = seedEdit->getValue();

		if (seedString.length() != 0)
		{
            // try to convert it to a long first
//            try {	// 4J - removed try/catch
            int64_t value = _fromString<int64_t>(seedString);
            if (value != 0)
			{
                seedValue = value;
            }
 //           } catch (NumberFormatException e) {
 //               // not a number, fetch hash value
 //               seedValue = seedString.hashCode();
 //           }
        }

// 4J Stu - This screen is not used, so removing this to stop the build failing
#if 0
        minecraft->gameMode = new SurvivalMode(minecraft);
        minecraft->selectLevel(resultFolder, nameEdit->getValue(), seedValue);
        minecraft->setScreen(NULL);
#endif
    }

}

void CreateWorldScreen::keyPressed(wchar_t ch, int eventKey)
{
    if (nameEdit->inFocus) nameEdit->keyPressed(ch, eventKey);
    else seedEdit->keyPressed(ch, eventKey);

    if (ch == 13)
	{
        buttonClicked(buttons[0]);
    }
    buttons[0]->active = nameEdit->getValue().length() > 0;

    updateResultFolder();
}

void CreateWorldScreen::mouseClicked(int x, int y, int buttonNum)
{
    Screen::mouseClicked(x, y, buttonNum);

    nameEdit->mouseClicked(x, y, buttonNum);
    seedEdit->mouseClicked(x, y, buttonNum);
}

void CreateWorldScreen::render(int xm, int ym, float a)
{
	Language *language = Language::getInstance();

	// fill(0, 0, width, height, 0x40000000);
	renderBackground();

	drawCenteredString(font, language->getElement(L"selectWorld.create"), width / 2, height / 4 - 60 + 20, 0xffffff);
	drawString(font, language->getElement(L"selectWorld.enterName"), width / 2 - 100, 47, 0xa0a0a0);
	drawString(font, language->getElement(L"selectWorld.resultFolder") + L" " + resultFolder, width / 2 - 100, 85, 0xa0a0a0);

	drawString(font, language->getElement(L"selectWorld.enterSeed"), width / 2 - 100, 104, 0xa0a0a0);
	drawString(font, language->getElement(L"selectWorld.seedInfo"), width / 2 - 100, 140, 0xa0a0a0);

	nameEdit->render();
	seedEdit->render();

	Screen::render(xm, ym, a);

}

void CreateWorldScreen::tabPressed()
{
    if (nameEdit->inFocus)
	{
        nameEdit->focus(false);
        seedEdit->focus(true);
    }
	else
	{
        nameEdit->focus(true);
        seedEdit->focus(false);
    }
}