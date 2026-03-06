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
	int64_t getSeed();
	int getXSpawn();
	int getYSpawn();
	int getZSpawn();
	int64_t getGameTime();
	int64_t getDayTime();
	int64_t getSizeOnDisk();
	CompoundTag *getLoadedPlayerTag();
	wstring getLevelName();
	int getVersion();
	int64_t getLastPlayed();
	bool isThundering();
	int getThunderTime();
	bool isRaining();
	int getRainTime();
	GameType *getGameType();
	void setSeed(int64_t seed);
	void setXSpawn(int xSpawn);
	void setYSpawn(int ySpawn);
	void setZSpawn(int zSpawn);
	void setGameTime(int64_t time);
	void setDayTime(int64_t time);
	void setSizeOnDisk(int64_t sizeOnDisk);
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
