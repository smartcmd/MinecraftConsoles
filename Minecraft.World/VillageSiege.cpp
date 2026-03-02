#include "stdafx.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.h"
#include "VillageSiege.h"

VillageSiege::VillageSiege(Level *level)
{
	hasSetupSiege = false;
	siegeState = SIEGE_NOT_INITED;
	siegeCount = 0;
	nextSpawnTime = 0;
	village = weak_ptr<Village>();
	spawnX = spawnY = spawnZ = 0;

	this->level = level;
}

void VillageSiege::tick()
{
	bool debug = false;
	if (debug)
	{
		if (siegeState == SIEGE_DONE)
		{
			siegeCount = 100;
			return;
		}
		// return;
	}
	else
	{
		if (level->isDay())
		{
			siegeState = SIEGE_CAN_ACTIVATE;
			return;
		}

		if (siegeState == SIEGE_DONE) return;

		if (siegeState == SIEGE_CAN_ACTIVATE)
		{
			float timeOfDay = level->getTimeOfDay(0);
			if (timeOfDay < 0.50 || timeOfDay > 0.501) return;
			siegeState = level->random->nextInt(10) == 0 ? SIEGE_TONIGHT : SIEGE_DONE;
			hasSetupSiege = false;
			if (siegeState == SIEGE_DONE) return;
		}
	}

	if (!hasSetupSiege)
	{
		if (tryToSetupSiege()) hasSetupSiege = true;
		else return;
	}

	// Siege!
	if (nextSpawnTime > 0)
	{
		--nextSpawnTime;
		return;
	}

	nextSpawnTime = 2; // 50 + level.random.nextInt(100);
	if (siegeCount > 0)
	{
		trySpawn();
		--siegeCount;
	}
	else
	{
		siegeState = SIEGE_DONE;
	}

}

bool VillageSiege::tryToSetupSiege()
{
	vector<shared_ptr<Player> > *players = &level->players;
	//for (Player player : players)
	for(AUTO_VAR(it, players->begin()); it != players->end(); ++it)
	{
		shared_ptr<Player> player = *it;
		shared_ptr<Village> _village = level->villages->getClosestVillage((int) player->x, (int) player->y, (int) player->z, 1);
		village = _village;

		if (_village == NULL) continue;
		if (_village->getDoorCount() < 10) continue;
		if (_village->getStableAge() < 20) continue;
		if (_village->getPopulationSize() < 20) continue;

		// setup siege origin
		Pos *center = _village->getCenter();
		float radius = _village->getRadius();

		bool overlaps = false;
		for (int i = 0; i < 10; ++i)
		{
			spawnX = center->x + (int) (Mth::cos(level->random->nextFloat() * PI * 2.f) * radius * 0.9);
			spawnY = center->y;
			spawnZ = center->z + (int) (Mth::sin(level->random->nextFloat() * PI * 2.f) * radius * 0.9);
			overlaps = false;
			vector<shared_ptr<Village> > *villages = level->villages->getVillages();
			//for (Village v : level.villages.getVillages())
			for(AUTO_VAR(itV, villages->begin()); itV != villages->end(); ++itV)
			{
				shared_ptr<Village>v = *itV;
				if (v == _village) continue;
				if (v->isInside(spawnX, spawnY, spawnZ))
				{
					overlaps = true;
					break;
				}
			}
			if (!overlaps) break;
		}
		if (overlaps) return false;

		Vec3 *spawnPos = findRandomSpawnPos(spawnX, spawnY, spawnZ);
		if (spawnPos == NULL) continue;

		nextSpawnTime = 0;
		siegeCount = 20;
		return true;
	}
	return false;
}

bool VillageSiege::trySpawn()
{
	Vec3 *spawnPos = findRandomSpawnPos(spawnX, spawnY, spawnZ);
	if (spawnPos == NULL) return false;
	shared_ptr<Zombie> mob;
	//try
	{
		mob = shared_ptr<Zombie>( new Zombie(level) );
		mob->finalizeMobSpawn();
		mob->setVillager(false);
	}
	//catch (Exception e) {
	//	e.printStackTrace();
	//	return false;
	//}
	mob->moveTo(spawnPos->x, spawnPos->y, spawnPos->z, level->random->nextFloat() * 360, 0);
	level->addEntity(mob);
	shared_ptr<Village> _village = village.lock();
	if( _village == NULL ) return false;

	Pos *center = _village->getCenter();
	mob->restrictTo(center->x, center->y, center->z, _village->getRadius());
	return true;
}

Vec3 *VillageSiege::findRandomSpawnPos(int x, int y, int z)
{
	shared_ptr<Village> _village = village.lock();
	if( _village == NULL ) return NULL;

	for (int i = 0; i < 10; ++i)
	{
		int xx = x + level->random->nextInt(16) - 8;
		int yy = y + level->random->nextInt(6) - 3;
		int zz = z + level->random->nextInt(16) - 8;
		if (!_village->isInside(xx, yy, zz)) continue;
		if (MobSpawner::isSpawnPositionOk(MobCategory::monster, level, xx, yy, zz)) return Vec3::newTemp(xx, yy, zz);
	}
	return NULL;
}