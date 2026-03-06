#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "net.minecraft.world.level.tile.h"
#include "Difficulty.h"
#include "WeighedRandom.h"
#include "Level.h"
#include "ChunkPos.h"
#include "TilePos.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "MobSpawner.h"
#include "Dimension.h"

const int MobSpawner::MIN_SPAWN_DISTANCE = 24;

TilePos MobSpawner::getRandomPosWithin(Level *level, int cx, int cz)
{
	// 4J Stu - Added 1.2.3 but we don't need it as it was only used to access sections
	// Leaving here though to help explain why chunk coords are not passed in rather than full coords
	//LevelChunk *chunk = level->getChunk(cx, cz);
	int x = cx * 16 + level->random->nextInt(16);
	int y = level->random->nextInt(level->getHeight());
	int z = cz * 16 + level->random->nextInt(16);

	return TilePos(x, y, z);
}

#ifdef __PSVITA__
	// AP - See CustomMap.h for an explanation of this
	CustomMap MobSpawner::chunksToPoll;
#else
	unordered_map<ChunkPos,bool,ChunkPosKeyHash,ChunkPosKeyEq> MobSpawner::chunksToPoll;
#endif

const int MobSpawner::tick(ServerLevel *level, bool spawnEnemies, bool spawnFriendlies, bool spawnPersistent)
{
#ifndef _CONTENT_PACKAGE

#if 0
	// PIX output for mob counters - generally disabling as Entity::countFlagsForPIX is reasonably expensive
	if( level->dimension->id == 0 )
	{
		Entity::countFlagsForPIX();
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_WATERANIMAL    ,false), "eTYPE_WATERANIMAL");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK			,false), "eTYPE_ANIMAL");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_MONSTER	    ,false), "eTYPE_MONSTER");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_SQUID		  	,true ), "eTYPE_SQUID");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_VILLAGER		  	,true ), "eTYPE_VILLAGER");

		unsigned int totalCount[4];
		unsigned int protectedCount[4];
		unsigned int unprotectedCount[4];
		unsigned int couldWanderCount[4];

		totalCount[0] = level->countInstanceOf(eTYPE_COW         	,true, &protectedCount[0], &couldWanderCount[0] );
		totalCount[1] = level->countInstanceOf(eTYPE_SHEEP       	,true, &protectedCount[1], &couldWanderCount[1] );
		totalCount[2] = level->countInstanceOf(eTYPE_CHICKEN     	,true, &protectedCount[2], &couldWanderCount[2] );
		totalCount[3] = level->countInstanceOf(eTYPE_PIG         	,true, &protectedCount[3], &couldWanderCount[3] );

		for( int i = 0; i < 4; i++ ) unprotectedCount[i] = totalCount[i] - protectedCount[i];

		PIXAddNamedCounter( unprotectedCount[0], "eTYPE_COW (unprotected)");
		PIXAddNamedCounter( unprotectedCount[1], "eTYPE_SHEEP (unprotected)");
		PIXAddNamedCounter( unprotectedCount[2], "eTYPE_CHICKEN (unprotected)");
		PIXAddNamedCounter( unprotectedCount[3], "eTYPE_PIG (unprotected)");

		PIXAddNamedCounter( protectedCount[0], "eTYPE_COW (protected)");
		PIXAddNamedCounter( protectedCount[1], "eTYPE_SHEEP (protected)");
		PIXAddNamedCounter( protectedCount[2], "eTYPE_CHICKEN (protected)");
		PIXAddNamedCounter( protectedCount[3], "eTYPE_PIG (protected)");

		PIXAddNamedCounter( couldWanderCount[0], "eTYPE_COW (could wander)");
		PIXAddNamedCounter( couldWanderCount[1], "eTYPE_SHEEP (could wander)");
		PIXAddNamedCounter( couldWanderCount[2], "eTYPE_CHICKEN (could wander)");
		PIXAddNamedCounter( couldWanderCount[3], "eTYPE_PIG (could wander)");

		PIXAddNamedCounter( level->countInstanceOf(eTYPE_WOLF        	,true ), "eTYPE_WOLF");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_CREEPER     	,true ), "eTYPE_CREEPER");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_GIANT       	,true ), "eTYPE_GIANT");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_SKELETON    	,true ), "eTYPE_SKELETON");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_SPIDER      	,true ), "eTYPE_SPIDER");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_ZOMBIE      	,true ), "eTYPE_ZOMBIE");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_PIGZOMBIE   	,true ), "eTYPE_PIGZOMBIE");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_SLIME   		,true ), "eTYPE_SLIME");
		PIXAddNamedCounter( level->countInstanceOf(eTYPE_GHAST   		,true ), "eTYPE_GHAST");
	}
