#include "stdafx.h"
#include "Button.h"
#include "OptionsScreen.h"
#include "SelectWorldScreen.h"
#include "JoinMultiplayerScreen.h"
#include "Tesselator.h"
#include "Textures.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\InputOutputStream.h"
#include "..\Minecraft.World\net.minecraft.locale.h"
#include "..\Minecraft.World\System.h"
#include "..\Minecraft.World\Random.h"
#include "TitleScreen.h"

Random *TitleScreen::random = new Random();

TitleScreen::TitleScreen()
{
	// 4J - added initialisers
	vo = 0;
	multiplayerButton = NULL;

    splash = L"missingno";
//    try {	// 4J - removed try/catch
    vector<wstring> splashes;

	/*
    BufferedReader *br = new BufferedReader(new InputStreamReader(InputStream::getResourceAsStream(L"res\\title\\splashes.txt"))); //, Charset.forName("UTF-8")
		
    wstring line = L"";
    while ( !(line = br->readLine()).empty() )
	{
        line = trimString( line );
        if (line.length() > 0)
		{
            splashes.push_back(line);
        }
    }
	
    br->close();
	delete br;
	*/

    splash = L""; //splashes.at(random->nextInt(splashes.size()));

//    } catch (Exception e) {
//    }
}

void TitleScreen::tick()
{
	//vo += 1.0f;
	//if( vo > 100.0f ) minecraft->setScreen(new SelectWorldScreen(this));		// 4J - temp testing
}

void TitleScreen::keyPressed(wchar_t eventCharacter, int eventKey)
{
}

void TitleScreen::init()
{
	/* 4J - Implemented in main menu instead
    Calendar c = Calendar.getInstance();
    c.setTime(new Date());

    if (c.get(Calendar.MONTH) + 1 == 11 && c.get(Calendar.DAY_OF_MONTH) == 9) {
        splash = "Happy birthday, ez!";
    } else if (c.get(Calendar.MONTH) + 1 == 6 && c.get(Calendar.DAY_OF_MONTH) == 1) {
        splash = "Happy birthday, Notch!";
    } else if (c.get(Calendar.MONTH) + 1 == 12 && c.get(Calendar.DAY_OF_MONTH) == 24) {
        splash = "Merry X-mas!";
    } else if (c.get(Calendar.MONTH) + 1 == 1 && c.get(Calendar.DAY_OF_MONTH) == 1) {
        splash = "Happy new year!";
    }
	*/

    Language *language = Language::getInstance();

    const int spacing = 24;
    const int topPos = height / 4 + spacing * 2;

    buttons.push_back(new Button(1, width / 2 - 100, topPos, language->getElement(L"menu.singleplayer")));
    buttons.push_back(multiplayerButton = new Button(2, width / 2 - 100, topPos + spacing * 1, language->getElement(L"menu.multiplayer")));
    buttons.push_back(new Button(3, width / 2 - 100, topPos + spacing * 2, language->getElement(L"menu.mods")));

    if (minecraft->appletMode)
	{
        buttons.push_back(new Button(0, width / 2 - 100, topPos + spacing * 3, language->getElement(L"menu.options")));
    }
	else
	{
        buttons.push_back(new Button(0, width / 2 - 100, topPos + spacing * 3 + 12, 98, 20, language->getElement(L"menu.options")));
        buttons.push_back(new Button(4, width / 2 + 2, topPos + spacing * 3 + 12, 98, 20, language->getElement(L"menu.quit")));
    }

    if (minecraft->user == NULL)
	{
        multiplayerButton->active = false;
    }

}

void TitleScreen::buttonClicked(Button *button)
{
    if (button->id == 0)
	{
        minecraft->setScreen(new OptionsScreen(this, minecraft->options));
    }
    if (button->id == 1)
	{
        minecraft->setScreen(new SelectWorldScreen(this));
    }
    if (button->id == 2)
	{
        minecraft->setScreen(new JoinMultiplayerScreen(this));
    }
    if (button->id == 3)
	{
 //       minecraft->setScreen(new TexturePackSelectScreen(this));		// 4J - TODO put back in
    }
    if (button->id == 4)
	{
        minecraft->stop();
    }
}

void TitleScreen::render(int xm, int ym, float a)
{
	// 4J Unused
#if 0
    renderBackground();
    Tesselator *t = Tesselator::getInstance();

    int logoWidth = 155 + 119;
    int logoX = width / 2 - logoWidth / 2;
    int logoY = 30;

    glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadTexture(L"/title/mclogo.png"));
    glColor4f(1, 1, 1, 1);
    blit(logoX + 0, logoY + 0, 0, 0, 155, 44);
    blit(logoX + 155, logoY + 0, 0, 45, 155, 44);
    t->color(0xffffff);
    glPushMatrix();
    glTranslatef((float)width / 2 + 90, 70, 0);

    glRotatef(-20, 0, 0, 1);
    float sss = 1.8f - Mth::abs(Mth::sin(System::currentTimeMillis() % 1000 / 1000.0f * PI * 2) * 0.1f);

    sss = sss * 100 / (font->width(splash) + 8 * 4);
    glScalef(sss, sss, sss);
    drawCenteredString(font, splash, 0, -8, 0xffff00);
    glPopMatrix();

    drawString(font, ClientConstants::VERSION_STRING, 2, 2, 0x505050);
    wstring msg = L"Copyright Mojang AB. Do not distribute.";
    drawString(font, msg, width - font->width(msg) - 2, height - 10, 0xffffff);

    Screen::render(xm, ym, a);
#endif
}
