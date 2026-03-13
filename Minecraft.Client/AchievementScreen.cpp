#include "stdafx.h"
#include "AchievementScreen.h"
#include "SmallButton.h"
#include "Options.h"
#include "KeyMapping.h"
#include "Font.h"
#include "Lighting.h"
#include "Textures.h"
#include "StatsCounter.h"
#include "ItemRenderer.h"
#include "../Minecraft.World/System.h"
#include "../Minecraft.World/net.minecraft.locale.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/JavaMath.h"



AchievementScreen::AchievementScreen(StatsCounter *statsCounter)
{
	// 4J - added initialisers
    imageWidth = 256;
    imageHeight = 202;
    xLastScroll = 0;
    yLastScroll = 0;
	scrolling = 0;

	// 4J - TODO - investigate - these were static final ints before, but based on members of Achievements which
	// aren't final Or actually initialised
	xMin = Achievements::xMin * ACHIEVEMENT_COORD_SCALE - BIGMAP_WIDTH / 2;
	yMin = Achievements::yMin * ACHIEVEMENT_COORD_SCALE - BIGMAP_WIDTH / 2;
	xMax = Achievements::xMax * ACHIEVEMENT_COORD_SCALE - BIGMAP_HEIGHT / 2;
	yMax = Achievements::yMax * ACHIEVEMENT_COORD_SCALE - BIGMAP_HEIGHT / 2;

    this->statsCounter = statsCounter;
    int wBigMap = 141;
    int hBigMap = 141;

    xScrollO = xScrollP = xScrollTarget = Achievements::openInventory->x * ACHIEVEMENT_COORD_SCALE - wBigMap / 2 - 12;
    yScrollO = yScrollP = yScrollTarget = Achievements::openInventory->y * ACHIEVEMENT_COORD_SCALE - hBigMap / 2;

}

void AchievementScreen::init()
{
    buttons.clear();
//        buttons.add(new SmallButton(0, width / 2 - 80 - 24, height / 2 + 74, 110, 20, I18n.get("gui.achievements")));
    buttons.push_back(new SmallButton(1, width / 2 + 24, height / 2 + 74, 80, 20, I18n::get(L"gui.done")));

}

void AchievementScreen::buttonClicked(Button *button)
{
    if (button->id == 1)
	{
        minecraft->setScreen(nullptr);
//        minecraft->grabMouse();	// 4J removed
    }
    Screen::buttonClicked(button);
}

void AchievementScreen::keyPressed(char eventCharacter, int eventKey)
{
    if (eventKey == minecraft->options->keyBuild->key)
	{
        minecraft->setScreen(nullptr);
//        minecraft->grabMouse();	// 4J removed
    }
	else
	{
        Screen::keyPressed(eventCharacter, eventKey);
    }
}

void AchievementScreen::render(int mouseX, int mouseY, float a)
{
    if (Mouse::isButtonDown(0))
	{
        int xo = (width - imageWidth) / 2;
        int yo = (height - imageHeight) / 2;

        int xBigMap = xo + 8;
        int yBigMap = yo + 17;

        if (scrolling == 0 || scrolling == 1)
		{
            if (mouseX >= xBigMap && mouseX < xBigMap + BIGMAP_WIDTH && mouseY >= yBigMap && mouseY < yBigMap + BIGMAP_HEIGHT)
			{
                if (scrolling == 0)
				{
                    scrolling = 1;
                }
				else
				{
                    xScrollP -= mouseX - xLastScroll;
                    yScrollP -= mouseY - yLastScroll;
                    xScrollTarget = xScrollO = xScrollP;
                    yScrollTarget = yScrollO = yScrollP;
                }
                xLastScroll = mouseX;
                yLastScroll = mouseY;
            }
        }

        if (xScrollTarget < xMin) xScrollTarget = xMin;
        if (yScrollTarget < yMin) yScrollTarget = yMin;
        if (xScrollTarget >= xMax) xScrollTarget = xMax - 1;
        if (yScrollTarget >= yMax) yScrollTarget = yMax - 1;
    }
	else
	{
        scrolling = 0;
    }

    renderBackground();

    renderBg(mouseX, mouseY, a);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    renderLabels();

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

}

void AchievementScreen::tick()
{
    xScrollO = xScrollP;
    yScrollO = yScrollP;

    double xd = (xScrollTarget - xScrollP);
    double yd = (yScrollTarget - yScrollP);
    if (xd * xd + yd * yd < 4)
	{
        xScrollP += xd;
        yScrollP += yd;
    }
	else
	{
        xScrollP += xd * 0.85;
        yScrollP += yd * 0.85;
    }
}

