#include "stdafx.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "BasicTypeContainers.h"
#include "Villages.h"

const wstring Villages::VILLAGE_FILE_ID = L"villages";

Villages::Villages(const wstring &id) : SavedData(id)
{
	_tick = 0;
	level = NULL;
}

Villages::Villages(Level *level) : SavedData(VILLAGE_FILE_ID)
{
	_tick = 0;
	this->level = level;
}

Villages::~Villages()
{
	for(AUTO_VAR(it,queries.begin()); it != queries.end(); ++it) delete *it;
}

void Villages::setLevel(Level *level)
{
	this->level = level;

	//for (Village village : villages)
	for(AUTO_VAR(it, villages.begin()); it != villages.end(); ++it)
	{
		shared_ptr<Village> village = *it;
		village->setLevel(level);
	}
}

void Villages::queryUpdateAround(int x, int y, int z)
{
	if (queries.size() > 64) return;
	if (!hasQuery(x, y, z)) queries.push_back(new Pos(x, y, z));
}

void Villages::tick()
{
	++_tick;
	//for (Village village : villages)
	for(AUTO_VAR(it, villages.begin()); it != villages.end(); ++it)
	{
		shared_ptr<Village> village = *it;
		village->tick(_tick);
	}
	removeVillages();
	processNextQuery();
	cluster();

	if ((_tick % 400) == 0)
	{
		setDirty();
	}
}

void Villages::removeVillages()
{
	//for (Iterator<Village> it = villages.iterator(); it.hasNext();)
	for(AUTO_VAR(it, villages.begin()); it != villages.end(); )
	{
		shared_ptr<Village> village = *it; //it.next();
		if (village->canRemove())
		{
			it = villages.erase(it);
			//it.remove();
			setDirty();
		}
		else
		{
			++it;
		}
	}
}

vector<shared_ptr<Village> > *Villages::getVillages()
{
	return &villages;
}

shared_ptr<Village> Villages::getClosestVillage(int x, int y, int z, int maxDist)
{
	shared_ptr<Village> closest = nullptr;
	float closestDistSqr = Float::MAX_VALUE;
	//for (Village village : villages)
	for(AUTO_VAR(it, villages.begin()); it != villages.end(); ++it)
	{
		shared_ptr<Village> village = *it;
		float distSqr = village->getCenter()->distSqr(x, y, z);
		if (distSqr >= closestDistSqr) continue;

		float requiredDist = maxDist + village->getRadius();
		if (distSqr > requiredDist * requiredDist) continue;

		closest = village;
		closestDistSqr = distSqr;
	}
	return closest;
}

void Villages::processNextQuery()
{
	if (queries.empty()) return;
	Pos *q = queries.front();
	queries.pop_front();
	addDoorInfos(q);
	delete q;
}

void Villages::cluster()
{
	// note doesn't merge or split existing villages
	//for (int i = 0; i < unclustered.size(); ++i)
	for(AUTO_VAR(it, unclustered.begin()); it != unclustered.end(); ++it)
	{
		shared_ptr<DoorInfo> di = *it; //unclustered.get(i);

		bool found = false;
		//for (Village village : villages)
		for(AUTO_VAR(itV, villages.begin()); itV != villages.end(); ++itV)
		{
			shared_ptr<Village> village = *itV;
			int dist = (int) village->getCenter()->distSqr(di->x, di->y, di->z);
			int radius = MaxDoorDist + village->getRadius();
			if (dist > radius * radius) continue;
			village->addDoorInfo(di);
			found = true;
			break;
		}
		if (found) continue;

		// create new Village
		shared_ptr<Village> village = shared_ptr<Village>(new Village(level));
		village->addDoorInfo(di);
		villages.push_back(village);
		setDirty();
	}
	unclustered.clear();
}

