#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.item.h"
#include "JavaMath.h"
#include "WeighedTreasure.h"
#include "MineShaftPieces.h"

WeighedTreasureArray MineShaftPieces::smallTreasureItems;;

void MineShaftPieces::staticCtor()
{
	smallTreasureItems = WeighedTreasureArray(11);
	smallTreasureItems[0] = new WeighedTreasure(Item::ironIngot_Id, 0, 1, 5, 10);
	smallTreasureItems[1] = new WeighedTreasure(Item::goldIngot_Id, 0, 1, 3, 5);
	smallTreasureItems[2] = new WeighedTreasure(Item::redStone_Id, 0, 4, 9, 5);
	smallTreasureItems[3] = new WeighedTreasure(Item::dye_powder_Id, DyePowderItem::BLUE, 4, 9, 5);
	smallTreasureItems[4] = new WeighedTreasure(Item::diamond_Id, 0, 1, 2, 3);
	smallTreasureItems[5] = new WeighedTreasure(Item::coal_Id, CoalItem::STONE_COAL, 3, 8, 10);
	smallTreasureItems[6] = new WeighedTreasure(Item::bread_Id, 0, 1, 3, 15);
	smallTreasureItems[7] = new WeighedTreasure(Item::pickAxe_iron_Id, 0, 1, 1, 1);
	smallTreasureItems[8] = new WeighedTreasure(Tile::rail_Id, 0, 4, 8, 1);
	smallTreasureItems[9] = new WeighedTreasure(Item::seeds_melon_Id, 0, 2, 4, 10);
	// 4J-PB - Adding from 1.2.3
	smallTreasureItems[10] = new WeighedTreasure(Item::seeds_pumpkin_Id, 0, 2, 4, 10);
}

StructurePiece *MineShaftPieces::createRandomShaftPiece(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int genDepth)
{
	int randomSelection = random->nextInt(100);
	if (randomSelection >= 80)
	{
		BoundingBox *crossingBox = MineShaftCrossing::findCrossing(pieces, random, footX, footY, footZ, direction);
		if (crossingBox != NULL)
		{
			return new MineShaftCrossing(genDepth, random, crossingBox, direction);
		}
	}
	else if (randomSelection >= 70)
	{
		BoundingBox *stairsBox = MineShaftStairs::findStairs(pieces, random, footX, footY, footZ, direction);
		if (stairsBox != NULL)
		{
			return new MineShaftPieces::MineShaftStairs(genDepth, random, stairsBox, direction);
		}
	}
	else
	{
		BoundingBox *corridorBox = MineShaftCorridor::findCorridorSize(pieces, random, footX, footY, footZ, direction);
		if (corridorBox != NULL)
		{
			return new MineShaftCorridor(genDepth, random, corridorBox, direction);
		}
	}

	return NULL;
}

StructurePiece *MineShaftPieces::generateAndAddPiece(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction, int depth)
{
	if (depth > MAX_DEPTH)
	{
		return NULL;
	}
	if (abs(footX - startPiece->getBoundingBox()->x0) > 5 * 16 || abs(footZ - startPiece->getBoundingBox()->z0) > 5 * 16)
	{
		return NULL;
	}

	StructurePiece *newPiece = createRandomShaftPiece(pieces, random, footX, footY, footZ, direction, depth + 1);
	if (newPiece != NULL)
	{
		MemSect(50);
		pieces->push_back(newPiece);
		MemSect(0);
		newPiece->addChildren(startPiece, pieces, random);
	}
	return newPiece;
}


MineShaftPieces::MineShaftRoom::MineShaftRoom(int genDepth, Random *random, int west, int north) : StructurePiece(genDepth)
{
	boundingBox = new BoundingBox(west, 50, north, west + 7 + random->nextInt(6), 54 + random->nextInt(6), north + 7 + random->nextInt(6));
}

MineShaftPieces::MineShaftRoom::~MineShaftRoom()
{
	for(AUTO_VAR(it, childEntranceBoxes.begin()); it != childEntranceBoxes.end(); ++it)
	{
		delete (*it);
	}
}

