#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.storage.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.levelgen.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.entity.npc.h"
#include "WeighedTreasure.h"
#include "VillagePieces.h"
#include "VillageFeature.h"
#include "Direction.h"
#include "JavaMath.h"
#include "BiomeSource.h"

WeighedTreasureArray VillagePieces::Smithy::treasureItems;

void VillagePieces::loadStatic()
{
	StructureFeatureIO::setPieceId(eStructurePiece_BookHouse, BookHouse::Create, L"ViBH");
	StructureFeatureIO::setPieceId(eStructurePiece_DoubleFarmland, DoubleFarmland::Create, L"ViDF");
	StructureFeatureIO::setPieceId(eStructurePiece_Farmland, Farmland::Create, L"ViF");
	StructureFeatureIO::setPieceId(eStructurePiece_LightPost, LightPost::Create, L"ViL");
	StructureFeatureIO::setPieceId(eStructurePiece_PigHouse, PigHouse::Create, L"ViPH");
	StructureFeatureIO::setPieceId(eStructurePiece_SimpleHouse, SimpleHouse::Create, L"ViSH");
	StructureFeatureIO::setPieceId(eStructurePiece_SmallHut, SmallHut::Create, L"ViSmH");
	StructureFeatureIO::setPieceId(eStructurePiece_SmallTemple, SmallTemple::Create, L"ViST");
	StructureFeatureIO::setPieceId(eStructurePiece_Smithy, Smithy::Create, L"ViS");
	StructureFeatureIO::setPieceId(eStructurePiece_VillageStartPiece, StartPiece::Create, L"ViStart");
	StructureFeatureIO::setPieceId(eStructurePiece_StraightRoad, StraightRoad::Create, L"ViSR");
	StructureFeatureIO::setPieceId(eStructurePiece_TwoRoomHouse, TwoRoomHouse::Create, L"ViTRH");
	StructureFeatureIO::setPieceId(eStructurePiece_Well, Well::Create, L"ViW");
}

VillagePieces::PieceWeight::PieceWeight(VillagePieces::EPieceClass pieceClass, int weight, int maxPlaceCount) : weight(weight)
{
	this->placeCount = 0;	// 4J added initialiser
	this->pieceClass = pieceClass;
	this->maxPlaceCount = maxPlaceCount;
}

bool VillagePieces::PieceWeight::doPlace(int depth)
{
	return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}

bool VillagePieces::PieceWeight::isValid()
{
	return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}

list<VillagePieces::PieceWeight *> *VillagePieces::createPieceSet(Random *random, int villageSize)
{
	list<PieceWeight *> *newPieces = new list<PieceWeight *>;

	newPieces->push_back(new PieceWeight(VillagePieces::EPieceClass_SimpleHouse, 4, Mth::nextInt(random, 2 + villageSize, 4 + villageSize * 2)));
	newPieces->push_back(new PieceWeight(VillagePieces::EPieceClass_SmallTemple, 20, Mth::nextInt(random, 0 + villageSize, 1 + villageSize)));
	newPieces->push_back(new PieceWeight(VillagePieces::EPieceClass_BookHouse, 20, Mth::nextInt(random, 0 + villageSize, 2 + villageSize)));
	newPieces->push_back(new PieceWeight(VillagePieces::EPieceClass_SmallHut, 3, Mth::nextInt(random, 2 + villageSize, 5 + villageSize * 3)));
	newPieces->push_back(new PieceWeight(VillagePieces::EPieceClass_PigHouse, 15, Mth::nextInt(random, 0 + villageSize, 2 + villageSize)));
	newPieces->push_back(new PieceWeight(VillagePieces::EPieceClass_DoubleFarmland, 3, Mth::nextInt(random, 1 + villageSize, 4 + villageSize)));
	newPieces->push_back(new PieceWeight(VillagePieces::EPieceClass_Farmland, 3, Mth::nextInt(random, 2 + villageSize, 4 + villageSize * 2)));
	newPieces->push_back(new PieceWeight(VillagePieces::EPieceClass_Smithy, 15, Mth::nextInt(random, 0, 1 + villageSize)));
	newPieces->push_back(new PieceWeight(VillagePieces::EPieceClass_TwoRoomHouse, 8, Mth::nextInt(random, 0 + villageSize, 3 + villageSize * 2)));

	// silly way of filtering "infinite" buildings
    auto it = newPieces->begin();
    while( it != newPieces->end() )
	{
		if( (*it)->maxPlaceCount == 0 )
		{
			delete (*it);
			it = newPieces->erase(it);
		}
		else
		{
			it++;
		}
	}

	return newPieces;
}

int VillagePieces::updatePieceWeight(list<PieceWeight *> *currentPieces)
{
	bool hasAnyPieces = false;
	int totalWeight = 0;
	for(auto& piece : *currentPieces)
	{
		if (piece->maxPlaceCount > 0 && piece->placeCount < piece->maxPlaceCount)
		{
			hasAnyPieces = true;
		}
		totalWeight += piece->weight;
	}
	return (hasAnyPieces ? totalWeight : -1);
}

