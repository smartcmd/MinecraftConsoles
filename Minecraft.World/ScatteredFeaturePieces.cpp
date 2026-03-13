#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "WeighedTreasure.h"
#include "ScatteredFeaturePieces.h"

void ScatteredFeaturePieces::loadStatic()
{
	StructureFeatureIO::setPieceId(eStructurePiece_DesertPyramidPiece, DesertPyramidPiece::Create, L"TeDP");
	StructureFeatureIO::setPieceId(eStructurePiece_JunglePyramidPiece, DesertPyramidPiece::Create, L"TeJP");
	StructureFeatureIO::setPieceId(eStructurePiece_SwamplandHut, DesertPyramidPiece::Create, L"TeSH");
}

ScatteredFeaturePieces::ScatteredFeaturePiece::ScatteredFeaturePiece()
{
	width = 0;
	height = 0;
	depth = 0;
	heightPosition = 0;
	// for reflection
}

ScatteredFeaturePieces::ScatteredFeaturePiece::ScatteredFeaturePiece(Random *random, int west, int floor, int north, int width, int height, int depth) : StructurePiece(0)
{
	heightPosition = -1;
	this->width = width;
	this->height = height;
	this->depth = depth;

	orientation = random->nextInt(4);

	LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
	if( levelGenOptions != nullptr )
	{
		int tempOrientation = 0;
		if(levelGenOptions->isFeatureChunk(west>>4,north>>4,StructureFeature::eFeature_Temples, &tempOrientation) )
		{
			orientation = tempOrientation;
		}
	}

	switch (orientation)
	{
	case Direction::NORTH:
	case Direction::SOUTH:
		boundingBox = new BoundingBox(west, floor, north, west + width - 1, floor + height - 1, north + depth - 1);
		break;
	default:
		boundingBox = new BoundingBox(west, floor, north, west + depth - 1, floor + height - 1, north + width - 1);
		break;
	}
}

void ScatteredFeaturePieces::ScatteredFeaturePiece::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putInt(L"Width", width);
	tag->putInt(L"Height", height);
	tag->putInt(L"Depth", depth);
	tag->putInt(L"HPos", heightPosition);
}

void ScatteredFeaturePieces::ScatteredFeaturePiece::readAdditonalSaveData(CompoundTag *tag)
{
	width = tag->getInt(L"Width");
	height = tag->getInt(L"Height");
	depth = tag->getInt(L"Depth");
	heightPosition = tag->getInt(L"HPos");
}

bool ScatteredFeaturePieces::ScatteredFeaturePiece::updateAverageGroundHeight(Level *level, BoundingBox *chunkBB, int offset)
{
	if (heightPosition >= 0)
	{
		return true;
	}

	int total = 0;
	int count = 0;
	for (int z = boundingBox->z0; z <= boundingBox->z1; z++)
	{
		for (int x = boundingBox->x0; x <= boundingBox->x1; x++)
		{
			if (chunkBB->isInside(x, 64, z))
			{
				total += max(level->getTopSolidBlock(x, z), level->dimension->getSpawnYPosition());
				count++;
			}
		}
	}

	if (count == 0)
	{
		return false;
	}
	heightPosition = total / count;
	boundingBox->move(0, heightPosition - boundingBox->y0 + offset, 0);
	return true;
}

WeighedTreasure *ScatteredFeaturePieces::DesertPyramidPiece::treasureItems[ScatteredFeaturePieces::DesertPyramidPiece::TREASURE_ITEMS_COUNT] = 
{
	new WeighedTreasure(Item::diamond_Id, 0, 1, 3, 3),
	new WeighedTreasure(Item::ironIngot_Id, 0, 1, 5, 10),
	new WeighedTreasure(Item::goldIngot_Id, 0, 2, 7, 15),
	new WeighedTreasure(Item::emerald_Id, 0, 1, 3, 2),
	new WeighedTreasure(Item::bone_Id, 0, 4, 6, 20),
	new WeighedTreasure(Item::rotten_flesh_Id, 0, 3, 7, 16),
	// very rare for pyramids ...
	new WeighedTreasure(Item::saddle_Id, 0, 1, 1, 3),
	new WeighedTreasure(Item::horseArmorMetal_Id, 0, 1, 1, 1),
	new WeighedTreasure(Item::horseArmorGold_Id, 0, 1, 1, 1),
	new WeighedTreasure(Item::horseArmorDiamond_Id, 0, 1, 1, 1),
	// ...
};

ScatteredFeaturePieces::DesertPyramidPiece::DesertPyramidPiece()
{
	hasPlacedChest[0] = false;
	hasPlacedChest[1] = false;
	hasPlacedChest[2] = false;
	hasPlacedChest[3] = false;
	// for reflection
}

ScatteredFeaturePieces::DesertPyramidPiece::DesertPyramidPiece(Random *random, int west, int north) : ScatteredFeaturePiece(random, west, 64, north, 21, 15, 21)
{
	hasPlacedChest[0] = false;
	hasPlacedChest[1] = false;
	hasPlacedChest[2] = false;
	hasPlacedChest[3] = false;
}

