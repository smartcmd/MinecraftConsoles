#include "stdafx.h"
#include "UI.h"
#include "UIControl_EnchantmentBook.h"
#include "..\..\Minecraft.h"
#include "..\..\TileEntityRenderDispatcher.h"
#include "..\..\EnchantTableRenderer.h"
#include "..\..\Lighting.h"
#include "..\..\BookModel.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"

UIControl_EnchantmentBook::UIControl_EnchantmentBook()
{
	UIControl::setControlType(UIControl::eEnchantmentBook);
	model = nullptr;
	last = nullptr;

	time = 0;
	flip = oFlip = flipT = flipA = 0.0f;
	open = oOpen = 0.0f;
}

void UIControl_EnchantmentBook::render(IggyCustomDrawCallbackRegion *region)
{
	glPushMatrix();
	float width = region->x1 - region->x0;
	float height = region->y1 - region->y0;

	// Revert the scale from the setup
	float ssX = width/m_width;
	float ssY = height/m_height;
	glScalef(ssX, ssY,1.0f);

	glTranslatef(m_width/2, m_height/2, 50.0f);

	// Add a uniform scale
	glScalef(-57/ssX, 57/ssX, 360.0f);

	glRotatef(45 + 90, 0, 1, 0);
	Lighting::turnOn();
	glRotatef(-45 - 90, 0, 1, 0);

    //float sss = 4;

    //glTranslatef(0, 3.3f, -16);
    //glScalef(sss, sss, sss);

	Minecraft *pMinecraft = Minecraft::GetInstance();
	int tex = pMinecraft->textures->loadTexture(TN_ITEM_BOOK); // 4J was L"/1_2_2/item/book.png"
    pMinecraft->textures->bind(tex);

    glRotatef(20, 1, 0, 0);

	float a = 1;
    float o = oOpen + (open - oOpen) * a;
    glTranslatef((1 - o) * 0.2f, (1 - o) * 0.1f, (1 - o) * 0.25f);
    glRotatef(-(1 - o) * 90 - 90, 0, 1, 0);
    glRotatef(180, 1, 0, 0);

    float ff1 = oFlip + (flip - oFlip) * a + 0.25f;
    float ff2 = oFlip + (flip - oFlip) * a + 0.75f;
    ff1 = (ff1 - floor(ff1)) * 1.6f - 0.3f;
    ff2 = (ff2 - floor(ff2)) * 1.6f - 0.3f;

    if (ff1 < 0) ff1 = 0;
    if (ff2 < 0) ff2 = 0;
    if (ff1 > 1) ff1 = 1;
    if (ff2 > 1) ff2 = 1;

	glEnable(GL_CULL_FACE);

	if(model == nullptr)
	{
		// Share the model the the EnchantTableRenderer

		EnchantTableRenderer *etr = static_cast<EnchantTableRenderer *>(TileEntityRenderDispatcher::instance->getRenderer(eTYPE_ENCHANTMENTTABLEENTITY));
		if(etr != nullptr)
		{
			model = etr->bookModel;
		}
		else
		{
			model = new BookModel();
		}
	}

    model->render(nullptr, 0, ff1, ff2, o, 0, 1 / 16.0f,true);
	glDisable(GL_CULL_FACE);

    glPopMatrix();
	Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);

	tickBook();
}

void UIControl_EnchantmentBook::tickBook()
{
	UIScene_EnchantingMenu *m_containerScene = static_cast<UIScene_EnchantingMenu *>(m_parentScene);
	EnchantmentMenu *menu = m_containerScene->getMenu();
	shared_ptr<ItemInstance> current = menu->getSlot(0)->getItem();
	if (!ItemInstance::matches(current, last))
	{
        last = current;

		do
		{
			flipT += random.nextInt(4) - random.nextInt(4);
		} while (flip <= flipT + 1 && flip >= flipT - 1);
    }

    time++;
    oFlip = flip;
    oOpen = open;

	bool shouldBeOpen = false;
    for (int i = 0; i < 3; i++)
	{
        if (menu->costs[i] != 0)
		{
            shouldBeOpen = true;
        }
    } 

    if (shouldBeOpen) open += 0.2f;
    else open -= 0.2f;
    if (open < 0) open = 0;
    if (open > 1) open = 1;


    float diff = (flipT - flip) * 0.4f;
    float max = 0.2f;
    if (diff < -max) diff = -max;
    if (diff > +max) diff = +max;
    flipA += (diff - flipA) * 0.9f;

    flip = flip + flipA;
}