VillagePieces::VillagePiece *VillagePieces::findAndCreatePieceFactory(StartPiece *startPiece, VillagePieces::PieceWeight *piece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth)
{
	VillagePieces::EPieceClass pieceClass = piece->pieceClass;
	VillagePiece *villagePiece = nullptr;

	if (pieceClass == VillagePieces::EPieceClass_SimpleHouse)
	{
		villagePiece = SimpleHouse::createPiece(startPiece, pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == VillagePieces::EPieceClass_SmallTemple)
	{
		villagePiece = SmallTemple::createPiece(startPiece, pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == VillagePieces::EPieceClass_BookHouse)
	{
		villagePiece = BookHouse::createPiece(startPiece, pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == VillagePieces::EPieceClass_SmallHut)
	{
		villagePiece = SmallHut::createPiece(startPiece, pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == VillagePieces::EPieceClass_PigHouse)
	{
		villagePiece = PigHouse::createPiece(startPiece, pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == VillagePieces::EPieceClass_DoubleFarmland)
	{
		villagePiece = DoubleFarmland::createPiece(startPiece, pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == VillagePieces::EPieceClass_Farmland)
	{
		villagePiece = Farmland::createPiece(startPiece, pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == VillagePieces::EPieceClass_Smithy)
	{
		villagePiece = Smithy::createPiece(startPiece, pieces, random, footX, footY, footZ, direction, depth);
	}
	else if (pieceClass == VillagePieces::EPieceClass_TwoRoomHouse)
	{
		villagePiece = TwoRoomHouse::createPiece(startPiece, pieces, random, footX, footY, footZ, direction, depth);
	}

	return villagePiece;
}

VillagePieces::VillagePiece *VillagePieces::generatePieceFromSmallDoor(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth)
{
	int totalWeight = updatePieceWeight(startPiece->pieceSet);
	if (totalWeight <= 0)
	{
		return nullptr;
	}

	int numAttempts = 0;
	while (numAttempts < 5)
	{
		numAttempts++;

		int weightSelection = random->nextInt(totalWeight);
        for ( PieceWeight *piece : *startPiece->pieceSet )
        {
			weightSelection -= piece->weight;
			if (weightSelection < 0)
			{
				if (!piece->doPlace(depth) || (piece == startPiece->previousPiece && startPiece->pieceSet->size() > 1))
				{
					break;
				}

				VillagePiece *villagePiece = findAndCreatePieceFactory(startPiece, piece, pieces, random, footX, footY, footZ, direction, depth);
				if (villagePiece != nullptr)
				{
					piece->placeCount++;
					startPiece->previousPiece = piece;

					if (!piece->isValid())
					{
						startPiece->pieceSet->remove(piece);
					}
					return villagePiece;
				}
			}
		}
	}

	// attempt to place a light post instead
	{
		BoundingBox *box = LightPost::findPieceBox(startPiece, pieces, random, footX, footY, footZ, direction);
		if (box != nullptr)
		{
			return new LightPost(startPiece, depth, random, box, direction);
		}
		delete box;
	}

	return nullptr;
}

StructurePiece *VillagePieces::generateAndAddPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth)
{
	if (depth > MAX_DEPTH)
	{
		return nullptr;
	}
	if (abs(footX - startPiece->getBoundingBox()->x0) > 7 * 16 || abs(footZ - startPiece->getBoundingBox()->z0) > 7 * 16)
	{
		return nullptr;
	}

	StructurePiece *newPiece = generatePieceFromSmallDoor(startPiece, pieces, random, footX, footY, footZ, direction, depth + 1);
	if (newPiece != nullptr)
	{
		int x = (newPiece->boundingBox->x0 + newPiece->boundingBox->x1) / 2;
		int z = (newPiece->boundingBox->z0 + newPiece->boundingBox->z1) / 2;
		int xs = newPiece->boundingBox->x1 - newPiece->boundingBox->x0;
		int zs = newPiece->boundingBox->z1 - newPiece->boundingBox->z0;
		int r = xs > zs ? xs : zs;
		if (startPiece->getBiomeSource()->containsOnly(x, z, r / 2 + 4, VillageFeature::allowedBiomes))
		{
			pieces->push_back(newPiece);
			startPiece->pendingHouses.push_back(newPiece);
			return newPiece;
		}
		delete newPiece;
	}
	return nullptr;
}

StructurePiece *VillagePieces::generateAndAddRoadPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth)
{
	if (depth > BASE_ROAD_DEPTH + startPiece->villageSize)
	{
		return nullptr;
	}
	if (abs(footX - startPiece->getBoundingBox()->x0) > 7 * 16 || abs(footZ - startPiece->getBoundingBox()->z0) > 7 * 16)
	{
		return nullptr;
	}

	BoundingBox *box = StraightRoad::findPieceBox(startPiece, pieces, random, footX, footY, footZ, direction);
	if (box != nullptr && box->y0 > LOWEST_Y_POSITION)
	{
		StructurePiece *newPiece = new StraightRoad(startPiece, depth, random, box, direction);
		int x = (newPiece->boundingBox->x0 + newPiece->boundingBox->x1) / 2;
		int z = (newPiece->boundingBox->z0 + newPiece->boundingBox->z1) / 2;
		int xs = newPiece->boundingBox->x1 - newPiece->boundingBox->x0;
		int zs = newPiece->boundingBox->z1 - newPiece->boundingBox->z0;
		int r = xs > zs ? xs : zs;
		if (startPiece->getBiomeSource()->containsOnly(x, z, r / 2 + 4, VillageFeature::allowedBiomes))
		{
			pieces->push_back(newPiece);
			startPiece->pendingRoads.push_back(newPiece);
			return newPiece;
		}
		// 4J Stu - The dtor for newPiece will destroy box
		delete newPiece;
	}
	else if(box != nullptr)
	{
		delete box;
	}

	return nullptr;
}

VillagePieces::VillagePiece::VillagePiece()
{
	heightPosition = -1;
	spawnedVillagerCount = 0;
	isDesertVillage = false;
	startPiece = nullptr;
	// for reflection
}

VillagePieces::VillagePiece::VillagePiece(StartPiece *startPiece, int genDepth) : StructurePiece(genDepth)
{
	heightPosition = -1;
	isDesertVillage = false;
	spawnedVillagerCount = 0;
	this->startPiece = startPiece;
	if (startPiece != nullptr)
	{
		this->isDesertVillage = startPiece->isDesertVillage;
	}
}

void VillagePieces::VillagePiece::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putInt(L"HPos", heightPosition);
	tag->putInt(L"VCount", spawnedVillagerCount);
	tag->putBoolean(L"Desert", isDesertVillage);
}

void VillagePieces::VillagePiece::readAdditonalSaveData(CompoundTag *tag)
{
	heightPosition = tag->getInt(L"HPos");
	spawnedVillagerCount = tag->getInt(L"VCount");
	isDesertVillage = tag->getBoolean(L"Desert");
}

StructurePiece *VillagePieces::VillagePiece::generateHouseNorthernLeft(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int yOff, int zOff)
{
	switch (orientation)
	{
	case Direction::NORTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 + yOff, boundingBox->z0 + zOff, Direction::WEST, getGenDepth());
	case Direction::SOUTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 + yOff, boundingBox->z0 + zOff, Direction::WEST, getGenDepth());
	case Direction::WEST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + zOff, boundingBox->y0 + yOff, boundingBox->z0 - 1, Direction::NORTH, getGenDepth());
	case Direction::EAST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + zOff, boundingBox->y0 + yOff, boundingBox->z0 - 1, Direction::NORTH, getGenDepth());
	}
	return nullptr;
}

StructurePiece *VillagePieces::VillagePiece::generateHouseNorthernRight(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int yOff, int zOff)
{
	switch (orientation)
	{
	case Direction::NORTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 + yOff, boundingBox->z0 + zOff, Direction::EAST, getGenDepth());
	case Direction::SOUTH:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 + yOff, boundingBox->z0 + zOff, Direction::EAST, getGenDepth());
	case Direction::WEST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + zOff, boundingBox->y0 + yOff, boundingBox->z1 + 1, Direction::SOUTH, getGenDepth());
	case Direction::EAST:
		return generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + zOff, boundingBox->y0 + yOff, boundingBox->z1 + 1, Direction::SOUTH, getGenDepth());
	}
	return nullptr;
}

int VillagePieces::VillagePiece::getAverageGroundHeight(Level *level, BoundingBox *chunkBB)
{
	int total = 0;
	int count = 0;
	for (int z = boundingBox->z0; z <= boundingBox->z1; z++)
	{
		for (int x = boundingBox->x0; x <= boundingBox->x1; x++)
		{
			if (chunkBB->isInside(x, 64, z))
			{
				total += Math::_max(level->getTopSolidBlock(x, z), level->dimension->getSpawnYPosition());
				count++;
			}
		}
	}

	if (count == 0)
	{
		return -1;
	}
	return total / count;
}

bool VillagePieces::VillagePiece::isOkBox(BoundingBox *box, StartPiece *startRoom)
{
	bool bIsOk = false;

	if(box != nullptr)
	{
		if( box->y0 > LOWEST_Y_POSITION ) bIsOk = true;

		int xzSize = startRoom->m_level->getLevelData()->getXZSize();
		int blockMin = -( (xzSize << 4) / 2) + 1;
		int blockMax = ( (xzSize << 4) / 2 ) - 1;

		if(box->x0 <= blockMin) bIsOk = false;
		if(box->z0 <= blockMin) bIsOk = false;
		if(box->x1 >= blockMax) bIsOk = false;
		if(box->z1 >= blockMax) bIsOk = false;
	}

	return bIsOk;
}

void VillagePieces::VillagePiece::spawnVillagers(Level *level, BoundingBox *chunkBB, int x, int y, int z, int count)
{
	if (spawnedVillagerCount >= count)
	{
		return;
	}

	for (int i = spawnedVillagerCount; i < count; i++)
	{
		int worldX = getWorldX(x + i, z);
		int worldY = getWorldY(y);
		int worldZ = getWorldZ(x + i, z);

		if (chunkBB->isInside(worldX, worldY, worldZ))
		{
			spawnedVillagerCount++;

			shared_ptr<Villager> villager = std::make_shared<Villager>(level, getVillagerProfession(i));
			villager->moveTo(worldX + 0.5, worldY, worldZ + 0.5, 0, 0);
			level->addEntity(villager);
		}
		else
		{
			// try again later
			break;
		}
	}
}

int VillagePieces::VillagePiece::getVillagerProfession(int villagerNumber)
{
	return Villager::PROFESSION_FARMER;
}

int VillagePieces::VillagePiece::biomeBlock(int tile, int data)
{
	if (isDesertVillage)
	{
		if (tile == Tile::treeTrunk_Id)
		{
			return Tile::sandStone_Id;
		}
		else if (tile == Tile::cobblestone_Id)
		{
			return Tile::sandStone_Id;
		}
		else if (tile == Tile::wood_Id)
		{
			return Tile::sandStone_Id;
		}
		else if (tile == Tile::stairs_wood_Id)
		{
			return Tile::stairs_sandstone_Id;
		}
		else if (tile == Tile::stairs_stone_Id)
		{
			return Tile::stairs_sandstone_Id;
		}
		else if (tile == Tile::gravel_Id)
		{
			return Tile::sandStone_Id;
		}
	}
	return tile;
}

int VillagePieces::VillagePiece::biomeData(int tile, int data)
{
	if (isDesertVillage)
	{
		if (tile == Tile::treeTrunk_Id)
		{
			return 0;
		}
		else if (tile == Tile::cobblestone_Id)
		{
			return SandStoneTile::TYPE_DEFAULT;
		}
		else if (tile == Tile::wood_Id)
		{
			return SandStoneTile::TYPE_SMOOTHSIDE;
		}
	}
	return data;
}

void VillagePieces::VillagePiece::placeBlock(Level *level, int block, int data, int x, int y, int z, BoundingBox *chunkBB)
{
	int bblock = biomeBlock(block, data);
	int bdata = biomeData(block, data);
	StructurePiece::placeBlock(level, bblock, bdata, x, y, z, chunkBB);
}

