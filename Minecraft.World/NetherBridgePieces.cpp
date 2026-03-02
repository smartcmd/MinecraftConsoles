#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.level.levelgen.h"
#include "net.minecraft.world.level.storage.h"
#include "NetherBridgePieces.h"
#include "Direction.h"

NetherBridgePieces::PieceWeight::PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount, bool allowInRow) : weight(weight)
{
	this->placeCount = 0;
	this->pieceClass = pieceClass;
	this->maxPlaceCount = maxPlaceCount;
	this->allowInRow = allowInRow;
}

NetherBridgePieces::PieceWeight::PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount) : weight(weight)
{
	this->placeCount = 0;
	this->pieceClass = pieceClass;
	this->maxPlaceCount = maxPlaceCount;
	this->allowInRow = false;
}

bool NetherBridgePieces::PieceWeight::doPlace(int depth)
{
	return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}

bool NetherBridgePieces::PieceWeight::isValid()
{
	return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}

NetherBridgePieces::PieceWeight *NetherBridgePieces::bridgePieceWeights[NetherBridgePieces::BRIDGE_PIECEWEIGHTS_COUNT] =
{
	new PieceWeight(EPieceClass_BridgeStraight, 30, 0, true),
	new PieceWeight(EPieceClass_BridgeCrossing, 10, 4),
	new PieceWeight(EPieceClass_MonsterThrone, 15, 2), // 4J Stu - Increased weight to ensure that we have these (was 5), required for Blazes and therefore required for brewing
	new PieceWeight(EPieceClass_CastleEntrance, 15, 1), // 4J Stu - Increased weight to ensure that we have these (was 5), required for CastleStalkRoom, and therefore required for brewing
	new PieceWeight(EPieceClass_RoomCrossing, 10, 4),
	new PieceWeight(EPieceClass_StairsRoom, 10, 3),
};

NetherBridgePieces::PieceWeight *NetherBridgePieces::castlePieceWeights[NetherBridgePieces::CASTLE_PIECEWEIGHTS_COUNT] = 
{
	new PieceWeight(EPieceClass_CastleStalkRoom, 30, 2), // 4J Stu - Increased weight to ensure that we have these (was 5), required for Nether Wart, and therefore required for brewing
	new PieceWeight(EPieceClass_CastleSmallCorridorPiece, 25, 0, true),
	new PieceWeight(EPieceClass_CastleSmallCorridorCrossingPiece, 15, 5),
	new PieceWeight(EPieceClass_CastleSmallCorridorRightTurnPiece, 5, 10),
	new PieceWeight(EPieceClass_CastleSmallCorridorLeftTurnPiece, 5, 10),
	new PieceWeight(EPieceClass_CastleCorridorStairsPiece, 10, 3, true),
	new PieceWeight(EPieceClass_CastleCorridorTBalconyPiece, 7, 2),
};

NetherBridgePieces::NetherBridgePiece *NetherBridgePieces::findAndCreateBridgePieceFactory(NetherBridgePieces::PieceWeight *piece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth)
{
	EPieceClass pieceClass = piece->pieceClass;
	NetherBridgePiece *structurePiece = NULL;

	if (pieceClass == EPieceClass_BridgeStraight)
	{
		structurePiece = BridgeStraight::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_BridgeCrossing)
	{
		structurePiece = BridgeCrossing::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_RoomCrossing)
	{
		structurePiece = RoomCrossing::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_StairsRoom)
	{
		structurePiece = StairsRoom::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_MonsterThrone)
	{
		structurePiece = MonsterThrone::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_CastleEntrance)
	{
		structurePiece = CastleEntrance::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_CastleSmallCorridorPiece)
	{
		structurePiece = CastleSmallCorridorPiece::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_CastleSmallCorridorRightTurnPiece)
	{
		structurePiece = CastleSmallCorridorRightTurnPiece::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_CastleSmallCorridorLeftTurnPiece)
	{
		structurePiece = CastleSmallCorridorLeftTurnPiece::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_CastleCorridorStairsPiece)
	{
		structurePiece = CastleCorridorStairsPiece::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_CastleCorridorTBalconyPiece)
	{
		structurePiece = CastleCorridorTBalconyPiece::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_CastleSmallCorridorCrossingPiece)
	{
		structurePiece = CastleSmallCorridorCrossingPiece::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == EPieceClass_CastleStalkRoom)
	{
		structurePiece = CastleStalkRoom::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	return structurePiece;
}

NetherBridgePieces::NetherBridgePiece::NetherBridgePiece(int genDepth) : StructurePiece(genDepth)
{
}

int NetherBridgePieces::NetherBridgePiece::updatePieceWeight(list<PieceWeight *> *currentPieces)
{
	bool hasAnyPieces = false;
	int totalWeight = 0;
	for( AUTO_VAR(it, currentPieces->begin()); it != currentPieces->end(); it++ )
	{
		PieceWeight *piece = *it;

		if (piece->maxPlaceCount > 0 && piece->placeCount < piece->maxPlaceCount)
		{
			hasAnyPieces = true;
		}
		totalWeight += piece->weight;
	}
	return (hasAnyPieces ? totalWeight : -1);
}

NetherBridgePieces::NetherBridgePiece *NetherBridgePieces::NetherBridgePiece::generatePiece(StartPiece *startPiece, list<NetherBridgePieces::PieceWeight *> *currentPieces, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth)
{
	int totalWeight = updatePieceWeight(currentPieces);
	boolean doStuff = totalWeight > 0 && depth <= MAX_DEPTH;

	int numAttempts = 0;
	while (numAttempts < 5 && doStuff)
	{
		numAttempts++;

		int weightSelection = random->nextInt(totalWeight);
		for( AUTO_VAR(it, currentPieces->begin()); it != currentPieces->end(); it++ )
		{
			PieceWeight *piece = *it;
			weightSelection -= piece->weight;
			if (weightSelection < 0)
			{
				if (!piece->doPlace(depth) || (piece == startPiece->previousPiece && !piece->allowInRow))
				{
					break;
				}

				NetherBridgePiece *structurePiece = findAndCreateBridgePieceFactory(piece, pieces, random, footX, footY, footZ, direction, depth);
				if (structurePiece != NULL)
				{
					piece->placeCount++;
					startPiece->previousPiece = piece;

					if (!piece->isValid())
					{
						currentPieces->remove(piece);
					}
					return structurePiece;
				}
			}
		}
	}
	{
		return BridgeEndFiller::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}

}

StructurePiece *NetherBridgePieces::NetherBridgePiece::generateAndAddPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth, bool isCastle)
{
	if (abs(footX - startPiece->getBoundingBox()->x0) > 7 * 16 || abs(footZ - startPiece->getBoundingBox()->z0) > 7 * 16)
	{
		return BridgeEndFiller::createPiece(pieces, random, footX, footY, footZ, direction, depth);
	}
	list<PieceWeight *> *availablePieces = &startPiece->availableBridgePieces;
	if (isCastle)
	{
		availablePieces = &startPiece->availableCastlePieces;
	}
	StructurePiece *newPiece = generatePiece(startPiece, availablePieces, pieces, random, footX, footY, footZ, direction, depth + 1);
	if (newPiece != NULL)
	{
		pieces->push_back(newPiece);
		startPiece->pendingChildren.push_back(newPiece);
	}
	return newPiece;
}