void MineShaftPieces::MineShaftRoom::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{

	int depth = getGenDepth();

	int pos;

	int heightSpace = boundingBox->getYSpan() - DEFAULT_SHAFT_HEIGHT - 1;
	if (heightSpace <= 0)
	{
		heightSpace = 1;
	}

	// northern exits
	pos = 0;
	while (pos < boundingBox->getXSpan())
	{
		pos += random->nextInt(boundingBox->getXSpan());
		if ((pos + DEFAULT_SHAFT_WIDTH) > boundingBox->getXSpan())
		{
			break;
		}
		StructurePiece *child = generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + pos, boundingBox->y0 + random->nextInt(heightSpace) + 1, boundingBox->z0 - 1, Direction::NORTH, depth);
		if (child != NULL)
		{
			BoundingBox *childBox = child->getBoundingBox();
			childEntranceBoxes.push_back(new BoundingBox(childBox->x0, childBox->y0, boundingBox->z0, childBox->x1, childBox->y1, boundingBox->z0 + 1));
		}
		pos += DEFAULT_SHAFT_WIDTH + 1;
	}
	// southern exits
	pos = 0;
	while (pos < boundingBox->getXSpan())
	{
		pos += random->nextInt(boundingBox->getXSpan());
		if ((pos + DEFAULT_SHAFT_WIDTH) > boundingBox->getXSpan())
		{
			break;
		}
		StructurePiece *child = generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + pos, boundingBox->y0 + random->nextInt(heightSpace) + 1, boundingBox->z1 + 1, Direction::SOUTH, depth);
		if (child != NULL)
		{
			BoundingBox *childBox = child->getBoundingBox();
			childEntranceBoxes.push_back(new BoundingBox(childBox->x0, childBox->y0, boundingBox->z1 - 1, childBox->x1, childBox->y1, boundingBox->z1));
		}
		pos += DEFAULT_SHAFT_WIDTH + 1;
	}
	// western exits
	pos = 0;
	while (pos < boundingBox->getZSpan())
	{
		pos += random->nextInt(boundingBox->getZSpan());
		if ((pos + DEFAULT_SHAFT_WIDTH) > boundingBox->getZSpan())
		{
			break;
		}
		StructurePiece *child = generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 + random->nextInt(heightSpace) + 1, boundingBox->z0 + pos, Direction::WEST, depth);
		if (child != NULL)
		{
			BoundingBox *childBox = child->getBoundingBox();
			childEntranceBoxes.push_back(new BoundingBox(boundingBox->x0, childBox->y0, childBox->z0, boundingBox->x0 + 1, childBox->y1, childBox->z1));
		}
		pos += DEFAULT_SHAFT_WIDTH + 1;
	}
	// eastern exits
	pos = 0;
	while (pos < boundingBox->getZSpan())
	{
		pos += random->nextInt(boundingBox->getZSpan());
		if ((pos + DEFAULT_SHAFT_WIDTH) > boundingBox->getZSpan())
		{
			break;
		}
		StructurePiece *child = generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 + random->nextInt(heightSpace) + 1, boundingBox->z0 + pos, Direction::EAST, depth);
		if (child != NULL)
		{
			BoundingBox *childBox = child->getBoundingBox();
			childEntranceBoxes.push_back(new BoundingBox(boundingBox->x1 - 1, childBox->y0, childBox->z0, boundingBox->x1, childBox->y1, childBox->z1));
		}
		pos += DEFAULT_SHAFT_WIDTH + 1;
	}
}

bool MineShaftPieces::MineShaftRoom::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (edgesLiquid(level, chunkBB))
	{
		return false;
	}

	// floor
	generateBox(level, chunkBB, boundingBox->x0, boundingBox->y0, boundingBox->z0, boundingBox->x1, boundingBox->y0, boundingBox->z1, Tile::dirt_Id, 0, true);

	// room air
	generateBox(level, chunkBB, boundingBox->x0, boundingBox->y0 + 1, boundingBox->z0, boundingBox->x1, min(boundingBox->y0 + 3, boundingBox->y1), boundingBox->z1, 0, 0, false);
	for(AUTO_VAR(it, childEntranceBoxes.begin()); it != childEntranceBoxes.end(); ++it)
	{
		BoundingBox *entranceBox = *it;
		generateBox(level, chunkBB, entranceBox->x0, entranceBox->y1 - (DEFAULT_SHAFT_HEIGHT - 1), entranceBox->z0, entranceBox->x1, entranceBox->y1, entranceBox->z1, 0, 0, false);
	}
	generateUpperHalfSphere(level, chunkBB, boundingBox->x0, boundingBox->y0 + 4, boundingBox->z0, boundingBox->x1, boundingBox->y1, boundingBox->z1, 0, false);

	return true;
}


