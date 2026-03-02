#pragma once
class LevelType;

class Abilities;
class LevelData;

// 4J Stu - Was Java enum class
class GameType
{
public:
	static GameType *NOT_SET;
	static GameType *SURVIVAL;
	static GameType *CREATIVE;
	static GameType *ADVENTURE;

	static void staticCtor();

private:
	int id;
	wstring name;

	GameType(int id, const wstring &name);

public:
	int getId();
	wstring getName();
	void updatePlayerAbilities(Abilities *abilities);
	bool isReadOnly();
	bool isCreative();
	bool isSurvival();
	static GameType *byId(int id);
	static GameType *byName(const wstring &name);
};

class LevelSettings
{
private:
	__int64 seed;
	GameType *gameType;
	bool generateMapFeatures;
	bool hardcore;
	bool newSeaLevel;
	LevelType *levelType;
	bool allowCommands;
	bool startingBonusItems;	// 4J - brought forward from 1.3.2
	int m_xzSize;	// 4J Added
	int m_hellScale;

	void _init(__int64 seed, GameType *gameType, bool generateMapFeatures, bool hardcore, bool newSeaLevel, LevelType *levelType, int xzSize, int hellScale); // 4J Added xzSize and hellScale param

public:
	LevelSettings(__int64 seed, GameType *gameType, bool generateMapFeatures, bool hardcore, bool newSeaLevel, LevelType *levelType, int xzSize, int hellScale); // 4J Added xzSize and hellScale param
	LevelSettings(LevelData *levelData);
	LevelSettings *enableStartingBonusItems();		// 4J - brought forward from 1.3.2
	LevelSettings *enableSinglePlayerCommands();
	bool hasStartingBonusItems(); // 4J - brought forward from 1.3.2
	__int64 getSeed();
	GameType *getGameType();
	bool isHardcore();
	LevelType *getLevelType();
	bool getAllowCommands();
	bool isGenerateMapFeatures();
	bool useNewSeaLevel();
	int getXZSize(); // 4J Added
	int getHellScale(); // 4J Added
	static GameType *validateGameType(int gameType);
};