void VillagePieces::VillagePiece::generateBox(Level *level, BoundingBox *chunkBB, int x0, int y0, int z0, int x1, int y1, int z1, int edgeTile, int fillTile, bool skipAir)
{
	int bEdge = biomeBlock(edgeTile, 0);
	int bEdgeData = biomeData(edgeTile, 0);
	int bFill = biomeBlock(fillTile, 0);
	int bFillData = biomeData(fillTile, 0);
	StructurePiece::generateBox(level, chunkBB, x0, y0, z0, x1, y1, z1, bEdge, bEdgeData, bFill, bFillData, skipAir);
}

void VillagePieces::VillagePiece::fillColumnDown(Level *level, int block, int data, int x, int startY, int z, BoundingBox *chunkBB)
{
	int bblock = biomeBlock(block, data);
	int bdata = biomeData(block, data);
	StructurePiece::fillColumnDown(level, bblock, bdata, x, startY, z, chunkBB);
}

VillagePieces::Well::Well()
{
	// for reflection
}

VillagePieces::Well::Well(StartPiece *startPiece, int genDepth, Random *random, int west, int north) : VillagePiece(startPiece, genDepth)
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

VillagePieces::Well::Well(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

void VillagePieces::Well::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x0 - 1, boundingBox->y1 - 4, boundingBox->z0 + 1, Direction::WEST, getGenDepth());
	generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x1 + 1, boundingBox->y1 - 4, boundingBox->z0 + 1, Direction::EAST, getGenDepth());
	generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x0 + 1, boundingBox->y1 - 4, boundingBox->z0 - 1, Direction::NORTH, getGenDepth());
	generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x0 + 1, boundingBox->y1 - 4, boundingBox->z1 + 1, Direction::SOUTH, getGenDepth());
}

bool VillagePieces::Well::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + 3, 0);
	}

	generateBox(level, chunkBB, 1, 0, 1, 4, height - 3, 4, Tile::cobblestone_Id, Tile::water_Id, false);
	placeBlock(level, 0, 0, 2, height - 3, 2, chunkBB);
	placeBlock(level, 0, 0, 3, height - 3, 2, chunkBB);
	placeBlock(level, 0, 0, 2, height - 3, 3, chunkBB);
	placeBlock(level, 0, 0, 3, height - 3, 3, chunkBB);

	placeBlock(level, Tile::fence_Id, 0, 1, height - 2, 1, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 1, height - 1, 1, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 4, height - 2, 1, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 4, height - 1, 1, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 1, height - 2, 4, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 1, height - 1, 4, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 4, height - 2, 4, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 4, height - 1, 4, chunkBB);
	generateBox(level, chunkBB, 1, height, 1, 4, height, 4, Tile::cobblestone_Id, Tile::cobblestone_Id, false);

	for (int z = 0; z <= 5; z++)
	{
		for (int x = 0; x <= 5; x++)
		{
			// only do the frame
			if (x != 0 && x != 5 && z != 0 && z != 5)
			{
				continue;
			}
			placeBlock(level, Tile::gravel_Id, 0, x, height - 4, z, chunkBB);
			generateAirColumnUp(level, x, height - 3, z, chunkBB);
		}
	}

	return true;

}

VillagePieces::StartPiece::StartPiece()
{
	// for reflection
}

VillagePieces::StartPiece::StartPiece(BiomeSource *biomeSource, int genDepth, Random *random, int west, int north, list<PieceWeight *> *pieceSet, int villageSize, Level *level) : Well(nullptr, 0, random, west, north)
{
	isLibraryAdded = false;		// 4J - added initialiser
	previousPiece = nullptr;		// 4J - added initialiser
	this->biomeSource = biomeSource;
	this->pieceSet = pieceSet;
	this->villageSize = villageSize;
	m_level = level;

	Biome *biome = biomeSource->getBiome(west, north);
	isDesertVillage = biome == Biome::desert || biome == Biome::desertHills;
}

VillagePieces::StartPiece::~StartPiece()
{
	for(auto& it : *pieceSet)
	{
		delete it;
	}
	delete pieceSet;
}

BiomeSource *VillagePieces::StartPiece::getBiomeSource()
{
	return biomeSource;
}

VillagePieces::StraightRoad::StraightRoad()
{
	// for reflection
}

VillagePieces::StraightRoad::StraightRoad(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillageRoadPiece(startPiece, genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
	length = Math::_max(stairsBox->getXSpan(), stairsBox->getZSpan());
}

void VillagePieces::StraightRoad::addAdditonalSaveData(CompoundTag *tag)
{
	VillageRoadPiece::addAdditonalSaveData(tag);
	tag->putInt(L"Length", length);
}

void VillagePieces::StraightRoad::readAdditonalSaveData(CompoundTag *tag)
{
	VillageRoadPiece::readAdditonalSaveData(tag);
	length = tag->getInt(L"Length");
}

void VillagePieces::StraightRoad::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	bool hasHouses = false;

	// place left houses
	int depth = random->nextInt(5);
	while (depth < length - 8)
	{
		StructurePiece *piece = generateHouseNorthernLeft(static_cast<StartPiece *>(startPiece), pieces, random, 0, depth);
		if (piece != nullptr)
		{
			depth += Math::_max(piece->boundingBox->getXSpan(), piece->boundingBox->getZSpan());
			hasHouses = true;
		}
		depth += 2 + random->nextInt(5);
	}

	// place right houses
	depth = random->nextInt(5);
	while (depth < length - 8)
	{
		StructurePiece *piece = generateHouseNorthernRight(static_cast<StartPiece *>(startPiece), pieces, random, 0, depth);
		if (piece != nullptr)
		{
			depth += Math::_max(piece->boundingBox->getXSpan(), piece->boundingBox->getZSpan());
			hasHouses = true;
		}
		depth += 2 + random->nextInt(5);
	}

	if (hasHouses && random->nextInt(3) > 0)
	{
		switch (orientation)
		{
		case Direction::NORTH:
			generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x0 - 1, boundingBox->y0, boundingBox->z0, Direction::WEST, getGenDepth());
			break;
		case Direction::SOUTH:
			generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x0 - 1, boundingBox->y0, boundingBox->z1 - 2, Direction::WEST, getGenDepth());
			break;
		case Direction::EAST:
			generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x1 - 2, boundingBox->y0, boundingBox->z0 - 1, Direction::NORTH, getGenDepth());
			break;
		case Direction::WEST:
			generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x0, boundingBox->y0, boundingBox->z0 - 1, Direction::NORTH, getGenDepth());
			break;
		}
	}
	if (hasHouses && random->nextInt(3) > 0)
	{
		switch (orientation)
		{
		case Direction::NORTH:
			generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x1 + 1, boundingBox->y0, boundingBox->z0, Direction::EAST, getGenDepth());
			break;
		case Direction::SOUTH:
			generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x1 + 1, boundingBox->y0, boundingBox->z1 - 2, Direction::EAST, getGenDepth());
			break;
		case Direction::EAST:
			generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x1 - 2, boundingBox->y0, boundingBox->z1 + 1, Direction::SOUTH, getGenDepth());
			break;
		case Direction::WEST:
			generateAndAddRoadPiece(static_cast<StartPiece *>(startPiece), pieces, random, boundingBox->x0, boundingBox->y0, boundingBox->z1 + 1, Direction::SOUTH, getGenDepth());
			break;
		}
	}
}

BoundingBox *VillagePieces::StraightRoad::findPieceBox(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction)
{
	int length = 7 * (Mth::nextInt(random, 3, 5));

	while (length >= 7)
	{
		BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, 3, length, direction);

		if (isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == nullptr)
		{
			return box;
		}
		delete box;
		length -= 7;
	}

	return nullptr;
}

bool VillagePieces::StraightRoad::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	int tile = biomeBlock(Tile::gravel_Id, 0);
	for (int x = boundingBox->x0; x <= boundingBox->x1; x++)
	{
		for (int z = boundingBox->z0; z <= boundingBox->z1; z++)
		{
			if (chunkBB->isInside(x, 64, z))
			{
				int y = level->getTopSolidBlock(x, z) - 1;
				level->setTileAndData(x, y, z,tile, 0, Tile::UPDATE_CLIENTS);
			}
		}
	}

	return true;
}

VillagePieces::SimpleHouse::SimpleHouse()
{
	hasTerrace = false;
	// for reflection
}