MineShaftPieces::MineShaftCorridor::MineShaftCorridor(int genDepth, Random *random, BoundingBox *corridorBox, int direction)
	: StructurePiece(genDepth)
{
	orientation = direction;
	boundingBox = corridorBox;
	hasRails = random->nextInt(3) == 0;
	hasPlacedSpider=false;
	spiderCorridor = !hasRails && random->nextInt(23) == 0;
	//debug
	//spiderCorridor = !hasRails ;//&& random->nextInt(23) == 0;

	if (orientation == Direction::NORTH || orientation == Direction::SOUTH)
	{
		numSections = corridorBox->getZSpan() / DEFAULT_SHAFT_LENGTH;
	}
	else
	{
		numSections = corridorBox->getXSpan() / DEFAULT_SHAFT_LENGTH;
	}
}

BoundingBox *MineShaftPieces::MineShaftCorridor::findCorridorSize(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction)
{

	BoundingBox *box = new BoundingBox(footX, footY, footZ, footX, footY + (DEFAULT_SHAFT_HEIGHT - 1), footZ);

	int corridorLength = random->nextInt(3) + 2;
	while (corridorLength > 0)
	{
		int blockLength = corridorLength * DEFAULT_SHAFT_LENGTH;

		switch (direction)
		{
		case Direction::NORTH:
			box->x1 = footX + (DEFAULT_SHAFT_WIDTH - 1);
			box->z0 = footZ - (blockLength - 1);
			break;
		case Direction::SOUTH:
			box->x1 = footX + (DEFAULT_SHAFT_WIDTH - 1);
			box->z1 = footZ + (blockLength - 1);
			break;
		case Direction::WEST:
			box->x0 = footX - (blockLength - 1);
			box->z1 = footZ + (DEFAULT_SHAFT_WIDTH - 1);
			break;
		case Direction::EAST:
			box->x1 = footX + (blockLength - 1);
			box->z1 = footZ + (DEFAULT_SHAFT_WIDTH - 1);
			break;
		}

		if (StructurePiece::findCollisionPiece(pieces, box) != NULL)
		{
			corridorLength--;
		}
		else
		{
			break;
		}
	}

	if (corridorLength > 0)
	{
		return box;
	}
	delete box;
	// unable to place corridor here
	return NULL;
}

void MineShaftPieces::MineShaftCorridor::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	int depth = getGenDepth();
	int endSelection = random->nextInt(4);
	switch (orientation)
	{
	case Direction::NORTH:
		if (endSelection <= 1)
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x0, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z0 - 1, orientation, depth);
		}
		else if (endSelection == 2)
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z0, Direction::WEST, depth);
		}
		else
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z0, Direction::EAST, depth);
		}
		break;
	case Direction::SOUTH:
		if (endSelection <= 1)
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x0, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z1 + 1, orientation, depth);
		}
		else if (endSelection == 2)
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z1 - DEFAULT_SHAFT_WIDTH, Direction::WEST, depth);
		}
		else
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z1 - DEFAULT_SHAFT_WIDTH, Direction::EAST, depth);
		}
		break;
	case Direction::WEST:
		if (endSelection <= 1)
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z0, orientation, depth);
		}
		else if (endSelection == 2)
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x0, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z0 - 1, Direction::NORTH, depth);
		}
		else
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x0, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z1 + 1, Direction::SOUTH, depth);
		}
		break;
	case Direction::EAST:
		if (endSelection <= 1)
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z0, orientation, depth);
		}
		else if (endSelection == 2)
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 - DEFAULT_SHAFT_WIDTH, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z0 - 1, Direction::NORTH, depth);
		}
		else
		{
			generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 - DEFAULT_SHAFT_WIDTH, boundingBox->y0 - 1 + random->nextInt(3), boundingBox->z1 + 1, Direction::SOUTH, depth);
		}
		break;
	}

	// generate cross sections using higher depth
	if (depth < MAX_DEPTH)
	{
		if (orientation == Direction::NORTH || orientation == Direction::SOUTH)
		{
			for (int z = boundingBox->z0 + 3; (z + DEFAULT_SHAFT_WIDTH) <= boundingBox->z1; z += DEFAULT_SHAFT_LENGTH)
			{
				int selection = random->nextInt(5);
				if (selection == 0)
				{
					generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0, z, Direction::WEST, depth + 1);
				}
				else if (selection == 1)
				{
					generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0, z, Direction::EAST, depth + 1);
				}
			}
		}
		else
		{
			for (int x = boundingBox->x0 + 3; (x + DEFAULT_SHAFT_WIDTH) <= boundingBox->x1; x += DEFAULT_SHAFT_LENGTH)
			{
				int selection = random->nextInt(5);
				if (selection == 0)
				{
					generateAndAddPiece(startPiece, pieces, random, x, boundingBox->y0, boundingBox->z0 - 1, Direction::NORTH, depth + 1);
				}
				else if (selection == 1)
				{
					generateAndAddPiece(startPiece, pieces, random, x, boundingBox->y0, boundingBox->z1 + 1, Direction::SOUTH, depth + 1);
				}
			}
		}
	}
}

