#include "stdafx.h"
#include "StructureFeature.h"
#include "StructureStart.h"
#include "StructurePiece.h"
#include "ChunkPos.h"
#include "BoundingBox.h"
#include "net.minecraft.world.level.h"
#include "LevelData.h"

StructureFeature::~StructureFeature()
{
	for( AUTO_VAR(it, cachedStructures.begin()); it != cachedStructures.end(); it++ )
	{
		delete it->second;
	}
}

void StructureFeature::addFeature(Level *level, int x, int z, int xOffs, int zOffs, byteArray blocks)
{
    // this method is called for each chunk within 8 chunk's distance from
    // the chunk being generated, but not all chunks are the sources of
    // structures

	if (cachedStructures.find(ChunkPos::hashCode(x, z)) != cachedStructures.end())
	{
        return;
    }

    // clear random key
    random->nextInt();
	// 4J-PB - want to know if it's a superflat land, so we don't generate so many villages - we've changed the distance required between villages on the xbox
    if (isFeatureChunk(x, z,level->getLevelData()->getGenerator() == LevelType::lvl_flat))
	{
        StructureStart *start = createStructureStart(x, z);
        cachedStructures[ChunkPos::hashCode(x, z)] = start;
    }
}

bool StructureFeature::postProcess(Level *level, Random *random, int chunkX, int chunkZ)
{
	// 4J Stu - The x and z used to be offset by (+8) here, but that means we can miss out half structures on the edge of the world
	// Normal feature generation offsets generation by half a chunk to ensure that it can generate the entire feature in chunks already created
	// Structure features don't need this, as the PlaceBlock function only places blocks inside the BoundingBox specified, and parts
	// of a struture piece can be added in more than one post-process call
    int cx = (chunkX << 4); // + 8;
    int cz = (chunkZ << 4); // + 8;

    bool intersection = false;
	for( AUTO_VAR(it, cachedStructures.begin()); it != cachedStructures.end(); it++ )
	{
		StructureStart *structureStart = it->second;

        if (structureStart->isValid())
		{
            if (structureStart->getBoundingBox()->intersects(cx, cz, cx + 15, cz + 15))
			{
				BoundingBox *bb = new BoundingBox(cx, cz, cx + 15, cz + 15);
                structureStart->postProcess(level, random, bb);
				delete bb;
                intersection = true;
            }
        }
    }

    return intersection;
}

bool StructureFeature::isIntersection(int cellX, int cellZ)
{
	for( AUTO_VAR(it, cachedStructures.begin()); it != cachedStructures.end(); it++ )
	{
		StructureStart *structureStart = it->second;
        if (structureStart->isValid())
		{
            if (structureStart->getBoundingBox()->intersects(cellX, cellZ, cellX, cellZ))
			{
				AUTO_VAR(it2, structureStart->getPieces()->begin());
				while( it2 != structureStart->getPieces()->end() )
				{
                    StructurePiece *next = *it2++;
                    if (next->getBoundingBox()->intersects(cellX, cellZ, cellX, cellZ))
					{
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

///////////////////////////////////////////
// 4J-PB - Below functions added from 1.2.3
///////////////////////////////////////////
bool StructureFeature::isInsideFeature(int cellX, int cellY, int cellZ) 
{
	//for (StructureStart structureStart : cachedStructures.values()) 
	for(AUTO_VAR(it, cachedStructures.begin()); it != cachedStructures.end(); ++it)
	{
		StructureStart *pStructureStart = it->second;

		if (pStructureStart->isValid()) 
		{
			if (pStructureStart->getBoundingBox()->intersects(cellX, cellZ, cellX, cellZ)) 
			{
				/*
				Iterator<StructurePiece> it = structureStart.getPieces().iterator();
				while (it.hasNext()) {
				StructurePiece next = it.next();
				if (next.getBoundingBox().isInside(cellX, cellY, cellZ)) {
				return true;
				}
				*/
				list<StructurePiece *> *pieces=pStructureStart->getPieces();

				for ( AUTO_VAR(it2, pieces->begin()); it2 != pieces->end(); it2++ )
				{
					StructurePiece* piece = *it2;
					if ( piece->getBoundingBox()->isInside(cellX, cellY, cellZ)  )
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

TilePos *StructureFeature::getNearestGeneratedFeature(Level *level, int cellX, int cellY, int cellZ) 
{

	// this is a hack that will "force" the feature to generate positions
	// even if the player hasn't generated new chunks yet
	this->level = level;

	random->setSeed(level->getSeed());
	__int64 xScale = random->nextLong();
	__int64 zScale = random->nextLong();
	__int64 xx = (cellX >> 4) * xScale;
	__int64 zz = (cellZ >> 4) * zScale;
	random->setSeed(xx ^ zz ^ level->getSeed());

	addFeature(level, cellX >> 4, cellZ >> 4, 0, 0, byteArray());

	double minDistance = DBL_MAX;
	TilePos *selected = NULL;

	for(AUTO_VAR(it, cachedStructures.begin()); it != cachedStructures.end(); ++it)
	{
		StructureStart *pStructureStart = it->second;

		if (pStructureStart->isValid()) 
		{

			//StructurePiece *pStructurePiece = pStructureStart->getPieces().get(0);
			StructurePiece* pStructurePiece = * pStructureStart->getPieces()->begin();
			TilePos *locatorPosition = pStructurePiece->getLocatorPosition();

			int dx = locatorPosition->x - cellX;
			int dy = locatorPosition->y - cellY;
			int dz = locatorPosition->z - cellZ;
			double dist = dx + dx * dy * dy + dz * dz;

			if (dist < minDistance) 
			{
				minDistance = dist;
				selected = locatorPosition;
			}
		}
	}
	if (selected != NULL) 
	{
		return selected;
	} 
	else 
	{
		vector<TilePos> *guesstimatedFeaturePositions = getGuesstimatedFeaturePositions();
		if (guesstimatedFeaturePositions != NULL) 
		{
 			TilePos *pSelectedPos = new TilePos(0,0,0);

			for(AUTO_VAR(it, guesstimatedFeaturePositions->begin()); it != guesstimatedFeaturePositions->end(); ++it)
			{
				int dx = (*it).x - cellX;
				int dy = (*it).y - cellY;
				int dz = (*it).z - cellZ;
				double dist = dx + dx * dy * dy + dz * dz;

				if (dist < minDistance) 
				{
					minDistance = dist;
					pSelectedPos->x = (*it).x;
					pSelectedPos->y = (*it).y;
					pSelectedPos->z = (*it).z;
				}
			}
			delete guesstimatedFeaturePositions;
			return pSelectedPos;
		}
	}
	return NULL;
}

vector<TilePos> *StructureFeature::getGuesstimatedFeaturePositions() 
{
	return NULL;
}