VillagePieces::SimpleHouse::SimpleHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth), hasTerrace(random->nextBoolean())
{
	orientation = direction;
	boundingBox = stairsBox;
}

void VillagePieces::SimpleHouse::addAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::addAdditonalSaveData(tag);
	tag->putBoolean(L"Terrace", hasTerrace);
}

void VillagePieces::SimpleHouse::readAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::readAdditonalSaveData(tag);
	hasTerrace = tag->getBoolean(L"Terrace");
}

VillagePieces::SimpleHouse *VillagePieces::SimpleHouse::createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return new SimpleHouse(startPiece, genDepth, random, box, direction);
}

bool VillagePieces::SimpleHouse::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// floor
	generateBox(level, chunkBB, 0, 0, 0, 4, 0, 4, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// roof
	generateBox(level, chunkBB, 0, 4, 0, 4, 4, 4, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 1, 4, 1, 3, 4, 3, Tile::wood_Id, Tile::wood_Id, false);

	// window walls
	placeBlock(level, Tile::cobblestone_Id, 0, 0, 1, 0, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 0, 2, 0, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 0, 3, 0, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 4, 1, 0, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 4, 2, 0, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 4, 3, 0, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 0, 1, 4, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 0, 2, 4, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 0, 3, 4, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 4, 1, 4, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 4, 2, 4, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 4, 3, 4, chunkBB);
	generateBox(level, chunkBB, 0, 1, 1, 0, 3, 3, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 4, 1, 1, 4, 3, 3, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 1, 4, 3, 3, 4, Tile::wood_Id, Tile::wood_Id, false);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 2, 4, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 4, 2, 2, chunkBB);

	// door wall
	placeBlock(level, Tile::wood_Id, 0, 1, 1, 0, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 1, 2, 0, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 1, 3, 0, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 2, 3, 0, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 3, 3, 0, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 3, 2, 0, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 3, 1, 0, chunkBB);
	if (getBlock(level, 2, 0, -1, chunkBB) == 0 && getBlock(level, 2, -1, -1, chunkBB) != 0)
	{
		placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), 2, 0, -1, chunkBB);
	}

	// fill room with air
	generateBox(level, chunkBB, 1, 1, 1, 3, 3, 3, 0, 0, false);

	// roof fence
	if (hasTerrace) {
		placeBlock(level, Tile::fence_Id, 0, 0, 5, 0, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 1, 5, 0, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 2, 5, 0, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 3, 5, 0, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 4, 5, 0, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 0, 5, 4, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 1, 5, 4, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 2, 5, 4, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 3, 5, 4, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 4, 5, 4, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 4, 5, 1, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 4, 5, 2, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 4, 5, 3, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 0, 5, 1, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 0, 5, 2, chunkBB);
		placeBlock(level, Tile::fence_Id, 0, 0, 5, 3, chunkBB);
	}

	// ladder
	if (hasTerrace)
	{
		int orientationData = getOrientationData(Tile::ladder_Id, 3);
		placeBlock(level, Tile::ladder_Id, orientationData, 3, 1, 3, chunkBB);
		placeBlock(level, Tile::ladder_Id, orientationData, 3, 2, 3, chunkBB);
		placeBlock(level, Tile::ladder_Id, orientationData, 3, 3, 3, chunkBB);
		placeBlock(level, Tile::ladder_Id, orientationData, 3, 4, 3, chunkBB);
	}

	// torch
	placeBlock(level, Tile::torch_Id, 0, 2, 3, 1, chunkBB);

	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::cobblestone_Id, 0, x, -1, z, chunkBB);
		}
	}

	spawnVillagers(level, chunkBB, 1, 1, 2, 1);

	return true;

}

VillagePieces::SmallTemple::SmallTemple()
{
	// for reflection
}

VillagePieces::SmallTemple::SmallTemple(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth)
{
	heightPosition = -1; // 4J added initialiser
	orientation = direction;
	boundingBox = stairsBox;
}

VillagePieces::SmallTemple *VillagePieces::SmallTemple::createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return new SmallTemple(startPiece, genDepth, random, box, direction);
}

bool VillagePieces::SmallTemple::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// fill inside with air
	generateBox(level, chunkBB, 1, 1, 1, 3, 3, 7, 0, 0, false);
	generateBox(level, chunkBB, 1, 5, 1, 3, 9, 3, 0, 0, false);

	// floor
	generateBox(level, chunkBB, 1, 0, 0, 3, 0, 8, Tile::cobblestone_Id, Tile::cobblestone_Id, false);

	// front wall
	generateBox(level, chunkBB, 1, 1, 0, 3, 10, 0, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// left tall wall
	generateBox(level, chunkBB, 0, 1, 1, 0, 10, 3, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// right tall wall
	generateBox(level, chunkBB, 4, 1, 1, 4, 10, 3, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// left low wall
	generateBox(level, chunkBB, 0, 0, 4, 0, 4, 7, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// right low wall
	generateBox(level, chunkBB, 4, 0, 4, 4, 4, 7, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// far low wall
	generateBox(level, chunkBB, 1, 1, 8, 3, 4, 8, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// far upper wall
	generateBox(level, chunkBB, 1, 5, 4, 3, 10, 4, Tile::cobblestone_Id, Tile::cobblestone_Id, false);

	// low roof
	generateBox(level, chunkBB, 1, 5, 5, 3, 5, 7, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// high roof
	generateBox(level, chunkBB, 0, 9, 0, 4, 9, 4, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// middle floor / roof
	generateBox(level, chunkBB, 0, 4, 0, 4, 4, 4, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	placeBlock(level, Tile::cobblestone_Id, 0, 0, 11, 2, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 4, 11, 2, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 2, 11, 0, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 2, 11, 4, chunkBB);

	// altar pieces
	placeBlock(level, Tile::cobblestone_Id, 0, 1, 1, 6, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 1, 1, 7, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 2, 1, 7, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 3, 1, 6, chunkBB);
	placeBlock(level, Tile::cobblestone_Id, 0, 3, 1, 7, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), 1, 1, 5, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), 2, 1, 6, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), 3, 1, 5, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 1), 1, 2, 7, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 0), 3, 2, 7, chunkBB);

	// windows
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 3, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 4, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 4, 3, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 6, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 7, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 4, 6, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 4, 7, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 6, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 7, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 6, 4, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 7, 4, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 3, 6, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 4, 3, 6, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 3, 8, chunkBB);

	// torches
	placeBlock(level, Tile::torch_Id, 0, 2, 4, 7, chunkBB);
	placeBlock(level, Tile::torch_Id, 0, 1, 4, 6, chunkBB);
	placeBlock(level, Tile::torch_Id, 0, 3, 4, 6, chunkBB);
	placeBlock(level, Tile::torch_Id, 0, 2, 4, 5, chunkBB);

	// ladder
	int orientationData = getOrientationData(Tile::ladder_Id, 4);
	for (int y = 1; y <= 9; y++)
	{
		placeBlock(level, Tile::ladder_Id, orientationData, 3, y, 3, chunkBB);
	}

	// entrance
	placeBlock(level, 0, 0, 2, 1, 0, chunkBB);
	placeBlock(level, 0, 0, 2, 2, 0, chunkBB);
	createDoor(level, chunkBB, random, 2, 1, 0, getOrientationData(Tile::door_wood_Id, 1));
	if (getBlock(level, 2, 0, -1, chunkBB) == 0 && getBlock(level, 2, -1, -1, chunkBB) != 0)
	{
		placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), 2, 0, -1, chunkBB);
	}


	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::cobblestone_Id, 0, x, -1, z, chunkBB);
		}
	}

	spawnVillagers(level, chunkBB, 2, 1, 2, 1);

	return true;

}

int VillagePieces::SmallTemple::getVillagerProfession(int villagerNumber)
{
	return Villager::PROFESSION_PRIEST;
}

VillagePieces::BookHouse::BookHouse()
{
	// for reflection
}

VillagePieces::BookHouse::BookHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth)
{
	heightPosition = -1; // 4J added initialiser
	orientation = direction;
	boundingBox = stairsBox;
}

VillagePieces::BookHouse *VillagePieces::BookHouse::createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return new BookHouse(startPiece, genDepth, random, box, direction);
}