StructurePiece *NetherBridgePieces::NetherBridgePiece::generateChildForward(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int xOff, int yOff, bool isCastle)
{
	switch (orientation)
	{
	case Direction::NORTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + xOff, boundingBox->y0 + yOff, boundingBox->z0 - 1, orientation, getGenDepth(), isCastle);
	case Direction::SOUTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + xOff, boundingBox->y0 + yOff, boundingBox->z1 + 1, orientation, getGenDepth(), isCastle);
	case Direction::WEST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 + yOff, boundingBox->z0 + xOff, orientation, getGenDepth(), isCastle);
	case Direction::EAST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 + yOff, boundingBox->z0 + xOff, orientation, getGenDepth(), isCastle);
	}
	return NULL;
}

StructurePiece *NetherBridgePieces::NetherBridgePiece::generateChildLeft(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int yOff, int zOff, bool isCastle)
{
	switch (orientation)
	{
	case Direction::NORTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 + yOff, boundingBox->z0 + zOff, Direction::WEST, getGenDepth(), isCastle);
	case Direction::SOUTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 + yOff, boundingBox->z0 + zOff, Direction::WEST, getGenDepth(), isCastle);
	case Direction::WEST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + zOff, boundingBox->y0 + yOff, boundingBox->z0 - 1, Direction::NORTH, getGenDepth(), isCastle);
	case Direction::EAST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + zOff, boundingBox->y0 + yOff, boundingBox->z0 - 1, Direction::NORTH, getGenDepth(), isCastle);
	}
	return NULL;
}

StructurePiece *NetherBridgePieces::NetherBridgePiece::generateChildRight(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int yOff, int zOff, bool isCastle)
{
	switch (orientation)
	{
	case Direction::NORTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 + yOff, boundingBox->z0 + zOff, Direction::EAST, getGenDepth(), isCastle);
	case Direction::SOUTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 + yOff, boundingBox->z0 + zOff, Direction::EAST, getGenDepth(), isCastle);
	case Direction::WEST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + zOff, boundingBox->y0 + yOff, boundingBox->z1 + 1, Direction::SOUTH, getGenDepth(), isCastle);
	case Direction::EAST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + zOff, boundingBox->y0 + yOff, boundingBox->z1 + 1, Direction::SOUTH, getGenDepth(), isCastle);
	}
	return NULL;
}

bool NetherBridgePieces::NetherBridgePiece::isOkBox(BoundingBox *box, StartPiece *startPiece)
{
	bool bIsOk = false;

	if(box != NULL)
	{
		if( box->y0 > LOWEST_Y_POSITION ) bIsOk = true;
		int xzSize = (startPiece->m_level->getLevelData()->getXZSize() / startPiece->m_level->getLevelData()->getHellScale()); //HellRandomLevelSource::XZSIZE;
		int blockMin = -( (xzSize << 4) / 2) + 1;
		int blockMax = ( (xzSize << 4) / 2 ) - 1;

		if(box->x0 <= blockMin) bIsOk = false;
		if(box->z0 <= blockMin) bIsOk = false;
		if(box->x1 >= blockMax) bIsOk = false;
		if(box->z1 >= blockMax) bIsOk = false;
	}

	return bIsOk;
}

void NetherBridgePieces::NetherBridgePiece::generateLightPost(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z, int xOff, int zOff)
{
	int worldX = getWorldX(x, z);
	int worldY = getWorldY(y);
	int worldZ = getWorldZ(x, z);

	if (!chunkBB->isInside(worldX, worldY, worldZ))
	{
		return;
	}

	if (level->isEmptyTile(worldX, worldY, worldZ) && level->isEmptyTile(worldX, worldY + 1, worldZ) && level->isEmptyTile(worldX, worldY + 2, worldZ)
		&& level->isEmptyTile(worldX, worldY + 3, worldZ))
	{
		level->setTileAndDataNoUpdate(worldX, worldY, worldZ, Tile::netherFence_Id, 0);
		level->setTileAndDataNoUpdate(worldX, worldY + 1, worldZ, Tile::netherFence_Id, 0);
		level->setTileAndDataNoUpdate(worldX, worldY + 2, worldZ, Tile::netherFence_Id, 0);
		level->setTileAndDataNoUpdate(worldX, worldY + 3, worldZ, Tile::netherFence_Id, 0);
		placeBlock(level, Tile::netherFence_Id, 0, x + xOff, y + 3, z + zOff, chunkBB);
		placeBlock(level, Tile::lightGem_Id, 0, x + xOff, y + 2, z + zOff, chunkBB);
	}
}