void ScatteredFeaturePieces::DesertPyramidPiece::addAdditonalSaveData(CompoundTag *tag)
{
	ScatteredFeaturePiece::addAdditonalSaveData(tag);
	tag->putBoolean(L"hasPlacedChest0", hasPlacedChest[0]);
	tag->putBoolean(L"hasPlacedChest1", hasPlacedChest[1]);
	tag->putBoolean(L"hasPlacedChest2", hasPlacedChest[2]);
	tag->putBoolean(L"hasPlacedChest3", hasPlacedChest[3]);
}

void ScatteredFeaturePieces::DesertPyramidPiece::readAdditonalSaveData(CompoundTag *tag)
{
	ScatteredFeaturePiece::readAdditonalSaveData(tag);
	hasPlacedChest[0] = tag->getBoolean(L"hasPlacedChest0");
	hasPlacedChest[1] = tag->getBoolean(L"hasPlacedChest1");
	hasPlacedChest[2] = tag->getBoolean(L"hasPlacedChest2");
	hasPlacedChest[3] = tag->getBoolean(L"hasPlacedChest3");
}

bool ScatteredFeaturePieces::DesertPyramidPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	// pyramid
	generateBox(level, chunkBB, 0, -4, 0, width - 1, 0, depth - 1, Tile::sandStone_Id, Tile::sandStone_Id, false);
	for (int pos = 1; pos <= 9; pos++)
	{
		generateBox(level, chunkBB, pos, pos, pos, width - 1 - pos, pos, depth - 1 - pos, Tile::sandStone_Id, Tile::sandStone_Id, false);
		generateBox(level, chunkBB, pos + 1, pos, pos + 1, width - 2 - pos, pos, depth - 2 - pos, 0, 0, false);
	}
	for (int x = 0; x < width; x++)
	{
		for (int z = 0; z < depth; z++)
		{
			fillColumnDown(level, Tile::sandStone_Id, 0, x, -5, z, chunkBB);
		}
	}

	int stairsNorth = getOrientationData(Tile::stairs_sandstone_Id, 3);
	int stairsSouth = getOrientationData(Tile::stairs_sandstone_Id, 2);
	int stairsEast = getOrientationData(Tile::stairs_sandstone_Id, 0);
	int stairsWest = getOrientationData(Tile::stairs_sandstone_Id, 1);
	int baseDecoColor = ~DyePowderItem::ORANGE & 0xf;
	int blue = ~DyePowderItem::BLUE & 0xf;

	// towers
	generateBox(level, chunkBB, 0, 0, 0, 4, 9, 4, Tile::sandStone_Id, 0, false);
	generateBox(level, chunkBB, 1, 10, 1, 3, 10, 3, Tile::sandStone_Id, Tile::sandStone_Id, false);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsNorth, 2, 10, 0, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsSouth, 2, 10, 4, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsEast, 0, 10, 2, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsWest, 4, 10, 2, chunkBB);
	generateBox(level, chunkBB, width - 5, 0, 0, width - 1, 9, 4, Tile::sandStone_Id, 0, false);
	generateBox(level, chunkBB, width - 4, 10, 1, width - 2, 10, 3, Tile::sandStone_Id, Tile::sandStone_Id, false);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsNorth, width - 3, 10, 0, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsSouth, width - 3, 10, 4, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsEast, width - 5, 10, 2, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsWest, width - 1, 10, 2, chunkBB);

	// entrance
	generateBox(level, chunkBB, 8, 0, 0, 12, 4, 4, Tile::sandStone_Id, 0, false);
	generateBox(level, chunkBB, 9, 1, 0, 11, 3, 4, 0, 0, false);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 9, 1, 1, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 9, 2, 1, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 9, 3, 1, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 10, 3, 1, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 11, 3, 1, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 11, 2, 1, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 11, 1, 1, chunkBB);

	// tower pathways
	generateBox(level, chunkBB, 4, 1, 1, 8, 3, 3, Tile::sandStone_Id, 0, false);
	generateBox(level, chunkBB, 4, 1, 2, 8, 2, 2, 0, 0, false);
	generateBox(level, chunkBB, 12, 1, 1, 16, 3, 3, Tile::sandStone_Id, 0, false);
	generateBox(level, chunkBB, 12, 1, 2, 16, 2, 2, 0, 0, false);

	// hall floor and pillars
	generateBox(level, chunkBB, 5, 4, 5, width - 6, 4, depth - 6, Tile::sandStone_Id, Tile::sandStone_Id, false);
	generateBox(level, chunkBB, 9, 4, 9, 11, 4, 11, 0, 0, false);
	generateBox(level, chunkBB, 8, 1, 8, 8, 3, 8, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, false);
	generateBox(level, chunkBB, 12, 1, 8, 12, 3, 8, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, false);
	generateBox(level, chunkBB, 8, 1, 12, 8, 3, 12, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, false);
	generateBox(level, chunkBB, 12, 1, 12, 12, 3, 12, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, false);

	// catwalks
	generateBox(level, chunkBB, 1, 1, 5, 4, 4, 11, Tile::sandStone_Id, Tile::sandStone_Id, false);
	generateBox(level, chunkBB, width - 5, 1, 5, width - 2, 4, 11, Tile::sandStone_Id, Tile::sandStone_Id, false);
	generateBox(level, chunkBB, 6, 7, 9, 6, 7, 11, Tile::sandStone_Id, Tile::sandStone_Id, false);
	generateBox(level, chunkBB, width - 7, 7, 9, width - 7, 7, 11, Tile::sandStone_Id, Tile::sandStone_Id, false);
	generateBox(level, chunkBB, 5, 5, 9, 5, 7, 11, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, false);
	generateBox(level, chunkBB, width - 6, 5, 9, width - 6, 7, 11, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, false);
	placeBlock(level, 0, 0, 5, 5, 10, chunkBB);
	placeBlock(level, 0, 0, 5, 6, 10, chunkBB);
	placeBlock(level, 0, 0, 6, 6, 10, chunkBB);
	placeBlock(level, 0, 0, width - 6, 5, 10, chunkBB);
	placeBlock(level, 0, 0, width - 6, 6, 10, chunkBB);
	placeBlock(level, 0, 0, width - 7, 6, 10, chunkBB);

	// tower stairs
	generateBox(level, chunkBB, 2, 4, 4, 2, 6, 4, 0, 0, false);
	generateBox(level, chunkBB, width - 3, 4, 4, width - 3, 6, 4, 0, 0, false);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsNorth, 2, 4, 5, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsNorth, 2, 3, 4, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsNorth, width - 3, 4, 5, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsNorth, width - 3, 3, 4, chunkBB);
	generateBox(level, chunkBB, 1, 1, 3, 2, 2, 3, Tile::sandStone_Id, Tile::sandStone_Id, false);
	generateBox(level, chunkBB, width - 3, 1, 3, width - 2, 2, 3, Tile::sandStone_Id, Tile::sandStone_Id, false);
	placeBlock(level, Tile::stairs_sandstone_Id, 0, 1, 1, 2, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, 0, width - 2, 1, 2, chunkBB);
	placeBlock(level, Tile::stoneSlabHalf_Id, StoneSlabTile::SAND_SLAB, 1, 2, 2, chunkBB);
	placeBlock(level, Tile::stoneSlabHalf_Id, StoneSlabTile::SAND_SLAB, width - 2, 2, 2, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsWest, 2, 1, 2, chunkBB);
	placeBlock(level, Tile::stairs_sandstone_Id, stairsEast, width - 3, 1, 2, chunkBB);

	// indoor decoration
	generateBox(level, chunkBB, 4, 3, 5, 4, 3, 18, Tile::sandStone_Id, Tile::sandStone_Id, false);
	generateBox(level, chunkBB, width - 5, 3, 5, width - 5, 3, 17, Tile::sandStone_Id, Tile::sandStone_Id, false);
	generateBox(level, chunkBB, 3, 1, 5, 4, 2, 16, 0, 0, false);
	generateBox(level, chunkBB, width - 6, 1, 5, width - 5, 2, 16, 0, 0, false);
	for (int z = 5; z <= 17; z += 2)
	{
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 4, 1, z, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, 4, 2, z, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, width - 5, 1, z, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, width - 5, 2, z, chunkBB);
	}
	placeBlock(level, Tile::wool_Id, baseDecoColor, 10, 0, 7, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 10, 0, 8, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 9, 0, 9, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 11, 0, 9, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 8, 0, 10, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 12, 0, 10, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 7, 0, 10, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 13, 0, 10, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 9, 0, 11, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 11, 0, 11, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 10, 0, 12, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 10, 0, 13, chunkBB);
	placeBlock(level, Tile::wool_Id, blue, 10, 0, 10, chunkBB);

	// outdoor decoration
	for (int x = 0; x <= width - 1; x += width - 1)
	{
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 2, 1, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 2, 2, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 2, 3, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 3, 1, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 3, 2, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 3, 3, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 4, 1, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, x, 4, 2, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 4, 3, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 5, 1, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 5, 2, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 5, 3, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 6, 1, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, x, 6, 2, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 6, 3, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 7, 1, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 7, 2, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 7, 3, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 8, 1, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 8, 2, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 8, 3, chunkBB);
	}
	for (int x = 2; x <= width - 3; x += width - 3 - 2)
	{
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x - 1, 2, 0, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 2, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x + 1, 2, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x - 1, 3, 0, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 3, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x + 1, 3, 0, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x - 1, 4, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, x, 4, 0, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x + 1, 4, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x - 1, 5, 0, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 5, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x + 1, 5, 0, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x - 1, 6, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, x, 6, 0, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x + 1, 6, 00, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x - 1, 7, 0, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x, 7, 0, chunkBB);
		placeBlock(level, Tile::wool_Id, baseDecoColor, x + 1, 7, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x - 1, 8, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x, 8, 0, chunkBB);
		placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, x + 1, 8, 0, chunkBB);
	}
	generateBox(level, chunkBB, 8, 4, 0, 12, 6, 0, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, false);
	placeBlock(level, 0, 0, 8, 6, 0, chunkBB);
	placeBlock(level, 0, 0, 12, 6, 0, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 9, 5, 0, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, 10, 5, 0, chunkBB);
	placeBlock(level, Tile::wool_Id, baseDecoColor, 11, 5, 0, chunkBB);

	// tombs
	generateBox(level, chunkBB, 8, -14, 8, 12, -11, 12, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, false);
	generateBox(level, chunkBB, 8, -10, 8, 12, -10, 12, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, false);
	generateBox(level, chunkBB, 8, -9, 8, 12, -9, 12, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, false);
	generateBox(level, chunkBB, 8, -8, 8, 12, -1, 12, Tile::sandStone_Id, Tile::sandStone_Id, false);
	generateBox(level, chunkBB, 9, -11, 9, 11, -1, 11, 0, 0, false);
	placeBlock(level, Tile::pressurePlate_stone_Id, 0, 10, -11, 10, chunkBB);
	generateBox(level, chunkBB, 9, -13, 9, 11, -13, 11, Tile::tnt_Id, 0, false);
	placeBlock(level, 0, 0, 8, -11, 10, chunkBB);
	placeBlock(level, 0, 0, 8, -10, 10, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, 7, -10, 10, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 7, -11, 10, chunkBB);
	placeBlock(level, 0, 0, 12, -11, 10, chunkBB);
	placeBlock(level, 0, 0, 12, -10, 10, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, 13, -10, 10, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 13, -11, 10, chunkBB);
	placeBlock(level, 0, 0, 10, -11, 8, chunkBB);
	placeBlock(level, 0, 0, 10, -10, 8, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, 10, -10, 7, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 10, -11, 7, chunkBB);
	placeBlock(level, 0, 0, 10, -11, 12, chunkBB);
	placeBlock(level, 0, 0, 10, -10, 12, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_HEIROGLYPHS, 10, -10, 13, chunkBB);
	placeBlock(level, Tile::sandStone_Id, SandStoneTile::TYPE_SMOOTHSIDE, 10, -11, 13, chunkBB);

	// chests!
	for (int i = 0; i < 4; i++)
	{
		if (!hasPlacedChest[i])
		{
			int xo = Direction::STEP_X[i] * 2;
			int zo = Direction::STEP_Z[i] * 2;
			hasPlacedChest[i] = createChest(level, chunkBB, random, 10 + xo, -11, 10 + zo, WeighedTreasure::addToTreasure(WeighedTreasureArray(treasureItems,TREASURE_ITEMS_COUNT), Item::enchantedBook->createForRandomTreasure(random)), 2 + random->nextInt(5));
		}
	}

	return true;
}