bool MineShaftPieces::MineShaftCorridor::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (edgesLiquid(level, chunkBB))
	{
		return false;
	}

	const int x0 = 0;
	const int x1 = DEFAULT_SHAFT_WIDTH - 1;
	const int y0 = 0;
	const int y1 = DEFAULT_SHAFT_HEIGHT - 1;
	const int length = (numSections * DEFAULT_SHAFT_LENGTH) - 1;

	// corridor air
	generateBox(level, chunkBB, x0, 0, y0, x1, y1 - 1, length, 0, 0, false);
	generateMaybeBox(level, chunkBB, random, .8f, x0, y1, y0, x1, y1, length, 0, 0, false);

	if (spiderCorridor)
	{
		generateMaybeBox(level, chunkBB, random, .6f, x0, 0, y0, x1, y1 - 1, length, Tile::web_Id, 0, false);
	}

	// place a support in every section
	for (int section = 0; section < numSections; section++)
	{

		int z = 2 + section * DEFAULT_SHAFT_LENGTH;

		// 4J-PB - Bringing forward the changes in 1.2.3
		generateBox(level, chunkBB, x0, y0, z, x0, y1 - 1, z, Tile::fence_Id, 0, false);
		generateBox(level, chunkBB, x1, y0, z, x1, y1 - 1, z, Tile::fence_Id, 0, false);
		if (random->nextInt(4) == 0)
		{
			generateBox(level, chunkBB, x0, y1, z, x0, y1, z, Tile::wood_Id, 0, false);
			generateBox(level, chunkBB, x1, y1, z, x1, y1, z, Tile::wood_Id, 0, false);
		}
		else
		{
			generateBox(level, chunkBB, x0, y1, z, x1, y1, z, Tile::wood_Id, 0, false);
		}
		maybeGenerateBlock(level, chunkBB, random, .1f, x0, y1, z - 1, Tile::web_Id, 0);
		maybeGenerateBlock(level, chunkBB, random, .1f, x1, y1, z - 1, Tile::web_Id, 0);
		maybeGenerateBlock(level, chunkBB, random, .1f, x0, y1, z + 1, Tile::web_Id, 0);
		maybeGenerateBlock(level, chunkBB, random, .1f, x1, y1, z + 1, Tile::web_Id, 0);
		maybeGenerateBlock(level, chunkBB, random, .05f, x0, y1, z - 2, Tile::web_Id, 0);
		maybeGenerateBlock(level, chunkBB, random, .05f, x1, y1, z - 2, Tile::web_Id, 0);
		maybeGenerateBlock(level, chunkBB, random, .05f, x0, y1, z + 2, Tile::web_Id, 0);
		maybeGenerateBlock(level, chunkBB, random, .05f, x1, y1, z + 2, Tile::web_Id, 0);

		maybeGenerateBlock(level, chunkBB, random, .05f, x0 + 1, y1, z - 1, Tile::torch_Id, 0);
		maybeGenerateBlock(level, chunkBB, random, .05f, x0 + 1, y1, z + 1, Tile::torch_Id, 0);

		if (random->nextInt(100) == 0)
		{
			createChest(level, chunkBB, random, x1, y0, z - 1, WeighedTreasure::addToTreasure(smallTreasureItems, Item::enchantedBook->createForRandomTreasure(random)), 3 + random->nextInt(4));
		}
		if (random->nextInt(100) == 0)
		{
			createChest(level, chunkBB, random, x0, y0, z + 1, WeighedTreasure::addToTreasure(smallTreasureItems, Item::enchantedBook->createForRandomTreasure(random)), 3 + random->nextInt(4));
		}

		if (spiderCorridor && !hasPlacedSpider)
		{
			int y = getWorldY(y0), newZ = z - 1 + random->nextInt(3);
			int x = getWorldX(x0 + 1, newZ);
			newZ = getWorldZ(x0 + 1, newZ);
			if (chunkBB->isInside(x, y, newZ))
			{
				hasPlacedSpider = true;
				level->setTile(x, y, newZ, Tile::mobSpawner_Id);
				shared_ptr<MobSpawnerTileEntity> entity = dynamic_pointer_cast<MobSpawnerTileEntity>( level->getTileEntity(x, y, newZ) );
				if (entity != NULL) entity->setEntityId(L"CaveSpider");
			}
		}
	}

	// prevent air floating
	for (int x = x0; x <= x1; x++) 
	{
		for (int z = 0; z <= length; z++) 
		{
			int block = getBlock(level, x, -1, z, chunkBB);
			if (block == 0) 
			{
				placeBlock(level, Tile::wood_Id, 0, x, -1, z, chunkBB);
			}
		}
	}

	if (hasRails)
	{
		for (int z = 0; z <= length; z++)
		{
			int floor = getBlock(level, x0 + 1, y0 - 1, z, chunkBB);
			if (floor > 0 && Tile::solid[floor])
			{
				maybeGenerateBlock(level, chunkBB, random, .7f, x0 + 1, y0, z, Tile::rail_Id, getOrientationData(Tile::rail_Id, RailTile::DIR_FLAT_Z));
			}
		}
	}

	return true;
}