void NetherBridgePieces::NetherBridgePiece::generateLightPostFacingRight(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z)
{
	generateLightPost(level, random, chunkBB, x, y, z, 1, 0);
}

void NetherBridgePieces::NetherBridgePiece::generateLightPostFacingLeft(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z)
{
	generateLightPost(level, random, chunkBB, x, y, z, -1, 0);
}

void NetherBridgePieces::NetherBridgePiece::generateLightPostFacingUp(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z)
{
	generateLightPost(level, random, chunkBB, x, y, z, 0, 1);
}

void NetherBridgePieces::NetherBridgePiece::generateLightPostFacingDown(Level *level, Random *random, BoundingBox *chunkBB, int x, int y, int z)
{
	generateLightPost(level, random, chunkBB, x, y, z, 0, -1);
}


NetherBridgePieces::BridgeStraight::BridgeStraight(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

void NetherBridgePieces::BridgeStraight::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildForward((StartPiece *) startPiece, pieces, random, 1, 3, false);
}

NetherBridgePieces::BridgeStraight *NetherBridgePieces::BridgeStraight::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -1, -3, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new BridgeStraight(genDepth, random, box, direction);
}

bool NetherBridgePieces::BridgeStraight::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 3, 0, width - 1, 4, depth - 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 1, 5, 0, 3, 7, depth - 1, 0, 0, false);

	// hand rails
	generateBox(level, chunkBB, 0, 5, 0, 0, 5, depth - 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 4, 5, 0, 4, 5, depth - 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// supports
	generateBox(level, chunkBB, 0, 2, 0, 4, 2, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 2, 13, 4, 2, 18, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 0, 0, 4, 1, 3, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 0, 15, 4, 1, 18, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	for (int x = 0; x <= 4; x++)
	{
		for (int z = 0; z <= 2; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, 18 - z, chunkBB);
		}
	}

	generateBox(level, chunkBB, 0, 1, 1, 0, 4, 1, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 0, 3, 4, 0, 4, 4, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 0, 3, 14, 0, 4, 14, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 0, 1, 17, 0, 4, 17, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 4, 1, 1, 4, 4, 1, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 4, 3, 4, 4, 4, 4, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 4, 3, 14, 4, 4, 14, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 4, 1, 17, 4, 4, 17, Tile::netherFence_Id, Tile::netherFence_Id, false);

	return true;
}

NetherBridgePieces::BridgeEndFiller::BridgeEndFiller(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
	selfSeed = random->nextInt();
}

NetherBridgePieces::BridgeEndFiller *NetherBridgePieces::BridgeEndFiller::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -1, -3, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new BridgeEndFiller(genDepth, random, box, direction);
}

bool NetherBridgePieces::BridgeEndFiller::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	Random *selfRandom = new Random(selfSeed);

	// floor
	for (int x = 0; x <= 4; x++)
	{
		for (int y = 3; y <= 4; y++)
		{
			int z = selfRandom->nextInt(8);
			generateBox(level, chunkBB, x, y, 0, x, y, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
		}
	}

	// hand rails
	{
		int z = selfRandom->nextInt(8);
		generateBox(level, chunkBB, 0, 5, 0, 0, 5, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	}
	{
		int z = selfRandom->nextInt(8);
		generateBox(level, chunkBB, 4, 5, 0, 4, 5, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	}

	// supports
	for (int x = 0; x <= 4; x++)
	{
		int z = selfRandom->nextInt(5);
		generateBox(level, chunkBB, x, 2, 0, x, 2, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	}
	for (int x = 0; x <= 4; x++)
	{
		for (int y = 0; y <= 1; y++)
		{
			int z = selfRandom->nextInt(3);
			generateBox(level, chunkBB, x, y, 0, x, y, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
		}
	}

	delete selfRandom;

	return true;
}

NetherBridgePieces::BridgeCrossing::BridgeCrossing(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

NetherBridgePieces::BridgeCrossing::BridgeCrossing(Random *random, int west, int north) : NetherBridgePiece(0)
{
	orientation = random->nextInt(4);

	switch (orientation)
	{
	case Direction::NORTH:
	case Direction::SOUTH:
		boundingBox = new BoundingBox(west, 64, north, west + width - 1, 64 + height - 1, north + depth - 1);
		break;
	default:
		boundingBox = new BoundingBox(west, 64, north, west + depth - 1, 64 + height - 1, north + width - 1);
		break;
	}
}

void NetherBridgePieces::BridgeCrossing::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildForward((StartPiece *) startPiece, pieces, random, 8, 3, false);
	generateChildLeft((StartPiece *) startPiece, pieces, random, 3, 8, false);
	generateChildRight((StartPiece *) startPiece, pieces, random, 3, 8, false);
}

NetherBridgePieces::BridgeCrossing *NetherBridgePieces::BridgeCrossing::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -8, -3, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new BridgeCrossing(genDepth, random, box, direction);
}

bool NetherBridgePieces::BridgeCrossing::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 7, 3, 0, 11, 4, 18, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 3, 7, 18, 4, 11, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 8, 5, 0, 10, 7, 18, 0, 0, false);
	generateBox(level, chunkBB, 0, 5, 8, 18, 7, 10, 0, 0, false);
	// hand rails
	generateBox(level, chunkBB, 7, 5, 0, 7, 5, 7, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 7, 5, 11, 7, 5, 18, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 11, 5, 0, 11, 5, 7, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 11, 5, 11, 11, 5, 18, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 5, 7, 7, 5, 7, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 11, 5, 7, 18, 5, 7, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 5, 11, 7, 5, 11, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 11, 5, 11, 18, 5, 11, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// supports
	generateBox(level, chunkBB, 7, 2, 0, 11, 2, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 7, 2, 13, 11, 2, 18, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 7, 0, 0, 11, 1, 3, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 7, 0, 15, 11, 1, 18, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	for (int x = 7; x <= 11; x++)
	{
		for (int z = 0; z <= 2; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, 18 - z, chunkBB);
		}
	}

	generateBox(level, chunkBB, 0, 2, 7, 5, 2, 11, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 13, 2, 7, 18, 2, 11, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 0, 7, 3, 1, 11, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 15, 0, 7, 18, 1, 11, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	for (int x = 0; x <= 2; x++)
	{
		for (int z = 7; z <= 11; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
			fillColumnDown(level, Tile::netherBrick_Id, 0, 18 - x, -1, z, chunkBB);
		}
	}

	return true;
}


NetherBridgePieces::StartPiece::StartPiece(Random *random, int west, int north, Level *level) : BridgeCrossing(random, west, north)
{
	isLibraryAdded = false;
	previousPiece = NULL;
	m_level = level;

	for( int i = 0; i < BRIDGE_PIECEWEIGHTS_COUNT; i++ )
	{
		PieceWeight *piece = bridgePieceWeights[i];
		piece->placeCount = 0;
		availableBridgePieces.push_back(piece);
	}

	for( int i = 0; i < CASTLE_PIECEWEIGHTS_COUNT; i++ )
	{
		PieceWeight *piece = castlePieceWeights[i];

		piece->placeCount = 0;
		availableCastlePieces.push_back(piece);
	}
}


NetherBridgePieces::RoomCrossing::RoomCrossing(int genDepth, Random *random, BoundingBox *box, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = box;
}

void NetherBridgePieces::RoomCrossing::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildForward((StartPiece *) startPiece, pieces, random, 2, 0, false);
	generateChildLeft((StartPiece *) startPiece, pieces, random, 0, 2, false);
	generateChildRight((StartPiece *) startPiece, pieces, random, 0, 2, false);
}

NetherBridgePieces::RoomCrossing *NetherBridgePieces::RoomCrossing::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -2, 0, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new RoomCrossing(genDepth, random, box, direction);
}

bool NetherBridgePieces::RoomCrossing::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 0, 0, width - 1, 1, depth - 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 0, 2, 0, 6, 7, 6, 0, 0, false);

	// walls
	generateBox(level, chunkBB, 0, 2, 0, 1, 6, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 2, 6, 1, 6, 6, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 5, 2, 0, 6, 6, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 5, 2, 6, 6, 6, 6, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 2, 0, 0, 6, 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 2, 5, 0, 6, 6, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 6, 2, 0, 6, 6, 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 6, 2, 5, 6, 6, 6, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// entries
	generateBox(level, chunkBB, 2, 6, 0, 4, 6, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 5, 0, 4, 5, 0, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 2, 6, 6, 4, 6, 6, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 5, 6, 4, 5, 6, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 0, 6, 2, 0, 6, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 5, 2, 0, 5, 4, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 6, 6, 2, 6, 6, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 6, 5, 2, 6, 5, 4, Tile::netherFence_Id, Tile::netherFence_Id, false);

	for (int x = 0; x <= 6; x++)
	{
		for (int z = 0; z <= 6; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
		}
	}

	return true;
}

