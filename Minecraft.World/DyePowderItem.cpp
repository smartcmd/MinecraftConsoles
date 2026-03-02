using namespace std;

#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.global.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.h"
#include "Material.h"
#include "DyePowderItem.h"

DyePowderItem::DyePowderItem(int id) : Item( id )
{
	setStackedByData(true);
	setMaxDamage(0);
	icons = NULL;
}

const unsigned int DyePowderItem::COLOR_DESCS[] = 
{ 
	IDS_ITEM_DYE_POWDER_BLACK,
	IDS_ITEM_DYE_POWDER_RED,
	IDS_ITEM_DYE_POWDER_GREEN,
	IDS_ITEM_DYE_POWDER_BROWN,
	IDS_ITEM_DYE_POWDER_BLUE,
	IDS_ITEM_DYE_POWDER_PURPLE,
	IDS_ITEM_DYE_POWDER_CYAN,
	IDS_ITEM_DYE_POWDER_SILVER,
	IDS_ITEM_DYE_POWDER_GRAY,
	IDS_ITEM_DYE_POWDER_PINK,
	IDS_ITEM_DYE_POWDER_LIME,
	IDS_ITEM_DYE_POWDER_YELLOW,
	IDS_ITEM_DYE_POWDER_LIGHT_BLUE,
	IDS_ITEM_DYE_POWDER_MAGENTA,
	IDS_ITEM_DYE_POWDER_ORANGE,
	IDS_ITEM_DYE_POWDER_WHITE
};

const unsigned int DyePowderItem::COLOR_USE_DESCS[] = 
{ 
	IDS_DESC_DYE_BLACK,
	IDS_DESC_DYE_RED,
	IDS_DESC_DYE_GREEN,
	IDS_DESC_DYE_BROWN,
	IDS_DESC_DYE_BLUE,
	IDS_DESC_DYE_PURPLE,
	IDS_DESC_DYE_CYAN,
	IDS_DESC_DYE_LIGHTGRAY,
	IDS_DESC_DYE_GRAY,
	IDS_DESC_DYE_PINK,
	IDS_DESC_DYE_LIME,
	IDS_DESC_DYE_YELLOW,
	IDS_DESC_DYE_LIGHTBLUE,
	IDS_DESC_DYE_MAGENTA,
	IDS_DESC_DYE_ORANGE,
	IDS_DESC_DYE_WHITE
};

const wstring DyePowderItem::COLOR_TEXTURES[] =
{ L"black", L"red", L"green", L"brown", L"blue", L"purple", L"cyan", L"silver", L"gray", L"pink",
L"lime", L"yellow", L"light_blue", L"magenta", L"orange", L"white"};

const int DyePowderItem::COLOR_RGB[] =
{
	0x1e1b1b,
	0xb3312c,
	0x3b511a,
	0x51301a,
	0x253192,
	0x7b2fbe,
	0x287697,
	0xababab,
	0x434343,
	0xd88198,
	0x41cd34,
	0xdecf2a,
	0x6689d3,
	0xc354cd,
	0xeb8844,
	0xf0f0f0
};

const int DyePowderItem::BLACK = 0;
const int DyePowderItem::RED = 1;
const int DyePowderItem::GREEN = 2;
const int DyePowderItem::BROWN = 3;
const int DyePowderItem::BLUE = 4;
const int DyePowderItem::PURPLE = 5;
const int DyePowderItem::CYAN = 6;
const int DyePowderItem::SILVER = 7;
const int DyePowderItem::GRAY = 8;
const int DyePowderItem::PINK = 9;
const int DyePowderItem::LIME = 10;
const int DyePowderItem::YELLOW = 11;
const int DyePowderItem::LIGHT_BLUE = 12;
const int DyePowderItem::MAGENTA = 13;
const int DyePowderItem::ORANGE = 14;
const int DyePowderItem::WHITE = 15;

Icon *DyePowderItem::getIcon(int itemAuxValue) 
{
	int colorValue = Mth::clamp(itemAuxValue, 0, 15);
	return icons[colorValue];
}