WeighedTreasure *ScatteredFeaturePieces::JunglePyramidPiece::treasureItems[ScatteredFeaturePieces::JunglePyramidPiece::TREASURE_ITEMS_COUNT] = 
{
	new WeighedTreasure(Item::diamond_Id, 0, 1, 3, 3),
	new WeighedTreasure(Item::ironIngot_Id, 0, 1, 5, 10),
	new WeighedTreasure(Item::goldIngot_Id, 0, 2, 7, 15),
	new WeighedTreasure(Item::emerald_Id, 0, 1, 3, 2),
	new WeighedTreasure(Item::bone_Id, 0, 4, 6, 20),
	new WeighedTreasure(Item::rotten_flesh_Id, 0, 3, 7, 16),
	// very rare for pyramids ...
	new WeighedTreasure(Item::saddle_Id, 0, 1, 1, 3),
	new WeighedTreasure(Item::horseArmorMetal_Id, 0, 1, 1, 1),
	new WeighedTreasure(Item::horseArmorGold_Id, 0, 1, 1, 1),
	new WeighedTreasure(Item::horseArmorDiamond_Id, 0, 1, 1, 1),
	// ...
};


WeighedTreasure *ScatteredFeaturePieces::JunglePyramidPiece::dispenserItems[ScatteredFeaturePieces::JunglePyramidPiece::DISPENSER_ITEMS_COUNT] = 
{
	new WeighedTreasure(Item::arrow_Id, 0, 2, 7, 30),
	//            new WeighedTreasure(Item.fireball.id, 0, 1, 1, 10),
};

