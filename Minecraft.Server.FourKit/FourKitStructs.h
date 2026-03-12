#pragma once

struct PlayerJoinData
{
	const char* playerName;
	
	float health;
	int food;
	float fallDistance;
	float yRot;
	float xRot;
	bool sneaking;
	bool sprinting;
	
	double x;
	double y;
	double z;
	
	int dimension;
	
	PlayerJoinData()
		: playerName(nullptr)
		, health(20.0f)
		, food(20)
		, fallDistance(0.0f)
		, yRot(0.0f)
		, xRot(0.0f)
		, sneaking(false)
		, sprinting(false)
		, x(0.0)
		, y(0.0)
		, z(0.0)
		, dimension(0)
	{
	}
};

struct PlayerLeaveData
{
	const char* playerName;
	
	PlayerLeaveData()
		: playerName(nullptr)
	{
	}
};

struct PlayerChatData
{
	const char* playerName;
	const char* message;

	PlayerChatData()
		: playerName(nullptr)
		, message(nullptr)
	{
	}
};

struct BlockBreakData
{
	const char* playerName;
	int x;
	int y;
	int z;
	int blockId;
	int blockData;

	BlockBreakData()
		: playerName(nullptr)
		, x(0)
		, y(0)
		, z(0)
		, blockId(0)
		, blockData(0)
	{
	}
};

struct BlockPlaceData
{
	const char* playerName;
	int x;
	int y;
	int z;
	int blockId;
	int blockData;

	BlockPlaceData()
		: playerName(nullptr)
		, x(0)
		, y(0)
		, z(0)
		, blockId(0)
		, blockData(0)
	{
	}
};

struct PlayerMoveData
{
	const char* playerName;
	double fromX;
	double fromY;
	double fromZ;
	double toX;
	double toY;
	double toZ;

	PlayerMoveData()
		: playerName(nullptr)
		, fromX(0.0)
		, fromY(0.0)
		, fromZ(0.0)
		, toX(0.0)
		, toY(0.0)
		, toZ(0.0)
	{
	}
};

struct PlayerNetworkAddressData
{
	char hostName[256];
	char hostString[256];
	char hostAddress[64];
	int port;
	bool unresolved;

	PlayerNetworkAddressData()
		: port(0)
		, unresolved(true)
	{
		hostName[0] = '\0';
		hostString[0] = '\0';
		hostAddress[0] = '\0';
	}
};

struct PlayerPortalData
{
	const char* playerName;
	int cause;
	double fromX;
	double fromY;
	double fromZ;
	double toX;
	double toY;
	double toZ;

	PlayerPortalData()
		: playerName(nullptr)
		, cause(0)
		, fromX(0.0)
		, fromY(0.0)
		, fromZ(0.0)
		, toX(0.0)
		, toY(0.0)
		, toZ(0.0)
	{
	}
};

struct SignChangeData
{
	const char* playerName;
	int x;
	int y;
	int z;
	int dimension;
	char lines[4][256];

	SignChangeData()
		: playerName(nullptr)
		, x(0)
		, y(0)
		, z(0)
		, dimension(0)
	{
		for (int i = 0; i < 4; ++i)
		{
			lines[i][0] = '\0';
		}
	}
};

struct PlayerInteractData
{
	const char* playerName;
	int action;
    int blockFace;
    bool hasBlock;
    bool hasItem;
	int x;
	int y;
	int z;
	int dimension;
	int blockId;
	int blockData;

	PlayerInteractData()
		: playerName(nullptr)
		, action(0)
		, blockFace(0)
		, hasBlock(false)
		, hasItem(false)
		, x(0)
		, y(0)
		, z(0)
		, dimension(0)
		, blockId(0)
		, blockData(0)
	{
	}
};

struct PlayerDeathData
{
	const char* playerName;
	char deathMessage[512];
	bool keepInventory;
	bool keepLevel;
	int newExp;
	int newLevel;
	int newTotalExp;

	PlayerDeathData()
		: playerName(nullptr)
		, keepInventory(false)
		, keepLevel(false)
		, newExp(0)
		, newLevel(0)
		, newTotalExp(0)
	{
		deathMessage[0] = '\0';
	}
};

struct WorldInfoData
{
	int dimension;
	long long seed;
	long long fullTime;
	long long dayTime;
	int spawnX;
	int spawnY;
	int spawnZ;
	int thunderDuration;
	int weatherDuration;
	bool hasStorm;
	bool thundering;

	WorldInfoData()
		: dimension(0)
		, seed(0)
		, fullTime(0)
		, dayTime(0)
		, spawnX(0)
		, spawnY(0)
		, spawnZ(0)
		, thunderDuration(0)
		, weatherDuration(0)
		, hasStorm(false)
		, thundering(false)
	{
	}
};

struct ItemInHandData
{
	int itemId;
	int count;
	int data;
	bool hasItem;

	ItemInHandData()
		: itemId(0)
		, count(0)
		, data(0)
		, hasItem(false)
	{
	}
};

struct PlayerDropItemData
{
	const char* playerName;
	int itemId;
	int itemCount;
	int itemData;
	int pickupDelay;

	PlayerDropItemData()
		: playerName(nullptr)
		, itemId(0)
		, itemCount(0)
		, itemData(0)
		, pickupDelay(40)
	{
	}
};

struct DroppedItemData
{
	int entityId;
	double x;
	double y;
	double z;
	int dimension;
	int itemId;
	int count;
	int data;

	DroppedItemData()
		: entityId(-1)
		, x(0.0)
		, y(0.0)
		, z(0.0)
		, dimension(0)
		, itemId(0)
		, count(0)
		, data(0)
	{
	}
};