MineShaftPieces::MineShaftCrossing::MineShaftCrossing(int genDepth, Random *random, BoundingBox *crossingBox, int direction)
	: StructurePiece(genDepth), direction(direction), isTwoFloored( crossingBox->getYSpan() > DEFAULT_SHAFT_HEIGHT )
{
	boundingBox = crossingBox;
}

BoundingBox *MineShaftPieces::MineShaftCrossing::findCrossing(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction)
{

	BoundingBox *box = new BoundingBox(footX, footY, footZ, footX, footY + (DEFAULT_SHAFT_HEIGHT - 1), footZ);

	if (random->nextInt(4) == 0)
	{
		box->y1 += DEFAULT_SHAFT_HEIGHT + 1; // two-floored
	}

	switch (direction)
	{
	case Direction::NORTH:
		box->x0 = footX - 1;
		box->x1 = footX + DEFAULT_SHAFT_WIDTH;
		box->z0 = footZ - (DEFAULT_SHAFT_WIDTH + 1);
		break;
	case Direction::SOUTH:
		box->x0 = footX - 1;
		box->x1 = footX + DEFAULT_SHAFT_WIDTH;
		box->z1 = footZ + (DEFAULT_SHAFT_WIDTH + 1);
		break;
	case Direction::WEST:
		box->x0 = footX - (DEFAULT_SHAFT_WIDTH + 1);
		box->z0 = footZ - 1;
		box->z1 = footZ + DEFAULT_SHAFT_WIDTH;
		break;
	case Direction::EAST:
		box->x1 = footX + (DEFAULT_SHAFT_WIDTH + 1);
		box->z0 = footZ - 1;
		box->z1 = footZ + DEFAULT_SHAFT_WIDTH;
		break;
	}

	if (StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return box;
}

void MineShaftPieces::MineShaftCrossing::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{

	int depth = getGenDepth();
	// crossings are coming from a direction and will generate children
	// in the
	// remaining three directions
	switch (direction)
	{
	case Direction::NORTH:
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z0 - 1, Direction::NORTH, depth);
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0, boundingBox->z0 + 1, Direction::WEST, depth);
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0, boundingBox->z0 + 1, Direction::EAST, depth);
		break;
	case Direction::SOUTH:
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z1 + 1, Direction::SOUTH, depth);
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0, boundingBox->z0 + 1, Direction::WEST, depth);
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0, boundingBox->z0 + 1, Direction::EAST, depth);
		break;
	case Direction::WEST:
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z0 - 1, Direction::NORTH, depth);
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z1 + 1, Direction::SOUTH, depth);
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0, boundingBox->z0 + 1, Direction::WEST, depth);
		break;
	case Direction::EAST:
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z0 - 1, Direction::NORTH, depth);
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z1 + 1, Direction::SOUTH, depth);
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0, boundingBox->z0 + 1, Direction::EAST, depth);
		break;
	}

	if (isTwoFloored)
	{
		if (random->nextBoolean()) generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + 1, boundingBox->y0 + DEFAULT_SHAFT_HEIGHT + 1, boundingBox->z0 - 1, Direction::NORTH, depth);
		if (random->nextBoolean()) generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0 + DEFAULT_SHAFT_HEIGHT + 1, boundingBox->z0 + 1, Direction::WEST, depth);
		if (random->nextBoolean()) generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0 + DEFAULT_SHAFT_HEIGHT + 1, boundingBox->z0 + 1, Direction::EAST, depth);
		if (random->nextBoolean()) generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 + 1, boundingBox->y0 + DEFAULT_SHAFT_HEIGHT + 1, boundingBox->z1 + 1, Direction::SOUTH, depth);
	}
}