ScatteredFeaturePieces::JunglePyramidPiece::JunglePyramidPiece()
{
	// for reflection
}

ScatteredFeaturePieces::JunglePyramidPiece::JunglePyramidPiece(Random *random, int west, int north) : ScatteredFeaturePiece(random, west, 64, north, 12, 10, 15)
{
	placedMainChest = false;
	placedHiddenChest = false;
	placedTrap1 = false;
	placedTrap2 = false;
}

void ScatteredFeaturePieces::JunglePyramidPiece::addAdditonalSaveData(CompoundTag *tag)
{
	ScatteredFeaturePiece::addAdditonalSaveData(tag);
	tag->putBoolean(L"placedMainChest", placedMainChest);
	tag->putBoolean(L"placedHiddenChest", placedHiddenChest);
	tag->putBoolean(L"placedTrap1", placedTrap1);
	tag->putBoolean(L"placedTrap2", placedTrap2);
}

void ScatteredFeaturePieces::JunglePyramidPiece::readAdditonalSaveData(CompoundTag *tag)
{
	ScatteredFeaturePiece::readAdditonalSaveData(tag);
	placedMainChest = tag->getBoolean(L"placedMainChest");
	placedHiddenChest = tag->getBoolean(L"placedHiddenChest");
	placedTrap1 = tag->getBoolean(L"placedTrap1");
	placedTrap2 = tag->getBoolean(L"placedTrap2");
}