#endif
#endif

	if (!spawnEnemies && !spawnFriendlies && !spawnPersistent)
	{
		return 0;
	}
	MemSect(20);
	chunksToPoll.clear();

#if 0
	for (auto& player : level->players)
	{
		int xx = Mth::floor(player->x / 16);
		int zz = Mth::floor(player->z / 16);

		int r = 128 / 16;
		for (int x = -r; x <= r; x++)
			for (int z = -r; z <= r; z++)
			{
				chunksToPoll.insert(ChunkPos(x + xx, z + zz));
			}
	}
#else
	// 4J - rewritten to add chunks interleaved by player, and to add them from the centre outwards. We're going to be
	// potentially adding less creatures than the original so that our count stays consistent with number of players added, so
	// we want to make sure as best we can that the ones we do add are near the active players
	int playerCount = (int)level->players.size();
	int *xx = new int[playerCount];
	int *zz = new int[playerCount];
	for (int i = 0; i < playerCount; i++)
	{
		shared_ptr<Player> player = level->players[i];
		xx[i] = Mth::floor(player->x / 16);
		zz[i] = Mth::floor(player->z / 16);
#ifdef __PSVITA__
		chunksToPoll.insert(ChunkPos(xx[i], zz[i] ),false);
#else
		chunksToPoll.insert(std::pair<ChunkPos,bool>(ChunkPos(xx[i], zz[i] ),false));
#endif
	}

	for( int r = 1; r <= 8; r++ )
	{
		for( int l = 0; l < ( r * 2 ) ; l++ )
		{
			for( int i = 0; i < playerCount; i++ )
			{
				bool edgeChunk = ( r == 8 );

				// If this chunk isn't at the edge of the region for this player, then always store with a flag of false
				// so that if it was at the edge of another player, then this will remove that
				if( !edgeChunk )
				{
#ifdef __PSVITA__
					chunksToPoll.insert(ChunkPos( ( xx[i] - r ) + l , ( zz[i] - r )		), false);
					chunksToPoll.insert(ChunkPos( ( xx[i] + r )	 , ( zz[i] - r ) + l    ), false);
					chunksToPoll.insert(ChunkPos( ( xx[i] + r ) - l , ( zz[i] + r )		), false);
					chunksToPoll.insert(ChunkPos( ( xx[i] - r )	 , ( zz[i] + r ) - l    ), false);
#else
					chunksToPoll.insert(std::pair<ChunkPos,bool>(ChunkPos( ( xx[i] - r ) + l , ( zz[i] - r )		), false));
					chunksToPoll.insert(std::pair<ChunkPos,bool>(ChunkPos( ( xx[i] + r )	 , ( zz[i] - r ) + l    ), false));
					chunksToPoll.insert(std::pair<ChunkPos,bool>(ChunkPos( ( xx[i] + r ) - l , ( zz[i] + r )		), false));
					chunksToPoll.insert(std::pair<ChunkPos,bool>(ChunkPos( ( xx[i] - r )	 , ( zz[i] + r ) - l    ), false));
#endif
				}
				else
				{
#ifdef __PSVITA__
					ChunkPos cp = ChunkPos( ( xx[i] - r  ) + l , ( zz[i] - r ));
					if( chunksToPoll.find( cp ) )	chunksToPoll.insert(cp, true);
					cp = ChunkPos( ( xx[i] + r ), ( zz[i] - r ) + l    );
					if( chunksToPoll.find( cp ) )	chunksToPoll.insert(cp, true);
					cp = ChunkPos( ( xx[i] + r ) - l , ( zz[i] + r ));
					if( chunksToPoll.find( cp ) )	chunksToPoll.insert(cp, true);
					cp = ChunkPos( ( xx[i] - r ), ( zz[i] + r ) - l);
					if( chunksToPoll.find( cp ) )	chunksToPoll.insert(cp, true);
#else
					ChunkPos cp = ChunkPos( ( xx[i] - r  ) + l , ( zz[i] - r ));
					if( chunksToPoll.find( cp ) == chunksToPoll.end() )	chunksToPoll.insert(std::pair<ChunkPos,bool>(cp, true));
					cp = ChunkPos( ( xx[i] + r ), ( zz[i] - r ) + l    );
					if( chunksToPoll.find( cp ) == chunksToPoll.end() )	chunksToPoll.insert(std::pair<ChunkPos,bool>(cp, true));
					cp = ChunkPos( ( xx[i] + r ) - l , ( zz[i] + r ));
					if( chunksToPoll.find( cp ) == chunksToPoll.end() )	chunksToPoll.insert(std::pair<ChunkPos,bool>(cp, true));
					cp = ChunkPos( ( xx[i] - r ), ( zz[i] + r ) - l);
					if( chunksToPoll.find( cp ) == chunksToPoll.end() )	chunksToPoll.insert(std::pair<ChunkPos,bool>(cp, true));
#endif

				}
			}
		}
	}
	delete [] xx;
	delete [] zz;
