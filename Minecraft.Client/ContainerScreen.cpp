#include "stdafx.h"
#include "ContainerScreen.h"
#include "Textures.h"
#include "../Minecraft.World/net.minecraft.world.inventory.h"

ContainerScreen::ContainerScreen(shared_ptr<Container> inventory, shared_ptr<Container> container) : AbstractContainerScreen(new ContainerMenu(inventory, container))
{
	this->inventory = inventory;
	this->container = container;
	this->passEvents = false;

	int defaultHeight = 222;
	int noRowHeight = defaultHeight - 6 * 18;
	containerRows = container->getContainerSize() / 9;

	imageHeight = noRowHeight + containerRows * 18;

}

void ContainerScreen::renderLabels()
{
#if 0
	font->draw(container->getName(), 8, 2 + 2 + 2, 0x404040);
	font->draw(inventory->getName(), 8, imageHeight - 96 + 2, 0x404040);
#endif
}

void ContainerScreen::renderBg(float a)
{
	// 4J Unused
#if 0
	int tex = minecraft->textures->loadTexture(L"/gui/container.png");
	glColor4f(1, 1, 1, 1);
	minecraft->textures->bind(tex);
	int xo = (width - imageWidth) / 2;
	int yo = (height - imageHeight) / 2;
	this->blit(xo, yo, 0, 0, imageWidth, containerRows * 18 + 17);
	this->blit(xo, yo + containerRows * 18 + 17, 0, 222 - 96, imageWidth, 96);
#endif
}