NetherBridgePieces::StairsRoom::StairsRoom(int genDepth, Random *random, BoundingBox *box, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = box;
}

void NetherBridgePieces::StairsRoom::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildRight((StartPiece *) startPiece, pieces, random, 6, 2, false);
}

NetherBridgePieces::StairsRoom *NetherBridgePieces::StairsRoom::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -2, 0, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new StairsRoom(genDepth, random, box, direction);
}

bool NetherBridgePieces::StairsRoom::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 0, 0, width - 1, 1, depth - 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 0, 2, 0, 6, 10, 6, 0, 0, false);

	// walls
	generateBox(level, chunkBB, 0, 2, 0, 1, 8, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 5, 2, 0, 6, 8, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 2, 1, 0, 8, 6, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 6, 2, 1, 6, 8, 6, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 2, 6, 5, 8, 6, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// wall decorations
	generateBox(level, chunkBB, 0, 3, 2, 0, 5, 4, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 6, 3, 2, 6, 5, 2, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 6, 3, 4, 6, 5, 4, Tile::netherFence_Id, Tile::netherFence_Id, false);

	// stair
	placeBlock(level, Tile::netherBrick_Id, 0, 5, 2, 5, chunkBB);
	generateBox(level, chunkBB, 4, 2, 5, 4, 3, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 3, 2, 5, 3, 4, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 2, 5, 2, 5, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 2, 5, 1, 6, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// top floor
	generateBox(level, chunkBB, 1, 7, 1, 5, 7, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 6, 8, 2, 6, 8, 4, 0, 0, false);

	// entries
	generateBox(level, chunkBB, 2, 6, 0, 4, 8, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 5, 0, 4, 5, 0, Tile::netherFence_Id, Tile::netherFence_Id, false);

	for (int x = 0; x <= 6; x++)
	{
		for (int z = 0; z <= 6; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
		}
	}

	return true;

}


NetherBridgePieces::MonsterThrone::MonsterThrone(int genDepth, Random *random, BoundingBox *box, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = box;
	hasPlacedMobSpawner = false;
}

NetherBridgePieces::MonsterThrone *NetherBridgePieces::MonsterThrone::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -2, 0, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new MonsterThrone(genDepth, random, box, direction);
}

