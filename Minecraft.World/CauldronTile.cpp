#include "stdafx.h"
#include "CauldronTile.h"
#include "Facing.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.h"
#include "..\Minecraft.Client\ServerPlayer.h"

const wstring CauldronTile::TEXTURE_INSIDE = L"cauldron_inner";
const wstring CauldronTile::TEXTURE_BOTTOM = L"cauldron_bottom";

CauldronTile::CauldronTile(int id) : Tile(id, Material::metal, isSolidRender())
{
	iconInner = NULL;
	iconTop = NULL;
	iconBottom = NULL;
}

Icon *CauldronTile::getTexture(int face, int data)
{
	if (face == Facing::UP)
	{
		return iconTop;
	}
	if (face == Facing::DOWN)
	{
		return iconBottom;
	}
	return icon;
}

void CauldronTile::registerIcons(IconRegister *iconRegister)
{
	iconInner = iconRegister->registerIcon(L"cauldron_inner");
	iconTop = iconRegister->registerIcon(L"cauldron_top");
	iconBottom = iconRegister->registerIcon(L"cauldron_bottom");
	icon = iconRegister->registerIcon(L"cauldron_side");
}

Icon *CauldronTile::getTexture(const wstring &name)
{
	if (name.compare(TEXTURE_INSIDE) == 0) return Tile::cauldron->iconInner;
	if (name.compare(TEXTURE_BOTTOM) == 0) return Tile::cauldron->iconBottom;
	return NULL;
}

void CauldronTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	setShape(0, 0, 0, 1, 5.0f / 16.0f, 1);
	Tile::addAABBs(level, x, y, z, box, boxes, source);
	float thickness = 2.0f / 16.0f;
	setShape(0, 0, 0, thickness, 1, 1);
	Tile::addAABBs(level, x, y, z, box, boxes, source);
	setShape(0, 0, 0, 1, 1, thickness);
	Tile::addAABBs(level, x, y, z, box, boxes, source);
	setShape(1 - thickness, 0, 0, 1, 1, 1);
	Tile::addAABBs(level, x, y, z, box, boxes, source);
	setShape(0, 0, 1 - thickness, 1, 1, 1);
	Tile::addAABBs(level, x, y, z, box, boxes, source);

	updateDefaultShape();
}

void CauldronTile::updateDefaultShape()
{
	setShape(0, 0, 0, 1, 1, 1);
}

bool CauldronTile::isSolidRender(bool isServerLevel)
{
	return false;
}

int CauldronTile::getRenderShape()
{
	return SHAPE_CAULDRON;
}

bool CauldronTile::isCubeShaped()
{
	return false;
}

bool CauldronTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if(soundOnly) return false;

	if (level->isClientSide)
	{
		return true;
	}

	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	if (item == NULL)
	{
		return true;
	}

	int currentData = level->getData(x, y, z);
	int fillLevel = getFillLevel(currentData);

	if (item->id == Item::bucket_water_Id)
	{
		if (fillLevel < 3)
		{
			if (!player->abilities.instabuild)
			{
				player->inventory->setItem(player->inventory->selected, shared_ptr<ItemInstance>(new ItemInstance(Item::bucket_empty)));
			}

			level->setData(x, y, z, 3, Tile::UPDATE_CLIENTS);
			level->updateNeighbourForOutputSignal(x, y, z, id);
		}
		return true;
	}
	else if (item->id == Item::glassBottle_Id)
	{
		if (fillLevel > 0)
		{
			shared_ptr<ItemInstance> potion = shared_ptr<ItemInstance>(new ItemInstance(Item::potion, 1, 0));
			if (!player->inventory->add(potion))
			{
				level->addEntity(shared_ptr<ItemEntity>(new ItemEntity(level, x + 0.5, y + 1.5, z + 0.5, potion)));
			}
			// 4J Stu - Brought forward change to update inventory when filling bottles with water
			else if (player->instanceof(eTYPE_SERVERPLAYER))
			{
				dynamic_pointer_cast<ServerPlayer>( player )->refreshContainer(player->inventoryMenu);
			}
			// 4J-PB - don't lose the water in creative mode
			if (player->abilities.instabuild==false)
			{
				item->count--;
				if (item->count <= 0)
				{
					player->inventory->setItem(player->inventory->selected, nullptr);
				}
			}
			level->setData(x, y, z, fillLevel - 1, Tile::UPDATE_CLIENTS);
			level->updateNeighbourForOutputSignal(x, y, z, id);
		}
	}
	else if (fillLevel > 0)
	{
		ArmorItem *armor = dynamic_cast<ArmorItem *>(item->getItem());
		if(armor && armor->getMaterial() == ArmorItem::ArmorMaterial::CLOTH)
		{
			armor->clearColor(item);
			level->setData(x, y, z, fillLevel - 1, Tile::UPDATE_CLIENTS);
			level->updateNeighbourForOutputSignal(x, y, z, id);
			return true;
		}
	}

	return true;

}

void CauldronTile::handleRain(Level *level, int x, int y, int z)
{
	if (level->random->nextInt(20) != 1) return;

	int data = level->getData(x, y, z);

	if (data < 3)
	{
		level->setData(x, y, z, data + 1, Tile::UPDATE_CLIENTS);
	}
}

int CauldronTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::cauldron_Id;
}

int CauldronTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::cauldron_Id;
}

bool CauldronTile::hasAnalogOutputSignal()
{
	return true;
}

int CauldronTile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	int data = level->getData(x, y, z);

	return getFillLevel(data);
}

int CauldronTile::getFillLevel(int data)
{
	return data;
}