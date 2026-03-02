#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "EatTileGoal.h"

EatTileGoal::EatTileGoal(Mob *mob)
{
	eatAnimationTick = 0;

	this->mob = mob;
	level = mob->level;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag | Control::JumpControlFlag);
}

bool EatTileGoal::canUse()
{
	if (mob->getRandom()->nextInt(mob->isBaby() ? 50 : 1000) != 0) return false;

	int xx = Mth::floor(mob->x);
	int yy = Mth::floor(mob->y);
	int zz = Mth::floor(mob->z);
	if (level->getTile(xx, yy, zz) == Tile::tallgrass_Id && level->getData(xx, yy, zz) == TallGrass::TALL_GRASS) return true;
	if (level->getTile(xx, yy - 1, zz) == Tile::grass_Id) return true;
	return false;
}

void EatTileGoal::start()
{
	eatAnimationTick = EAT_ANIMATION_TICKS;
	level->broadcastEntityEvent(mob->shared_from_this(), EntityEvent::EAT_GRASS);
	mob->getNavigation()->stop();
}

void EatTileGoal::stop()
{
	eatAnimationTick = 0;
}

bool EatTileGoal::canContinueToUse()
{
	return eatAnimationTick > 0;
}

int EatTileGoal::getEatAnimationTick()
{
	return eatAnimationTick;
}

void EatTileGoal::tick()
{
	eatAnimationTick = max(0, eatAnimationTick - 1);
	if (eatAnimationTick != 4) return;

	int xx = Mth::floor(mob->x);
	int yy = Mth::floor(mob->y);
	int zz = Mth::floor(mob->z);

	if (level->getTile(xx, yy, zz) == Tile::tallgrass_Id)
	{
		level->destroyTile(xx, yy, zz, false);
		mob->ate();
	}
	else if (level->getTile(xx, yy - 1, zz) == Tile::grass_Id)
	{
		level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, xx, yy - 1, zz, Tile::grass_Id);
		level->setTileAndData(xx, yy - 1, zz, Tile::dirt_Id, 0, Tile::UPDATE_CLIENTS);
		mob->ate();
	}
}