bool VillagePieces::BookHouse::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// fill inside with air
	generateBox(level, chunkBB, 1, 1, 1, 7, 5, 4, 0, 0, false);

	// floor
	generateBox(level, chunkBB, 0, 0, 0, 8, 0, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	// roof
	generateBox(level, chunkBB, 0, 5, 0, 8, 5, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 0, 6, 1, 8, 6, 4, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 0, 7, 2, 8, 7, 3, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	int southStairs = getOrientationData(Tile::stairs_wood_Id, 3);
	int northStairs = getOrientationData(Tile::stairs_wood_Id, 2);
	for (int d = -1; d <= 2; d++) {
		for (int w = 0; w <= 8; w++) {
			placeBlock(level, Tile::stairs_wood_Id, southStairs, w, 6 + d, d, chunkBB);
			placeBlock(level, Tile::stairs_wood_Id, northStairs, w, 6 + d, 5 - d, chunkBB);
		}
	}

	// rock supports
	generateBox(level, chunkBB, 0, 1, 0, 0, 1, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 1, 1, 5, 8, 1, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 8, 1, 0, 8, 1, 4, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 2, 1, 0, 7, 1, 0, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 0, 2, 0, 0, 4, 0, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 0, 2, 5, 0, 4, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 8, 2, 5, 8, 4, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 8, 2, 0, 8, 4, 0, Tile::cobblestone_Id, Tile::cobblestone_Id, false);

	// wooden walls
	generateBox(level, chunkBB, 0, 2, 1, 0, 4, 4, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 2, 5, 7, 4, 5, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 8, 2, 1, 8, 4, 4, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 2, 0, 7, 4, 0, Tile::wood_Id, Tile::wood_Id, false);

	// windows
	placeBlock(level, Tile::thinGlass_Id, 0, 4, 2, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 5, 2, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 6, 2, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 4, 3, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 5, 3, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 6, 3, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 3, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 3, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 3, 3, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 2, 3, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 3, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 3, 3, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 2, 5, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 3, 2, 5, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 5, 2, 5, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 6, 2, 5, chunkBB);

	// roof inside and bookshelf
	generateBox(level, chunkBB, 1, 4, 1, 7, 4, 1, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 4, 4, 7, 4, 4, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 3, 4, 7, 3, 4, Tile::bookshelf_Id, Tile::bookshelf_Id, false);

	// couch
	placeBlock(level, Tile::wood_Id, 0, 7, 1, 4, chunkBB);
	placeBlock(level, Tile::stairs_wood_Id, getOrientationData(Tile::stairs_wood_Id, 0), 7, 1, 3, chunkBB);
	int orientationData = getOrientationData(Tile::stairs_wood_Id, 3);
	placeBlock(level, Tile::stairs_wood_Id, orientationData, 6, 1, 4, chunkBB);
	placeBlock(level, Tile::stairs_wood_Id, orientationData, 5, 1, 4, chunkBB);
	placeBlock(level, Tile::stairs_wood_Id, orientationData, 4, 1, 4, chunkBB);
	placeBlock(level, Tile::stairs_wood_Id, orientationData, 3, 1, 4, chunkBB);

	// tables
	placeBlock(level, Tile::fence_Id, 0, 6, 1, 3, chunkBB);
	placeBlock(level, Tile::pressurePlate_wood_Id, 0, 6, 2, 3, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 4, 1, 3, chunkBB);
	placeBlock(level, Tile::pressurePlate_wood_Id, 0, 4, 2, 3, chunkBB);
	placeBlock(level, Tile::workBench_Id, 0, 7, 1, 1, chunkBB);

	// entrance
	placeBlock(level, 0, 0, 1, 1, 0, chunkBB);
	placeBlock(level, 0, 0, 1, 2, 0, chunkBB);
	createDoor(level, chunkBB, random, 1, 1, 0, getOrientationData(Tile::door_wood_Id, 1));
	if (getBlock(level, 1, 0, -1, chunkBB) == 0 && getBlock(level, 1, -1, -1, chunkBB) != 0)
	{
		placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), 1, 0, -1, chunkBB);
	}

	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::cobblestone_Id, 0, x, -1, z, chunkBB);
		}
	}

	spawnVillagers(level, chunkBB, 2, 1, 2, 1);

	return true;

}

int VillagePieces::BookHouse::getVillagerProfession(int villagerNumber)
{
	return Villager::PROFESSION_LIBRARIAN;
}

VillagePieces::SmallHut::SmallHut()
{
	// for reflection
}

VillagePieces::SmallHut::SmallHut(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth), lowCeiling(random->nextBoolean()), tablePlacement(random->nextInt(3))
{
	heightPosition = -1; // 4J added initialiser

	orientation = direction;
	boundingBox = stairsBox;
}

void VillagePieces::SmallHut::addAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::addAdditonalSaveData(tag);
	tag->putInt(L"T", tablePlacement);
	tag->putBoolean(L"C", lowCeiling);
}

void VillagePieces::SmallHut::readAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::readAdditonalSaveData(tag);
	tablePlacement = tag->getInt(L"T");
	lowCeiling = tag->getBoolean(L"C");
}

VillagePieces::SmallHut *VillagePieces::SmallHut::createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return new SmallHut(startPiece, genDepth, random, box, direction);
}

bool VillagePieces::SmallHut::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// fill inside with air
	generateBox(level, chunkBB, 1, 1, 1, 3, 5, 4, 0, 0, false);

	// floor
	generateBox(level, chunkBB, 0, 0, 0, 3, 0, 4, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 1, 0, 1, 2, 0, 3, Tile::dirt_Id, Tile::dirt_Id, false);
	// roof
	if (lowCeiling) {
		generateBox(level, chunkBB, 1, 4, 1, 2, 4, 3, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	} else {
		generateBox(level, chunkBB, 1, 5, 1, 2, 5, 3, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	}
	placeBlock(level, Tile::treeTrunk_Id, 0, 1, 4, 0, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 2, 4, 0, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 1, 4, 4, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 2, 4, 4, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 0, 4, 1, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 0, 4, 2, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 0, 4, 3, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 3, 4, 1, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 3, 4, 2, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 3, 4, 3, chunkBB);

	// corners
	generateBox(level, chunkBB, 0, 1, 0, 0, 3, 0, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 3, 1, 0, 3, 3, 0, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 0, 1, 4, 0, 3, 4, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 3, 1, 4, 3, 3, 4, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);

	// wooden walls
	generateBox(level, chunkBB, 0, 1, 1, 0, 3, 3, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 3, 1, 1, 3, 3, 3, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 1, 0, 2, 3, 0, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 1, 4, 2, 3, 4, Tile::wood_Id, Tile::wood_Id, false);

	// windows
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 3, 2, 2, chunkBB);

	// table
	if (tablePlacement > 0) {
		placeBlock(level, Tile::fence_Id, 0, tablePlacement, 1, 3, chunkBB);
		placeBlock(level, Tile::pressurePlate_wood_Id, 0, tablePlacement, 2, 3, chunkBB);
	}

	// entrance
	placeBlock(level, 0, 0, 1, 1, 0, chunkBB);
	placeBlock(level, 0, 0, 1, 2, 0, chunkBB);
	createDoor(level, chunkBB, random, 1, 1, 0, getOrientationData(Tile::door_wood_Id, 1));
	if (getBlock(level, 1, 0, -1, chunkBB) == 0 && getBlock(level, 1, -1, -1, chunkBB) != 0)
	{
		placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), 1, 0, -1, chunkBB);
	}

	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::cobblestone_Id, 0, x, -1, z, chunkBB);
		}
	}

	spawnVillagers(level, chunkBB, 1, 1, 2, 1);

	return true;

}

VillagePieces::PigHouse::PigHouse()
{
	// for reflection
}

VillagePieces::PigHouse::PigHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;
}

VillagePieces::PigHouse *VillagePieces::PigHouse::createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{

	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return new PigHouse(startPiece, genDepth, random, box, direction);
}