bool MineShaftPieces::MineShaftCrossing::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (edgesLiquid(level, chunkBB))
	{
		return false;
	}

	// corridor air
	if (isTwoFloored)
	{
		generateBox(level, chunkBB, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z0, boundingBox->x1 - 1, boundingBox->y0 + DEFAULT_SHAFT_HEIGHT - 1, boundingBox->z1, 0, 0, false);
		generateBox(level, chunkBB, boundingBox->x0, boundingBox->y0, boundingBox->z0 + 1, boundingBox->x1, boundingBox->y0 + DEFAULT_SHAFT_HEIGHT - 1, boundingBox->z1 - 1, 0, 0, false);
		generateBox(level, chunkBB, boundingBox->x0 + 1, boundingBox->y1 - (DEFAULT_SHAFT_HEIGHT - 1), boundingBox->z0, boundingBox->x1 - 1, boundingBox->y1, boundingBox->z1, 0, 0, false);
		generateBox(level, chunkBB, boundingBox->x0, boundingBox->y1 - (DEFAULT_SHAFT_HEIGHT - 1), boundingBox->z0 + 1, boundingBox->x1, boundingBox->y1, boundingBox->z1 - 1, 0, 0, false);
		generateBox(level, chunkBB, boundingBox->x0 + 1, boundingBox->y0 + DEFAULT_SHAFT_HEIGHT, boundingBox->z0 + 1, boundingBox->x1 - 1, boundingBox->y0 + DEFAULT_SHAFT_HEIGHT, boundingBox->z1 - 1, 0, 0, false);
	}
	else
	{
		generateBox(level, chunkBB, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z0, boundingBox->x1 - 1, boundingBox->y1, boundingBox->z1, 0, 0, false);
		generateBox(level, chunkBB, boundingBox->x0, boundingBox->y0, boundingBox->z0 + 1, boundingBox->x1, boundingBox->y1, boundingBox->z1 - 1, 0, 0, false);
	}

	// support pillars
	generateBox(level, chunkBB, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z0 + 1, boundingBox->x0 + 1, boundingBox->y1, boundingBox->z0 + 1, Tile::wood_Id, 0, false);
	generateBox(level, chunkBB, boundingBox->x0 + 1, boundingBox->y0, boundingBox->z1 - 1, boundingBox->x0 + 1, boundingBox->y1, boundingBox->z1 - 1, Tile::wood_Id, 0, false);
	generateBox(level, chunkBB, boundingBox->x1 - 1, boundingBox->y0, boundingBox->z0 + 1, boundingBox->x1 - 1, boundingBox->y1, boundingBox->z0 + 1, Tile::wood_Id, 0, false);
	generateBox(level, chunkBB, boundingBox->x1 - 1, boundingBox->y0, boundingBox->z1 - 1, boundingBox->x1 - 1, boundingBox->y1, boundingBox->z1 - 1, Tile::wood_Id, 0, false);

	// prevent air floating
	// note: use world coordinates because the corridor hasn't defined
	// orientation
	for (int x = boundingBox->x0; x <= boundingBox->x1; x++) 
	{
		for (int z = boundingBox->z0; z <= boundingBox->z1; z++) 
		{
			int block = getBlock(level, x, boundingBox->y0 - 1, z, chunkBB);
			if (block == 0) 
			{
				placeBlock(level, Tile::wood_Id, 0, x, boundingBox->y0 - 1, z, chunkBB);
			}
		}
	}

	return true;
}


