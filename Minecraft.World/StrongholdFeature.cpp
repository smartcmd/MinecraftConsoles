#include "stdafx.h"
#include "StrongholdFeature.h"
#include "StrongholdPieces.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.dimension.h"
#include "Mth.h"
#include "FileHeader.h"
#include "JavaMath.h"

const wstring StrongholdFeature::OPTION_DISTANCE = L"distance";
const wstring StrongholdFeature::OPTION_COUNT = L"count";
const wstring StrongholdFeature::OPTION_SPREAD = L"spread";

vector<Biome *> StrongholdFeature::allowedBiomes;

void StrongholdFeature::staticCtor()
{
	allowedBiomes.push_back(Biome::desert);
	allowedBiomes.push_back(Biome::forest);
	allowedBiomes.push_back(Biome::extremeHills);
	allowedBiomes.push_back(Biome::swampland);
	allowedBiomes.push_back(Biome::taiga);
	allowedBiomes.push_back(Biome::iceFlats);
	allowedBiomes.push_back(Biome::iceMountains);
	allowedBiomes.push_back(Biome::desertHills);
	allowedBiomes.push_back(Biome::forestHills);
	allowedBiomes.push_back(Biome::smallerExtremeHills);
	allowedBiomes.push_back(Biome::taigaHills);
	allowedBiomes.push_back(Biome::jungle);
	allowedBiomes.push_back(Biome::jungleHills);
};

void StrongholdFeature::_init()
{
	distance = 32;
	spread = 3;

	// 4J added initialisers
	for (int i = 0; i < strongholdPos_length; i++)
	{
		strongholdPos[i] = NULL;
	}
	isSpotSelected = false;
}

StrongholdFeature::StrongholdFeature() : StructureFeature()
{
	_init();
}

StrongholdFeature::StrongholdFeature(unordered_map<wstring, wstring> options)
{
	_init();

	for (auto& option : options)
	{
		if (option.first.compare(OPTION_DISTANCE) == 0)
		{
			distance = Mth::getDouble(option.second, distance, 1);
		}
		else if (option.first.compare(OPTION_COUNT) == 0)
		{
			// 4J-JEV: Removed, we only have the one stronghold.
			//strongholdPos = new ChunkPos[ Mth::getInt(it->second, strongholdPos_length, 1) ];
			assert(false);
		}
		else if (option.first.compare(OPTION_SPREAD) == 0)
		{
			spread = Mth::getInt(option.second, spread, 1);
		}
	}
}

StrongholdFeature::~StrongholdFeature()
{
	for (int i = 0; i < strongholdPos_length; i++)
	{
		delete strongholdPos[i];
	}
}

wstring StrongholdFeature::getFeatureName()
{
	return LargeFeature::STRONGHOLD;
}

