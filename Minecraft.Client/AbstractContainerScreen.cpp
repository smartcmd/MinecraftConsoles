#include "stdafx.h"
#include "AbstractContainerScreen.h"
#include "ItemRenderer.h"
#include "MultiplayerLocalPlayer.h"
#include "Lighting.h"
#include "GameMode.h"
#include "KeyMapping.h"
#include "Options.h"
#include "..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\Minecraft.World\net.minecraft.locale.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"

ItemRenderer *AbstractContainerScreen::itemRenderer = new ItemRenderer();

AbstractContainerScreen::AbstractContainerScreen(AbstractContainerMenu *menu)
{
	// 4J - added initialisers
	imageWidth = 176;
	imageHeight = 166;

	this->menu = menu;
}

void AbstractContainerScreen::init()
{
    Screen::init();
    minecraft->player->containerMenu = menu;
// 	leftPos = (width - imageWidth) / 2;
// 	topPos = (height - imageHeight) / 2;

}

void AbstractContainerScreen::render(int xm, int ym, float a)
{
	// 4J Stu - Not used
#if 0
    renderBackground();
    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;

    renderBg(a);

    glPushMatrix();
    glRotatef(120, 1, 0, 0);
    Lighting::turnOn();
    glPopMatrix();

    glPushMatrix();
    glTranslatef((float)xo, (float)yo, 0);

    glColor4f(1, 1, 1, 1);
    glEnable(GL_RESCALE_NORMAL);

    Slot *hoveredSlot = nullptr;

	for ( Slot *slot : *menu->slots )
	{
        renderSlot(slot);

        if (isHovering(slot, xm, ym))
		{
            hoveredSlot = slot;

            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);

            int x = slot->x;
            int y = slot->y;
            fillGradient(x, y, x + 16, y + 16, 0x80ffffff, 0x80ffffff);
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);
        }
    }

    shared_ptr<Inventory> inventory = minecraft->player->inventory;
    if (inventory->getCarried() != nullptr)
	{
        glTranslatef(0, 0, 32);
        // Slot old = carriedSlot;
        // carriedSlot = null;
        itemRenderer->renderGuiItem(font, minecraft->textures, inventory->getCarried(), xm - xo - 8, ym - yo - 8);
        itemRenderer->renderGuiItemDecorations(font, minecraft->textures, inventory->getCarried(), xm - xo - 8, ym - yo - 8);
        // carriedSlot = old;
    }
    glDisable(GL_RESCALE_NORMAL);
    Lighting::turnOff();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    renderLabels();

    if (inventory->getCarried() == nullptr && hoveredSlot != nullptr && hoveredSlot->hasItem())
	{

        wstring elementName = trimString(Language::getInstance()->getElementName(hoveredSlot->getItem()->getDescriptionId()));

        if (elementName.length() > 0)
		{
            int x = xm - xo + 12;
            int y = ym - yo - 12;
            int width = font->width(elementName);
            fillGradient(x - 3, y - 3, x + width + 3, y + 8 + 3, 0xc0000000, 0xc0000000);

            font->drawShadow(elementName, x, y, 0xffffffff);
        }

    }

    glPopMatrix();

    Screen::render(xm, ym, a);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
#endif
}

void AbstractContainerScreen::renderLabels()
{
}

void AbstractContainerScreen::renderSlot(Slot *slot)
{
	// 4J Unused
#if 0
    int x = slot->x;
    int y = slot->y;
    shared_ptr<ItemInstance> item = slot->getItem();

    if (item == nullptr)
	{
        int icon = slot->getNoItemIcon();
        if (icon >= 0)
		{
            glDisable(GL_LIGHTING);
            minecraft->textures->bind(minecraft->textures->loadTexture(TN_GUI_ITEMS));//L"/gui/items.png"));
            blit(x, y, icon % 16 * 16, icon / 16 * 16, 16, 16);
            glEnable(GL_LIGHTING);
            return;
        }
    }

    itemRenderer->renderGuiItem(font, minecraft->textures, item, x, y);
    itemRenderer->renderGuiItemDecorations(font, minecraft->textures, item, x, y);
#endif
}

Slot *AbstractContainerScreen::findSlot(int x, int y)
{
    for (Slot* slot : menu->slots )
	{
        if (isHovering(slot, x, y)) return slot;
    }
    return nullptr;
}

bool AbstractContainerScreen::isHovering(Slot *slot, int xm, int ym)
{
    int xo = (width - imageWidth) / 2;
    int yo = (height - imageHeight) / 2;
    xm -= xo;
    ym -= yo;

    return xm >= slot->x - 1 && xm < slot->x + 16 + 1 && ym >= slot->y - 1 && ym < slot->y + 16 + 1;

}

void AbstractContainerScreen::mouseClicked(int x, int y, int buttonNum)
{
    Screen::mouseClicked(x, y, buttonNum);
    if (buttonNum == 0 || buttonNum == 1)
	{
        Slot *slot = findSlot(x, y);

        int xo = (width - imageWidth) / 2;
        int yo = (height - imageHeight) / 2;
        bool clickedOutside = (x < xo || y < yo || x >= xo + imageWidth || y >= yo + imageHeight);

        int slotId = -1;
        if (slot != nullptr) slotId = slot->index;

        if (clickedOutside)
		{
            slotId = AbstractContainerMenu::CLICKED_OUTSIDE;
        }

        if (slotId != -1)
		{
            bool quickKey = slotId != AbstractContainerMenu::CLICKED_OUTSIDE && (Keyboard::isKeyDown(Keyboard::KEY_LSHIFT) || Keyboard::isKeyDown(Keyboard::KEY_RSHIFT));
            minecraft->gameMode->handleInventoryMouseClick(menu->containerId, slotId, buttonNum, quickKey, minecraft->player);
        }
    }

}

void AbstractContainerScreen::mouseReleased(int x, int y, int buttonNum)
{
    if (buttonNum == 0)
	{
    }
}

void AbstractContainerScreen::keyPressed(wchar_t eventCharacter, int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE || eventKey == minecraft->options->keyBuild->key)
	{
        minecraft->player->closeContainer();
    }
}

void AbstractContainerScreen::removed()
{
    if (minecraft->player == nullptr) return;
}

void AbstractContainerScreen::slotsChanged(shared_ptr<Container> container)
{
}

bool AbstractContainerScreen::isPauseScreen()
{
	return false;
}

void AbstractContainerScreen::tick()
{
    Screen::tick();
    if (!minecraft->player->isAlive() || minecraft->player->removed) minecraft->player->closeContainer();

}