bool VillagePieces::PigHouse::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// fill inside with air
	generateBox(level, chunkBB, 1, 1, 1, 7, 4, 4, 0, 0, false);
	generateBox(level, chunkBB, 2, 1, 6, 8, 4, 10, 0, 0, false);

	// pig floor
	generateBox(level, chunkBB, 2, 0, 6, 8, 0, 10, Tile::dirt_Id, Tile::dirt_Id, false);
	placeBlock(level, Tile::cobblestone_Id, 0, 6, 0, 6, chunkBB);
	// pig fence
	generateBox(level, chunkBB, 2, 1, 6, 2, 1, 10, Tile::fence_Id, Tile::fence_Id, false);
	generateBox(level, chunkBB, 8, 1, 6, 8, 1, 10, Tile::fence_Id, Tile::fence_Id, false);
	generateBox(level, chunkBB, 3, 1, 10, 7, 1, 10, Tile::fence_Id, Tile::fence_Id, false);

	// floor
	generateBox(level, chunkBB, 1, 0, 1, 7, 0, 4, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 0, 0, 0, 3, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 8, 0, 0, 8, 3, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 1, 0, 0, 7, 1, 0, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 1, 0, 5, 7, 1, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);

	// roof
	generateBox(level, chunkBB, 1, 2, 0, 7, 3, 0, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 2, 5, 7, 3, 5, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 4, 1, 8, 4, 1, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 4, 4, 8, 4, 4, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 5, 2, 8, 5, 3, Tile::wood_Id, Tile::wood_Id, false);
	placeBlock(level, Tile::wood_Id, 0, 0, 4, 2, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 0, 4, 3, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 8, 4, 2, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 8, 4, 3, chunkBB);

	int southStairs = getOrientationData(Tile::stairs_wood_Id, 3);
	int northStairs = getOrientationData(Tile::stairs_wood_Id, 2);
	for (int d = -1; d <= 2; d++)
	{
		for (int w = 0; w <= 8; w++)
		{
			placeBlock(level, Tile::stairs_wood_Id, southStairs, w, 4 + d, d, chunkBB);
			placeBlock(level, Tile::stairs_wood_Id, northStairs, w, 4 + d, 5 - d, chunkBB);
		}
	}

	// windows etc
	placeBlock(level, Tile::treeTrunk_Id, 0, 0, 2, 1, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 0, 2, 4, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 8, 2, 1, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 8, 2, 4, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 3, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 2, 3, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 2, 5, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 3, 2, 5, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 5, 2, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 6, 2, 5, chunkBB);

	// table
	placeBlock(level, Tile::fence_Id, 0, 2, 1, 3, chunkBB);
	placeBlock(level, Tile::pressurePlate_wood_Id, 0, 2, 2, 3, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 1, 1, 4, chunkBB);
	placeBlock(level, Tile::stairs_wood_Id, getOrientationData(Tile::stairs_wood_Id, 3), 2, 1, 4, chunkBB);
	placeBlock(level, Tile::stairs_wood_Id, getOrientationData(Tile::stairs_wood_Id, 1), 1, 1, 3, chunkBB);

	// butcher table
	generateBox(level, chunkBB, 5, 0, 1, 7, 0, 3, Tile::stoneSlab_Id, Tile::stoneSlab_Id, false);
	placeBlock(level, Tile::stoneSlab_Id, 0, 6, 1, 1, chunkBB);
	placeBlock(level, Tile::stoneSlab_Id, 0, 6, 1, 2, chunkBB);

	// entrance
	placeBlock(level, 0, 0, 2, 1, 0, chunkBB);
	placeBlock(level, 0, 0, 2, 2, 0, chunkBB);
	placeBlock(level, Tile::torch_Id, 0, 2, 3, 1, chunkBB);
	createDoor(level, chunkBB, random, 2, 1, 0, getOrientationData(Tile::door_wood_Id, 1));
	if (getBlock(level, 2, 0, -1, chunkBB) == 0 && getBlock(level, 2, -1, -1, chunkBB) != 0)
	{
		placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), 2, 0, -1, chunkBB);
	}

	// pig entrance
	placeBlock(level, 0, 0, 6, 1, 5, chunkBB);
	placeBlock(level, 0, 0, 6, 2, 5, chunkBB);
	placeBlock(level, Tile::torch_Id, 0, 6, 3, 4, chunkBB);
	createDoor(level, chunkBB, random, 6, 1, 5, getOrientationData(Tile::door_wood_Id, 1));

	for (int z = 0; z < 5; z++)
	{
		for (int x = 0; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::cobblestone_Id, 0, x, -1, z, chunkBB);
		}
	}

	spawnVillagers(level, chunkBB, 4, 1, 2, 2);

	return true;

}

int VillagePieces::PigHouse::getVillagerProfession(int villagerNumber)
{
	if (villagerNumber == 0)
	{
		return Villager::PROFESSION_BUTCHER;
	}
	return Villager::PROFESSION_FARMER;
}

VillagePieces::TwoRoomHouse::TwoRoomHouse()
{
	// for reflection
}

VillagePieces::TwoRoomHouse::TwoRoomHouse(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth)
{
	heightPosition = -1;	// 4J added initialiser

	orientation = direction;
	boundingBox = stairsBox;
}

VillagePieces::TwoRoomHouse *VillagePieces::TwoRoomHouse::createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return new TwoRoomHouse(startPiece, genDepth, random, box, direction);
}

bool VillagePieces::TwoRoomHouse::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// fill inside with air
	generateBox(level, chunkBB, 1, 1, 1, 7, 4, 4, 0, 0, false);
	generateBox(level, chunkBB, 2, 1, 6, 8, 4, 10, 0, 0, false);

	// floor
	generateBox(level, chunkBB, 2, 0, 5, 8, 0, 10, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 0, 1, 7, 0, 4, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 0, 0, 0, 3, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 8, 0, 0, 8, 3, 10, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 1, 0, 0, 7, 2, 0, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 1, 0, 5, 2, 1, 5, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 2, 0, 6, 2, 3, 10, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 3, 0, 10, 7, 3, 10, Tile::cobblestone_Id, Tile::cobblestone_Id, false);

	// room 1 roof
	generateBox(level, chunkBB, 1, 2, 0, 7, 3, 0, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 2, 5, 2, 3, 5, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 4, 1, 8, 4, 1, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 4, 4, 3, 4, 4, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 5, 2, 8, 5, 3, Tile::wood_Id, Tile::wood_Id, false);
	placeBlock(level, Tile::wood_Id, 0, 0, 4, 2, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 0, 4, 3, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 8, 4, 2, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 8, 4, 3, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 8, 4, 4, chunkBB);

	int southStairs = getOrientationData(Tile::stairs_wood_Id, 3);
	int northStairs = getOrientationData(Tile::stairs_wood_Id, 2);
	for (int d = -1; d <= 2; d++)
	{
		for (int w = 0; w <= 8; w++)
		{
			placeBlock(level, Tile::stairs_wood_Id, southStairs, w, 4 + d, d, chunkBB);
			if ((d > -1 || w <= 1) && (d > 0 || w <= 3) && (d > 1 || w <= 4 || w >= 6)) {
				placeBlock(level, Tile::stairs_wood_Id, northStairs, w, 4 + d, 5 - d, chunkBB);
			}
		}
	}

	// room 2 roof
	generateBox(level, chunkBB, 3, 4, 5, 3, 4, 10, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 7, 4, 2, 7, 4, 10, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 4, 5, 4, 4, 5, 10, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 6, 5, 4, 6, 5, 10, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 5, 6, 3, 5, 6, 10, Tile::wood_Id, Tile::wood_Id, false);
	int westStairs = getOrientationData(Tile::stairs_wood_Id, 0);
	for (int w = 4; w >= 1; w--)
	{
		placeBlock(level, Tile::wood_Id, 0, w, 2 + w, 7 - w, chunkBB);
		for (int d = 8 - w; d <= 10; d++)
		{
			placeBlock(level, Tile::stairs_wood_Id, westStairs, w, 2 + w, d, chunkBB);
		}
	}
	int eastStairs = getOrientationData(Tile::stairs_wood_Id, 1);
	placeBlock(level, Tile::wood_Id, 0, 6, 6, 3, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 7, 5, 4, chunkBB);
	placeBlock(level, Tile::stairs_wood_Id, eastStairs, 6, 6, 4, chunkBB);
	for (int w = 6; w <= 8; w++)
	{
		for (int d = 5; d <= 10; d++)
		{
			placeBlock(level, Tile::stairs_wood_Id, eastStairs, w, 12 - w, d, chunkBB);
		}
	}

	// windows etc
	placeBlock(level, Tile::treeTrunk_Id, 0, 0, 2, 1, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 0, 2, 4, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 3, chunkBB);

	placeBlock(level, Tile::treeTrunk_Id, 0, 4, 2, 0, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 5, 2, 0, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 6, 2, 0, chunkBB);

	placeBlock(level, Tile::treeTrunk_Id, 0, 8, 2, 1, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 2, 3, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 8, 2, 4, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 8, 2, 5, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 8, 2, 6, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 2, 7, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 8, 2, 8, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 8, 2, 9, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 2, 2, 6, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 2, 7, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 2, 8, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 2, 2, 9, chunkBB);

	placeBlock(level, Tile::treeTrunk_Id, 0, 4, 4, 10, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 5, 4, 10, chunkBB);
	placeBlock(level, Tile::treeTrunk_Id, 0, 6, 4, 10, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 5, 5, 10, chunkBB);

	// entrance
	placeBlock(level, 0, 0, 2, 1, 0, chunkBB);
	placeBlock(level, 0, 0, 2, 2, 0, chunkBB);
	placeBlock(level, Tile::torch_Id, 0, 2, 3, 1, chunkBB);
	createDoor(level, chunkBB, random, 2, 1, 0, getOrientationData(Tile::door_wood_Id, 1));
	generateBox(level, chunkBB, 1, 0, -1, 3, 2, -1, 0, 0, false);
	if (getBlock(level, 2, 0, -1, chunkBB) == 0 && getBlock(level, 2, -1, -1, chunkBB) != 0) {
		placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), 2, 0, -1, chunkBB);
	}

	for (int z = 0; z < 5; z++)
	{
		for (int x = 0; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::cobblestone_Id, 0, x, -1, z, chunkBB);
		}
	}
	for (int z = 5; z < depth - 1; z++)
	{
		for (int x = 2; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::cobblestone_Id, 0, x, -1, z, chunkBB);
		}
	}

	spawnVillagers(level, chunkBB, 4, 1, 2, 2);

	return true;

}

