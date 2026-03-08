#include "stdafx.h"
#include "FurnaceScreen.h"
#include "Textures.h"
#include "LocalPlayer.h"
#include "Font.h"
#include "../Minecraft.World/net.minecraft.world.inventory.h"
#include "../Minecraft.World/FurnaceTileEntity.h"

FurnaceScreen::FurnaceScreen(shared_ptr<Inventory> inventory, shared_ptr<FurnaceTileEntity> furnace) : AbstractContainerScreen(new FurnaceMenu(inventory, furnace))
{
	this->furnace = furnace;
}

void FurnaceScreen::renderLabels()
{
	font->draw(L"Furnace", 16 + 4 + 40, 2 + 2 + 2, 0x404040);
	font->draw(L"Inventory", 8, imageHeight - 96 + 2, 0x404040);
}

void FurnaceScreen::renderBg(float a)
{
	// 4J Unused
#if 0
	int tex = minecraft->textures->loadTexture(L"/gui/furnace.png");
	glColor4f(1, 1, 1, 1);
	minecraft->textures->bind(tex);
	int xo = (width - imageWidth) / 2;
	int yo = (height - imageHeight) / 2;
	this->blit(xo, yo, 0, 0, imageWidth, imageHeight);
	if (furnace->isLit())
	{
		int p = furnace->getLitProgress(12);
		this->blit(xo + 56, yo + 36 + 12 - p, 176, 12 - p, 14, p + 2);
	}

	int p = furnace->getBurnProgress(24);
	this->blit(xo + 79, yo + 34, 176, 14, p + 1, 16);
#endif
}