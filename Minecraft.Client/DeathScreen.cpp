#include "stdafx.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "DeathScreen.h"
#include "Button.h"
#include "MultiplayerLocalPlayer.h"
#include "TitleScreen.h"

void DeathScreen::init()
{
    buttons.clear();
    buttons.push_back(new Button(1, width / 2 - 100, height / 4 + 24 * 3, L"Respawn"));
    buttons.push_back(new Button(2, width / 2 - 100, height / 4 + 24 * 4, L"Title menu"));

    if (minecraft->user == NULL)
	{
        buttons[1]->active = false;
    }
}

void DeathScreen::keyPressed(char eventCharacter, int eventKey)
{
}

void DeathScreen::buttonClicked(Button *button)
{
    if (button->id == 0)
	{
        //            minecraft.setScreen(new OptionsScreen(this, minecraft.options));
    }
    if (button->id == 1)
	{
        minecraft->player->respawn();
        minecraft->setScreen(NULL);
        //          minecraft.setScreen(new NewLevelScreen(this));
    }
    if (button->id == 2)
	{
        minecraft->setLevel(NULL);
        minecraft->setScreen(new TitleScreen());
    }
}

void DeathScreen::render(int xm, int ym, float a)
{
    fillGradient(0, 0, width, height, 0x60500000, 0xa0803030);

    glPushMatrix();
    glScalef(2, 2, 2);
    drawCenteredString(font, L"Game over!", width / 2 / 2, 60 / 2, 0xffffff);
    glPopMatrix();
    drawCenteredString(font, L"Score: &e" + std::to_wstring( minecraft->player->getScore() ), width / 2, 100, 0xffffff);

    Screen::render(xm, ym, a);

	// 4J - debug code - remove
	static int count = 0;
	if( count++ == 100 )
	{
		count = 0;
		buttonClicked(buttons[0]);
	}
}

bool DeathScreen::isPauseScreen()
{
	return false;
}