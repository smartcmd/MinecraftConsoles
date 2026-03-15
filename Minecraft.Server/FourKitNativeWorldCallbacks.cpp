#include "stdafx.h"
#include "FourKitNativeInternal.h"

namespace FourKitInternal
{
	ServerLevel* GetServerLevel(int dimension)
	{
		MinecraftServer* server = MinecraftServer::getInstance();
		if (server == NULL)
		{
			return NULL;
		}

		return server->getLevel(dimension);
	}

	LevelData* GetLevelData(ServerLevel* level)
	{
		return level != NULL ? level->getLevelData() : NULL;
	}

	void BroadcastBlockUpdate(ServerLevel* level, int x, int y, int z)
	{
		if (level == NULL)
		{
			return;
		}

		MinecraftServer* server = MinecraftServer::getInstance();
		if (server == NULL)
		{
			return;
		}

		server->getPlayers()->prioritiseTileChanges(x, y, z, level->dimension->id);

		PlayerList* playerList = server->getPlayerList();
		if (playerList == NULL)
		{
			return;
		}

		for (size_t i = 0; i < playerList->players.size(); ++i)
		{
			if (playerList->players[i] && playerList->players[i]->connection)
			{
				playerList->players[i]->connection->send(shared_ptr<TileUpdatePacket>(new TileUpdatePacket(x, y, z, level)));
			}
		}
	}
}