MineShaftPieces::MineShaftStairs::MineShaftStairs(int genDepth, Random *random, BoundingBox *stairsBox, int direction) : StructurePiece(genDepth)
{
	this->orientation = direction;
	boundingBox = stairsBox;
}

BoundingBox *MineShaftPieces::MineShaftStairs::findStairs(list<StructurePiece *> *pieces, Random *random, int footX, int footY, int footZ, int direction)
{
	// stairs are two steps in, 5x5 steps down, two steps out

	BoundingBox *box = new BoundingBox(footX, footY - 5, footZ, footX, footY + (DEFAULT_SHAFT_HEIGHT - 1), footZ);

	switch (direction)
	{
	case Direction::NORTH:
		box->x1 = footX + (DEFAULT_SHAFT_WIDTH - 1);
		box->z0 = footZ - 8;
		break;
	case Direction::SOUTH:
		box->x1 = footX + (DEFAULT_SHAFT_WIDTH - 1);
		box->z1 = footZ + 8;
		break;
	case Direction::WEST:
		box->x0 = footX - 8;
		box->z1 = footZ + (DEFAULT_SHAFT_WIDTH - 1);
		break;
	case Direction::EAST:
		box->x1 = footX + 8;
		box->z1 = footZ + (DEFAULT_SHAFT_WIDTH - 1);
		break;
	}

	if (StructurePiece::findCollisionPiece(pieces, box) != NULL)
	{
		delete box;
		return NULL;
	}

	return box;
}

void MineShaftPieces::MineShaftStairs::addChildren(StructurePiece *startPiece, list<StructurePiece *> *pieces, Random *random)
{
	int depth = getGenDepth();
	// crossings are coming from a direction and will generate children
	// in the
	// remaining three directions
	switch (orientation)
	{
	case Direction::NORTH:
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0, boundingBox->y0, boundingBox->z0 - 1, Direction::NORTH, depth);
		break;
	case Direction::SOUTH:
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0, boundingBox->y0, boundingBox->z1 + 1, Direction::SOUTH, depth);
		break;
	case Direction::WEST:
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x0 - 1, boundingBox->y0, boundingBox->z0, Direction::WEST, depth);
		break;
	case Direction::EAST:
		generateAndAddPiece(startPiece, pieces, random, boundingBox->x1 + 1, boundingBox->y0, boundingBox->z0, Direction::EAST, depth);
		break;
	}

}

bool MineShaftPieces::MineShaftStairs::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{

	if (edgesLiquid(level, chunkBB))
	{
		return false;
	}

	// upper floor
	generateBox(level, chunkBB, 0, 5, 0, (DEFAULT_SHAFT_WIDTH - 1), 5 + (DEFAULT_SHAFT_HEIGHT - 1), 1, 0, 0, false);
	// lower floor
	generateBox(level, chunkBB, 0, 0, 7, (DEFAULT_SHAFT_WIDTH - 1), (DEFAULT_SHAFT_HEIGHT - 1), 8, 0, 0, false);
	// stairs
	for (int i = 0; i < 5; i++)
	{
		generateBox(level, chunkBB, 0, 5 - i - ((i < 4) ? 1 : 0), 2 + i, (DEFAULT_SHAFT_WIDTH - 1), 5 + (DEFAULT_SHAFT_HEIGHT - 1) - i, 2 + i, 0, 0, false);
	}

	return true;
}