void VillagePieces::Smithy::staticCtor()
{
	treasureItems = WeighedTreasureArray(17);
	treasureItems[0] = new WeighedTreasure(Item::diamond_Id, 0, 1, 3, 3);
	treasureItems[1] = new WeighedTreasure(Item::ironIngot_Id, 0, 1, 5, 10);
	treasureItems[2] = new WeighedTreasure(Item::goldIngot_Id, 0, 1, 3, 5);
	treasureItems[3] = new WeighedTreasure(Item::bread_Id, 0, 1, 3, 15);
	treasureItems[4] = new WeighedTreasure(Item::apple_Id, 0, 1, 3, 15);
	treasureItems[5] = new WeighedTreasure(Item::pickAxe_iron_Id, 0, 1, 1, 5);
	treasureItems[6] = new WeighedTreasure(Item::sword_iron_Id, 0, 1, 1, 5);
	treasureItems[7] = new WeighedTreasure(Item::chestplate_iron_Id, 0, 1, 1, 5);
	treasureItems[8] = new WeighedTreasure(Item::helmet_iron_Id, 0, 1, 1, 5);
	treasureItems[9] = new WeighedTreasure(Item::leggings_iron_Id, 0, 1, 1, 5);
	treasureItems[10] = new WeighedTreasure(Item::boots_iron_Id, 0, 1, 1, 5);
	treasureItems[11] = new WeighedTreasure(Tile::obsidian_Id, 0, 3, 7, 5);
	treasureItems[12] = new WeighedTreasure(Tile::sapling_Id, 0, 3, 7, 5);
	// very rare for villages ...
	treasureItems[13] = new WeighedTreasure(Item::saddle_Id, 0, 1, 1, 3);
	treasureItems[14] = new WeighedTreasure(Item::horseArmorMetal_Id, 0, 1, 1, 1);
	treasureItems[15] = new WeighedTreasure(Item::horseArmorGold_Id, 0, 1, 1, 1);
	treasureItems[16] = new WeighedTreasure(Item::horseArmorDiamond_Id, 0, 1, 1, 1);
	// ...
}

VillagePieces::Smithy::Smithy()
{
	// for reflection
}

VillagePieces::Smithy::Smithy(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth)
{
	hasPlacedChest = false;

	orientation = direction;
	boundingBox = stairsBox;
}

VillagePieces::Smithy *VillagePieces::Smithy::createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return new Smithy(startPiece, genDepth, random, box, direction);
}

void VillagePieces::Smithy::addAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::addAdditonalSaveData(tag);
	tag->putBoolean(L"Chest", hasPlacedChest);
}

void VillagePieces::Smithy::readAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::readAdditonalSaveData(tag);
	hasPlacedChest = tag->getBoolean(L"Chest");
}

bool VillagePieces::Smithy::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// fill inside with air
	generateBox(level, chunkBB, 0, 1, 0, 9, 4, 6, 0, 0, false);

	// floor
	generateBox(level, chunkBB, 0, 0, 0, 9, 0, 6, Tile::cobblestone_Id, Tile::cobblestone_Id, false);

	// roof
	generateBox(level, chunkBB, 0, 4, 0, 9, 4, 6, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	generateBox(level, chunkBB, 0, 5, 0, 9, 5, 6, Tile::stoneSlabHalf_Id, Tile::stoneSlabHalf_Id, false);
	generateBox(level, chunkBB, 1, 5, 1, 8, 5, 5, 0, 0, false);

	// room walls
	generateBox(level, chunkBB, 1, 1, 0, 2, 3, 0, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 1, 0, 0, 4, 0, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 3, 1, 0, 3, 4, 0, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 0, 1, 6, 0, 4, 6, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	placeBlock(level, Tile::wood_Id, 0, 3, 3, 1, chunkBB);
	generateBox(level, chunkBB, 3, 1, 2, 3, 3, 2, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 4, 1, 3, 5, 3, 3, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 0, 1, 1, 0, 3, 5, Tile::wood_Id, Tile::wood_Id, false);
	generateBox(level, chunkBB, 1, 1, 6, 5, 3, 6, Tile::wood_Id, Tile::wood_Id, false);

	// pillars
	generateBox(level, chunkBB, 5, 1, 0, 5, 3, 0, Tile::fence_Id, Tile::fence_Id, false);
	generateBox(level, chunkBB, 9, 1, 0, 9, 3, 0, Tile::fence_Id, Tile::fence_Id, false);

	// furnace
	generateBox(level, chunkBB, 6, 1, 4, 9, 4, 6, Tile::cobblestone_Id, Tile::cobblestone_Id, false);
	placeBlock(level, Tile::lava_Id, 0, 7, 1, 5, chunkBB);
	placeBlock(level, Tile::lava_Id, 0, 8, 1, 5, chunkBB);
	placeBlock(level, Tile::ironFence_Id, 0, 9, 2, 5, chunkBB);
	placeBlock(level, Tile::ironFence_Id, 0, 9, 2, 4, chunkBB);
	generateBox(level, chunkBB, 7, 2, 4, 8, 2, 5, 0, 0, false);
	placeBlock(level, Tile::cobblestone_Id, 0, 6, 1, 3, chunkBB);
	placeBlock(level, Tile::furnace_Id, 0, 6, 2, 3, chunkBB);
	placeBlock(level, Tile::furnace_Id, 0, 6, 3, 3, chunkBB);
	placeBlock(level, Tile::stoneSlab_Id, 0, 8, 1, 1, chunkBB);

	// windows etc
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 2, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 0, 2, 4, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 2, 2, 6, chunkBB);
	placeBlock(level, Tile::thinGlass_Id, 0, 4, 2, 6, chunkBB);

	// table
	placeBlock(level, Tile::fence_Id, 0, 2, 1, 4, chunkBB);
	placeBlock(level, Tile::pressurePlate_wood_Id, 0, 2, 2, 4, chunkBB);
	placeBlock(level, Tile::wood_Id, 0, 1, 1, 5, chunkBB);
	placeBlock(level, Tile::stairs_wood_Id, getOrientationData(Tile::stairs_wood_Id, 3), 2, 1, 5, chunkBB);
	placeBlock(level, Tile::stairs_wood_Id, getOrientationData(Tile::stairs_wood_Id, 1), 1, 1, 4, chunkBB);

	if (!hasPlacedChest)
	{
		int y = getWorldY(1);
		int x = getWorldX(5, 5), z = getWorldZ(5, 5);
		if (chunkBB->isInside(x, y, z))
		{
			hasPlacedChest = true;
			createChest(level, chunkBB, random, 5, 1, 5, treasureItems, 3 + random->nextInt(6));
		}
	}

	// entrance
	for (int x = 6; x <= 8; x++)
	{
		if (getBlock(level, x, 0, -1, chunkBB) == 0 && getBlock(level, x, -1, -1, chunkBB) != 0 )
		{
			placeBlock(level, Tile::stairs_stone_Id, getOrientationData(Tile::stairs_stone_Id, 3), x, 0, -1, chunkBB);
		}
	}

	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::cobblestone_Id, 0, x, -1, z, chunkBB);
		}
	}

	spawnVillagers(level, chunkBB, 7, 1, 1, 1);

	return true;

}

int VillagePieces::Smithy::getVillagerProfession(int villagerNumber)
{
	return Villager::PROFESSION_SMITH;
}

VillagePieces::Farmland::Farmland()
{
	cropsA = 0;
	cropsB = 0;
	// for reflection
}

