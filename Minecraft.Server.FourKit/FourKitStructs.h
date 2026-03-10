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
