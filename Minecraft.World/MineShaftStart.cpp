#include "stdafx.h"
#include "net.minecraft.world.level.levelgen.structure.h"

MineShaftStart::MineShaftStart()
{
	// for reflection
}

MineShaftStart::MineShaftStart(Level *level, Random *random, int chunkX, int chunkZ) : StructureStart(chunkX, chunkZ)
{
	MineShaftPieces::MineShaftRoom *mineShaftRoom = new MineShaftPieces::MineShaftRoom(0, random, (chunkX << 4) + 2, (chunkZ << 4) + 2);
	pieces.push_back(mineShaftRoom);
	mineShaftRoom->addChildren(mineShaftRoom, &pieces, random);

	calculateBoundingBox();
	moveBelowSeaLevel(level, random, 10);
}