#include "stdafx.h"
#include "CraftingScreen.h"
#include "Textures.h"
#include "MultiplayerLocalPlayer.h"
#include "..\Minecraft.World\net.minecraft.world.inventory.h"

CraftingScreen::CraftingScreen(shared_ptr<Inventory> inventory, Level *level, int x, int y, int z) : AbstractContainerScreen(new CraftingMenu(inventory, level, x, y, z))
{
}

void CraftingScreen::removed()
{
    AbstractContainerScreen::removed();
    menu->removed(dynamic_pointer_cast<Player>(minecraft->player));
}

void CraftingScreen::renderLabels()
{
    font->draw(L"Crafting", 8 + 16 + 4, 2 + 2 + 2, 0x404040);
    font->draw(L"Inventory", 8, imageHeight - 96 + 2, 0x404040);
}

void CraftingScreen::renderBg(float a)
{
	// 4J Unused
#if 0
    int tex = minecraft->textures->loadTexture(L"/gui/crafting.png");
    glColor4f(1, 1, 1, 1);
    minecraft->textures->bind(tex);
    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;
    this->blit(xo, yo, 0, 0, imageWidth, imageHeight);
#endif
}