bool NetherBridgePieces::MonsterThrone::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	generateBox(level, chunkBB, 0, 2, 0, 6, 7, 7, 0, 0, false);

	// floors
	generateBox(level, chunkBB, 1, 0, 0, 5, 1, 7, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 2, 1, 5, 2, 7, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 3, 2, 5, 3, 7, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 4, 3, 5, 4, 7, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// rails
	generateBox(level, chunkBB, 1, 2, 0, 1, 4, 2, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 5, 2, 0, 5, 4, 2, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 5, 2, 1, 5, 3, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 5, 5, 2, 5, 5, 3, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 5, 3, 0, 5, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 6, 5, 3, 6, 5, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 5, 8, 5, 5, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	placeBlock(level, Tile::netherFence_Id, 0, 1, 6, 3, chunkBB);
	placeBlock(level, Tile::netherFence_Id, 0, 5, 6, 3, chunkBB);
	generateBox(level, chunkBB, 0, 6, 3, 0, 6, 8, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 6, 6, 3, 6, 6, 8, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 1, 6, 8, 5, 7, 8, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 2, 8, 8, 4, 8, 8, Tile::netherFence_Id, Tile::netherFence_Id, false);


	if (!hasPlacedMobSpawner)
	{
		int y = getWorldY(5), x = getWorldX(3, 5), z = getWorldZ(3, 5);
		if (chunkBB->isInside(x, y, z))
		{
			hasPlacedMobSpawner = true;
			level->setTile(x, y, z, Tile::mobSpawner_Id);
			shared_ptr<MobSpawnerTileEntity> entity = dynamic_pointer_cast<MobSpawnerTileEntity>( level->getTileEntity(x, y, z) );
			if (entity != NULL) entity->setEntityId(L"Blaze");
		}
	}

	for (int x = 0; x <= 6; x++)
	{
		for (int z = 0; z <= 6; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
		}
	}

	return true;
}


NetherBridgePieces::CastleEntrance::CastleEntrance(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

void NetherBridgePieces::CastleEntrance::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildForward((StartPiece *) startPiece, pieces, random, 5, 3, true);
}

NetherBridgePieces::CastleEntrance *NetherBridgePieces::CastleEntrance::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -5, -3, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new CastleEntrance(genDepth, random, box, direction);
}