namespace FourKit
{
	void NativeCallback_BlockBreakNaturally(int x, int y, int z, int dimension)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level != NULL)
		{
			level->destroyTile(x, y, z, true);
		}
	}

	int NativeCallback_GetBlockType(int x, int y, int z, int dimension)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level != NULL)
		{
			return level->getTile(x, y, z);
		}
		return 0;
	}

	void NativeCallback_SetBlockType(int x, int y, int z, int dimension, int id)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level != NULL)
		{
			int data = level->getData(x, y, z);
			bool result = level->setTileAndData(x, y, z, id, data, 3);
			if (result)
			{
				FourKitInternal::BroadcastBlockUpdate(level, x, y, z);
			}
		}
	}

	int NativeCallback_GetBlockData(int x, int y, int z, int dimension)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level != NULL)
		{
			return level->getData(x, y, z);
		}
		return 0;
	}

	void NativeCallback_SetBlockData(int x, int y, int z, int dimension, int data)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level != NULL)
		{
			bool result = level->setData(x, y, z, data, 3);
			if (result)
			{
				FourKitInternal::BroadcastBlockUpdate(level, x, y, z);
			}
		}
	}

	bool NativeCallback_GetWorldInfo(int dimension, WorldInfoData* outData)
	{
		if (outData == nullptr)
		{
			return false;
		}

		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		LevelData* levelData = FourKitInternal::GetLevelData(level);
		if (level == NULL || levelData == NULL)
		{
			return false;
		}

		Pos* spawn = level->getSharedSpawnPos();
		outData->dimension = dimension;
		outData->seed = level->getSeed();
		outData->fullTime = level->getGameTime();
		outData->dayTime = level->getDayTime();
		outData->spawnX = spawn != NULL ? spawn->x : 0;
		outData->spawnY = spawn != NULL ? spawn->y : 0;
		outData->spawnZ = spawn != NULL ? spawn->z : 0;
		outData->thunderDuration = levelData->getThunderTime();
		outData->weatherDuration = levelData->getRainTime();
		outData->hasStorm = levelData->isRaining();
		outData->thundering = levelData->isThundering();
		if (spawn != NULL)
		{
			delete spawn;
		}
		return true;
	}

	int NativeCallback_CreateExplosion(int dimension, double x, double y, double z, float power, int setFire, int breakBlocks)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level == NULL)
		{
			return 0;
		}

		level->explode(nullptr, x, y, z, power, setFire != 0, breakBlocks != 0);
		return 1;
	}

	bool NativeCallback_DropItem(int dimension, double x, double y, double z, int itemId, int count, int data, int naturalOffset, DroppedItemData* outData)
	{
		if (outData == nullptr || count <= 0)
		{
			return false;
		}

		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level == NULL)
		{
			return false;
		}

		shared_ptr<ItemInstance> item = shared_ptr<ItemInstance>(new ItemInstance(itemId, count, data));
		double spawnX = x;
		double spawnY = y;
		double spawnZ = z;
		shared_ptr<ItemEntity> itemEntity;

		if (naturalOffset != 0)
		{
			float xo = level->random->nextFloat() * 0.8f + 0.1f;
			float yo = level->random->nextFloat() * 0.8f + 0.1f;
			float zo = level->random->nextFloat() * 0.8f + 0.1f;
			itemEntity = shared_ptr<ItemEntity>(new ItemEntity(level, spawnX + xo, spawnY + yo, spawnZ + zo, item));
			float pow = 0.05f;
			itemEntity->xd = (float)level->random->nextGaussian() * pow;
			itemEntity->yd = (float)level->random->nextGaussian() * pow + 0.2f;
			itemEntity->zd = (float)level->random->nextGaussian() * pow;
		}
		else
		{
			itemEntity = shared_ptr<ItemEntity>(new ItemEntity(level, spawnX, spawnY, spawnZ, item));
			itemEntity->throwTime = 10;
		}

		if (!level->addEntity(itemEntity))
		{
			return false;
		}

		outData->entityId = itemEntity->entityId;
		outData->x = itemEntity->x;
		outData->y = itemEntity->y;
		outData->z = itemEntity->z;
		outData->dimension = dimension;
		outData->itemId = itemId;
		outData->count = count;
		outData->data = data;
		return true;
	}

	int NativeCallback_GetHighestBlockYAt(int dimension, int x, int z)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level == NULL)
		{
			return -1;
		}

		int top = level->getTopSolidBlock(x, z);
		return top > 0 ? (top - 1) : -1;
	}

	void NativeCallback_SetFullTime(int dimension, long long time)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level != NULL)
		{
			level->setTimeAndAdjustTileTicks(time);
		}
	}

	int NativeCallback_SetSpawnLocation(int dimension, int x, int y, int z)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level == NULL)
		{
			return 0;
		}

		level->setSpawnPos(x, y, z);
		return 1;
	}

	void NativeCallback_SetStorm(int dimension, int hasStorm)
	{
		LevelData* levelData = FourKitInternal::GetLevelData(FourKitInternal::GetServerLevel(dimension));
		if (levelData != NULL)
		{
			levelData->setRaining(hasStorm != 0);
		}
	}

	void NativeCallback_SetThunderDuration(int dimension, int duration)
	{
		LevelData* levelData = FourKitInternal::GetLevelData(FourKitInternal::GetServerLevel(dimension));
		if (levelData != NULL)
		{
			levelData->setThunderTime(duration);
		}
	}

	void NativeCallback_SetThundering(int dimension, int thundering)
	{
		LevelData* levelData = FourKitInternal::GetLevelData(FourKitInternal::GetServerLevel(dimension));
		if (levelData != NULL)
		{
			levelData->setThundering(thundering != 0);
		}
	}

	void NativeCallback_SetTime(int dimension, long long time)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level == NULL)
		{
			return;
		}

		const long long ticksPerDay = 24000;
		long long dayTime = level->getDayTime();
		long long normalized = time % ticksPerDay;
		if (normalized < 0)
		{
			normalized += ticksPerDay;
		}

		long long dayBase = dayTime - (dayTime % ticksPerDay);
		level->setDayTime(dayBase + normalized);
	}

	void NativeCallback_SetWeatherDuration(int dimension, int duration)
	{
		LevelData* levelData = FourKitInternal::GetLevelData(FourKitInternal::GetServerLevel(dimension));
		if (levelData != NULL)
		{
			levelData->setRainTime(duration);
		}
	}

	int NativeCallback_StrikeLightning(int dimension, double x, double y, double z, int effectOnly)
	{
		ServerLevel* level = FourKitInternal::GetServerLevel(dimension);
		if (level == NULL)
		{
			return 0;
		}

		shared_ptr<Entity> lightning = shared_ptr<Entity>(new LightningBolt(level, x, y, z));
		if (effectOnly != 0)
		{
			PlayerList* playerList = MinecraftServer::getPlayerList();
			if (playerList == NULL)
			{
				return 0;
			}

			playerList->broadcast(x, y, z, 512.0, dimension, shared_ptr<Packet>(new AddGlobalEntityPacket(lightning)));
			return 1;
		}

		return level->addGlobalEntity(lightning) ? 1 : 0;
	}
}
