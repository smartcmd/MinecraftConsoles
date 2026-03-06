#include "stdafx.h"
#include "SelectWorldScreen.h"
#include "Button.h"
#include "ConfirmScreen.h"
#include "CreateWorldScreen.h"
#include "RenameWorldScreen.h"
#include "DemoMode.h"
#include "..\Minecraft.World\net.minecraft.locale.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"


SelectWorldScreen::SelectWorldScreen(Screen *lastScreen)
{
	// 4J - added initialisers
    title = L"Select world";
    done = false;
	selectedWorld = 0;
	worldSelectionList = NULL;
	isDeleting = false;
	deleteButton = NULL;
    selectButton = NULL;
    renameButton = NULL;

	this->lastScreen = lastScreen;
}

void SelectWorldScreen::init()
{
    Language *language = Language::getInstance();
    title = language->getElement(L"selectWorld.title");

    worldLang = language->getElement(L"selectWorld.world");
    conversionLang = language->getElement(L"selectWorld.conversion");
    loadLevelList();

    worldSelectionList = new WorldSelectionList(this);
    worldSelectionList->init(&buttons, BUTTON_UP_ID, BUTTON_DOWN_ID);

    postInit();
}

void SelectWorldScreen::loadLevelList()
{
	LevelStorageSource *levelSource = minecraft->getLevelSource();
	levelList = levelSource->getLevelList();
//	Collections.sort(levelList);	// 4J - TODO - get sort functor etc.
	selectedWorld = -1;
}

wstring SelectWorldScreen::getWorldId(int id)
{
	return levelList->at(id)->getLevelId();
}

wstring SelectWorldScreen::getWorldName(int id)
{
    wstring levelName = levelList->at(id)->getLevelName();

	if ( levelName.length() == 0 )
	{
        Language *language = Language::getInstance();
        levelName = language->getElement(L"selectWorld.world") + L" " + std::to_wstring(id + 1);
    }

    return levelName;
}

void SelectWorldScreen::postInit()
{
    Language *language = Language::getInstance();

    buttons.push_back(selectButton = new Button(BUTTON_SELECT_ID, width / 2 - 154, height - 52, 150, 20, language->getElement(L"selectWorld.select")));
    buttons.push_back(deleteButton = new Button(BUTTON_RENAME_ID, width / 2 - 154, height - 28, 70, 20, language->getElement(L"selectWorld.rename")));
    buttons.push_back(renameButton = new Button(BUTTON_DELETE_ID, width / 2 - 74, height - 28, 70, 20, language->getElement(L"selectWorld.delete")));
    buttons.push_back(new Button(BUTTON_CREATE_ID, width / 2 + 4, height - 52, 150, 20, language->getElement(L"selectWorld.create")));
    buttons.push_back(new Button(BUTTON_CANCEL_ID, width / 2 + 4, height - 28, 150, 20, language->getElement(L"gui.cancel")));

    selectButton->active = false;
    deleteButton->active = false;
    renameButton->active = false;

}

void SelectWorldScreen::buttonClicked(Button *button)
{
    if (!button->active) return;
    if (button->id == BUTTON_DELETE_ID)
	{
        wstring worldName = getWorldName(selectedWorld);
        if (worldName != L"")
		{
            isDeleting = true;

            Language *language = Language::getInstance();
            wstring title = language->getElement(L"selectWorld.deleteQuestion");
            wstring warning = L"'" + worldName + L"' " + language->getElement(L"selectWorld.deleteWarning");
            wstring yes = language->getElement(L"selectWorld.deleteButton");
            wstring no = language->getElement(L"gui.cancel");

            ConfirmScreen *confirmScreen = new ConfirmScreen(this, title, warning, yes, no, selectedWorld);
            minecraft->setScreen(confirmScreen);
        }
    }
	else if (button->id == BUTTON_SELECT_ID)
	{
        worldSelected(selectedWorld);
    }
	else if (button->id == BUTTON_CREATE_ID)
	{
        if (ProfileManager.IsFullVersion())
		{
            minecraft->setScreen(new CreateWorldScreen(this));
        }
		else
		{
            // create demo world
            minecraft->setScreen(NULL);
            if (done) return;
            done = true;
// 4J Stu - Not used, so commenting to stop the build failing
#if 0
            minecraft->gameMode = new DemoMode(minecraft);
            minecraft->selectLevel(CreateWorldScreen::findAvailableFolderName(minecraft->getLevelSource(), L"Demo"), L"Demo World", 0L);
            minecraft->setScreen(NULL);
#endif
        }
    }
	else if (button->id == BUTTON_RENAME_ID)
	{
        minecraft->setScreen(new RenameWorldScreen(this, getWorldId(selectedWorld)));
    }
	else if (button->id == BUTTON_CANCEL_ID)
	{
        minecraft->setScreen(lastScreen);
    }
	else
	{
        worldSelectionList->buttonClicked(button);
    }
}

