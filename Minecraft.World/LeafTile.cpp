#include "stdafx.h"
#include "../Minecraft.Client/Minecraft.h"
#include "LeafTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.h"

const unsigned int LeafTile::LEAF_NAMES[LEAF_NAMES_LENGTH] = {	IDS_TILE_LEAVES_OAK,
	IDS_TILE_LEAVES_SPRUCE,
	IDS_TILE_LEAVES_BIRCH,
	IDS_TILE_LEAVES_JUNGLE,
};

const wstring LeafTile::TEXTURES[2][4] = { {L"leaves", L"leaves_spruce", L"leaves", L"leaves_jungle"}, {L"leaves_opaque", L"leaves_spruce_opaque", L"leaves_opaque", L"leaves_jungle_opaque"},};

LeafTile::LeafTile(int id) : TransparentTile(id, Material::leaves, false, isSolidRender())
{
	checkBuffer = NULL;
	fancyTextureSet = 0;
	setTicking(true);
}

LeafTile::~LeafTile()
{
	delete [] checkBuffer;
}

int LeafTile::getColor() const
{
	// 4J Stu - Not using this any more
	//double temp = 0.5;
	//double rain = 1.0;

	//return FoliageColor::get(temp, rain);

	return Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Foliage_Common );
}

int LeafTile::getColor(int data)
{
	if ((data & LEAF_TYPE_MASK) == EVERGREEN_LEAF)
	{
		return FoliageColor::getEvergreenColor();
	}
	if ((data & LEAF_TYPE_MASK) == BIRCH_LEAF)
	{
		return FoliageColor::getBirchColor();
	}

	return FoliageColor::getDefaultColor();
}

int LeafTile::getColor(LevelSource *level, int x, int y, int z)
{
	return getColor(level, x, y, z, level->getData(x, y, z) );
}

// 4J - changed interface to have data passed in, and put existing interface as wrapper above
int LeafTile::getColor(LevelSource *level, int x, int y, int z, int data)
{
	if ((data & LEAF_TYPE_MASK) == EVERGREEN_LEAF)
	{
		return FoliageColor::getEvergreenColor();
	}
	if ((data & LEAF_TYPE_MASK) == BIRCH_LEAF)
	{
		return FoliageColor::getBirchColor();
	}

	int totalRed = 0;
	int totalGreen = 0;
	int totalBlue = 0;

	for (int oz = -1; oz <= 1; oz++)
	{
		for (int ox = -1; ox <= 1; ox++)
		{
			int foliageColor = level->getBiome(x + ox, z + oz)->getFolageColor();

			totalRed += (foliageColor & 0xff0000) >> 16;
			totalGreen += (foliageColor & 0xff00) >> 8;
			totalBlue += (foliageColor & 0xff);
		}
	}

	return (((totalRed / 9) & 0xFF) << 16) | (((totalGreen / 9) & 0xFF) << 8) | (((totalBlue / 9) & 0xFF));
}

void LeafTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	int r = 1;
	int r2 = r + 1;

	if (level->hasChunksAt(x - r2, y - r2, z - r2, x + r2, y + r2, z + r2))
	{
		for (int xo = -r; xo <= r; xo++)
			for (int yo = -r; yo <= r; yo++)
				for (int zo = -r; zo <= r; zo++)
				{
					int t = level->getTile(x + xo, y + yo, z + zo);
					if (t == Tile::leaves_Id)
					{
						int currentData = level->getData(x + xo, y + yo, z + zo);
						level->setData(x + xo, y + yo, z + zo, currentData | UPDATE_LEAF_BIT, Tile::UPDATE_NONE);
					}
				}
	}

}

void LeafTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isClientSide) return;

	int currentData = level->getData(x, y, z);
	if ((currentData & UPDATE_LEAF_BIT) != 0 && (currentData & PERSISTENT_LEAF_BIT) == 0)
	{
		int r = REQUIRED_WOOD_RANGE;
		int r2 = r + 1;

		int W = 32;
		int WW = W * W;
		int WO = W / 2;
		if (checkBuffer == NULL)
		{
			checkBuffer = new int[W * W * W];
		}

		if (level->hasChunksAt(x - r2, y - r2, z - r2, x + r2, y + r2, z + r2))
		{
			// 4J Stu - Assuming we remain in the same chunk, getTile accesses an array that varies least by y
			// Changing the ordering here to loop by y last
			for (int xo = -r; xo <= r; xo++)
				for (int zo = -r; zo <= r; zo++)
					for (int yo = -r; yo <= r; yo++)
					{
						int t = level->getTile(x + xo, y + yo, z + zo);
						if (t == Tile::treeTrunk_Id)
						{
							checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO)] = 0;
						}
						else if (t == Tile::leaves_Id)
						{
							checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO)] = -2;
						}
						else
						{
							checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO)] = -1;
						}
					}
					for (int i = 1; i <= REQUIRED_WOOD_RANGE; i++)
					{
						for (int xo = -r; xo <= r; xo++)
							for (int yo = -r; yo <= r; yo++)
								for (int zo = -r; zo <= r; zo++)
								{
									if (checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO)] == i - 1)
									{
										if (checkBuffer[(xo + WO - 1) * WW + (yo + WO) * W + (zo + WO)] == -2)
										{
											checkBuffer[(xo + WO - 1) * WW + (yo + WO) * W + (zo + WO)] = i;
										}
										if (checkBuffer[(xo + WO + 1) * WW + (yo + WO) * W + (zo + WO)] == -2)
										{
											checkBuffer[(xo + WO + 1) * WW + (yo + WO) * W + (zo + WO)] = i;
										}
										if (checkBuffer[(xo + WO) * WW + (yo + WO - 1) * W + (zo + WO)] == -2)
										{
											checkBuffer[(xo + WO) * WW + (yo + WO - 1) * W + (zo + WO)] = i;
										}
										if (checkBuffer[(xo + WO) * WW + (yo + WO + 1) * W + (zo + WO)] == -2)
										{
											checkBuffer[(xo + WO) * WW + (yo + WO + 1) * W + (zo + WO)] = i;
										}
										if (checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO - 1)] == -2)
										{
											checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO - 1)] = i;
										}
										if (checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO + 1)] == -2)
										{
											checkBuffer[(xo + WO) * WW + (yo + WO) * W + (zo + WO + 1)] = i;
										}
									}
								}
					}
		}

		int mid = checkBuffer[(WO) * WW + (WO) * W + (WO)];
		if (mid >= 0)
		{
			level->setData(x, y, z, currentData & ~UPDATE_LEAF_BIT, Tile::UPDATE_NONE);
		}
		else
		{
			die(level, x, y, z);
		}
	}

}