bool StrongholdFeature::isFeatureChunk(int x, int z,bool bIsSuperflat)
{
	if (!isSpotSelected)
	{
		Random random;

		random.setSeed(level->getSeed());
		double angle = random.nextDouble() * PI * 2.0;
		int circle = 1;

		// 4J Stu - Changed so that we keep trying more until we have found somewhere in the world to place a stronghold
		bool hasFoundValidPos = false;
		int findAttempts = 0;
		do
		{
			for (int i = 0; i < strongholdPos_length; i++)
			{
				double dist = 0.0;
#ifdef _LARGE_WORLDS
				if(level->dimension->getXZSize() < (2.25f * 32.0f) )
				{
					// Xbox360/PS3 distances
					dist = (1.25 + random.nextDouble()) * (3 + random.nextInt(4));
				}
				else
				{
					// Original Java
					dist = (1.25 * circle + random.nextDouble()) * (distance * circle);
				}
#else
				// 4J Stu - Design change: Original spawns at *32 chunks rather than *10 chunks from (0,0) but that is outside our world
				// double dist = (1.25 + random->nextDouble()) * 32.0;
				// The max of the first part is 2.25, and we have 27 chunks in each direction
				// Therefore 27/2.25 = 12, which should be the max of the second part
				// The constant part and random part can be tuned to move the strongholds further from the spawn
				// 4J Stu - The original (pre-TU9) calculation for selecting a start point could put the stronghold very close to the edge
				// of the world, causing some parts to fail to generate. If the save is a newer save then we bring that generation in
				if(level->getOriginalSaveVersion() >= SAVE_FILE_VERSION_MOVED_STRONGHOLD)
				{
					// Post TU9
					// The stronghold cannot extend more than 7 chunks in any direction from the start position
					// Therefore as long as the the start x/z are less than 20 it will be fully contained
					dist = (1.25 + random.nextDouble()) * (3 + random.nextInt(4));
				}
				else
				{
					// Pre TU9
					dist = (1.25 + random.nextDouble()) * (5.0 + random.nextInt(7));
				}
#endif

				int selectedX = (int) (Math::round(cos(angle) * dist));
				int selectedZ = (int) (Math::round(sin(angle) * dist));

				TilePos *position = level->getBiomeSource()->findBiome((selectedX << 4) + 8, (selectedZ << 4) + 8, 7 << 4, allowedBiomes, &random);
				if (position != NULL)
				{
					selectedX = position->x >> 4;
					selectedZ = position->z >> 4;

#ifndef _CONTENT_PACKAGE
					if(position->x > 2560 || position->x < -2560 || position->z > 2560 || position->z < -2560)
					{
						__debugbreak();
					}
#endif

					app.DebugPrintf("Placed stronghold in valid biome at (%d, %d), (%d, %d)\n", selectedX, selectedZ, position->x, position->z);
					// 4J added
					app.AddTerrainFeaturePosition(eTerrainFeature_Stronghold,selectedX,selectedZ);

					// 4J Added
					hasFoundValidPos = true;
					delete position;
				}

				delete strongholdPos[i];
				strongholdPos[i] = new ChunkPos(selectedX, selectedZ);

				angle += PI * 2.0 / (double) strongholdPos_length;
			}

			// 4J Stu - We want to make sure that we have at least one stronghold in this world
			++findAttempts;

			// 4J Stu - Randomise the angles for retries as well
#ifdef _LARGE_WORLDS
			angle = random.nextDouble() * PI * 2.0 * circle / (double) spread;
#endif
		}
		while(!hasFoundValidPos && findAttempts < MAX_STRONGHOLD_ATTEMPTS);

		if(!hasFoundValidPos)
		{
			// Even if it's not a valid position we are still creating the last one we tried, so store it in the save so Eye of Ender works
			// Fix for #81933 - GAMEPLAY: The Eye of Ender occasionally does not appear when used to try and locate the End Portal.
			app.AddTerrainFeaturePosition(eTerrainFeature_Stronghold,strongholdPos[0]->x,strongholdPos[0]->z);
		}

		isSpotSelected = true;
	}

	for (int i = 0; i < strongholdPos_length; i++)
	{
		bool forcePlacement = false;
		LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
		if( levelGenOptions != NULL )
		{
			forcePlacement = levelGenOptions->isFeatureChunk(x,z,eFeature_Stronghold);
		}

		ChunkPos *pos = strongholdPos[i];
		if (forcePlacement || (pos && x == pos->x && z == pos->z) )
		{
			return true;
		}
	}
	return false;
}

vector<TilePos> *StrongholdFeature::getGuesstimatedFeaturePositions()
{
	vector<TilePos> *positions = new vector<TilePos>();
	for( int i = 0; i < strongholdPos_length; i++ )
	{
		ChunkPos *chunkPos = strongholdPos[i];
		if (chunkPos != NULL)
		{
			positions->push_back(chunkPos->getMiddleBlockPosition(64));
		}
	}
	return positions;
}

StructureStart *StrongholdFeature::createStructureStart(int x, int z)
{

	StrongholdStart *start = new StrongholdStart(level, random, x, z);

	// 4J - front() was get(0)
	while (start->getPieces()->empty() || ((StrongholdPieces::StartPiece *) start->getPieces()->front())->portalRoomPiece == NULL)
	{
		delete start;
		// regenerate stronghold without changing seed
		start = new StrongholdStart(level, random, x, z);
	}

	return start;

	// System.out.println("Creating stronghold at (" + x + ", " + z + ")");
	// return new StrongholdStart(level, random, x, z);
}

StrongholdFeature::StrongholdStart::StrongholdStart()
{
	// for reflection
}

StrongholdFeature::StrongholdStart::StrongholdStart(Level *level, Random *random, int chunkX, int chunkZ) : StructureStart(chunkX, chunkZ)
{
	StrongholdPieces::resetPieces();

	StrongholdPieces::StartPiece *startRoom = new StrongholdPieces::StartPiece(0, random, (chunkX << 4) + 2, (chunkZ << 4) + 2, level);
	pieces.push_back(startRoom);
	startRoom->addChildren(startRoom, &pieces, random);

	vector<StructurePiece *> *pendingChildren = &startRoom->pendingChildren;
	while (!pendingChildren->empty())
	{
		int pos = random->nextInt((int)pendingChildren->size());
        auto it = pendingChildren->begin() + pos;
        StructurePiece *structurePiece = *it;
		pendingChildren->erase(it);
		structurePiece->addChildren(startRoom, &pieces, random);
	}

	calculateBoundingBox();
	moveBelowSeaLevel(level, random, 10);
}