void AchievementScreen::renderLabels()
{
    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;
    font->draw(L"Achievements", xo + 15, yo + 5, 0x404040);

//        font.draw(xScrollP + ", " + yScrollP, xo + 5, yo + 5 + BIGMAP_HEIGHT + 18, 0x404040);
//        font.drawWordWrap("Ride a pig off a cliff.", xo + 5, yo + 5 + BIGMAP_HEIGHT + 16, BIGMAP_WIDTH, 0x404040);

}

void AchievementScreen::renderBg(int xm, int ym, float a)
{
	// 4J Unused
#if 0
    int xScroll = Mth::floor(xScrollO + (xScrollP - xScrollO) * a);
    int yScroll = Mth::floor(yScrollO + (yScrollP - yScrollO) * a);

    if (xScroll < xMin) xScroll = xMin;
    if (yScroll < yMin) yScroll = yMin;
    if (xScroll >= xMax) xScroll = xMax - 1;
    if (yScroll >= yMax) yScroll = yMax - 1;


    int terrainTex = minecraft->textures->loadTexture(L"/terrain.png");
    int tex = minecraft->textures->loadTexture(L"/achievement/bg.png");

    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;

    int xBigMap = xo + BIGMAP_X;
    int yBigMap = yo + BIGMAP_Y;

    blitOffset = 0;
//        glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glPushMatrix();
    glTranslatef(0, 0, -200);

    {
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_RESCALE_NORMAL);
        glEnable(GL_COLOR_MATERIAL);

        minecraft->textures->bind(terrainTex);

        int leftTile = (xScroll + EDGE_VALUE_X) >> 4;
        int topTile = (yScroll + EDGE_VALUE_Y) >> 4;
        int xMod = (xScroll + EDGE_VALUE_X) % 16;
        int yMod = (yScroll + EDGE_VALUE_Y) % 16;

        const int rockLevel = (Achievements::ACHIEVEMENT_HEIGHT_POSITION * 4) / 10;
        const int coalLevel = (Achievements::ACHIEVEMENT_HEIGHT_POSITION * 7) / 10;
        const int ironLevel = (Achievements::ACHIEVEMENT_HEIGHT_POSITION * 9) / 10;
        const int diamondLevel = (Achievements::ACHIEVEMENT_HEIGHT_POSITION * 19) / 10;
        const int bedrockLevel = (Achievements::ACHIEVEMENT_HEIGHT_POSITION * 31) / 10;

        Random *random = new Random();

        for (int tileY = 0; (tileY * 16) - yMod < BIGMAP_HEIGHT; tileY++)
		{

            float amount = .6f - (float) (topTile + tileY) / (float) (Achievements::ACHIEVEMENT_HEIGHT_POSITION * 2 + 1) * .3f;
            glColor4f(amount, amount, amount, 1);

            for (int tileX = 0; (tileX * 16) - xMod < BIGMAP_WIDTH; tileX++)
			{

                random->setSeed(1234 + leftTile + tileX);
                random->nextInt();
                int heightValue = random->nextInt(1 + topTile + tileY) + (topTile + tileY) / 2;
                int tileType = Tile::sand->tex;

                if (heightValue > bedrockLevel || (topTile + tileY) == MAX_BG_TILE_Y)
				{
                    tileType = Tile::unbreakable->tex;
                }
				else if (heightValue == diamondLevel)
				{
                    if (random->nextInt(2) == 0)
					{
                        tileType = Tile::diamondOre->tex;
                    }
					else
					{
                        tileType = Tile::redStoneOre->tex;
                    }
                }
				else if (heightValue == ironLevel)
				{
                    tileType = Tile::ironOre->tex;
                }
				else if (heightValue == coalLevel)
				{
                    tileType = Tile::coalOre->tex;
                }
				else if (heightValue > rockLevel)
				{
                    tileType = Tile::rock->tex;
				}
				else if (heightValue > 0)
				{
                    tileType = Tile::dirt->tex;
                }

                this->blit(xBigMap + tileX * 16 - xMod, yBigMap + tileY * 16 - yMod, (tileType % 16) << 4, (tileType >> 4) << 4, 16, 16);
            }
        }

    }
    glEnable(GL_DEPTH_TEST);


    glDepthFunc(GL_LEQUAL);

    glDisable(GL_TEXTURE_2D);

	for ( Achievement *ach : *Achievements::achievements )
	{
        if ( ach == nullptr || ach->requires == nullptr) continue;

        int x1 = ach->x * ACHIEVEMENT_COORD_SCALE - (int) xScroll + 11 + xBigMap;
        int y1 = ach->y * ACHIEVEMENT_COORD_SCALE - (int) yScroll + 11 + yBigMap;

        int x2 = ach->requires->x * ACHIEVEMENT_COORD_SCALE - (int) xScroll + 11 + xBigMap;
        int y2 = ach->requires->y * ACHIEVEMENT_COORD_SCALE - (int) yScroll + 11 + yBigMap;

        int color = 0;

        bool taken = statsCounter->hasTaken(ach);
        bool canTake = statsCounter->canTake(ach);

        int alph = (int) (sin(System::currentTimeMillis() % 600 / 600.0 * PI * 2) > 0.6 ? 255 : 130);
        if (taken) color = 0xff707070;
        else if (canTake) color = 0x00ff00 + (alph << 24);
        else color = 0xff000000;

        hLine(x1, x2, y1, color);
        vLine(x2, y1, y2, color);
    }

    Achievement *hoveredAchievement = nullptr;
    ItemRenderer *ir = new ItemRenderer();

    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    Lighting::turnOn();
    glPopMatrix();
    glDisable(GL_LIGHTING);
    glEnable(GL_RESCALE_NORMAL);
    glEnable(GL_COLOR_MATERIAL);

	for ( Achievement *ach : *Achievements::achievements )
	{
        int x = ach->x * ACHIEVEMENT_COORD_SCALE - (int) xScroll;
        int y = ach->y * ACHIEVEMENT_COORD_SCALE - (int) yScroll;

        if (x >= -24 && y >= -24 && x <= BIGMAP_WIDTH && y <= BIGMAP_HEIGHT)
		{

            if (statsCounter->hasTaken(ach))
			{
                float br = 1.0f;
                glColor4f(br, br, br, 1);
            }
			else if (statsCounter->canTake(ach))
			{
                float br = (sin(System::currentTimeMillis() % 600 / 600.0 * PI * 2) < 0.6 ? 0.6f : 0.8f);
                glColor4f(br, br, br, 1);
            }
			else
			{
                float br = 0.3f;
                glColor4f(br, br, br, 1);
            }

            minecraft->textures->bind(tex);
            int xx = xBigMap + x;
            int yy = yBigMap + y;
            if (ach->isGolden())
			{
                this->blit(xx - 2, yy - 2, 26, 202, 26, 26);
            }
			else
			{
                this->blit(xx - 2, yy - 2, 0, 202, 26, 26);
            }

            if (!statsCounter->canTake(ach))
			{
                float br = 0.1f;
                glColor4f(br, br, br, 1);
                ir->setColor = false;
            }
            glEnable(GL_LIGHTING);
            glEnable(GL_CULL_FACE);
            ir->renderGuiItem(minecraft->font, minecraft->textures, ach->icon, xx + 3, yy + 3);
            glDisable(GL_LIGHTING);
            if (!statsCounter->canTake(ach))
			{
                ir->setColor = true;
            }
            glColor4f(1, 1, 1, 1);


            if (xm >= xBigMap && ym >= yBigMap && xm < xBigMap + BIGMAP_WIDTH && ym < yBigMap + BIGMAP_HEIGHT && xm >= xx && xm <= xx + 22 && ym >= yy && ym <= yy + 22) {
                hoveredAchievement = ach;
            }
        }
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glColor4f(1, 1, 1, 1);
    minecraft->textures->bind(tex);
    blit(xo, yo, 0, 0, imageWidth, imageHeight);


    glPopMatrix();

    blitOffset = 0;
    glDepthFunc(GL_LEQUAL);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    Screen::render(xm, ym, a);

    if (hoveredAchievement != nullptr)
	{
        Achievement *ach = hoveredAchievement;
        wstring name = ach->name;
        wstring descr = ach->getDescription();

        int x = xm + 12;
        int y = ym - 4;

        if (statsCounter->canTake(ach))
		{
            int width = Math::_max(font->width(name), 120);
            int height = font->wordWrapHeight(descr, width);
            if (statsCounter->hasTaken(ach))
			{
                height += 12;
            }
            fillGradient(x - 3, y - 3, x + width + 3, y + height + 3 + 12, 0xc0000000, 0xc0000000);

            font->drawWordWrap(descr, x, y + 12, width, 0xffa0a0a0);
            if (statsCounter->hasTaken(ach))
			{
                font->drawShadow(I18n::get(L"achievement.taken"), x, y + height + 4, 0xff9090ff);
            }
        }
		else
		{
            int width = Math::_max(font->width(name), 120);
            wstring msg = I18n::get(L"achievement.requires", ach->requires->name);
            int height = font->wordWrapHeight(msg, width);
            fillGradient(x - 3, y - 3, x + width + 3, y + height + 12 + 3, 0xc0000000, 0xc0000000);
            font->drawWordWrap(msg, x, y + 12, width, 0xff705050);
        }
        font->drawShadow(name, x, y, statsCounter->canTake(ach) ? ach->isGolden() ? 0xffffff80 : 0xffffffff : ach->isGolden() ? 0xff808040 : 0xff808080);


    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    Lighting::turnOff();
#endif
}

bool AchievementScreen::isPauseScreen()
{
	return true;
}