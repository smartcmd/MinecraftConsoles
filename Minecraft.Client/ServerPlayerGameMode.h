#pragma once
class ServerPlayer;
class Level;
class ItemInstance;
class ServerLevel;
class GameRulesInstance;
class GameType;

class ServerPlayerGameMode
{
public:
	Level *level;
	shared_ptr<ServerPlayer> player;

private:
	GameType *gameModeForPlayer;

private:
	bool isDestroyingBlock;
    int destroyProgressStart;
    int xDestroyBlock, yDestroyBlock, zDestroyBlock;
    int gameTicks;

    bool hasDelayedDestroy;
    int delayedDestroyX, delayedDestroyY, delayedDestroyZ;
    int delayedTickStart;
	int lastSentState;

private:
	// 4J Added
	GameRulesInstance *m_gameRules;
public:
	void setGameRules(GameRulesInstance *rules);
	GameRulesInstance *getGameRules() { return m_gameRules; }

public:
	ServerPlayerGameMode(Level *level);
	~ServerPlayerGameMode();

	void setGameModeForPlayer(GameType *gameModeForPlayer);
	GameType *getGameModeForPlayer();
	bool isSurvival();
	bool isCreative();
	void updateGameMode(GameType *gameType);

    void tick();
    void startDestroyBlock(int x, int y, int z, int face);
    void stopDestroyBlock(int x, int y, int z);
    void abortDestroyBlock(int x, int y, int z);

private:
    bool superDestroyBlock(int x, int y, int z);

public:
    bool destroyBlock(int x, int y, int z);
    bool useItem(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, bool bTestUseOnly=false);
    bool useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false, bool *pbUsedItem=nullptr);

	void setLevel(ServerLevel *newLevel);
};