unsigned int DyePowderItem::getDescriptionId(shared_ptr<ItemInstance> itemInstance) 
{
	int colorValue = Mth::clamp(itemInstance->getAuxValue(), 0, 15);
	return COLOR_DESCS[colorValue];
}

unsigned int DyePowderItem::getUseDescriptionId(shared_ptr<ItemInstance> itemInstance) 
{
	return COLOR_USE_DESCS[itemInstance->getAuxValue()];
}

bool DyePowderItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly) 
{
	if (!player->mayUseItemAt(x, y, z, face, itemInstance)) return false;

	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	if (itemInstance->getAuxValue() == WHITE) 
	{
		// bone meal is a fertilizer, so instantly grow trees and stuff

		if (growCrop(itemInstance, level, x, y, z, bTestUseOnOnly))
		{
			if (!level->isClientSide) level->levelEvent(LevelEvent::PARTICLES_PLANT_GROWTH, x, y, z, 0);
			return true;
		}
	}
	else if (itemInstance->getAuxValue() == BROWN)
	{
		// plant cocoa

		int tile = level->getTile(x, y, z);
		int data = level->getData(x, y, z);

		if (tile == Tile::treeTrunk_Id && TreeTile::getWoodType(data) == TreeTile::JUNGLE_TRUNK)
		{
			if (face == 0) return false;
			if (face == 1) return false;
			if (face == 2) z--;
			if (face == 3) z++;
			if (face == 4) x--;
			if (face == 5) x++;

			if(!bTestUseOnOnly)
			{
				if (level->isEmptyTile(x, y, z))
				{
					int cocoaData = Tile::tiles[Tile::cocoa_Id]->getPlacedOnFaceDataValue(level, x, y, z, face, clickX, clickY, clickZ, 0);
					level->setTileAndData(x, y, z, Tile::cocoa_Id, cocoaData, Tile::UPDATE_CLIENTS);
					if (!player->abilities.instabuild)
					{
						itemInstance->count--;
					}
				}
			}
			return true;
		}
	}
	return false;
}