#endif
	MemSect(0);
	int count = 0;
	MemSect(31);
	Pos *spawnPos = level->getSharedSpawnPos();
	MemSect(0);

	for (unsigned int i = 0; i < MobCategory::values.length; i++)
	{
		MobCategory *mobCategory = MobCategory::values[i];
		if ((mobCategory->isFriendly() && !spawnFriendlies) || (!mobCategory->isFriendly() && !spawnEnemies) || (mobCategory->isPersistent() && !spawnPersistent))
		{
			continue;
		}

		// 4J - early out for non-main dimensions, if spawning anything friendly
		if( mobCategory->isFriendly() )
		{
			if( level->dimension->id != 0 )
			{
				continue;
			}
		}

		// 4J - this is now quite different to the java version. We just have global max counts for the level whereas the original has a max per chunk that
		// scales with the number of chunks to be polled.
		int categoryCount = level->countInstanceOf( mobCategory->getEnumBaseClass(), mobCategory->isSingleType());
		if( categoryCount >= mobCategory->getMaxInstancesPerLevel())
		{
			continue;
		}

#ifdef __PSVITA__
		for( int i = 0;i < chunksToPoll.end();i += 1 )
		{
			SCustomMapNode *it = chunksToPoll.get(i);
#else
		for (auto& it : chunksToPoll)
		   {
#endif
			   if( it.second )
			   {
				// don't add mobs to edge chunks, to prevent adding mobs "outside" of the active playground
				   continue;
			   }
			   ChunkPos *cp = (ChunkPos *) (&it.first);

			   // 4J - don't let this actually create/load a chunk that isn't here already - we'll let the normal updateDirtyChunks etc. processes do that, so it can happen on another thread
			   if( !level->hasChunk(cp->x,cp->z) ) continue;

			   TilePos start = getRandomPosWithin(level, cp->x, cp->z);
			   int xStart = start.x;
			   int yStart = start.y;
			   int zStart = start.z;

			   if (level->isSolidBlockingTile(xStart, yStart, zStart)) continue;
			   if (level->getMaterial(xStart, yStart, zStart) != mobCategory->getSpawnPositionMaterial()) continue;
			   int clusterSize = 0;

			   for (int dd = 0; dd < 3; dd++)
			   {
				   int x = xStart;
				   int y = yStart;
				   int z = zStart;
				   int ss = 6;

				   Biome::MobSpawnerData *currentMobType = NULL;
				MobGroupData *groupData = NULL;

				   for (int ll = 0; ll < 4; ll++)
				   {
					   x += level->random->nextInt(ss) - level->random->nextInt(ss);
					   y += level->random->nextInt(1) - level->random->nextInt(1);
					   z += level->random->nextInt(ss) - level->random->nextInt(ss);
					   // int y = heightMap[x + z * w] + 1;

					   // 4J - don't let this actually create/load a chunk that isn't here already - we'll let the normal updateDirtyChunks etc. processes do that, so it can happen on another thread
					   if( !level->hasChunkAt( x, y, z ) ) continue;

					   if (isSpawnPositionOk(mobCategory, level, x, y, z))
					   {
						   float xx = x + 0.5f;
						   float yy = (float) y;
						   float zz = z + 0.5f;
						   if (level->getNearestPlayer(xx, yy, zz, MIN_SPAWN_DISTANCE) != NULL)
						   {
							   continue;
						   }
						   else
						   {
							   float xd = xx - spawnPos->x;
							   float yd = yy - spawnPos->y;
							   float zd = zz - spawnPos->z;
							   float sd = xd * xd + yd * yd + zd * zd;
							   if (sd < MIN_SPAWN_DISTANCE * MIN_SPAWN_DISTANCE)
							   {
								   continue;
							   }
						   }

                            if (currentMobType == NULL)
							{
                                currentMobType = level->getRandomMobSpawnAt(mobCategory, x, y, z);
                                if (currentMobType == NULL)
								{
                                    break;
                                }
                            }

						   shared_ptr<Mob> mob;
						   // 4J - removed try/catch
//						   try
//						   {
	MemSect(29);
							   //mob = type.mobClass.getConstructor(Level.class).newInstance(level);
							   mob = dynamic_pointer_cast<Mob>(EntityIO::newByEnumType(currentMobType->mobClass, level));
	MemSect(0);
//						   }
//						   catch (exception e)
//						   {
//							   // TODO 4J We can't print a stack trace, and the newInstance function doesn't throw an exception just now anyway
//							   //e.printStackTrace();
//							   return count;
//						   }

							// 4J - If it is an animal or a monster, don't let any one type of mob represent more than 50% of the total amount of these things. This
							// was added initially to stop flat lands being totally populated with slimes but seems like a generally good rule.
						   eINSTANCEOF mobType = mob->GetType();

						   if( ( mobType & eTYPE_ANIMALS_SPAWN_LIMIT_CHECK ) || ( mobType & eTYPE_MONSTER ) )
						   {
							   // even more special rule for ghasts, because filling up the nether with 25 of them is a bit unpleasant. In the java version they are
							   // only limited by the fact that the world fills up with pig zombies (the only other type of enemy mob in the nether) before them - they
							   // aren't actually even counted properly themselves
							   if( mobType == eTYPE_GHAST )
							   {
								   if( level->countInstanceOf(mobType, true) >= 4 ) continue;
							   }
							   else if( mobType == eTYPE_ENDERMAN && level->dimension->id == 1 )
							   {
								   // Special rule for the end, as we only have Endermen (plus the dragon). Increase the spawnable counts based on level difficulty
								   int maxEndermen = mobCategory->getMaxInstancesPerLevel();

								   if( level->difficulty == Difficulty::NORMAL )
								   {
									   maxEndermen -= mobCategory->getMaxInstancesPerLevel()/4;
								   }
								   else if( level->difficulty <= Difficulty::EASY)
								   {
									   maxEndermen -= mobCategory->getMaxInstancesPerLevel()/2;
								   }

								   if( level->countInstanceOf(mobType, true) >= maxEndermen ) continue;
							   }
							   else if( level->countInstanceOf(mobType, true) >= ( mobCategory->getMaxInstancesPerLevel() / 2 ) ) continue;
						   }

						   mob->moveTo(xx, yy, zz, level->random->nextFloat() * 360, 0);

						   if (mob->canSpawn())
						   {
							   // 4J - check if we are going to despawn straight away too, and don't add if we will - otherwise we'll be sending
							   // network packets for adding & removal that we don't need
							   mob->checkDespawn();
							   if( !mob->removed )
							   {
								   clusterSize++;
								   categoryCount++;
								   mob->setDespawnProtected();	// 4J added - default to protected against despawning
								   level->addEntity(mob);
								groupData = mob->finalizeMobSpawn(groupData);
								   // 4J - change here so that we can't ever make more than the desired amount of entities in each priority. In the original java version
								   // depending on the random spawn positions being considered the only limit as to the number of entities created per category is the number
								   // of chunks to poll.
								   if (categoryCount >= mobCategory->getMaxInstancesPerLevel() ) goto categoryLoop;
								   if (clusterSize >= mob->getMaxSpawnClusterSize()) goto chunkLoop;
							   }
						   }
						   count += clusterSize;
					   }
				   }
			   }
			   chunkLoop: continue;
		   }
		categoryLoop: continue;
	}
	delete spawnPos;

	return count;
}

