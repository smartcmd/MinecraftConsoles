#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "BasicTypeContainers.h"
#include "Arrays.h"
#include "OcelotSitOnTileGoal.h"

const int OcelotSitOnTileGoal::GIVE_UP_TICKS = 3 * SharedConstants::TICKS_PER_SECOND;
const int OcelotSitOnTileGoal::SIT_TICKS = 60 * SharedConstants::TICKS_PER_SECOND;
const int OcelotSitOnTileGoal::SEARCH_RANGE = 8;
const double OcelotSitOnTileGoal::SIT_CHANCE = 0.0065f;

OcelotSitOnTileGoal::OcelotSitOnTileGoal(Ozelot *ocelot, float speed)
{
	_tick = 0;
	tryTicks = 0;
	maxTicks = 0;
	tileX = 0;
	tileY = 0;
	tileZ = 0;

	this->ocelot = ocelot;
	this->speed = speed;
	setRequiredControlFlags(Control::MoveControlFlag | Control::JumpControlFlag);
}

bool OcelotSitOnTileGoal::canUse()
{
	return ocelot->isTame() && !ocelot->isSitting() && ocelot->getRandom()->nextDouble() <= SIT_CHANCE && findNearestTile();
}

bool OcelotSitOnTileGoal::canContinueToUse()
{
	return _tick <= maxTicks && tryTicks <= GIVE_UP_TICKS && isValidTarget(ocelot->level, tileX, tileY, tileZ);
}

void OcelotSitOnTileGoal::start()
{
	ocelot->getNavigation()->moveTo((float) tileX + 0.5, tileY + 1, (float) tileZ + 0.5, speed);
	_tick = 0;
	tryTicks = 0;
	maxTicks = ocelot->getRandom()->nextInt(ocelot->getRandom()->nextInt(SIT_TICKS) + SIT_TICKS) + SIT_TICKS;
	ocelot->getSitGoal()->wantToSit(false);
}

void OcelotSitOnTileGoal::stop()
{
	ocelot->setSitting(false);
}

void OcelotSitOnTileGoal::tick()
{
	_tick++;
	ocelot->getSitGoal()->wantToSit(false);
	if (ocelot->distanceToSqr(tileX, tileY + 1, tileZ) > 1)
	{
		ocelot->setSitting(false);
		ocelot->getNavigation()->moveTo((float) tileX + 0.5, tileY + 1, (float) tileZ + 0.5, speed);
		tryTicks++;
	}
	else if (!ocelot->isSitting())
	{
		ocelot->setSitting(true);
	}
	else
	{
		tryTicks--;
	}
}

bool OcelotSitOnTileGoal::findNearestTile()
{
	int y = (int) ocelot->y;
	double distSqr = Integer::MAX_VALUE;

	for (int x = (int) ocelot->x - SEARCH_RANGE; x < ocelot->x + SEARCH_RANGE; x++)
	{
		for (int z = (int) ocelot->z - SEARCH_RANGE; z < ocelot->z + SEARCH_RANGE; z++)
		{
			if (isValidTarget(ocelot->level, x, y, z) && ocelot->level->isEmptyTile(x, y + 1, z))
			{
				double dist = ocelot->distanceToSqr(x, y, z);

				if (dist < distSqr)
				{
					this->tileX = x;
					this->tileY = y;
					this->tileZ = z;
					distSqr = dist;
				}
			}
		}
	}

	return distSqr < Integer::MAX_VALUE;
}

bool OcelotSitOnTileGoal::isValidTarget(Level *level, int x, int y, int z)
{
	int tile = level->getTile(x, y, z);
	int data = level->getData(x, y, z);

	if (tile == Tile::chest_Id)
	{
		shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(level->getTileEntity(x, y, z));

		if (chest->openCount < 1)
		{
			return true;
		}
	}
	else if (tile == Tile::furnace_lit_Id)
	{
		return true;
	}
	else if (tile == Tile::bed_Id && !BedTile::isHeadPiece(data))
	{
		return true;
	}

	return false;
}