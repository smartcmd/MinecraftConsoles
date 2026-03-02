#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.animal.h"
#include "JungleBiome.h"

JungleBiome::JungleBiome(int id) : Biome(id)
{
	decorator->treeCount = 50;
	decorator->grassCount = 25;
	decorator->flowerCount = 4;

	enemies.push_back(new MobSpawnerData(eTYPE_OCELOT, 2, 1, 1));

	// make chicken a lot more common in the jungle
	friendlies.push_back(new MobSpawnerData(eTYPE_CHICKEN, 10, 4, 4));
}


Feature *JungleBiome::getTreeFeature(Random *random)
{
	if (random->nextInt(10) == 0)
	{
		return new BasicTree(false); // 4J used to return member fancyTree, now returning newly created object so that caller can be consistently resposible for cleanup
	}
	if (random->nextInt(2) == 0)
	{
		return new GroundBushFeature(TreeTile::JUNGLE_TRUNK, LeafTile::NORMAL_LEAF);
	}
	if (random->nextInt(3) == 0)
	{
		return new MegaTreeFeature(false, 10 + random->nextInt(20), TreeTile::JUNGLE_TRUNK, LeafTile::JUNGLE_LEAF);
	}
	return new TreeFeature(false, 4 + random->nextInt(7), TreeTile::JUNGLE_TRUNK, LeafTile::JUNGLE_LEAF, true);
}

Feature *JungleBiome::getGrassFeature(Random *random)
{
	if (random->nextInt(4) == 0)
	{
		return new TallGrassFeature(Tile::tallgrass_Id, TallGrass::FERN);
	}
	return new TallGrassFeature(Tile::tallgrass_Id, TallGrass::TALL_GRASS);
}

void JungleBiome::decorate(Level *level, Random *random, int xo, int zo)
{
	Biome::decorate(level, random, xo, zo);

	PIXBeginNamedEvent(0, "Adding vines");
	VinesFeature *vines = new VinesFeature();

	for (int i = 0; i < 50; i++)
	{
		int x = xo + random->nextInt(16) + 8;
		int y = Level::genDepth / 2;
		int z = zo + random->nextInt(16) + 8;
		vines->place(level, random, x, y, z);
	}
	PIXEndNamedEvent();
}