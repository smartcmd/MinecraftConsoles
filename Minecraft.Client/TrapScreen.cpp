#include "stdafx.h"
#include "TrapScreen.h"
#include "LocalPlayer.h"
#include "Textures.h"
#include "../Minecraft.World/net.minecraft.world.inventory.h"
#include "../Minecraft.World/DispenserTileEntity.h"
#include "../Minecraft.World/net.minecraft.world.h"

TrapScreen::TrapScreen(shared_ptr<Inventory> inventory, shared_ptr<DispenserTileEntity> trap) : AbstractContainerScreen(new TrapMenu(inventory, trap))
{

}

void TrapScreen::renderLabels()
{
    font->draw(L"Dispenser", 16 + 4 + 40, 2 + 2 + 2, 0x404040);
    font->draw(L"Inventory", 8, imageHeight - 96 + 2, 0x404040);
}

void TrapScreen::renderBg(float a)
{
	// 4J Unused
#if 0
	int tex = minecraft->textures->loadTexture(L"/gui/trap.png");
	glColor4f(1, 1, 1, 1);
	minecraft->textures->bind(tex);
	int xo = (width - imageWidth) / 2;
	int yo = (height - imageHeight) / 2;
	this->blit(xo, yo, 0, 0, imageWidth, imageHeight);
#endif
}