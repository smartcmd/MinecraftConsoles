#include "stdafx.h"
#include "HumanoidMobRenderer.h"
#include "SkullTileRenderer.h"
#include "HumanoidModel.h"
#include "ModelPart.h"
#include "EntityRenderDispatcher.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "..\Minecraft.World\net.minecraft.h"

void HumanoidMobRenderer::_init(HumanoidModel *humanoidModel, float scale)
{
	this->humanoidModel = humanoidModel;
	this->_scale = scale;
	armorParts1 = NULL;
	armorParts2 = NULL;
}

HumanoidMobRenderer::HumanoidMobRenderer(HumanoidModel *humanoidModel, float shadow) : MobRenderer(humanoidModel, shadow)
{
	_init(humanoidModel, 1.0f);
}

HumanoidMobRenderer::HumanoidMobRenderer(HumanoidModel *humanoidModel, float shadow, float scale) : MobRenderer(humanoidModel, shadow)
{
	_init(humanoidModel, scale);

	createArmorParts();
}

void HumanoidMobRenderer::createArmorParts()
{
	armorParts1 = new HumanoidModel(1.0f);
	armorParts2 = new HumanoidModel(0.5f);
}

void HumanoidMobRenderer::additionalRendering(shared_ptr<Mob> mob, float a)
{
	float brightness = SharedConstants::TEXTURE_LIGHTING ? 1 : mob->getBrightness(a);
	glColor3f(brightness, brightness, brightness);
    shared_ptr<ItemInstance> item = mob->getCarriedItem();
	shared_ptr<ItemInstance> headGear = mob->getArmor(3);

	if (headGear != NULL)
	{
		// don't render the pumpkin of skulls for the skins with that disabled
		// 4J-PB - need to disable rendering armour/skulls/pumpkins for some special skins (Daleks)

		if((mob->getAnimOverrideBitmask()&(1<<HumanoidModel::eAnim_DontRenderArmour))==0)
		{	
			glPushMatrix();
			humanoidModel->head->translateTo(1 / 16.0f);

			if (headGear->getItem()->id < 256)
			{
				if (Tile::tiles[headGear->id] != NULL && TileRenderer::canRender(Tile::tiles[headGear->id]->getRenderShape()))
				{
					float s = 10 / 16.0f;
					glTranslatef(-0 / 16.0f, -4 / 16.0f, 0 / 16.0f);
					glRotatef(90, 0, 1, 0);
					glScalef(s, -s, -s);
				}

				this->entityRenderDispatcher->itemInHandRenderer->renderItem(mob, headGear, 0);
			}
			else if (headGear->getItem()->id == Item::skull_Id)
			{
				float s = 17 / 16.0f;
				glScalef(s, -s, -s);

				wstring extra = L"";
				if (headGear->hasTag() && headGear->getTag()->contains(L"SkullOwner"))
				{
					extra = headGear->getTag()->getString(L"SkullOwner");
				}
				SkullTileRenderer::instance->renderSkull(-0.5f, 0, -0.5f, Facing::UP, 180, headGear->getAuxValue(), extra);
			}

			glPopMatrix();
		}
	}

    if (item != NULL)
	{
        glPushMatrix();

		if (model->young)
		{
			float s = 0.5f;
			glTranslatef(0 / 16.0f, 10 / 16.0f, 0 / 16.0f);
			glRotatef(-20, -1, 0, 0);
			glScalef(s, s, s);
		}

        humanoidModel->arm0->translateTo(1 / 16.0f);
        glTranslatef(-1 / 16.0f, 7 / 16.0f, 1 / 16.0f);

        if (item->id < 256 && TileRenderer::canRender(Tile::tiles[item->id]->getRenderShape()))
		{
            float s = 8 / 16.0f;
            glTranslatef(-0 / 16.0f, 3 / 16.0f, -5 / 16.0f);
            s *= 0.75f;
            glRotatef(20, 1, 0, 0);
            glRotatef(45, 0, 1, 0);
            glScalef(-s, -s, s);
        }
		else if (item->id == Item::bow_Id)
		{
			float s = 10 / 16.0f;
			glTranslatef(0/16.0f, 2 / 16.0f, 5 / 16.0f);
			glRotatef(-20, 0, 1, 0);
			glScalef(s, -s, s);
			glRotatef(-100, 1, 0, 0);
			glRotatef(45, 0, 1, 0);
		}
		else if (Item::items[item->id]->isHandEquipped())
		{
            float s = 10 / 16.0f;
            glTranslatef(0, 3 / 16.0f, 0);
            glScalef(s, -s, s);
            glRotatef(-100, 1, 0, 0);
            glRotatef(45, 0, 1, 0);
        }
		else
		{
            float s = 6 / 16.0f;
            glTranslatef(+4 / 16.0f, +3 / 16.0f, -3 / 16.0f);
            glScalef(s, s, s);
            glRotatef(60, 0, 0, 1);
            glRotatef(-90, 1, 0, 0);
            glRotatef(20, 0, 0, 1);
        }

		this->entityRenderDispatcher->itemInHandRenderer->renderItem(mob, item, 0);
		if (item->getItem()->hasMultipleSpriteLayers())
		{
			this->entityRenderDispatcher->itemInHandRenderer->renderItem(mob, item, 1);
		}
		
        glPopMatrix();
    }

}

void HumanoidMobRenderer::scale(shared_ptr<Mob> mob, float a)
{
	glScalef(_scale, _scale, _scale);
}