void SelectWorldScreen::worldSelected(int id)
{
    minecraft->setScreen(NULL);
    if (done) return;
    done = true;
    minecraft->gameMode = NULL; //new SurvivalMode(minecraft);

    wstring worldFolderName = getWorldId(id);
    if (worldFolderName == L"")	// 4J - was NULL comparison
	{
        worldFolderName = L"World" + std::to_wstring(id);
    }
// 4J Stu - Not used, so commenting to stop the build failing
#if 0
    minecraft->selectLevel(worldFolderName, getWorldName(id), 0);
    minecraft->setScreen(NULL);
#endif
}

void SelectWorldScreen::confirmResult(bool result, int id)
{
    if (isDeleting)
	{
        isDeleting = false;
        if (result)
		{
            LevelStorageSource *levelSource = minecraft->getLevelSource();
            levelSource->clearAll();
            levelSource->deleteLevel(getWorldId(id));

            loadLevelList();
        }
        minecraft->setScreen(this);
    }
}

void SelectWorldScreen::render(int xm, int ym, float a)
{
    // fill(0, 0, width, height, 0x40000000);
    worldSelectionList->render(xm, ym, a);

    drawCenteredString(font, title, width / 2, 20, 0xffffff);

    Screen::render(xm, ym, a);

	// 4J - debug code - remove
	static int count = 0;
	static bool forceCreateLevel = false;
	if( count++ >= 100 )
	{
		if( !forceCreateLevel && levelList->size() > 0 )
		{
			// 4J Stu - For some obscures reason the "delete" button is called "renameButton" and vice versa.
			//if( levelList->size() > 2 && deleteButton->active )
			//{
			//	this->selectedWorld = 2;
			//	count = 0;
			//	buttonClicked(deleteButton);
			//}
			//else
			if( levelList->size() > 1 && renameButton->active )
			{
				this->selectedWorld = 1;
				count = 0;
				buttonClicked(renameButton);
			}
			else
				if( selectButton->active == true )
			{
				this->selectedWorld = 0;
				buttonClicked(selectButton);
				//this->worldSelected( 0 );
			}
			else
			{
				selectButton->active = true;
				deleteButton->active = true;
				renameButton->active = true;
				count = 0;
			}
		}
		else
		{
			minecraft->setScreen(new CreateWorldScreen(this));
		}
	}
}

SelectWorldScreen::WorldSelectionList::WorldSelectionList(SelectWorldScreen *sws) : ScrolledSelectionList(sws->minecraft, sws->width, sws->height, 32, sws->height - 64, 36)
{
	parent = sws;
}

int SelectWorldScreen::WorldSelectionList::getNumberOfItems()
{
	return (int)this->parent->levelList->size();
}

void SelectWorldScreen::WorldSelectionList::selectItem(int item, bool doubleClick)
{
    parent->selectedWorld = item;
    bool active = (this->parent->selectedWorld >= 0 && this->parent->selectedWorld < getNumberOfItems());
    parent->selectButton->active = active;
    parent->deleteButton->active = active;
    parent->renameButton->active = active;

    if (doubleClick && active)
	{
        parent->worldSelected(item);
    }
}

bool SelectWorldScreen::WorldSelectionList::isSelectedItem(int item)
{
	return item == parent->selectedWorld;
}

int SelectWorldScreen::WorldSelectionList::getMaxPosition()
{
	return (int)parent->levelList->size() * 36;
}

void SelectWorldScreen::WorldSelectionList::renderBackground()
{
	parent->renderBackground();	// 4J - was SelectWorldScreen.this.renderBackground();
}

void SelectWorldScreen::WorldSelectionList::renderItem(int i, int x, int y, int h, Tesselator *t)
{
    LevelSummary *levelSummary = parent->levelList->at(i);

    wstring name = levelSummary->getLevelName();
    if (name.length()==0)
	{
        name = parent->worldLang + L" " + std::to_wstring(i + 1);
    }

    wstring id = levelSummary->getLevelId();

	ULARGE_INTEGER rawtime;
	rawtime.QuadPart = levelSummary->getLastPlayed() * 10000; // Convert it from milliseconds back to FileTime

	FILETIME timeasfiletime;
	timeasfiletime.dwHighDateTime = rawtime.HighPart;
	timeasfiletime.dwLowDateTime = rawtime.LowPart;

	SYSTEMTIME time;
	FileTimeToSystemTime( &timeasfiletime, &time );

	wchar_t buffer[20];
	// 4J Stu - Currently shows years as 4 digits, where java only showed 2
	swprintf(buffer,20,L"%d/%d/%d %d:%02d",time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute); // 4J - TODO Localise this
    id = id + L" (" + buffer;

    __int64 size = levelSummary->getSizeOnDisk();
    id = id + L", " + std::to_wstring(static_cast<float>(size / 1024 * 100 / 1024 / 100.0f)) + L" MB)";
    wstring info;

    if (levelSummary->isRequiresConversion())
	{
        info = parent->conversionLang + L" " + info;
    }

    parent->drawString(parent->font, name, x + 2, y + 1, 0xffffff);
    parent->drawString(parent->font, id, x + 2, y + 12, 0x808080);
    parent->drawString(parent->font, info, x + 2, y + 12 + 10, 0x808080);

}