void LeafTile::animateTick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isRainingAt(x, y + 1, z) && !level->isTopSolidBlocking(x, y - 1, z) && random->nextInt(15) == 1)
	{
		double xx = x + random->nextFloat();
		double yy = y - 0.05;
		double zz = z + random->nextFloat();

		level->addParticle(eParticleType_dripWater, xx, yy, zz, 0, 0, 0);
	}
}

void LeafTile::die(Level *level, int x, int y, int z)
{
	Tile::spawnResources(level, x, y, z, level->getData(x, y, z), 0);
	level->removeTile(x, y, z);
}

int LeafTile::getResourceCount(Random *random)
{
	return random->nextInt(20) == 0 ? 1 : 0;
}

int LeafTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::sapling_Id;
}

// 4J DCR: Brought forward from 1.2
void LeafTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	if (!level->isClientSide)
	{
		int chance = 20;
		if ((data & LEAF_TYPE_MASK) == JUNGLE_LEAF)
		{
			chance = 40;
		}
		if (playerBonusLevel > 0)
		{
			chance -= 2 << playerBonusLevel;
			if (chance < 10)
			{
				chance = 10;
			}
		}
		if (level->random->nextInt(chance) == 0)
		{
			int type = getResource(data, level->random,playerBonusLevel);
			popResource(level, x, y, z, shared_ptr<ItemInstance>( new ItemInstance(type, 1, getSpawnResourcesAuxValue(data))));
		}

		chance = 200;
		if (playerBonusLevel > 0)
		{
			chance -= 10 << playerBonusLevel;
			if (chance < 40)
			{
				chance = 40;
			}
		}
		if ((data & LEAF_TYPE_MASK) == NORMAL_LEAF && level->random->nextInt(chance) == 0)
		{
			popResource(level, x, y, z, shared_ptr<ItemInstance>(new ItemInstance(Item::apple_Id, 1, 0)));
		}
	}
}

void LeafTile::playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data)
{
	if (!level->isClientSide && player->getSelectedItem() != NULL && player->getSelectedItem()->id == Item::shears->id)
	{
		player->awardStat(
			GenericStats::blocksMined(id),
			GenericStats::param_blocksMined(id,data,1)
			);

		// drop leaf block instead of sapling
		popResource(level, x, y, z, shared_ptr<ItemInstance>(new ItemInstance(Tile::leaves_Id, 1, data & LEAF_TYPE_MASK)));
	}
	else
	{
		TransparentTile::playerDestroy(level, player, x, y, z, data);
	}
}

int LeafTile::getSpawnResourcesAuxValue(int data)
{
	return data & LEAF_TYPE_MASK;
}

bool LeafTile::isSolidRender(bool isServerLevel)
{
	// 4J Stu - The server level shouldn't care how the tile is rendered!
	// Fix for #9407 - Gameplay: Destroying a block of snow on top of trees, removes any adjacent snow.
	if(isServerLevel) return true;
	return !allowSame;
}

Icon *LeafTile::getTexture(int face, int data)
{
	if ((data & LEAF_TYPE_MASK) == EVERGREEN_LEAF)
	{
		return icons[fancyTextureSet][EVERGREEN_LEAF];
	}
	if ((data & LEAF_TYPE_MASK) == JUNGLE_LEAF)
	{
		return icons[fancyTextureSet][JUNGLE_LEAF];
	}
	if ((data & LEAF_TYPE_MASK) == BIRCH_LEAF)
	{
		return icons[fancyTextureSet][BIRCH_LEAF];
	}
	return icons[fancyTextureSet][0];
}

void LeafTile::setFancy(bool fancyGraphics)
{
	allowSame = fancyGraphics;
	fancyTextureSet = (fancyGraphics ? 0 : 1);
}

shared_ptr<ItemInstance> LeafTile::getSilkTouchItemInstance(int data)
{
	return shared_ptr<ItemInstance>( new ItemInstance(id, 1, data & LEAF_TYPE_MASK) );
}

void LeafTile::stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	TransparentTile::stepOn(level, x, y, z, entity);
}

bool LeafTile::shouldTileTick(Level *level, int x,int y,int z)
{
	int currentData = level->getData(x, y, z);
	return (currentData & UPDATE_LEAF_BIT) != 0;
}

unsigned int LeafTile::getDescriptionId(int iData /*= -1*/)
{
	int leafIndex = iData & LEAF_TYPE_MASK;
	return LeafTile::LEAF_NAMES[leafIndex];
}

void LeafTile::registerIcons(IconRegister *iconRegister)
{
	for (int fancy = 0; fancy < 2; fancy++)
	{
		//icons[fancy] = new Icon[TEXTURES[fancy].length];

		for (int i = 0; i < 4; i++)
		{
			icons[fancy][i] = iconRegister->registerIcon(TEXTURES[fancy][i]);
		}
	}
}
