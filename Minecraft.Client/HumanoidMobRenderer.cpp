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

const wstring HumanoidMobRenderer::MATERIAL_NAMES[5] = { L"cloth", L"chain", L"iron", L"diamond", L"gold" };
std::map<wstring, ResourceLocation> HumanoidMobRenderer::ARMOR_LOCATION_CACHE;

void HumanoidMobRenderer::_init(HumanoidModel *humanoidModel, float scale)
{
	this->humanoidModel = humanoidModel;
	this->_scale = scale;
	armorParts1 = NULL;
	armorParts2 = NULL;

	createArmorParts();
}

HumanoidMobRenderer::HumanoidMobRenderer(HumanoidModel *humanoidModel, float shadow) : MobRenderer(humanoidModel, shadow)
{
	_init(humanoidModel, 1.0f);
}

HumanoidMobRenderer::HumanoidMobRenderer(HumanoidModel *humanoidModel, float shadow, float scale) : MobRenderer(humanoidModel, shadow)
{
	_init(humanoidModel, scale);
}

ResourceLocation *HumanoidMobRenderer::getArmorLocation(ArmorItem *armorItem, int layer)
{
	return getArmorLocation(armorItem, layer, false);
}

ResourceLocation *HumanoidMobRenderer::getArmorLocation(ArmorItem *armorItem, int layer, bool overlay)
{
	switch(armorItem->modelIndex)
	{
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		break;
	};
	wstring path = wstring(L"armor/" + MATERIAL_NAMES[armorItem->modelIndex]).append(L"_").append(std::to_wstring(layer == 2 ? 2 : 1)).append((overlay ? L"_b" :L"")).append(L".png");

	std::map<wstring, ResourceLocation>::iterator it = ARMOR_LOCATION_CACHE.find(path);

	ResourceLocation *location;
	if (it != ARMOR_LOCATION_CACHE.end())
	{
		location = &it->second;
	}
	else
	{
		ARMOR_LOCATION_CACHE.insert(std::pair<wstring, ResourceLocation>(path, ResourceLocation(path)));

		it = ARMOR_LOCATION_CACHE.find(path);
		location = &it->second;
	}

	return location;
}

void HumanoidMobRenderer::prepareSecondPassArmor(shared_ptr<LivingEntity> mob, int layer, float a)
{
	shared_ptr<ItemInstance> itemInstance = mob->getArmor(3 - layer);
	if (itemInstance != NULL) {
		Item *item = itemInstance->getItem();
		if (dynamic_cast<ArmorItem *>(item) != NULL)
		{
			bindTexture(getArmorLocation(dynamic_cast<ArmorItem *>(item), layer, true));

			float brightness = SharedConstants::TEXTURE_LIGHTING ? 1 : mob->getBrightness(a);
			glColor3f(brightness, brightness, brightness);
		}
	}
}

void HumanoidMobRenderer::createArmorParts()
{
	armorParts1 = new HumanoidModel(1.0f);
	armorParts2 = new HumanoidModel(0.5f);
}

int HumanoidMobRenderer::prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a)
{
	shared_ptr<LivingEntity> mob = dynamic_pointer_cast<LivingEntity>(_mob);

	shared_ptr<ItemInstance> itemInstance = mob->getArmor(3 - layer);
	if (itemInstance != NULL)
	{
		Item *item = itemInstance->getItem();
		if (dynamic_cast<ArmorItem *>(item) != NULL)
		{
			ArmorItem *armorItem = dynamic_cast<ArmorItem *>(item);
			bindTexture(getArmorLocation(armorItem, layer));

			HumanoidModel *armor = layer == 2 ? armorParts2 : armorParts1;

			armor->head->visible = layer == 0;
			armor->hair->visible = layer == 0;
			armor->body->visible = layer == 1 || layer == 2;
			armor->arm0->visible = layer == 1;
			armor->arm1->visible = layer == 1;
			armor->leg0->visible = layer == 2 || layer == 3;
			armor->leg1->visible = layer == 2 || layer == 3;

			setArmor(armor);
			armor->attackTime = model->attackTime;
			armor->riding = model->riding;
			armor->young = model->young;

			float brightness = SharedConstants::TEXTURE_LIGHTING ? 1 : mob->getBrightness(a);
			if (armorItem->getMaterial() == ArmorItem::ArmorMaterial::CLOTH)
			{
				int color = armorItem->getColor(itemInstance);
				float red = (float) ((color >> 16) & 0xFF) / 0xFF;
				float green = (float) ((color >> 8) & 0xFF) / 0xFF;
				float blue = (float) (color & 0xFF) / 0xFF;
				glColor3f(brightness * red, brightness * green, brightness * blue);

				if (itemInstance->isEnchanted()) return 0x1f;
				return 0x10;

			}
			else
			{
				glColor3f(brightness, brightness, brightness);
			}

			if (itemInstance->isEnchanted()) return 15;

			return 1;
		}
	}
	return -1;
}

void HumanoidMobRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	shared_ptr<LivingEntity> mob = dynamic_pointer_cast<LivingEntity>(_mob);

	float brightness = SharedConstants::TEXTURE_LIGHTING ? 1 : mob->getBrightness(a);
	glColor3f(brightness, brightness, brightness);
	shared_ptr<ItemInstance> item = mob->getCarriedItem();

	prepareCarriedItem(mob, item);

	double yp = y - mob->heightOffset;
	if (mob->isSneaking()) {
		yp -= 2 / 16.0f;
	}
	MobRenderer::render(mob, x, yp, z, rot, a);
	armorParts1->bowAndArrow = armorParts2->bowAndArrow = humanoidModel->bowAndArrow = false;
	armorParts1->sneaking = armorParts2->sneaking = humanoidModel->sneaking = false;
	armorParts1->holdingRightHand = armorParts2->holdingRightHand = humanoidModel->holdingRightHand = 0;
}

ResourceLocation *HumanoidMobRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
	// TODO -- Figure out of we need some data in here
	return NULL;
}

void HumanoidMobRenderer::prepareCarriedItem(shared_ptr<Entity> mob, shared_ptr<ItemInstance> item)
{
	armorParts1->holdingRightHand = armorParts2->holdingRightHand = humanoidModel->holdingRightHand = item != NULL ? 1 : 0;
	armorParts1->sneaking = armorParts2->sneaking = humanoidModel->sneaking = mob->isSneaking();
}

void HumanoidMobRenderer::additionalRendering(shared_ptr<LivingEntity> mob, float a)
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

void HumanoidMobRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
	glScalef(_scale, _scale, _scale);
}