bool DyePowderItem::growCrop(shared_ptr<ItemInstance> itemInstance, Level *level, int x, int y, int z, bool bTestUseOnOnly)
{
	int tile = level->getTile(x, y, z);
	if (tile == Tile::sapling_Id) 
	{
		if(!bTestUseOnOnly)
		{	
			if (!level->isClientSide) 
			{
				if (level->random->nextFloat() < 0.45) ((Sapling *) Tile::sapling)->advanceTree(level, x, y, z, level->random);
				itemInstance->count--;
			}
		}
		return true;
	}
	else if (tile == Tile::mushroom_brown_Id || tile == Tile::mushroom_red_Id)
	{
		if(!bTestUseOnOnly)
		{
			if (!level->isClientSide)
			{
				if (level->random->nextFloat() < 0.4) ((Mushroom *) Tile::tiles[tile])->growTree(level, x, y, z, level->random);
				itemInstance->count--;
			}
		}
		return true;
	}
	else if (tile == Tile::melonStem_Id || tile == Tile::pumpkinStem_Id)
	{
		if (level->getData(x, y, z) == 7) return false;
		if(!bTestUseOnOnly)
		{
			if (!level->isClientSide)
			{
				((StemTile *) Tile::tiles[tile])->growCrops(level, x, y, z);
				itemInstance->count--;
			}
		}
		return true;
	} 
	else if (tile == Tile::carrots_Id || tile == Tile::potatoes_Id)
	{
		if (level->getData(x, y, z) == 7) return false;
		if(!bTestUseOnOnly)
		{
			if (!level->isClientSide) 
			{
				((CropTile *) Tile::tiles[tile])->growCrops(level, x, y, z);
				itemInstance->count--;
			}
		}
		return true;
	}
	else if (tile == Tile::wheat_Id) 
	{
		if (level->getData(x, y, z) == 7) return false;
		if(!bTestUseOnOnly)
		{	
			if (!level->isClientSide) 
			{
				((CropTile *) Tile::tiles[tile])->growCrops(level, x, y, z);
				itemInstance->count--;
			}
		}
		return true;
	}
	else if (tile == Tile::cocoa_Id)
	{
		if(!bTestUseOnOnly)
		{
			int data = level->getData(x, y, z);
			int direction = DirectionalTile::getDirection(data);
			int age = CocoaTile::getAge(data);
			if (age >= 2) return false;
			if (!level->isClientSide)
			{
				age++;
				level->setData(x, y, z, (age << 2) | direction, Tile::UPDATE_CLIENTS);
				itemInstance->count--;
			}
		}
		return true;
	} 
	else if (tile == Tile::grass_Id) 
	{
		if(!bTestUseOnOnly)
		{	
			if (!level->isClientSide) 
			{
				itemInstance->count--;

				for (int j = 0; j < 128; j++) 
				{
					int xx = x;
					int yy = y + 1;
					int zz = z;
					for (int i = 0; i < j / 16; i++) 
					{
						xx += random->nextInt(3) - 1;
						yy += (random->nextInt(3) - 1) * random->nextInt(3) / 2;
						zz += random->nextInt(3) - 1;
						if (level->getTile(xx, yy - 1, zz) != Tile::grass_Id || level->isSolidBlockingTile(xx, yy, zz)) 
						{
							goto mainloop;
						}
					}

					if (level->getTile(xx, yy, zz) == 0) 
					{
						if (random->nextInt(10) != 0) 
						{
							if (Tile::tallgrass->canSurvive(level, xx, yy, zz)) level->setTileAndData(xx, yy, zz, Tile::tallgrass_Id, TallGrass::TALL_GRASS, Tile::UPDATE_ALL);
						} 
						else if (random->nextInt(3) != 0) 
						{
							if (Tile::flower->canSurvive(level, xx, yy, zz)) level->setTileAndUpdate(xx, yy, zz, Tile::flower_Id);
						} 
						else 
						{
							if (Tile::rose->canSurvive(level, xx, yy, zz)) level->setTileAndUpdate(xx, yy, zz, Tile::rose_Id);
						}
					}

					// 4J - Stops infinite loops.
mainloop: continue;
				}
			}
		}

		return true;
	}
	return false;
}

void DyePowderItem::addGrowthParticles(Level *level, int x, int y, int z, int count)
{
    int id = level->getTile(x, y, z);
    if (count == 0) count = 15;
    Tile *tile = id > 0 && id < Tile::TILE_NUM_COUNT ? Tile::tiles[id] : NULL;

    if (tile == NULL) return;
    tile->updateShape(level, x, y, z);

    for (int i = 0; i < count; i++)
	{
        double xa = level->random->nextGaussian() * 0.02;
        double ya = level->random->nextGaussian() * 0.02;
        double za = level->random->nextGaussian() * 0.02;
        level->addParticle(eParticleType_happyVillager, x + level->random->nextFloat(), y + level->random->nextFloat() * tile->getShapeY1(), z + level->random->nextFloat(), xa, ya, za);
    }
}

bool DyePowderItem::interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> mob) 
{
	if (dynamic_pointer_cast<Sheep>( mob ) != NULL) 
	{
		shared_ptr<Sheep> sheep = dynamic_pointer_cast<Sheep>(mob);
		// convert to tile-based color value (0 is white instead of black)
		int newColor = ColoredTile::getTileDataForItemAuxValue(itemInstance->getAuxValue());
		if (!sheep->isSheared() && sheep->getColor() != newColor) 
		{
			sheep->setColor(newColor);
			itemInstance->count--;
		}
		return true;
	}
	return false;
}

void DyePowderItem::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon *[DYE_POWDER_ITEM_TEXTURE_COUNT];

	for (int i = 0; i < DYE_POWDER_ITEM_TEXTURE_COUNT; i++)
	{
		icons[i] = iconRegister->registerIcon(getIconName() + L"_" + COLOR_TEXTURES[i]);
	}
}