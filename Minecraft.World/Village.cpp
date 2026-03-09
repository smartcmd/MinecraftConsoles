#include "stdafx.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "BasicTypeContainers.h"
#include "Village.h"

Village::Aggressor::Aggressor(shared_ptr<LivingEntity> mob, int timeStamp)
{
	this->mob = mob;
	this->timeStamp = timeStamp;
}

Village::Village()
{
	accCenter = new Pos(0, 0, 0);
	center = new Pos(0, 0, 0);
	radius = 0;
	stableSince = 0;
	_tick = 0;
	populationSize = 0;
	golemCount = 0;
	noBreedTimer = 0;

	level = nullptr;
}

Village::Village(Level *level)
{
	accCenter = new Pos(0, 0, 0);
	center = new Pos(0, 0, 0);
	radius = 0;
	stableSince = 0;
	_tick = 0;
	populationSize = 0;
	golemCount = 0;
	noBreedTimer = 0;

	this->level = level;
}

Village::~Village()
{
	delete accCenter;
	delete center;
	for(auto& aggressor : aggressors)
	{
		delete aggressor;
	}
}

void Village::setLevel(Level *level)
{
	this->level = level;
}

void Village::tick(int tick)
{
	this->_tick = tick;
	updateDoors();
	updateAggressors();
	if (tick % 20 == 0) countPopulation();
	if (tick % 30 == 0) countGolem();

	int idealGolemCount = populationSize / 10;
	if (golemCount < idealGolemCount && doorInfos.size() > 20 && level->random->nextInt(7000) == 0)
	{
		Vec3 *spawnPos = findRandomSpawnPos(center->x, center->y, center->z, 2, 4, 2);
		if (spawnPos != nullptr)
		{
			shared_ptr<VillagerGolem> vg = std::make_shared<VillagerGolem>(level);
			vg->setPos(spawnPos->x, spawnPos->y, spawnPos->z);
			level->addEntity(vg);
			++golemCount;
		}
	}

	// 4J - All commented out in java
	//        for (DoorInfo di : doorInfos) {
	//            level.addParticle("heart", di.getIndoorX() + 0.5, di.getIndoorY() + .5f, di.getIndoorZ() + 0.5, 0, 1, 0);
	//        }
	//
	//        for (int i = 0; i < 8; ++i)
	//            for (int j = 0; j < 8; ++j)
	//                level.addParticle("heart", center.x + 0.5 + i, center.y + .5f, center.z + 0.5 + j, 0, 1, 0);
	//        for (float i = 0; i < Math.PI * 2; i += 0.1) {
	//            int x = center.x + (int) (Math.cos(i) * radius);
	//            int z = center.z + (int) (Math.sin(i) * radius);
	//            level.addParticle("heart", x, center.y + .5f, z, 0, 1, 0);
	//        }
}

Vec3 *Village::findRandomSpawnPos(int x, int y, int z, int sx, int sy, int sz)
{
	for (int i = 0; i < 10; ++i)
	{
		int xx = x + level->random->nextInt(16) - 8;
		int yy = y + level->random->nextInt(6) - 3;
		int zz = z + level->random->nextInt(16) - 8;
		if (!isInside(xx, yy, zz)) continue;
		if (canSpawnAt(xx, yy, zz, sx, sy, sz)) return Vec3::newTemp(xx, yy, zz);
	}
	return nullptr;
}

bool Village::canSpawnAt(int x, int y, int z, int sx, int sy, int sz)
{
	if (!level->isTopSolidBlocking(x, y - 1, z)) return false;

	int startX = x - sx / 2;
	int startZ = z - sz / 2;
	for (int xx = startX; xx < startX + sx; xx++)
		for (int yy = y; yy < y + sy; yy++)
			for (int zz = startZ; zz < startZ + sz; zz++)
				if (level->isSolidBlockingTile(xx, yy, zz)) return false;

	return true;
}

void Village::countGolem()
{
	// Fix - let bots report themselves?
	vector<shared_ptr<Entity> > *golems = level->getEntitiesOfClass(typeid(VillagerGolem), AABB::newTemp(center->x - radius, center->y - 4, center->z - radius, center->x + radius, center->y + 4, center->z + radius));
	golemCount = golems->size();
	delete golems;
}

void Village::countPopulation()
{
	vector<shared_ptr<Entity> > *villagers = level->getEntitiesOfClass(typeid(Villager), AABB::newTemp(center->x - radius, center->y - 4, center->z - radius, center->x + radius, center->y + 4, center->z + radius));
	populationSize = villagers->size();
	delete villagers;

	if (populationSize == 0)
	{
		// forget standing
		playerStanding.clear();
	}
}

Pos *Village::getCenter()
{
	return center;
}

int Village::getRadius()
{
	return radius;
}

int Village::getDoorCount()
{
	return doorInfos.size();
}

int Village::getStableAge()
{
	return _tick - stableSince;
}

int Village::getPopulationSize()
{
	return populationSize;
}

bool Village::isInside(int xx, int yy, int zz)
{
	return center->distSqr(xx, yy, zz) < radius * radius;
}