VillagePieces::Farmland::Farmland(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth)
{
	orientation = direction;
	boundingBox = stairsBox;

	cropsA = selectCrops(random);
	cropsB = selectCrops(random);
}

int VillagePieces::Farmland::selectCrops(Random *random)
{
	switch (random->nextInt(5))
	{
	default:
		return Tile::wheat_Id;
	case 0:
		return Tile::carrots_Id;
	case 1:
		return Tile::potatoes_Id;
	}
}

void VillagePieces::Farmland::addAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::addAdditonalSaveData(tag);
	tag->putInt(L"CA", cropsA);
	tag->putInt(L"CB", cropsB);
}

void VillagePieces::Farmland::readAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::readAdditonalSaveData(tag);
	cropsA = tag->getInt(L"CA");
	cropsB = tag->getInt(L"CB");
}

VillagePieces::Farmland *VillagePieces::Farmland::createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return new Farmland(startPiece, genDepth, random, box, direction);
}

bool VillagePieces::Farmland::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// fill inside with air
	generateBox(level, chunkBB, 0, 1, 0, 6, 4, 8, 0, 0, false);

	// farmlands
	generateBox(level, chunkBB, 1, 0, 1, 2, 0, 7, Tile::farmland_Id, Tile::farmland_Id, false);
	generateBox(level, chunkBB, 4, 0, 1, 5, 0, 7, Tile::farmland_Id, Tile::farmland_Id, false);
	// walkpaths
	generateBox(level, chunkBB, 0, 0, 0, 0, 0, 8, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 6, 0, 0, 6, 0, 8, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 1, 0, 0, 5, 0, 0, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 1, 0, 8, 5, 0, 8, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	// water
	generateBox(level, chunkBB, 3, 0, 1, 3, 0, 7, Tile::water_Id, Tile::water_Id, false);
	// crops
	for (int d = 1; d <= 7; d++)
	{
		placeBlock(level, cropsA, Mth::nextInt(random, 2, 7), 1, 1, d, chunkBB);
		placeBlock(level, cropsA, Mth::nextInt(random, 2, 7), 2, 1, d, chunkBB);
		placeBlock(level, cropsB, Mth::nextInt(random, 2, 7), 4, 1, d, chunkBB);
		placeBlock(level, cropsB, Mth::nextInt(random, 2, 7), 5, 1, d, chunkBB);
	}

	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::dirt_Id, 0, x, -1, z, chunkBB);
		}
	}

	return true;

}

VillagePieces::DoubleFarmland::DoubleFarmland()
{
	cropsA = 0;
	cropsB = 0;
	cropsC = 0;
	cropsD = 0;
	// for reflection
}

VillagePieces::DoubleFarmland::DoubleFarmland(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *stairsBox, int direction) : VillagePiece(startPiece, genDepth)
{
	heightPosition = -1;	// 4J added initialiser
	orientation = direction;
	boundingBox = stairsBox;

	cropsA = selectCrops(random);
	cropsB = selectCrops(random);
	cropsC = selectCrops(random);
	cropsD = selectCrops(random);
}

void VillagePieces::DoubleFarmland::addAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::addAdditonalSaveData(tag);
	tag->putInt(L"CA", cropsA);
	tag->putInt(L"CB", cropsB);
	tag->putInt(L"CC", cropsC);
	tag->putInt(L"CD", cropsD);
}

void VillagePieces::DoubleFarmland::readAdditonalSaveData(CompoundTag *tag)
{
	VillagePiece::readAdditonalSaveData(tag);
	cropsA = tag->getInt(L"CA");
	cropsB = tag->getInt(L"CB");
	cropsC = tag->getInt(L"CC");
	cropsD = tag->getInt(L"CD");
}

int VillagePieces::DoubleFarmland::selectCrops(Random *random)
{
	switch (random->nextInt(5))
	{
	default:
		return Tile::wheat_Id;
	case 0:
		return Tile::carrots_Id;
	case 1:
		return Tile::potatoes_Id;
	}
}

VillagePieces::DoubleFarmland *VillagePieces::DoubleFarmland::createPiece(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return new DoubleFarmland(startPiece, genDepth, random, box, direction);
}

bool VillagePieces::DoubleFarmland::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// fill inside with air
	generateBox(level, chunkBB, 0, 1, 0, 12, 4, 8, 0, 0, false);

	// farmlands
	generateBox(level, chunkBB, 1, 0, 1, 2, 0, 7, Tile::farmland_Id, Tile::farmland_Id, false);
	generateBox(level, chunkBB, 4, 0, 1, 5, 0, 7, Tile::farmland_Id, Tile::farmland_Id, false);
	generateBox(level, chunkBB, 7, 0, 1, 8, 0, 7, Tile::farmland_Id, Tile::farmland_Id, false);
	generateBox(level, chunkBB, 10, 0, 1, 11, 0, 7, Tile::farmland_Id, Tile::farmland_Id, false);
	// walkpaths
	generateBox(level, chunkBB, 0, 0, 0, 0, 0, 8, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 6, 0, 0, 6, 0, 8, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 12, 0, 0, 12, 0, 8, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 1, 0, 0, 11, 0, 0, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 1, 0, 8, 11, 0, 8, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	// water
	generateBox(level, chunkBB, 3, 0, 1, 3, 0, 7, Tile::water_Id, Tile::water_Id, false);
	generateBox(level, chunkBB, 9, 0, 1, 9, 0, 7, Tile::water_Id, Tile::water_Id, false);
	// crops
	for (int d = 1; d <= 7; d++)
	{
		placeBlock(level, cropsA, Mth::nextInt(random, 2, 7), 1, 1, d, chunkBB);
		placeBlock(level, cropsA, Mth::nextInt(random, 2, 7), 2, 1, d, chunkBB);
		placeBlock(level, cropsB, Mth::nextInt(random, 2, 7), 4, 1, d, chunkBB);
		placeBlock(level, cropsB, Mth::nextInt(random, 2, 7), 5, 1, d, chunkBB);
		placeBlock(level, cropsC, Mth::nextInt(random, 2, 7), 7, 1, d, chunkBB);
		placeBlock(level, cropsC, Mth::nextInt(random, 2, 7), 8, 1, d, chunkBB);
		placeBlock(level, cropsD, Mth::nextInt(random, 2, 7), 10, 1, d, chunkBB);
		placeBlock(level, cropsD, Mth::nextInt(random, 2, 7), 11, 1, d, chunkBB);
	}

	for (int z = 0; z < depth; z++)
	{
		for (int x = 0; x < width; x++)
		{
			generateAirColumnUp(level, x, height, z, chunkBB);
			fillColumnDown(level, Tile::dirt_Id, 0, x, -1, z, chunkBB);
		}
	}


	return true;

}

VillagePieces::LightPost::LightPost()
{
	// for reflection
}

VillagePieces::LightPost::LightPost(StartPiece *startPiece, int genDepth, Random *random, BoundingBox *box, int direction) : VillagePiece(startPiece, genDepth)
{
	heightPosition = -1;	// 4J - added initialiser
	orientation = direction;
	boundingBox = box;
}

BoundingBox *VillagePieces::LightPost::findPieceBox(StartPiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction)
{
	BoundingBox *box = BoundingBox::orientBox(footX, footY, footZ, 0, 0, 0, width, height, depth, direction);

	if (!isOkBox(box, startPiece) || StructurePiece::findCollisionPiece(pieces, box) != nullptr)
	{
		delete box;
		return nullptr;
	}

	return box;
}

bool VillagePieces::LightPost::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (heightPosition < 0)
	{
		heightPosition = getAverageGroundHeight(level, chunkBB);
		if (heightPosition < 0)
		{
			return true;
		}
		boundingBox->move(0, heightPosition - boundingBox->y1 + height - 1, 0);
	}

	// fill with air
	generateBox(level, chunkBB, 0, 0, 0, 2, 3, 1, 0, 0, false);

	// pillar
	placeBlock(level, Tile::fence_Id, 0, 1, 0, 0, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 1, 1, 0, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 1, 2, 0, chunkBB);

	// head
	placeBlock(level, Tile::wool_Id, DyePowderItem::WHITE, 1, 3, 0, chunkBB);

	// torches
	placeBlock(level, Tile::torch_Id, 0, 0, 3, 0, chunkBB);
	placeBlock(level, Tile::torch_Id, 0, 1, 3, 1, chunkBB);
	placeBlock(level, Tile::torch_Id, 0, 2, 3, 0, chunkBB);
	placeBlock(level, Tile::torch_Id, 0, 1, 3, -1, chunkBB);

	return true;
}
