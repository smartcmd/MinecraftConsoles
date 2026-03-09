#include "stdafx.h"
#include "Biome.h"
#include "NetherBridgeFeature.h"
#include "NetherBridgePieces.h"
#include "MobSpawner.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"


NetherBridgeFeature::NetherBridgeFeature() : StructureFeature()
{
	bridgeEnemies.push_back(new Biome::MobSpawnerData(eTYPE_BLAZE, 10, 2, 3));
	bridgeEnemies.push_back(new Biome::MobSpawnerData(eTYPE_PIGZOMBIE, 5, 4, 4));
	bridgeEnemies.push_back(new Biome::MobSpawnerData(eTYPE_SKELETON, 10, 4, 4));
	bridgeEnemies.push_back(new Biome::MobSpawnerData(eTYPE_LAVASLIME, 3, 4, 4));
	isSpotSelected=false;
	netherFortressPos = nullptr;

}

NetherBridgeFeature::~NetherBridgeFeature()
{
	if( netherFortressPos != nullptr ) delete netherFortressPos;
}

wstring NetherBridgeFeature::getFeatureName()
{
	return L"Fortress";
}

vector<Biome::MobSpawnerData *> *NetherBridgeFeature::getBridgeEnemies()
{
	return &bridgeEnemies;
}

bool NetherBridgeFeature::isFeatureChunk(int x, int z, bool bIsSuperflat)
{
	// 4J Stu - New implementation to force a nether fortress
	if (!isSpotSelected)
	{
		// Set the random
		random->setSeed(level->getSeed());
		random->nextInt();

		// Due to our nether size we want to accept chunks in the range [(-3,-3),(3,3)] (7x7). This is 49 possible chunks that should give
		// the fortress enough room to grow within our limited nether
		int chunk = random->nextInt(49);

		int xCoord = chunk % 7;
		int zCoord = chunk / 7;

		netherFortressPos = new ChunkPos(xCoord, zCoord);

		isSpotSelected = true;
	}

	bool forcePlacement = false;
	LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
	if( levelGenOptions != nullptr )
	{
		forcePlacement = levelGenOptions->isFeatureChunk(x,z,eFeature_NetherBridge);
	}

	if(forcePlacement || (x == netherFortressPos->x && z == netherFortressPos->z) ) return true;

#ifdef _LARGE_WORLDS
	int xzSize = level->dimension->getXZSize();
	if(xzSize > 30)
	{
		// For large worlds, lets allow the PC version of the spawning to place nether fortresses (plus the one we forced above)
		int cx = x >> 4;
		int cz = z >> 4;

		random->setSeed(cx ^ (cz << 4) ^ level->getSeed());
		random->nextInt();

		if (random->nextInt(3) != 0)
		{
			return false;
		}
		if (x != ((cx << 4) + 4 + random->nextInt(8)))
		{
			return false;
		}
		if (z != ((cz << 4) + 4 + random->nextInt(8)))
		{
			return false;
		}
		return true;
	}
#endif

	return false;
}

StructureStart *NetherBridgeFeature::createStructureStart(int x, int z)
{
	return new NetherBridgeStart(level, random, x, z);
}

void NetherBridgeFeature::clearCachedBuildings()
{
	cachedStructures.clear();
}

NetherBridgeFeature::NetherBridgeStart::NetherBridgeStart()
{
	// for reflection
}

NetherBridgeFeature::NetherBridgeStart::NetherBridgeStart(Level *level, Random *random, int chunkX, int chunkZ) : StructureStart(chunkX, chunkZ)
{
	NetherBridgePieces::StartPiece *start = new NetherBridgePieces::StartPiece(random, (chunkX << 4) + 2, (chunkZ << 4) + 2, level);
	pieces.push_back(start);
	start->addChildren(start, &pieces, random);

	vector<StructurePiece *> *pendingChildren = &start->pendingChildren;
	while (!pendingChildren->empty())
	{
		int pos = random->nextInt(static_cast<int>(pendingChildren->size()));
        auto it = pendingChildren->begin() + pos;
        StructurePiece *structurePiece = *it;
		pendingChildren->erase(it);
		structurePiece->addChildren(start, &pieces, random);
	}

	calculateBoundingBox();
	moveInsideHeights(level, random, 48, 70);
}