vector<shared_ptr<DoorInfo> > *Village::getDoorInfos()
{
	return &doorInfos;
}

shared_ptr<DoorInfo> Village::getClosestDoorInfo(int x, int y, int z)
{
	shared_ptr<DoorInfo> closest = nullptr;
	int closestDistSqr = Integer::MAX_VALUE;
	//for (DoorInfo dm : doorInfos)
	for(auto& dm : doorInfos)
	{
		int distSqr = dm->distanceToSqr(x, y, z);
		if (distSqr < closestDistSqr)
		{
			closest = dm;
			closestDistSqr = distSqr;
		}
	}
	return closest;
}

shared_ptr<DoorInfo>Village::getBestDoorInfo(int x, int y, int z)
{
	shared_ptr<DoorInfo> closest = nullptr;
	int closestDist = Integer::MAX_VALUE;
	//for (DoorInfo dm : doorInfos)
	for(auto& dm : doorInfos)
	{
		int distSqr = dm->distanceToSqr(x, y, z);
		if (distSqr > 16 * 16) distSqr *= 1000;
		else distSqr = dm->getBookingsCount();

		if (distSqr < closestDist)
		{
			closest = dm;
			closestDist = distSqr;
		}
	}
	return closest;
}

bool Village::hasDoorInfo(int x, int y, int z)
{
	return getDoorInfo(x, y, z) != nullptr;
}

shared_ptr<DoorInfo> Village::getDoorInfo(int x, int y, int z)
{
	if (center->distSqr(x, y, z) > radius * radius) return nullptr;
	for( auto& di : doorInfos)
	{
		if (di->x == x && di->z == z && abs(di->y - y) <= 1)
			return di;
	}
	return nullptr;
}

void Village::addDoorInfo(shared_ptr<DoorInfo> di)
{
	doorInfos.push_back(di);
	accCenter->x += di->x;
	accCenter->y += di->y;
	accCenter->z += di->z;
	calcInfo();
	stableSince = di->timeStamp;
}

bool Village::canRemove()
{
	return doorInfos.empty();
}

void Village::addAggressor(shared_ptr<LivingEntity> mob)
{
	for(auto& a : aggressors)
	{
		if (a->mob == mob)
		{
			a->timeStamp = _tick;
			return;
		}
	}
	aggressors.push_back(new Aggressor(mob, _tick));
}

shared_ptr<LivingEntity> Village::getClosestAggressor(shared_ptr<LivingEntity> from)
{
	double closestSqr = Double::MAX_VALUE;
	Aggressor *closest = nullptr;
	for(auto& a : aggressors)
	{
		double distSqr = a->mob->distanceToSqr(from);
		if (distSqr > closestSqr) continue;
		closest = a;
		closestSqr = distSqr;
	}
	return closest != nullptr ? closest->mob : nullptr;
}

shared_ptr<Player> Village::getClosestBadStandingPlayer(shared_ptr<LivingEntity> from)
{
	double closestSqr = Double::MAX_VALUE;
	shared_ptr<Player> closest = nullptr;

	for(auto& it : playerStanding)
	{
		wstring player = it.first;
		if (isVeryBadStanding(player))
		{
			shared_ptr<Player> mob = level->getPlayerByName(player);
			if (mob != nullptr)
			{
				double distSqr = mob->distanceToSqr(from);
				if (distSqr > closestSqr) continue;
				closest = mob;
				closestSqr = distSqr;
			}
		}
	}

	return closest;
}

void Village::updateAggressors()
{
    for (auto it = aggressors.begin(); it != aggressors.end();)
    {
		Aggressor *a = *it; //it.next();
		if (!a->mob->isAlive() || abs(_tick - a->timeStamp) > 300)
		{
			delete *it;
			it = aggressors.erase(it);
			//it.remove();
		}
		else
		{
			++it;
		}
	}
}

void Village::updateDoors()
{
	bool removed = false;
	bool resetBookings = level->random->nextInt(50) == 0;
	//for (Iterator<DoorInfo> it = doorInfos.iterator(); it.hasNext();)
    for (auto it = doorInfos.begin(); it != doorInfos.end();)
    {
		shared_ptr<DoorInfo> dm = *it; //it.next();
		if (resetBookings) dm->resetBookingCount();
		if (!isDoor(dm->x, dm->y, dm->z) || abs(_tick - dm->timeStamp) > 1200)
		{
			accCenter->x -= dm->x;
			accCenter->y -= dm->y;
			accCenter->z -= dm->z;
			removed = true;
			dm->removed = true;

			it = doorInfos.erase(it);
			//it.remove();
		}
		else
		{
			++it;
		}
	}

	if (removed) calcInfo();
}

bool Village::isDoor(int x, int y, int z)
{
	int tileId = level->getTile(x, y, z);
	if (tileId <= 0) return false;
	return tileId == Tile::door_wood_Id;
}