bool ScatteredFeaturePieces::JunglePyramidPiece::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (!updateAverageGroundHeight(level, chunkBB, 0))
	{
		return false;
	}

	int stairsNorth = getOrientationData(Tile::stairs_stone_Id, 3);
	int stairsSouth = getOrientationData(Tile::stairs_stone_Id, 2);
	int stairsEast = getOrientationData(Tile::stairs_stone_Id, 0);
	int stairsWest = getOrientationData(Tile::stairs_stone_Id, 1);

	// floor
	generateBox(level, chunkBB, 0, -4, 0, width - 1, 0, depth - 1, false, random, &stoneSelector);

	// first floor walls
	generateBox(level, chunkBB, 2, 1, 2, 9, 2, 2, false, random, &stoneSelector);
	generateBox(level, chunkBB, 2, 1, 12, 9, 2, 12, false, random, &stoneSelector);
	generateBox(level, chunkBB, 2, 1, 3, 2, 2, 11, false, random, &stoneSelector);
	generateBox(level, chunkBB, 9, 1, 3, 9, 2, 11, false, random, &stoneSelector);

	// second floor walls
	generateBox(level, chunkBB, 1, 3, 1, 10, 6, 1, false, random, &stoneSelector);
	generateBox(level, chunkBB, 1, 3, 13, 10, 6, 13, false, random, &stoneSelector);
	generateBox(level, chunkBB, 1, 3, 2, 1, 6, 12, false, random, &stoneSelector);
	generateBox(level, chunkBB, 10, 3, 2, 10, 6, 12, false, random, &stoneSelector);

	// roof levels
	generateBox(level, chunkBB, 2, 3, 2, 9, 3, 12, false, random, &stoneSelector);
	generateBox(level, chunkBB, 2, 6, 2, 9, 6, 12, false, random, &stoneSelector);
	generateBox(level, chunkBB, 3, 7, 3, 8, 7, 11, false, random, &stoneSelector);
	generateBox(level, chunkBB, 4, 8, 4, 7, 8, 10, false, random, &stoneSelector);

	// clear interior
	generateAirBox(level, chunkBB, 3, 1, 3, 8, 2, 11);
	generateAirBox(level, chunkBB, 4, 3, 6, 7, 3, 9);
	generateAirBox(level, chunkBB, 2, 4, 2, 9, 5, 12);
	generateAirBox(level, chunkBB, 4, 6, 5, 7, 6, 9);
	generateAirBox(level, chunkBB, 5, 7, 6, 6, 7, 8);

	// doors and windows
	generateAirBox(level, chunkBB, 5, 1, 2, 6, 2, 2);
	generateAirBox(level, chunkBB, 5, 2, 12, 6, 2, 12);
	generateAirBox(level, chunkBB, 5, 5, 1, 6, 5, 1);
	generateAirBox(level, chunkBB, 5, 5, 13, 6, 5, 13);
	placeBlock(level, 0, 0, 1, 5, 5, chunkBB);
	placeBlock(level, 0, 0, 10, 5, 5, chunkBB);
	placeBlock(level, 0, 0, 1, 5, 9, chunkBB);
	placeBlock(level, 0, 0, 10, 5, 9, chunkBB);

	// outside decoration
	for (int z = 0; z <= 14; z += 14)
	{
		generateBox(level, chunkBB, 2, 4, z, 2, 5, z, false, random, &stoneSelector);
		generateBox(level, chunkBB, 4, 4, z, 4, 5, z, false, random, &stoneSelector);
		generateBox(level, chunkBB, 7, 4, z, 7, 5, z, false, random, &stoneSelector);
		generateBox(level, chunkBB, 9, 4, z, 9, 5, z, false, random, &stoneSelector);
	}
	generateBox(level, chunkBB, 5, 6, 0, 6, 6, 0, false, random, &stoneSelector);
	for (int x = 0; x <= 11; x += 11)
	{
		for (int z = 2; z <= 12; z += 2)
		{
			generateBox(level, chunkBB, x, 4, z, x, 5, z, false, random, &stoneSelector);
		}
		generateBox(level, chunkBB, x, 6, 5, x, 6, 5, false, random, &stoneSelector);
		generateBox(level, chunkBB, x, 6, 9, x, 6, 9, false, random, &stoneSelector);
	}
	generateBox(level, chunkBB, 2, 7, 2, 2, 9, 2, false, random, &stoneSelector);
	generateBox(level, chunkBB, 9, 7, 2, 9, 9, 2, false, random, &stoneSelector);
	generateBox(level, chunkBB, 2, 7, 12, 2, 9, 12, false, random, &stoneSelector);
	generateBox(level, chunkBB, 9, 7, 12, 9, 9, 12, false, random, &stoneSelector);
	generateBox(level, chunkBB, 4, 9, 4, 4, 9, 4, false, random, &stoneSelector);
	generateBox(level, chunkBB, 7, 9, 4, 7, 9, 4, false, random, &stoneSelector);
	generateBox(level, chunkBB, 4, 9, 10, 4, 9, 10, false, random, &stoneSelector);
	generateBox(level, chunkBB, 7, 9, 10, 7, 9, 10, false, random, &stoneSelector);
	generateBox(level, chunkBB, 5, 9, 7, 6, 9, 7, false, random, &stoneSelector);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 5, 9, 6, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 6, 9, 6, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsSouth, 5, 9, 8, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsSouth, 6, 9, 8, chunkBB);

	// front stairs
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 4, 0, 0, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 5, 0, 0, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 6, 0, 0, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 7, 0, 0, chunkBB);

	// indoor stairs up
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 4, 1, 8, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 4, 2, 9, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 4, 3, 10, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 7, 1, 8, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 7, 2, 9, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsNorth, 7, 3, 10, chunkBB);
	generateBox(level, chunkBB, 4, 1, 9, 4, 1, 9, false, random, &stoneSelector);
	generateBox(level, chunkBB, 7, 1, 9, 7, 1, 9, false, random, &stoneSelector);
	generateBox(level, chunkBB, 4, 1, 10, 7, 2, 10, false, random, &stoneSelector);

	// indoor hand rail
	generateBox(level, chunkBB, 5, 4, 5, 6, 4, 5, false, random, &stoneSelector);
	placeBlock(level, Tile::stairs_stone_Id, stairsEast, 4, 4, 5, chunkBB);
	placeBlock(level, Tile::stairs_stone_Id, stairsWest, 7, 4, 5, chunkBB);

	// indoor stairs down
	for (int i = 0; i < 4; i++)
	{
		placeBlock(level, Tile::stairs_stone_Id, stairsSouth, 5, 0 - i, 6 + i, chunkBB);
		placeBlock(level, Tile::stairs_stone_Id, stairsSouth, 6, 0 - i, 6 + i, chunkBB);
		generateAirBox(level, chunkBB, 5, 0 - i, 7 + i, 6, 0 - i, 9 + i);
	}

	// underground corridors
	generateAirBox(level, chunkBB, 1, -3, 12, 10, -1, 13);
	generateAirBox(level, chunkBB, 1, -3, 1, 3, -1, 13);
	generateAirBox(level, chunkBB, 1, -3, 1, 9, -1, 5);
	for (int z = 1; z <= 13; z += 2)
	{
		generateBox(level, chunkBB, 1, -3, z, 1, -2, z, false, random, &stoneSelector);
	}
	for (int z = 2; z <= 12; z += 2)
	{
		generateBox(level, chunkBB, 1, -1, z, 3, -1, z, false, random, &stoneSelector);
	}
	generateBox(level, chunkBB, 2, -2, 1, 5, -2, 1, false, random, &stoneSelector);
	generateBox(level, chunkBB, 7, -2, 1, 9, -2, 1, false, random, &stoneSelector);
	generateBox(level, chunkBB, 6, -3, 1, 6, -3, 1, false, random, &stoneSelector);
	generateBox(level, chunkBB, 6, -1, 1, 6, -1, 1, false, random, &stoneSelector);

	// trip wire trap 1
	placeBlock(level, Tile::tripWireSource_Id, getOrientationData(Tile::tripWireSource_Id, Direction::EAST) | TripWireSourceTile::MASK_ATTACHED, 1, -3, 8, chunkBB);
	placeBlock(level, Tile::tripWireSource_Id, getOrientationData(Tile::tripWireSource_Id, Direction::WEST) | TripWireSourceTile::MASK_ATTACHED, 4, -3, 8, chunkBB);
	placeBlock(level, Tile::tripWire_Id, TripWireTile::MASK_ATTACHED, 2, -3, 8, chunkBB);
	placeBlock(level, Tile::tripWire_Id, TripWireTile::MASK_ATTACHED, 3, -3, 8, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 5, -3, 7, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 5, -3, 6, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 5, -3, 5, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 5, -3, 4, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 5, -3, 3, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 5, -3, 2, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 5, -3, 1, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 4, -3, 1, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 3, -3, 1, chunkBB);
	if (!placedTrap1)
	{
		placedTrap1 = createDispenser(level, chunkBB, random, 3, -2, 1, Facing::NORTH, WeighedTreasureArray(dispenserItems,DISPENSER_ITEMS_COUNT), 2);
	}
	placeBlock(level, Tile::vine_Id, 0xf, 3, -2, 2, chunkBB);

	// trip wire trap 2
	placeBlock(level, Tile::tripWireSource_Id, getOrientationData(Tile::tripWireSource_Id, Direction::NORTH) | TripWireSourceTile::MASK_ATTACHED, 7, -3, 1, chunkBB);
	placeBlock(level, Tile::tripWireSource_Id, getOrientationData(Tile::tripWireSource_Id, Direction::SOUTH) | TripWireSourceTile::MASK_ATTACHED, 7, -3, 5, chunkBB);
	placeBlock(level, Tile::tripWire_Id, TripWireTile::MASK_ATTACHED, 7, -3, 2, chunkBB);
	placeBlock(level, Tile::tripWire_Id, TripWireTile::MASK_ATTACHED, 7, -3, 3, chunkBB);
	placeBlock(level, Tile::tripWire_Id, TripWireTile::MASK_ATTACHED, 7, -3, 4, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 8, -3, 6, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 9, -3, 6, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 9, -3, 5, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 9, -3, 4, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 9, -2, 4, chunkBB);
	if (!placedTrap2)
	{
		placedTrap2 = createDispenser(level, chunkBB, random, 9, -2, 3, Facing::WEST, WeighedTreasureArray(dispenserItems,DISPENSER_ITEMS_COUNT), 2);
	}
	placeBlock(level, Tile::vine_Id, 0xf, 8, -1, 3, chunkBB);
	placeBlock(level, Tile::vine_Id, 0xf, 8, -2, 3, chunkBB);
	if (!placedMainChest)
	{
		placedMainChest = createChest(level, chunkBB, random, 8, -3, 3, WeighedTreasure::addToTreasure(WeighedTreasureArray(treasureItems,TREASURE_ITEMS_COUNT), Item::enchantedBook->createForRandomTreasure(random)), 2 + random->nextInt(5));
	}
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 9, -3, 2, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 8, -3, 1, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 4, -3, 5, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 5, -2, 5, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 5, -1, 5, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 6, -3, 5, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 7, -2, 5, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 7, -1, 5, chunkBB);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 8, -3, 5, chunkBB);
	generateBox(level, chunkBB, 9, -1, 1, 9, -1, 5, false, random, &stoneSelector);

	// hidden room
	generateAirBox(level, chunkBB, 8, -3, 8, 10, -1, 10);
	placeBlock(level, Tile::stoneBrick_Id, SmoothStoneBrickTile::TYPE_DETAIL, 8, -2, 11, chunkBB);
	placeBlock(level, Tile::stoneBrick_Id, SmoothStoneBrickTile::TYPE_DETAIL, 9, -2, 11, chunkBB);
	placeBlock(level, Tile::stoneBrick_Id, SmoothStoneBrickTile::TYPE_DETAIL, 10, -2, 11, chunkBB);
	placeBlock(level, Tile::lever_Id, LeverTile::getLeverFacing(getOrientationData(Tile::lever_Id, Facing::NORTH)), 8, -2, 12, chunkBB);
	placeBlock(level, Tile::lever_Id, LeverTile::getLeverFacing(getOrientationData(Tile::lever_Id, Facing::NORTH)), 9, -2, 12, chunkBB);
	placeBlock(level, Tile::lever_Id, LeverTile::getLeverFacing(getOrientationData(Tile::lever_Id, Facing::NORTH)), 10, -2, 12, chunkBB);
	generateBox(level, chunkBB, 8, -3, 8, 8, -3, 10, false, random, &stoneSelector);
	generateBox(level, chunkBB, 10, -3, 8, 10, -3, 10, false, random, &stoneSelector);
	placeBlock(level, Tile::mossyCobblestone_Id, 0, 10, -2, 9, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 8, -2, 9, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 8, -2, 10, chunkBB);
	placeBlock(level, Tile::redStoneDust_Id, 0, 10, -1, 9, chunkBB);
	placeBlock(level, Tile::pistonStickyBase_Id, Facing::UP, 9, -2, 8, chunkBB);
	placeBlock(level, Tile::pistonStickyBase_Id, getOrientationData(Tile::pistonStickyBase_Id, Facing::WEST), 10, -2, 8, chunkBB);
	placeBlock(level, Tile::pistonStickyBase_Id, getOrientationData(Tile::pistonStickyBase_Id, Facing::WEST), 10, -1, 8, chunkBB);
	placeBlock(level, Tile::diode_off_Id, getOrientationData(Tile::diode_off_Id, Direction::NORTH), 10, -2, 10, chunkBB);
	if (!placedHiddenChest)
	{
		placedHiddenChest = createChest(level, chunkBB, random, 9, -3, 10, WeighedTreasure::addToTreasure(WeighedTreasureArray(treasureItems,TREASURE_ITEMS_COUNT), Item::enchantedBook->createForRandomTreasure(random)), 2 + random->nextInt(5));
	}

	return true;
}