bool MobSpawner::isSpawnPositionOk(MobCategory *category, Level *level, int x, int y, int z)
{
	// 4J - don't let this actually create/load a chunk that isn't here already - we'll let the normal updateDirtyChunks etc. processes do that, so it can happen on another thread
	if( !level->hasChunkAt(x, y, z ) ) return false;

#ifdef __PSVITA__
	// AP - added this for Vita. Make sure a new spawn point has 2 chunks around it. This will make sure monsters don't keep getting spawned on the edge preventing other new monsters
	// from being spawned
	int r = 32;
	if( !level->hasChunksAt(x - r, 0, z - r, x + r, 0, z + r))
	{
		return false;
	}
#endif

	if (category->getSpawnPositionMaterial() == Material::water)
	{
		// 4J - changed to spawn water things only in deep water
		int yo = 0;
		int liquidCount = 0;

		while( ( y - yo ) >= 0 && ( yo < 5 ) )
		{
			if( level->getMaterial(x, y - yo, z)->isLiquid() ) liquidCount++;
			yo++;
		}

		// 4J - Sometimes deep water could be just a waterfall, so check that it's wide as well
		bool inEnoughWater = false;
		if( liquidCount == 5 )
		{
			if( level->getMaterial(x+5, y, z)->isLiquid() &&
				level->getMaterial(x-5, y, z)->isLiquid() &&
				level->getMaterial(x, y, z+5)->isLiquid() &&
				level->getMaterial(x, y, z-5)->isLiquid()
				)
			{
				inEnoughWater = true;
			}
		}

		return inEnoughWater && !level->isSolidBlockingTile(x, y + 1, z);
	}
	else
	{
		if (!level->isTopSolidBlocking(x, y - 1, z)) return false;
		int tt = level->getTile(x, y - 1, z);
		return tt != Tile::unbreakable_Id && !level->isSolidBlockingTile(x, y, z) && !level->getMaterial(x, y, z)->isLiquid() && !level->isSolidBlockingTile(x, y + 1, z);
	}
				}