bool NetherBridgePieces::CastleEntrance::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 3, 0, 12, 4, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 0, 5, 0, 12, 13, 12, 0, 0, false);

	// walls
	generateBox(level, chunkBB, 0, 5, 0, 1, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 11, 5, 0, 12, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 5, 11, 4, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 8, 5, 11, 10, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 5, 9, 11, 7, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 5, 0, 4, 12, 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 8, 5, 0, 10, 12, 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 5, 9, 0, 7, 12, 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// roof
	generateBox(level, chunkBB, 2, 11, 2, 10, 12, 10, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// entrance decoration
	generateBox(level, chunkBB, 5, 8, 0, 7, 8, 0, Tile::netherFence_Id, Tile::netherFence_Id, false);

	// wall decorations
	for (int i = 1; i <= 11; i += 2)
	{
		generateBox(level, chunkBB, i, 10, 0, i, 11, 0, Tile::netherFence_Id, Tile::netherFence_Id, false);
		generateBox(level, chunkBB, i, 10, 12, i, 11, 12, Tile::netherFence_Id, Tile::netherFence_Id, false);
		generateBox(level, chunkBB, 0, 10, i, 0, 11, i, Tile::netherFence_Id, Tile::netherFence_Id, false);
		generateBox(level, chunkBB, 12, 10, i, 12, 11, i, Tile::netherFence_Id, Tile::netherFence_Id, false);
		placeBlock(level, Tile::netherBrick_Id, 0, i, 13, 0, chunkBB);
		placeBlock(level, Tile::netherBrick_Id, 0, i, 13, 12, chunkBB);
		placeBlock(level, Tile::netherBrick_Id, 0, 0, 13, i, chunkBB);
		placeBlock(level, Tile::netherBrick_Id, 0, 12, 13, i, chunkBB);
		placeBlock(level, Tile::netherFence_Id, 0, i + 1, 13, 0, chunkBB);
		placeBlock(level, Tile::netherFence_Id, 0, i + 1, 13, 12, chunkBB);
		placeBlock(level, Tile::netherFence_Id, 0, 0, 13, i + 1, chunkBB);
		placeBlock(level, Tile::netherFence_Id, 0, 12, 13, i + 1, chunkBB);
	}
	placeBlock(level, Tile::netherFence_Id, 0, 0, 13, 0, chunkBB);
	placeBlock(level, Tile::netherFence_Id, 0, 0, 13, 12, chunkBB);
	placeBlock(level, Tile::netherFence_Id, 0, 0, 13, 0, chunkBB);
	placeBlock(level, Tile::netherFence_Id, 0, 12, 13, 0, chunkBB);

	// inside decorations
	for (int z = 3; z <= 9; z += 2)
	{
		generateBox(level, chunkBB, 1, 7, z, 1, 8, z, Tile::netherFence_Id, Tile::netherFence_Id, false);
		generateBox(level, chunkBB, 11, 7, z, 11, 8, z, Tile::netherFence_Id, Tile::netherFence_Id, false);
	}

	// supports
	generateBox(level, chunkBB, 4, 2, 0, 8, 2, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 2, 4, 12, 2, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	generateBox(level, chunkBB, 4, 0, 0, 8, 1, 3, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 4, 0, 9, 8, 1, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 0, 4, 3, 1, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 9, 0, 4, 12, 1, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	for (int x = 4; x <= 8; x++)
	{
		for (int z = 0; z <= 2; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, 12 - z, chunkBB);
		}
	}
	for (int x = 0; x <= 2; x++)
	{
		for (int z = 4; z <= 8; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
			fillColumnDown(level, Tile::netherBrick_Id, 0, 12 - x, -1, z, chunkBB);
		}
	}

	// lava well
	generateBox(level, chunkBB, 5, 5, 5, 7, 5, 7, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 6, 1, 6, 6, 4, 6, 0, 0, false);
	placeBlock(level, Tile::netherBrick_Id, 0, 6, 0, 6, chunkBB);
	placeBlock(level, Tile::lava_Id, 0, 6, 5, 6, chunkBB);
	// tick lava well
	int x = getWorldX(6, 6);
	int y = getWorldY(5);
	int z = getWorldZ(6, 6);
	if (chunkBB->isInside(x, y, z))
	{
		level->setInstaTick(true);
		Tile::tiles[Tile::lava_Id]->tick(level, x, y, z, random);
		level->setInstaTick(false);
	}


	return true;
}


NetherBridgePieces::CastleStalkRoom::CastleStalkRoom(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

void NetherBridgePieces::CastleStalkRoom::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildForward((StartPiece *) startPiece, pieces, random, 5, 3, true);
	generateChildForward((StartPiece *) startPiece, pieces, random, 5, 11, true);
}

NetherBridgePieces::CastleStalkRoom *NetherBridgePieces::CastleStalkRoom::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -5, -3, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new CastleStalkRoom(genDepth, random, box, direction);
}

bool NetherBridgePieces::CastleStalkRoom::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 3, 0, 12, 4, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 0, 5, 0, 12, 13, 12, 0, 0, false);

	// walls
	generateBox(level, chunkBB, 0, 5, 0, 1, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 11, 5, 0, 12, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 5, 11, 4, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 8, 5, 11, 10, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 5, 9, 11, 7, 12, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 5, 0, 4, 12, 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 8, 5, 0, 10, 12, 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 5, 9, 0, 7, 12, 1, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// roof
	generateBox(level, chunkBB, 2, 11, 2, 10, 12, 10, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// wall decorations
	for (int i = 1; i <= 11; i += 2)
	{
		generateBox(level, chunkBB, i, 10, 0, i, 11, 0, Tile::netherFence_Id, Tile::netherFence_Id, false);
		generateBox(level, chunkBB, i, 10, 12, i, 11, 12, Tile::netherFence_Id, Tile::netherFence_Id, false);
		generateBox(level, chunkBB, 0, 10, i, 0, 11, i, Tile::netherFence_Id, Tile::netherFence_Id, false);
		generateBox(level, chunkBB, 12, 10, i, 12, 11, i, Tile::netherFence_Id, Tile::netherFence_Id, false);
		placeBlock(level, Tile::netherBrick_Id, 0, i, 13, 0, chunkBB);
		placeBlock(level, Tile::netherBrick_Id, 0, i, 13, 12, chunkBB);
		placeBlock(level, Tile::netherBrick_Id, 0, 0, 13, i, chunkBB);
		placeBlock(level, Tile::netherBrick_Id, 0, 12, 13, i, chunkBB);
		placeBlock(level, Tile::netherFence_Id, 0, i + 1, 13, 0, chunkBB);
		placeBlock(level, Tile::netherFence_Id, 0, i + 1, 13, 12, chunkBB);
		placeBlock(level, Tile::netherFence_Id, 0, 0, 13, i + 1, chunkBB);
		placeBlock(level, Tile::netherFence_Id, 0, 12, 13, i + 1, chunkBB);
	}
	placeBlock(level, Tile::netherFence_Id, 0, 0, 13, 0, chunkBB);
	placeBlock(level, Tile::netherFence_Id, 0, 0, 13, 12, chunkBB);
	placeBlock(level, Tile::netherFence_Id, 0, 0, 13, 0, chunkBB);
	placeBlock(level, Tile::netherFence_Id, 0, 12, 13, 0, chunkBB);

	// inside decorations
	for (int z = 3; z <= 9; z += 2)
	{
		generateBox(level, chunkBB, 1, 7, z, 1, 8, z, Tile::netherFence_Id, Tile::netherFence_Id, false);
		generateBox(level, chunkBB, 11, 7, z, 11, 8, z, Tile::netherFence_Id, Tile::netherFence_Id, false);
	}

	// inside stair
	int stairOrientation = getOrientationData(Tile::stairs_netherBricks_Id, 3);
	for (int i = 0; i <= 6; i++)
	{
		int z = i + 4;
		for (int x = 5; x <= 7; x++)
		{
			placeBlock(level, Tile::stairs_netherBricks_Id, stairOrientation, x, 5 + i, z, chunkBB);
		}
		if (z >= 5 && z <= 8)
		{
			generateBox(level, chunkBB, 5, 5, z, 7, i + 4, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
		}
		else if (z >= 9 && z <= 10)
		{
			generateBox(level, chunkBB, 5, 8, z, 7, i + 4, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
		}
		if (i >= 1)
		{
			generateBox(level, chunkBB, 5, 6 + i, z, 7, 9 + i, z, 0, 0, false);
		}
	}
	for (int x = 5; x <= 7; x++)
	{
		placeBlock(level, Tile::stairs_netherBricks_Id, stairOrientation, x, 12, 11, chunkBB);
	}
	generateBox(level, chunkBB, 5, 6, 7, 5, 7, 7, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 7, 6, 7, 7, 7, 7, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 5, 13, 12, 7, 13, 12, 0, 0, false);

	// farmland catwalks
	generateBox(level, chunkBB, 2, 5, 2, 3, 5, 3, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 5, 9, 3, 5, 10, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 2, 5, 4, 2, 5, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 9, 5, 2, 10, 5, 3, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 9, 5, 9, 10, 5, 10, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 10, 5, 4, 10, 5, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	int eastOrientation = getOrientationData(Tile::stairs_netherBricks_Id, 0);
	int westOrientation = getOrientationData(Tile::stairs_netherBricks_Id, 1);
	placeBlock(level, Tile::stairs_netherBricks_Id, westOrientation, 4, 5, 2, chunkBB);
	placeBlock(level, Tile::stairs_netherBricks_Id, westOrientation, 4, 5, 3, chunkBB);
	placeBlock(level, Tile::stairs_netherBricks_Id, westOrientation, 4, 5, 9, chunkBB);
	placeBlock(level, Tile::stairs_netherBricks_Id, westOrientation, 4, 5, 10, chunkBB);
	placeBlock(level, Tile::stairs_netherBricks_Id, eastOrientation, 8, 5, 2, chunkBB);
	placeBlock(level, Tile::stairs_netherBricks_Id, eastOrientation, 8, 5, 3, chunkBB);
	placeBlock(level, Tile::stairs_netherBricks_Id, eastOrientation, 8, 5, 9, chunkBB);
	placeBlock(level, Tile::stairs_netherBricks_Id, eastOrientation, 8, 5, 10, chunkBB);

	// farmlands
	generateBox(level, chunkBB, 3, 4, 4, 4, 4, 8, Tile::hellSand_Id, Tile::hellSand_Id, false);
	generateBox(level, chunkBB, 8, 4, 4, 9, 4, 8, Tile::hellSand_Id, Tile::hellSand_Id, false);
	generateBox(level, chunkBB, 3, 5, 4, 4, 5, 8, Tile::netherStalk_Id, Tile::netherStalk_Id, false);
	generateBox(level, chunkBB, 8, 5, 4, 9, 5, 8, Tile::netherStalk_Id, Tile::netherStalk_Id, false);

	// supports
	generateBox(level, chunkBB, 4, 2, 0, 8, 2, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 2, 4, 12, 2, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	generateBox(level, chunkBB, 4, 0, 0, 8, 1, 3, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 4, 0, 9, 8, 1, 12, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 0, 4, 3, 1, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 9, 0, 4, 12, 1, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	for (int x = 4; x <= 8; x++)
	{
		for (int z = 0; z <= 2; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, 12 - z, chunkBB);
		}
	}
	for (int x = 0; x <= 2; x++)
	{
		for (int z = 4; z <= 8; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
			fillColumnDown(level, Tile::netherBrick_Id, 0, 12 - x, -1, z, chunkBB);
		}
	}

	return true;

}


NetherBridgePieces::CastleSmallCorridorPiece::CastleSmallCorridorPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}


void NetherBridgePieces::CastleSmallCorridorPiece::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildForward((StartPiece *) startPiece, pieces, random, 1, 0, true);
}

NetherBridgePieces::CastleSmallCorridorPiece *NetherBridgePieces::CastleSmallCorridorPiece::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{

	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -1, 0, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new CastleSmallCorridorPiece(genDepth, random, box, direction);
}

bool NetherBridgePieces::CastleSmallCorridorPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 0, 0, 4, 1, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 0, 2, 0, 4, 5, 4, 0, 0, false);

	// walls
	generateBox(level, chunkBB, 0, 2, 0, 0, 5, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 4, 2, 0, 4, 5, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 3, 1, 0, 4, 1, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 0, 3, 3, 0, 4, 3, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 4, 3, 1, 4, 4, 1, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 4, 3, 3, 4, 4, 3, Tile::netherFence_Id, Tile::netherFence_Id, false);

	// roof
	generateBox(level, chunkBB, 0, 6, 0, 4, 6, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// supports
	for (int x = 0; x <= 4; x++)
	{
		for (int z = 0; z <= 4; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
		}
	}

	return true;
}


NetherBridgePieces::CastleSmallCorridorCrossingPiece::CastleSmallCorridorCrossingPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

void NetherBridgePieces::CastleSmallCorridorCrossingPiece::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildForward((StartPiece *) startPiece, pieces, random, 1, 0, true);
	generateChildLeft((StartPiece *) startPiece, pieces, random, 0, 1, true);
	generateChildRight((StartPiece *) startPiece, pieces, random, 0, 1, true);
}

NetherBridgePieces::CastleSmallCorridorCrossingPiece *NetherBridgePieces::CastleSmallCorridorCrossingPiece::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -1, 0, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new CastleSmallCorridorCrossingPiece(genDepth, random, box, direction);
}