void ScatteredFeaturePieces::JunglePyramidPiece::MossStoneSelector::next(Random *random, int worldX, int worldY, int worldZ, bool isEdge)
{
	if (random->nextFloat() < .4f)
	{
		nextId = Tile::cobblestone_Id;
	}
	else
	{
		nextId = Tile::mossyCobblestone_Id;
	}
}

ScatteredFeaturePieces::JunglePyramidPiece::MossStoneSelector ScatteredFeaturePieces::JunglePyramidPiece::stoneSelector;

ScatteredFeaturePieces::SwamplandHut::SwamplandHut()
{
	spawnedWitch = false;
	// for reflection
}

ScatteredFeaturePieces::SwamplandHut::SwamplandHut(Random *random, int west, int north) : ScatteredFeaturePiece(random, west, 64, north, 7, 5, 9)
{
	spawnedWitch = false;
}

void ScatteredFeaturePieces::SwamplandHut::addAdditonalSaveData(CompoundTag *tag)
{
	ScatteredFeaturePiece::addAdditonalSaveData(tag);
	tag->putBoolean(L"Witch", spawnedWitch);
}

void ScatteredFeaturePieces::SwamplandHut::readAdditonalSaveData(CompoundTag *tag)
{
	ScatteredFeaturePiece::readAdditonalSaveData(tag);
	spawnedWitch = tag->getBoolean(L"Witch");
}

