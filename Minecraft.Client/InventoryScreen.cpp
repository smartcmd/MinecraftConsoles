#include "stdafx.h"
#include "InventoryScreen.h"
#include "MultiPlayerLocalPlayer.h"
#include "Font.h"
#include "EntityRenderDispatcher.h"
#include "Lighting.h"
#include "Textures.h"
#include "Button.h"
#include "AchievementScreen.h"
#include "StatsScreen.h"
#include "../Minecraft.World/net.minecraft.stats.h"

InventoryScreen::InventoryScreen(shared_ptr<Player> player) : AbstractContainerScreen(player->inventoryMenu)
{
	xMouse = yMouse = 0.0f; // 4J added

    this->passEvents = true;
    player->awardStat(GenericStats::openInventory(), GenericStats::param_noArgs());
}

void InventoryScreen::init()
{
	buttons.clear();
}

void InventoryScreen::renderLabels()
{
	font->draw(L"Crafting", 84 + 2, 8 * 2, 0x404040);
}

void InventoryScreen::render(int xm, int ym, float a)
{
    AbstractContainerScreen::render(xm, ym, a);
    this->xMouse = (float)xm;
    this->yMouse = (float)ym;
}

void InventoryScreen::renderBg(float a)
{
	// 4J Unused
#if 0
    int tex = minecraft->textures->loadTexture(L"/gui/inventory.png");
    glColor4f(1, 1, 1, 1);
    minecraft->textures->bind(tex);
    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;
    this->blit(xo, yo, 0, 0, imageWidth, imageHeight);

    glEnable(GL_RESCALE_NORMAL);
    glEnable(GL_COLOR_MATERIAL);

    glPushMatrix();
    glTranslatef((float)xo + 51, (float)yo + 75, 50);
    float ss = 30;
    glScalef(-ss, ss, ss);
    glRotatef(180, 0, 0, 1);

    float oybr = minecraft->player->yBodyRot;
    float oyr = minecraft->player->yRot;
    float oxr = minecraft->player->xRot;

    float xd = (xo + 51) - xMouse;
    float yd = (yo + 75 - 50) - yMouse;

    glRotatef(45 + 90, 0, 1, 0);
    Lighting::turnOn();
    glRotatef(-45 - 90, 0, 1, 0);

    glRotatef(-(float) atan(yd / 40.0f) * 20, 1, 0, 0);

    minecraft->player->yBodyRot = (float) atan(xd / 40.0f) * 20;
    minecraft->player->yRot = (float) atan(xd / 40.0f) * 40;
    minecraft->player->xRot = -(float) atan(yd / 40.0f) * 20;
    glTranslatef(0, minecraft->player->heightOffset, 0);
    EntityRenderDispatcher::instance->playerRotY = 180;
    EntityRenderDispatcher::instance->render(minecraft->player, 0, 0, 0, 0, 1);
    minecraft->player->yBodyRot = oybr;
    minecraft->player->yRot = oyr;
    minecraft->player->xRot = oxr;
    glPopMatrix();
    Lighting::turnOff();
    glDisable(GL_RESCALE_NORMAL);
#endif
}

void InventoryScreen::buttonClicked(Button *button)
{
    if (button->id == 0)
	{
		minecraft->setScreen(new AchievementScreen(minecraft->stats[minecraft->player->GetXboxPad()]));
    }
    if (button->id == 1)
	{
        minecraft->setScreen(new StatsScreen(this, minecraft->stats[minecraft->player->GetXboxPad()]));
    }
}