bool NetherBridgePieces::CastleSmallCorridorCrossingPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 0, 0, 4, 1, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 0, 2, 0, 4, 5, 4, 0, 0, false);

	// walls
	generateBox(level, chunkBB, 0, 2, 0, 0, 5, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 4, 2, 0, 4, 5, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 2, 4, 0, 5, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 4, 2, 4, 4, 5, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// roof
	generateBox(level, chunkBB, 0, 6, 0, 4, 6, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// supports
	for (int x = 0; x <= 4; x++)
	{
		for (int z = 0; z <= 4; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
		}
	}

	return true;
}


NetherBridgePieces::CastleSmallCorridorRightTurnPiece::CastleSmallCorridorRightTurnPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

void NetherBridgePieces::CastleSmallCorridorRightTurnPiece::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildRight((StartPiece *) startPiece, pieces, random, 0, 1, true);
}

NetherBridgePieces::CastleSmallCorridorRightTurnPiece *NetherBridgePieces::CastleSmallCorridorRightTurnPiece::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -1, 0, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new CastleSmallCorridorRightTurnPiece(genDepth, random, box, direction);
}

bool NetherBridgePieces::CastleSmallCorridorRightTurnPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 0, 0, 4, 1, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 0, 2, 0, 4, 5, 4, 0, 0, false);

	// walls
	generateBox(level, chunkBB, 0, 2, 0, 0, 5, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 0, 3, 1, 0, 4, 1, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 0, 3, 3, 0, 4, 3, Tile::netherFence_Id, Tile::netherFence_Id, false);

	generateBox(level, chunkBB, 4, 2, 0, 4, 5, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	generateBox(level, chunkBB, 1, 2, 4, 4, 5, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 3, 4, 1, 4, 4, Tile::netherFence_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 3, 3, 4, 3, 4, 4, Tile::netherFence_Id, Tile::netherBrick_Id, false);

	// roof
	generateBox(level, chunkBB, 0, 6, 0, 4, 6, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// supports
	for (int x = 0; x <= 4; x++)
	{
		for (int z = 0; z <= 4; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
		}
	}

	return true;
}


NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::CastleSmallCorridorLeftTurnPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

void NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildLeft((StartPiece *) startPiece, pieces, random, 0, 1, true);
}

NetherBridgePieces::CastleSmallCorridorLeftTurnPiece *NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -1, 0, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new CastleSmallCorridorLeftTurnPiece(genDepth, random, box, direction);
}