void Village::calcInfo()
{
	int s = doorInfos.size();
	if (s == 0)
	{
		center->set(0, 0, 0);
		radius = 0;
		return;
	}
	center->set(accCenter->x / s, accCenter->y / s, accCenter->z / s);
	int maxRadiusSqr = 0;
	//for (DoorInfo dm : doorInfos)
	for(auto& dm : doorInfos)
	{
		maxRadiusSqr = std::max<int>(dm->distanceToSqr(center->x, center->y, center->z), maxRadiusSqr);
	}
	int doorDist= Villages::MaxDoorDist;			// Take into local int for PS4 as max takes a reference to the const int there and then needs the value to exist for the linker
	radius = max(doorDist, (int) sqrt((float)maxRadiusSqr) + 1);
}

int Village::getStanding(const wstring &playerName)
{
    auto it = playerStanding.find(playerName);
    if (it != playerStanding.end())
	{
		return it->second;
	}
	return 0;
}

int Village::modifyStanding(const wstring &playerName, int delta)
{
	int current = getStanding(playerName);
	int newValue = Mth::clamp(current + delta, -30, 10);
	playerStanding.insert(pair<wstring,int>(playerName, newValue));
	return newValue;
}

bool Village::isGoodStanding(const wstring &playerName)
{
	return getStanding(playerName) >= 0;
}

bool Village::isBadStanding(const wstring &playerName)
{
	return getStanding(playerName) <= -5;
}

bool Village::isVeryBadStanding(const wstring playerName)
{
	return getStanding(playerName) <= -15;
}

void Village::readAdditionalSaveData(CompoundTag *tag)
{
	populationSize = tag->getInt(L"PopSize");
	radius = tag->getInt(L"Radius");
	golemCount = tag->getInt(L"Golems");
	stableSince = tag->getInt(L"Stable");
	_tick = tag->getInt(L"Tick");
	noBreedTimer = tag->getInt(L"MTick");
	center->x = tag->getInt(L"CX");
	center->y = tag->getInt(L"CY");
	center->z = tag->getInt(L"CZ");
	accCenter->x = tag->getInt(L"ACX");
	accCenter->y = tag->getInt(L"ACY");
	accCenter->z = tag->getInt(L"ACZ");

	ListTag<CompoundTag> *doorTags = (ListTag<CompoundTag> *) tag->getList(L"Doors");
	for (int i = 0; i < doorTags->size(); i++)
	{
		CompoundTag *dTag = doorTags->get(i);

		shared_ptr<DoorInfo> door = std::make_shared<DoorInfo>(dTag->getInt(L"X"), dTag->getInt(L"Y"), dTag->getInt(L"Z"), dTag->getInt(L"IDX"), dTag->getInt(L"IDZ"), dTag->getInt(L"TS"));
		doorInfos.push_back(door);
	}

	ListTag<CompoundTag> *playerTags = (ListTag<CompoundTag> *) tag->getList(L"Players");
	for (int i = 0; i < playerTags->size(); i++)
	{
		CompoundTag *pTag = playerTags->get(i);
		playerStanding.insert(pair<wstring,int>(pTag->getString(L"Name"), pTag->getInt(L"S")));
	}
}

void Village::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putInt(L"PopSize", populationSize);
	tag->putInt(L"Radius", radius);
	tag->putInt(L"Golems", golemCount);
	tag->putInt(L"Stable", stableSince);
	tag->putInt(L"Tick", _tick);
	tag->putInt(L"MTick", noBreedTimer);
	tag->putInt(L"CX", center->x);
	tag->putInt(L"CY", center->y);
	tag->putInt(L"CZ", center->z);
	tag->putInt(L"ACX", accCenter->x);
	tag->putInt(L"ACY", accCenter->y);
	tag->putInt(L"ACZ", accCenter->z);

	ListTag<CompoundTag> *doorTags = new ListTag<CompoundTag>(L"Doors");
	//for (DoorInfo dm : doorInfos)
	for(auto& dm : doorInfos)
	{
		CompoundTag *doorTag = new CompoundTag(L"Door");
		doorTag->putInt(L"X", dm->x);
		doorTag->putInt(L"Y", dm->y);
		doorTag->putInt(L"Z", dm->z);
		doorTag->putInt(L"IDX", dm->insideDx);
		doorTag->putInt(L"IDZ", dm->insideDz);
		doorTag->putInt(L"TS", dm->timeStamp);
		doorTags->add(doorTag);
	}
	tag->put(L"Doors", doorTags);

	ListTag<CompoundTag> *playerTags = new ListTag<CompoundTag>(L"Players");
	//for (String player : playerStanding.keySet())
	for(auto& it : playerStanding)
	{
		wstring player = it.first;
		CompoundTag *playerTag = new CompoundTag(player);
		playerTag->putString(L"Name", player);
		playerTag->putInt(L"S", it.second);
		playerTags->add(playerTag);
	}
	tag->put(L"Players", playerTags);

}

void Village::resetNoBreedTimer()
{
	noBreedTimer = _tick;
}

bool Village::isBreedTimerOk()
{
	// prevent new villagers if a villager was killed by a mob within 3
	// minutes
	return noBreedTimer == 0 || (_tick - noBreedTimer) >= (SharedConstants::TICKS_PER_SECOND * 60 * 3);
}

void Village::rewardAllPlayers(int amount)
{
	for(auto& it : playerStanding)
	{
		modifyStanding(it.first, amount);
	}
}