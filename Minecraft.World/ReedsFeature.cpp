#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "ReedsFeature.h"

bool ReedsFeature::place(Level *level, Random *random, int x, int y, int z)
{
	for (int i = 0; i < 20; i++)
	{
		int x2 = x + random->nextInt(4) - random->nextInt(4);
		int y2 = y;
		int z2 = z + random->nextInt(4) - random->nextInt(4);

		// 4J Stu Added to stop reed features generating areas previously place by game rule generation
		if(app.getLevelGenerationOptions() != NULL)
		{
			LevelGenerationOptions *levelGenOptions = app.getLevelGenerationOptions();
			bool intersects = levelGenOptions->checkIntersects(x2, y2, z2, x2, y2, z2);
			if(intersects)
			{
				//app.DebugPrintf("Skipping reeds feature generation as it overlaps a game rule structure\n");
				continue;
			}
		}
		if (level->isEmptyTile(x2, y2, z2)) 
		{
			if (level->getMaterial(x2-1, y2-1, z2) == Material::water || 
				level->getMaterial(x2+1, y2-1, z2) == Material::water || 
				level->getMaterial(x2, y2-1, z2-1) == Material::water || 
				level->getMaterial(x2, y2-1, z2+1) == Material::water)
			{

				int h = 2 + random->nextInt(random->nextInt(3) + 1);
				for (int yy = 0; yy < h; yy++)
				{
					if ( Tile::reeds->canSurvive(level, x2, y2 + yy, z2) )
					{
						level->setTileAndData(x2, y2 + yy, z2, Tile::reeds_Id, 0, Tile::UPDATE_CLIENTS);
					}
				}
			}
		}
	}

	return true;
}