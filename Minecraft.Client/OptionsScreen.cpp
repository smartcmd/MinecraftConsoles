#include "stdafx.h"
#include "OptionsScreen.h"
#include "SmallButton.h"
#include "SlideButton.h"
#include "Options.h"
#include "ControlsScreen.h"
#include "VideoSettingsScreen.h"
#include "..\Minecraft.World\net.minecraft.locale.h"

OptionsScreen::OptionsScreen(Screen *lastScreen, Options *options)
{
	title = L"Options";	// 4J added

    this->lastScreen = lastScreen;
    this->options = options;
}

void OptionsScreen::init()
{
    Language *language = Language::getInstance();
    this->title = language->getElement(L"options.title");

    int position = 0;

	// 4J - this was as static array but moving it into the function to remove any issues with static initialisation order
	const Options::Option *items[5] = {Options::Option::MUSIC, Options::Option::SOUND, Options::Option::INVERT_MOUSE, Options::Option::SENSITIVITY, Options::Option::DIFFICULTY};
    for (int i = 0; i < 5; i++)
	{
		const Options::Option *item = items[i];
        if (!item->isProgress())
		{
            buttons.push_back(new SmallButton(item->getId(), width / 2 - 155 + position % 2 * 160, height / 6 + 24 * (position >> 1), item, options->getMessage(item)));
        }
		else
		{
            buttons.push_back(new SlideButton(item->getId(), width / 2 - 155 + position % 2 * 160, height / 6 + 24 * (position >> 1), item, options->getMessage(item), options->getProgressValue(item)));
        }
        position++;
    }

    buttons.push_back(new Button(VIDEO_BUTTON_ID, width / 2 - 100, height / 6 + 24 * 4 + 12, language->getElement(L"options.video")));
    buttons.push_back(new Button(CONTROLS_BUTTON_ID, width / 2 - 100, height / 6 + 24 * 5 + 12, language->getElement(L"options.controls")));
    buttons.push_back(new Button(200, width / 2 - 100, height / 6 + 24 * 7, language->getElement(L"gui.done")));

}

void OptionsScreen::buttonClicked(Button *button)
{
    if (!button->active) return;
    if (button->id < 100 && (dynamic_cast<SmallButton *>(button) != NULL))
	{
        options->toggle(((SmallButton *) button)->getOption(), 1);
        button->msg = options->getMessage(Options::Option::getItem(button->id));
    }
    if (button->id == VIDEO_BUTTON_ID)
	{
        minecraft->options->save();
        minecraft->setScreen(new VideoSettingsScreen(this, options));
    }
    if (button->id == CONTROLS_BUTTON_ID)
	{
        minecraft->options->save();
        minecraft->setScreen(new ControlsScreen(this, options));
    }
    if (button->id == 200)
	{
        minecraft->options->save();
        minecraft->setScreen(lastScreen);
    }
}

void OptionsScreen::render(int xm, int ym, float a)
{
    renderBackground();
    drawCenteredString(font, title, width / 2, 20, 0xffffff);
    Screen::render(xm, ym, a);
}