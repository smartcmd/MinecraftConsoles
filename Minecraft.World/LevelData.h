#pragma once
using namespace std;

class Player;
class CompoundTag;
class LevelSettings;
class LevelType;
class GameType;

class LevelData
{
	friend class DerivedLevelData;
private:
	__int64 seed;
	LevelType *m_pGenerator;// = LevelType.normal;
    int xSpawn;
    int ySpawn;
    int zSpawn;
    __int64 time;
    __int64 lastPlayed;
    __int64 sizeOnDisk;
//    CompoundTag *loadedPlayerTag;	// 4J removed
    int dimension;
    wstring levelName;
    int version;

    bool raining;
    int rainTime;

    bool thundering;
    int thunderTime;
	GameType *gameType;
	bool generateMapFeatures;
	bool hardcore;
	bool allowCommands;
	bool initialized;
	bool newSeaLevel;		// 4J added
	bool hasBeenInCreative; // 4J added
	bool spawnBonusChest;   // 4J added
	int m_xzSize;	// 4J Added
	int m_hellScale; // 4J Added

	// 4J added
	int xStronghold;
	int yStronghold;
	int zStronghold;
	bool bStronghold;

	int xStrongholdEndPortal;
	int zStrongholdEndPortal;
	bool bStrongholdEndPortal;

protected:
	LevelData();

public:
	LevelData(CompoundTag *tag);
    LevelData(LevelSettings *levelSettings, const wstring& levelName);
    LevelData(LevelData *copy);
    CompoundTag *createTag();
    CompoundTag *createTag(vector<shared_ptr<Player> > *players);

	enum
	{
		DIMENSION_NETHER=-1,
		DIMENSION_OVERWORLD=0,
		DIMENSION_END=1
	};

protected:
	virtual void setTagData(CompoundTag *tag);		// 4J - removed  CompoundTag *playerTag

public:
	virtual  __int64 getSeed();
	virtual  int getXSpawn();
	virtual  int getYSpawn();
	virtual  int getZSpawn();
	virtual  int getXStronghold();
	virtual  int getZStronghold();
	virtual  int getXStrongholdEndPortal();
	virtual  int getZStrongholdEndPortal();
	virtual  __int64 getTime();
    virtual  __int64 getSizeOnDisk();
    virtual  CompoundTag *getLoadedPlayerTag();
    //int getDimension(); // 4J Removed TU 9 as it's never accurate
    virtual  void setSeed(__int64 seed);
    virtual  void setXSpawn(int xSpawn);
    virtual  void setYSpawn(int ySpawn);
    virtual  void setZSpawn(int zSpawn);
	virtual  void setHasStronghold();
	virtual  bool getHasStronghold();
	virtual  void setXStronghold(int xStronghold);
	virtual  void setZStronghold(int zStronghold);
	virtual  void setHasStrongholdEndPortal();
	virtual  bool getHasStrongholdEndPortal();
	virtual  void setXStrongholdEndPortal(int xStrongholdEndPortal);
	virtual  void setZStrongholdEndPortal(int zStrongholdEndPortal);

    virtual  void setTime(__int64 time);
    virtual  void setSizeOnDisk(__int64 sizeOnDisk);
    virtual  void setLoadedPlayerTag(CompoundTag *loadedPlayerTag);
    //void setDimension(int dimension); // 4J Removed TU 9 as it's never used
    virtual  void setSpawn(int xSpawn, int ySpawn, int zSpawn);
    virtual  wstring getLevelName();
    virtual  void setLevelName(const wstring& levelName);
    virtual  int getVersion();
    virtual  void setVersion(int version);
    virtual  __int64 getLastPlayed();
    virtual  bool isThundering();
    virtual  void setThundering(bool thundering);
    virtual  int getThunderTime();
    virtual  void setThunderTime(int thunderTime);
    virtual  bool isRaining();
    virtual  void setRaining(bool raining);
    virtual  int getRainTime();
    virtual  void setRainTime(int rainTime);
	virtual  GameType *getGameType();
	virtual  bool isGenerateMapFeatures();
	virtual  bool getSpawnBonusChest();
	virtual  void setGameType(GameType *gameType);
	virtual  bool useNewSeaLevel();
	virtual  bool getHasBeenInCreative(); // 4J Added
	virtual  void setHasBeenInCreative(bool value); // 4J Added
	virtual  LevelType *getGenerator();
	virtual  void setGenerator(LevelType *generator);
	virtual  bool isHardcore();
	virtual  bool getAllowCommands();
	virtual  void setAllowCommands(bool allowCommands);
	virtual  bool isInitialized();
	virtual  void setInitialized(bool initialized);
	virtual  int getXZSize(); // 4J Added
	virtual  int getHellScale(); // 4J Addded
};
