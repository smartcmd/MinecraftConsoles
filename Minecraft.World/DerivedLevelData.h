#pragma once

#include "LevelData.h"

class GameRules;

class DerivedLevelData : public LevelData
{
private:
	LevelData *wrapped;

public:
	DerivedLevelData(LevelData *wrapped);

protected:
	virtual void setTagData(CompoundTag *tag); // 4J Added

public:
	CompoundTag *createTag();
	CompoundTag *createTag(vector<shared_ptr<Player> > *players);
	__int64 getSeed();
	int getXSpawn();
	int getYSpawn();
	int getZSpawn();
	__int64 getGameTime();
	__int64 getDayTime();
	__int64 getSizeOnDisk();
	CompoundTag *getLoadedPlayerTag();
	wstring getLevelName();
	int getVersion();
	__int64 getLastPlayed();
	bool isThundering();
	int getThunderTime();
	bool isRaining();
	int getRainTime();
	GameType *getGameType();
	void setSeed(__int64 seed);
	void setXSpawn(int xSpawn);
	void setYSpawn(int ySpawn);
	void setZSpawn(int zSpawn);
	void setGameTime(__int64 time);
	void setDayTime(__int64 time);
	void setSizeOnDisk(__int64 sizeOnDisk);
	void setLoadedPlayerTag(CompoundTag *loadedPlayerTag);
	void setDimension(int dimension);
	void setSpawn(int xSpawn, int ySpawn, int zSpawn);
	void setLevelName(const wstring &levelName);
	void setVersion(int version);
	void setThundering(bool thundering);
	void setThunderTime(int thunderTime);
	void setRaining(bool raining);
	void setRainTime(int rainTime);
	bool isGenerateMapFeatures();
	void setGameType(GameType *gameType);
	bool isHardcore();
	LevelType *getGenerator();
	void setGenerator(LevelType *generator);
	bool getAllowCommands();
	void setAllowCommands(bool allowCommands);
	bool isInitialized();
	void setInitialized(bool initialized);
	GameRules *getGameRules();
	int getXZSize(); // 4J Added
	int getHellScale(); // 4J Addded
};