bool ScatteredFeaturePieces::SwamplandHut::postProcess(Level *level, Random *random, BoundingBox *chunkBB)
{
	if (!updateAverageGroundHeight(level, chunkBB, 0))
	{
		return false;
	}

	// floor and ceiling
	generateBox(level, chunkBB, 1, 1, 1, 5, 1, 7, Tile::wood_Id, TreeTile::DARK_TRUNK, Tile::wood_Id, TreeTile::DARK_TRUNK, false);
	generateBox(level, chunkBB, 1, 4, 2, 5, 4, 7, Tile::wood_Id, TreeTile::DARK_TRUNK, Tile::wood_Id, TreeTile::DARK_TRUNK, false);
	generateBox(level, chunkBB, 2, 1, 0, 4, 1, 0, Tile::wood_Id, TreeTile::DARK_TRUNK, Tile::wood_Id, TreeTile::DARK_TRUNK, false);

	// walls
	generateBox(level, chunkBB, 2, 2, 2, 3, 3, 2, Tile::wood_Id, TreeTile::DARK_TRUNK, Tile::wood_Id, TreeTile::DARK_TRUNK, false);
	generateBox(level, chunkBB, 1, 2, 3, 1, 3, 6, Tile::wood_Id, TreeTile::DARK_TRUNK, Tile::wood_Id, TreeTile::DARK_TRUNK, false);
	generateBox(level, chunkBB, 5, 2, 3, 5, 3, 6, Tile::wood_Id, TreeTile::DARK_TRUNK, Tile::wood_Id, TreeTile::DARK_TRUNK, false);
	generateBox(level, chunkBB, 2, 2, 7, 4, 3, 7, Tile::wood_Id, TreeTile::DARK_TRUNK, Tile::wood_Id, TreeTile::DARK_TRUNK, false);

	// pillars
	generateBox(level, chunkBB, 1, 0, 2, 1, 3, 2, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 5, 0, 2, 5, 3, 2, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 1, 0, 7, 1, 3, 7, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);
	generateBox(level, chunkBB, 5, 0, 7, 5, 3, 7, Tile::treeTrunk_Id, Tile::treeTrunk_Id, false);

	// windows
	placeBlock(level, Tile::fence_Id, 0, 2, 3, 2, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 3, 3, 7, chunkBB);
	placeBlock(level, 0, 0, 1, 3, 4, chunkBB);
	placeBlock(level, 0, 0, 5, 3, 4, chunkBB);
	placeBlock(level, 0, 0, 5, 3, 5, chunkBB);
	placeBlock(level, Tile::flowerPot_Id, FlowerPotTile::TYPE_MUSHROOM_RED, 1, 3, 5, chunkBB);

	// decoration
	placeBlock(level, Tile::workBench_Id, 0, 3, 2, 6, chunkBB);
	placeBlock(level, Tile::cauldron_Id, 0, 4, 2, 6, chunkBB);

	// front railings
	placeBlock(level, Tile::fence_Id, 0, 1, 2, 1, chunkBB);
	placeBlock(level, Tile::fence_Id, 0, 5, 2, 1, chunkBB);
	// placeBlock(level, Tile.torch.id, 0, 1, 3, 1, chunkBB);
	// placeBlock(level, Tile.torch.id, 0, 5, 3, 1, chunkBB);

	// ceiling edges
	int south = getOrientationData(Tile::stairs_wood_Id, StairTile::DIR_NORTH);
	int east = getOrientationData(Tile::stairs_wood_Id, StairTile::DIR_WEST);
	int west = getOrientationData(Tile::stairs_wood_Id, StairTile::DIR_EAST);
	int north = getOrientationData(Tile::stairs_wood_Id, StairTile::DIR_SOUTH);

	generateBox(level, chunkBB, 0, 4, 1, 6, 4, 1, Tile::stairs_sprucewood_Id, south, Tile::stairs_sprucewood_Id, south, false);
	generateBox(level, chunkBB, 0, 4, 2, 0, 4, 7, Tile::stairs_sprucewood_Id, west, Tile::stairs_sprucewood_Id, west, false);
	generateBox(level, chunkBB, 6, 4, 2, 6, 4, 7, Tile::stairs_sprucewood_Id, east, Tile::stairs_sprucewood_Id, east, false);
	generateBox(level, chunkBB, 0, 4, 8, 6, 4, 8, Tile::stairs_sprucewood_Id, north, Tile::stairs_sprucewood_Id, north, false);

	// fill pillars down to solid ground
	for (int z = 2; z <= 7; z += 5)
	{
		for (int x = 1; x <= 5; x += 4)
		{
			fillColumnDown(level, Tile::treeTrunk_Id, 0, x, -1, z, chunkBB);
		}
	}

	if (!spawnedWitch)
	{
		int wx = getWorldX(2, 5);
		int wy = getWorldY(2);
		int wz = getWorldZ(2, 5);

		if (chunkBB->isInside(wx, wy, wz))
		{
			spawnedWitch = true;

			shared_ptr<Witch> witch = std::make_shared<Witch>(level);
			witch->moveTo(wx + .5, wy, wz + .5, 0, 0);
			witch->finalizeMobSpawn(nullptr);
			level->addEntity(witch);
		}
	}

	return true;
}