void Villages::addDoorInfos(Pos *pos)
{
	int scanX = 16, scanY = 4, scanZ = 16;
	for (int xx = pos->x - scanX; xx < pos->x + scanX; xx++)
	{
		for (int yy = pos->y - scanY; yy < pos->y + scanY; yy++)
		{
			for (int zz = pos->z - scanZ; zz < pos->z + scanZ; zz++)
			{
				if (isDoor(xx, yy, zz))
				{
					shared_ptr<DoorInfo> currentDoor = getDoorInfo(xx, yy, zz);
					if (currentDoor == NULL) createDoorInfo(xx, yy, zz);
					else currentDoor->timeStamp = _tick;
				}
			}
		}
	}
}

shared_ptr<DoorInfo> Villages::getDoorInfo(int x, int y, int z)
{
	//for (DoorInfo di : unclustered)
	for(AUTO_VAR(it,unclustered.begin()); it != unclustered.end(); ++it)
	{
		shared_ptr<DoorInfo> di = *it;
		if (di->x == x && di->z == z && abs(di->y - y) <= 1) return di;
	}
	//for (Village v : villages)
	for(AUTO_VAR(it,villages.begin()); it != villages.end(); ++it)
	{
		shared_ptr<Village> v = *it;
		shared_ptr<DoorInfo> di = v->getDoorInfo(x, y, z);
		if (di != NULL) return di;
	}
	return nullptr;
}

void Villages::createDoorInfo(int x, int y, int z)
{
	int dir = ((DoorTile *) Tile::door_wood)->getDir(level, x, y, z);
	if (dir == 0 || dir == 2)
	{
		int canSeeX = 0;
		for (int i = -5; i < 0; ++i)
			if (level->canSeeSky(x + i, y, z)) canSeeX--;
		for (int i = 1; i <= 5; ++i)
			if (level->canSeeSky(x + i, y, z)) canSeeX++;
		if (canSeeX != 0) unclustered.push_back(shared_ptr<DoorInfo>(new DoorInfo(x, y, z, canSeeX > 0 ? -2 : 2, 0, _tick)));
	}
	else
	{
		int canSeeZ = 0;
		for (int i = -5; i < 0; ++i)
			if (level->canSeeSky(x, y, z + i)) canSeeZ--;
		for (int i = 1; i <= 5; ++i)
			if (level->canSeeSky(x, y, z + i)) canSeeZ++;
		if (canSeeZ != 0) unclustered.push_back(shared_ptr<DoorInfo>(new DoorInfo(x, y, z, 0, canSeeZ > 0 ? -2 : 2, _tick)));
	}
}

bool Villages::hasQuery(int x, int y, int z)
{
	//for (Pos pos : queries)
	for(AUTO_VAR(it, queries.begin()); it != queries.end(); ++it)
	{
		Pos *pos = *it;
		if (pos->x == x && pos->y == y && pos->z == z) return true;
	}
	return false;
}

bool Villages::isDoor(int x, int y, int z)
{
	int tileId = level->getTile(x, y, z);
	return tileId == Tile::door_wood_Id;
}

void Villages::load(CompoundTag *tag)
{
	_tick = tag->getInt(L"Tick");
	ListTag<CompoundTag> *villageTags = (ListTag<CompoundTag> *) tag->getList(L"Villages");
	for (int i = 0; i < villageTags->size(); i++)
	{
		CompoundTag *compoundTag = villageTags->get(i);
		shared_ptr<Village> village = shared_ptr<Village>(new Village());
		village->readAdditionalSaveData(compoundTag);
		villages.push_back(village);
	}
}

void Villages::save(CompoundTag *tag)
{
	tag->putInt(L"Tick", _tick);
	ListTag<CompoundTag> *villageTags = new ListTag<CompoundTag>(L"Villages");
	//for (Village village : villages)
	for(AUTO_VAR(it, villages.begin()); it != villages.end(); ++it)
	{
		shared_ptr<Village> village = *it;
		CompoundTag *villageTag = new CompoundTag(L"Village");
		village->addAdditonalSaveData(villageTag);
		villageTags->add(villageTag);
	}
	tag->put(L"Villages", villageTags);
}