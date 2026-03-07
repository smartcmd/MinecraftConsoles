#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.phys.h"
#include "LeashItem.h"

LeashItem::LeashItem(int id) : Item(id)
{
}

bool LeashItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	int tile = level->getTile(x, y, z);
	if (Tile::tiles[tile] != NULL && Tile::tiles[tile]->getRenderShape() == Tile::SHAPE_FENCE)
	{
		if (bTestUseOnOnly) return bindPlayerMobsTest(player, level, x,y,z);

		if (level->isClientSide)
		{
			return true;
		}

		bindPlayerMobs(player, level, x, y, z);
		return true;
	}
	return false;
}

bool LeashItem::bindPlayerMobs(shared_ptr<Player> player, Level *level, int x, int y, int z)
{
	// check if there is a knot at the given coordinate
	shared_ptr<LeashFenceKnotEntity> activeKnot = LeashFenceKnotEntity::findKnotAt(level, x, y, z);

	// look for entities that can be attached to the fence
	bool foundMobs = false;
	double range = 7;
	vector<shared_ptr<Entity> > *mobs = level->getEntitiesOfClass(typeid(Mob), AABB::newTemp(x - range, y - range, z - range, x + range, y + range, z + range));
	if (mobs != NULL)
	{
		for(auto& it : *mobs)
		{
			shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(it);
			if (mob->isLeashed() && mob->getLeashHolder() == player)
			{
				if (activeKnot == NULL)
				{
					activeKnot = LeashFenceKnotEntity::createAndAddKnot(level, x, y, z);
				}
				mob->setLeashedTo(activeKnot, true);
				foundMobs = true;
			}
		}
	}
	return foundMobs;
}

// 4J-JEV: Similar to bindPlayerMobs, but doesn't actually bind mobs,
bool LeashItem::bindPlayerMobsTest(shared_ptr<Player> player, Level *level, int x, int y, int z)
{
	// look for entities that can be attached to the fence
	double range = 7;
	vector<shared_ptr<Entity> > *mobs = level->getEntitiesOfClass(typeid(Mob), AABB::newTemp(x - range, y - range, z - range, x + range, y + range, z + range));

	if (mobs != NULL)
	{
		for(auto& it : *mobs)
		{
			shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(it);
			if (mob->isLeashed() && mob->getLeashHolder() == player) return true;
		}
	}
	return false;
}