bool NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 0, 0, 4, 1, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 0, 2, 0, 4, 5, 4, 0, 0, false);

	// walls
	generateBox(level, chunkBB, 4, 2, 0, 4, 5, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 4, 3, 1, 4, 4, 1, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 4, 3, 3, 4, 4, 3, Tile::netherFence_Id, Tile::netherFence_Id, false);

	generateBox(level, chunkBB, 0, 2, 0, 0, 5, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	generateBox(level, chunkBB, 0, 2, 4, 3, 5, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 3, 4, 1, 4, 4, Tile::netherFence_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 3, 3, 4, 3, 4, 4, Tile::netherFence_Id, Tile::netherBrick_Id, false);

	// roof
	generateBox(level, chunkBB, 0, 6, 0, 4, 6, 4, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// supports
	for (int x = 0; x <= 4; x++)
	{
		for (int z = 0; z <= 4; z++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
		}
	}

	return true;
}


NetherBridgePieces::CastleCorridorStairsPiece::CastleCorridorStairsPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

void NetherBridgePieces::CastleCorridorStairsPiece::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateChildForward((StartPiece *) startPiece, pieces, random, 1, 0, true);
}

NetherBridgePieces::CastleCorridorStairsPiece *NetherBridgePieces::CastleCorridorStairsPiece::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -1, -7, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new CastleCorridorStairsPiece(genDepth, random, box, direction);
}

bool NetherBridgePieces::CastleCorridorStairsPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// generate stairs
	int stairsData = getOrientationData(Tile::stairs_netherBricks_Id, 2);
	for (int step = 0; step <= 9; step++)
	{
		int floor = max(1, 7 - step);
		int roof = min(max(floor + 5, 14 - step), 13);
		int z = step;

		// floor
		generateBox(level, chunkBB, 0, 0, z, 4, floor, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
		// room air
		generateBox(level, chunkBB, 1, floor + 1, z, 3, roof - 1, z, 0, 0, false);
		if (step <= 6)
		{
			placeBlock(level, Tile::stairs_netherBricks_Id, stairsData, 1, floor + 1, z, chunkBB);
			placeBlock(level, Tile::stairs_netherBricks_Id, stairsData, 2, floor + 1, z, chunkBB);
			placeBlock(level, Tile::stairs_netherBricks_Id, stairsData, 3, floor + 1, z, chunkBB);
		}
		// roof
		generateBox(level, chunkBB, 0, roof, z, 4, roof, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
		// walls
		generateBox(level, chunkBB, 0, floor + 1, z, 0, roof - 1, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
		generateBox(level, chunkBB, 4, floor + 1, z, 4, roof - 1, z, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
		if ((step & 1) == 0)
		{
			generateBox(level, chunkBB, 0, floor + 2, z, 0, floor + 3, z, Tile::netherFence_Id, Tile::netherFence_Id, false);
			generateBox(level, chunkBB, 4, floor + 2, z, 4, floor + 3, z, Tile::netherFence_Id, Tile::netherFence_Id, false);
		}

		// supports
		for (int x = 0; x <= 4; x++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
		}
	}


	return true;
}


NetherBridgePieces::CastleCorridorTBalconyPiece::CastleCorridorTBalconyPiece(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : NetherBridgePiece(genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

void NetherBridgePieces::CastleCorridorTBalconyPiece::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	int zOff = 1;
	// compensate for weird negative-facing behaviour
	if (orientation == Direction::WEST || orientation == Direction::NORTH)
	{
		zOff = 5;
	}

	generateChildLeft((StartPiece *) startPiece, pieces, random, 0, zOff, random->nextInt(8) > 0);
	generateChildRight((StartPiece *) startPiece, pieces, random, 0, zOff, random->nextInt(8) > 0);
}

NetherBridgePieces::CastleCorridorTBalconyPiece *NetherBridgePieces::CastleCorridorTBalconyPiece::createPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, -3, 0, 0, width, height, depth, direction);

	StartPiece *startPiece = NULL;	
	if(pieces != NULL) startPiece = ((NetherBridgePieces::StartPiece *) pieces->front());

    if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return new CastleCorridorTBalconyPiece(genDepth, random, box, direction);
}

bool NetherBridgePieces::CastleCorridorTBalconyPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// floor
	generateBox(level, chunkBB, 0, 0, 0, 8, 1, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	// room air
	generateBox(level, chunkBB, 0, 2, 0, 8, 5, 8, 0, 0, false);
	// corridor roof
	generateBox(level, chunkBB, 0, 6, 0, 8, 6, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);

	// inside walls
	generateBox(level, chunkBB, 0, 2, 0, 2, 5, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 6, 2, 0, 8, 5, 0, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 3, 0, 1, 4, 0, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 7, 3, 0, 7, 4, 0, Tile::netherFence_Id, Tile::netherFence_Id, false);

	// balcony floor
	generateBox(level, chunkBB, 0, 2, 4, 8, 2, 8, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 1, 4, 2, 2, 4, 0, 0, false);
	generateBox(level, chunkBB, 6, 1, 4, 7, 2, 4, 0, 0, false);

	// hand rails
	generateBox(level, chunkBB, 0, 3, 8, 8, 3, 8, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 0, 3, 6, 0, 3, 7, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 8, 3, 6, 8, 3, 7, Tile::netherFence_Id, Tile::netherFence_Id, false);

	// balcony walls
	generateBox(level, chunkBB, 0, 3, 4, 0, 5, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 8, 3, 4, 8, 5, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 3, 5, 2, 5, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 6, 3, 5, 7, 5, 5, Tile::netherBrick_Id, Tile::netherBrick_Id, false);
	generateBox(level, chunkBB, 1, 4, 5, 1, 5, 5, Tile::netherFence_Id, Tile::netherFence_Id, false);
	generateBox(level, chunkBB, 7, 4, 5, 7, 5, 5, Tile::netherFence_Id, Tile::netherFence_Id, false);

	// supports
	for (int z = 0; z <= 5; z++)
	{
		for (int x = 0; x <= 8; x++)
		{
			fillColumnDown(level, Tile::netherBrick_Id, 0, x, -1, z, chunkBB);
		}
	}

	return true;
}


