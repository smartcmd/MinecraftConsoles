#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "RedStoneOreTile.h"
#include "net.minecraft.world.item.h"

RedStoneOreTile::RedStoneOreTile(int id, bool lit) : Tile(id, Material::stone)
{
	if (lit)
	{
		this->setTicking(true);
	}
	this->lit = lit;
}

int RedStoneOreTile::getTickDelay(Level *level)
{
	return 30;
}

void RedStoneOreTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	interact(level, x, y, z);
	Tile::attack(level, x, y, z, player);
}

void RedStoneOreTile::stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	interact(level, x, y, z);
	Tile::stepOn(level, x, y, z, entity);
}

// 4J-PB - Adding a TestUse for tooltip display
bool RedStoneOreTile::TestUse()
{
	return id == Tile::redStoneOre_Id;
}

bool RedStoneOreTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (soundOnly) return false;
	interact(level, x, y, z);
	return Tile::use(level, x, y, z, player, clickedFace, clickX, clickY, clickZ);
}

void RedStoneOreTile::interact(Level *level, int x, int y, int z)
{
	poofParticles(level, x, y, z);
	if (level->isClientSide) return;		// 4J added
	if (id == Tile::redStoneOre_Id)
	{
		level->setTileAndUpdate(x, y, z, Tile::redStoneOre_lit_Id);
	}
}

void RedStoneOreTile::tick(Level *level, int x, int y, int z, Random* random)
{
	if (id == Tile::redStoneOre_lit_Id)
	{
		level->setTileAndUpdate(x, y, z, Tile::redStoneOre_Id);
	}
}

int RedStoneOreTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::redStone->id;
}

int RedStoneOreTile::getResourceCountForLootBonus(int bonusLevel, Random *random)
{
	return getResourceCount(random) + random->nextInt(bonusLevel + 1);
}

int RedStoneOreTile::getResourceCount(Random *random)
{
	return 4 + random->nextInt(2);
}

void RedStoneOreTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	Tile::spawnResources(level, x, y, z, data, odds, playerBonusLevel);

	// also spawn experience if the block is broken
	if (getResource(data, level->random, playerBonusLevel) != id)
	{
		int magicCount = 1 + level->random->nextInt(5);
		popExperience(level, x, y, z, magicCount);
	}
}

void RedStoneOreTile::animateTick(Level *level, int x, int y, int z, Random *random)
{
	if (lit)
	{
		poofParticles(level, x, y, z);
	}
}

void RedStoneOreTile::poofParticles(Level *level, int x, int y, int z)
{
	Random *random = level->random;
	double r = 1 / 16.0f;
	for (int i = 0; i < 6; i++)
	{
		double xx = x + random->nextFloat();
		double yy = y + random->nextFloat();
		double zz = z + random->nextFloat();
		if (i == 0 && !level->isSolidRenderTile(x, y + 1, z)) yy = y + 1 + r;
		if (i == 1 && !level->isSolidRenderTile(x, y - 1, z)) yy = y + 0 - r;
		if (i == 2 && !level->isSolidRenderTile(x, y, z + 1)) zz = z + 1 + r;
		if (i == 3 && !level->isSolidRenderTile(x, y, z - 1)) zz = z + 0 - r;
		if (i == 4 && !level->isSolidRenderTile(x + 1, y, z)) xx = x + 1 + r;
		if (i == 5 && !level->isSolidRenderTile(x - 1, y, z)) xx = x + 0 - r;
		if (xx < x || xx > x + 1 || yy < 0 || yy > y + 1 || zz < z || zz > z + 1)
		{
			level->addParticle(eParticleType_reddust, xx, yy, zz, 0, 0, 0);
		}
	}
}

bool RedStoneOreTile::shouldTileTick(Level *level, int x,int y,int z)
{
	return id == Tile::redStoneOre_lit_Id;
}

shared_ptr<ItemInstance> RedStoneOreTile::getSilkTouchItemInstance(int data)
{
	return shared_ptr<ItemInstance>(new ItemInstance(Tile::redStoneOre));
}