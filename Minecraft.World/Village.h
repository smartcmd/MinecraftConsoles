#pragma once

class Village
{
private:
	Level *level;
	vector<shared_ptr<DoorInfo> > doorInfos;

	Pos *accCenter;
	Pos *center;
	int radius;
	int stableSince;
	int _tick;
	int populationSize;
	int noBreedTimer;

	unordered_map<wstring, int> playerStanding;

	class Aggressor
	{
	public:
		shared_ptr<LivingEntity> mob;
		int timeStamp;

		Aggressor(shared_ptr<LivingEntity> mob, int timeStamp);
	};

	vector<Aggressor *> aggressors;
	int golemCount;

public:
	Village();
	Village(Level *level);
	~Village();

	void setLevel(Level *level);

	void tick(int tick);

private:
	Vec3 *findRandomSpawnPos(int x, int y, int z, int sx, int sy, int sz);
	bool canSpawnAt(int x, int y, int z, int sx, int sy, int sz);
	void countGolem();
	void countPopulation();

public:
	Pos *getCenter();
	int getRadius();
	int getDoorCount();
	int getStableAge();
	int getPopulationSize();
	bool isInside(int xx, int yy, int zz);
	vector<shared_ptr<DoorInfo> > *getDoorInfos();
	shared_ptr<DoorInfo> getClosestDoorInfo(int x, int y, int z);
	shared_ptr<DoorInfo> getBestDoorInfo(int x, int y, int z);
	bool hasDoorInfo(int x, int y, int z);
	shared_ptr<DoorInfo> getDoorInfo(int x, int y, int z);
	void addDoorInfo(shared_ptr<DoorInfo> di);
	bool canRemove();
	void addAggressor(shared_ptr<LivingEntity> mob);
	shared_ptr<LivingEntity> getClosestAggressor(shared_ptr<LivingEntity> from);
	shared_ptr<Player> getClosestBadStandingPlayer(shared_ptr<LivingEntity> from);

private:
	void updateAggressors();
	void updateDoors();
	bool isDoor(int x, int y, int z);
	void calcInfo();

public:
	int getStanding(const wstring &playerName);
	int modifyStanding(const wstring &playerName, int delta);
	bool isGoodStanding(const wstring &playerName);
	bool isBadStanding(const wstring &playerName);
	bool isVeryBadStanding(const wstring playerName);
	void readAdditionalSaveData(CompoundTag *tag);
	void addAdditonalSaveData(CompoundTag *tag);
	void resetNoBreedTimer();
	bool isBreedTimerOk();
	void rewardAllPlayers(int amount);
};