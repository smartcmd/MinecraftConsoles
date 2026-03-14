#include "stdafx.h"
#include "AchievementPopup.h"
#include "ItemRenderer.h"
#include "Font.h"
#include "Textures.h"
#include "Lighting.h"
#include "../Minecraft.World/System.h"
#include "../Minecraft.World/net.minecraft.locale.h"
#include "../Minecraft.World/net.minecraft.stats.h"
#include "../Minecraft.World/SharedConstants.h"

AchievementPopup::AchievementPopup(Minecraft *mc)
{
	// 4J - added initialisers
	width = 0;
	height = 0;
	ach = nullptr;
	startTime = 0;
	isHelper = false;

    this->mc = mc;
    ir = new ItemRenderer();
}

void AchievementPopup::popup(Achievement *ach)
{
    title = I18n::get(L"achievement.get");
    desc = ach->name;
    startTime = System::currentTimeMillis();
    this->ach = ach;
    isHelper = false;
}

void AchievementPopup::permanent(Achievement *ach)
{
    title = ach->name;
    desc = ach->getDescription();

    startTime = System::currentTimeMillis() - 2500;
    this->ach = ach;
    isHelper = true;
}

void AchievementPopup::prepareWindow()
{
    glViewport(0, 0, mc->width, mc->height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    this->width = mc->width;
    this->height = mc->height;

    ScreenSizeCalculator ssc(mc->options, mc->width, mc->height);
    width = ssc.getWidth();
    height = ssc.getHeight();

    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, static_cast<float>(width), static_cast<float>(height), 0, 1000, 3000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -2000);

}

void AchievementPopup::render()
{
// 4J Unused
#if 0
    if (Minecraft::warezTime > 0)
	{
        glDisable(GL_DEPTH_TEST);
        glDepthMask(false);
        Lighting::turnOff();
        prepareWindow();

        wstring title = L"Minecraft " + SharedConstants::VERSION_STRING + L"   Unlicensed Copy :(";
        wstring msg1 = L"(Or logged in from another location)";
        wstring msg2 = L"Purchase at minecraft.net";

        mc->font->drawShadow(title, 2, 2 + 9 * 0, 0xffffff);
        mc->font->drawShadow(msg1, 2, 2 + 9 * 1, 0xffffff);
        mc->font->drawShadow(msg2, 2, 2 + 9 * 2, 0xffffff);

        glDepthMask(true);
        glEnable(GL_DEPTH_TEST);
    }
    if (ach == nullptr || startTime == 0) return;

    double time = (System::currentTimeMillis() - startTime) / 3000.0;
    if (isHelper)
	{
    }
	else if (!isHelper && (time < 0 || time > 1))
	{
        startTime = 0;
        return;
    }


    prepareWindow();
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);

    double yo = time * 2;
    if (yo > 1) yo = 2 - yo;
    yo = yo * 4;
    yo = 1 - yo;
    if (yo < 0) yo = 0;
    yo = yo * yo;
    yo = yo * yo;

    int xx = width - 160;
    int yy = 0 - (int) (yo * 36);
    int tex = mc->textures->loadTexture(L"/achievement/bg.png");
    glColor4f(1, 1, 1, 1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDisable(GL_LIGHTING);

    blit(xx, yy, 96, 202, 160, 32);

    if (isHelper)
	{
        mc->font->drawWordWrap(desc, xx + 30, yy + 7, 120, 0xffffffff);
    }
	else
	{
        mc->font->draw(title, xx + 30, yy + 7, 0xffffff00);
        mc->font->draw(desc, xx + 30, yy + 18, 0xffffffff);
    }

    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    Lighting::turnOn();
    glPopMatrix();
    glDisable(GL_LIGHTING);
    glEnable(GL_RESCALE_NORMAL);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHTING);
    ir->renderGuiItem(mc->font, mc->textures, ach->icon, xx + 8, yy + 8);
    glDisable(GL_LIGHTING);

    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
#endif
}