void MobSpawner::postProcessSpawnMobs(Level *level, Biome *biome, int xo, int zo, int cellWidth, int cellHeight, Random *random)
{
	// 4J - not for our version. Creates a few too many mobs.
#if 0
	vector<Biome::MobSpawnerData *> *mobs = biome->getMobs(MobCategory::creature);
	if (mobs->empty())
	{
		return;
	}

	while (random->nextFloat() < biome->getCreatureProbability())
	{
		Biome::MobSpawnerData *type = (Biome::MobSpawnerData *) WeighedRandom::getRandomItem(level->random, ((vector<WeighedRandomItem *> *)mobs));
		MobGroupData *groupData = NULL;
		int count = type->minCount + random->nextInt(1 + type->maxCount - type->minCount);

		int x = xo + random->nextInt(cellWidth);
		int z = zo + random->nextInt(cellHeight);
		int startX = x, startZ = z;

		for (int c = 0; c < count; c++)
		{
			bool success = false;
			for (int attempts = 0; !success && attempts < 4; attempts++)
			{
				// these mobs always spawn at the topmost position
				int y = level->getTopSolidBlock(x, z);
				if (isSpawnPositionOk(MobCategory::creature, level, x, y, z))
				{

					float xx = x + 0.5f;
					float yy = (float)y;
					float zz = z + 0.5f;

					shared_ptr<Mob> mob;
					//try {
					mob = dynamic_pointer_cast<Mob>( EntityIO::newByEnumType(type->mobClass, level ) );
					//} catch (Exception e) {
					//	e.printStackTrace();
					//	continue;
					//}

					// System.out.println("Placing night mob");
					mob->moveTo(xx, yy, zz, random->nextFloat() * 360, 0);

					mob->setDespawnProtected();

					level->addEntity(mob);
					groupData = mob->finalizeMobSpawn(groupData);
					success = true;
				}

				x += random->nextInt(5) - random->nextInt(5);
				z += random->nextInt(5) - random->nextInt(5);
				while (x < xo || x >= (xo + cellWidth) || z < zo || z >= (zo + cellWidth))
				{
					x = startX + random->nextInt(5) - random->nextInt(5);
					z = startZ + random->nextInt(5) - random->nextInt(5);
				}
			}
		}
	}
#endif
}
