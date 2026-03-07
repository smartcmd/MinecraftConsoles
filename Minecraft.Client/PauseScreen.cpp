#include "stdafx.h"
#include "PauseScreen.h"
#include "Button.h"
#include "StatsCounter.h"
#include "OptionsScreen.h"
#include "TitleScreen.h"
#include "MultiPlayerLevel.h"
#include "../Minecraft.World/net.minecraft.locale.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.stats.h"
#include "../Minecraft.Client/LocalPlayer.h"

PauseScreen::PauseScreen()
{
	saveStep = 0;
	visibleTime = 0;
}

void PauseScreen::init()
{
    saveStep = 0;
    buttons.clear();
    int yo = -16;
    buttons.push_back(new Button(1, width / 2 - 100, height / 4 + 24 * 5 + yo, L"Save and quit to title"));
    if (minecraft->isClientSide())
	{
        buttons[0]->msg = L"Disconnect";
    }


    buttons.push_back(new Button(4, width / 2 - 100, height / 4 + 24 * 1 + yo, L"LBack to game"));
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 24 * 4 + yo, L"LOptions..."));

    buttons.push_back(new Button(5, width / 2 - 100, height / 4 + 24 * 2 + yo, 98, 20, I18n::get(L"gui.achievements")));
    buttons.push_back(new Button(6, width / 2 + 2, height / 4 + 24 * 2 + yo, 98, 20, I18n::get(L"gui.stats")));
    /*
        * if (minecraft->serverConnection!=null) { buttons.get(1).active =
        * false; buttons.get(2).active = false; buttons.get(3).active = false;
        * }
        */

}

void PauseScreen::buttonClicked(Button button)
{
    if (button.id == 0)
	{
        minecraft->setScreen(new OptionsScreen(this, minecraft->options));
    }
    if (button.id == 1)
	{
        if (minecraft->isClientSide())
		{
            minecraft->level->disconnect();
        }

        minecraft->setLevel(NULL);
        minecraft->setScreen(new TitleScreen());
    }
    if (button.id == 4)
	{
        minecraft->setScreen(NULL);
 //       minecraft->grabMouse();		// 4J - removed
    }

    if (button.id == 5)
	{
//        minecraft->setScreen(new AchievementScreen(minecraft->stats));	// 4J TODO - put back
    }
    if (button.id == 6)
	{
//        minecraft->setScreen(new StatsScreen(this, minecraft->stats));	// 4J TODO - put back
    }
}

void PauseScreen::tick()
{
    Screen::tick();
    visibleTime++;
}

void PauseScreen::render(int xm, int ym, float a)
{
    renderBackground();

    bool isSaving = false; //!minecraft->level->pauseSave(saveStep++);
    if (isSaving || visibleTime < 20)
	{
        float col = ((visibleTime % 10) + a) / 10.0f;
        col = Mth::sin(col * PI * 2) * 0.2f + 0.8f;
        int br = (int) (255 * col);

        drawString(font, L"Saving level..", 8, height - 16, br << 16 | br << 8 | br);
    }

    drawCenteredString(font, L"Game menu", width / 2, 40, 0xffffff);

    Screen::render(xm, ym, a);
}