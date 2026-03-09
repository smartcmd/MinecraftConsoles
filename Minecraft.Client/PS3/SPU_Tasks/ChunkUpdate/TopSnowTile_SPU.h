#pragma once

#include "Tile_SPU.h"
#include "ChunkRebuildData.h"

class TopSnowTile_SPU : public Tile_SPU
{
public:
	static const int MAX_HEIGHT = 6;
	static const int HEIGHT_MASK = 7;

	TopSnowTile_SPU(int id) : Tile_SPU(id) {}

	bool blocksLight() { return false; }
	bool isSolidRender(bool isServerLevel = false) { return false; }
	bool isCubeShaped() { return false; }
	void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		int height = level->getData(x, y, z) & HEIGHT_MASK;
		float o = 2 * (1 + height) / 16.0f;
		setShape(0, 0, 0, 1, o, 1);
	}
	bool shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face)
	{
		//        Material m = level.getMaterial(x, y, z);
		if (face == 1) return true;
		// 4J - don't render faces if neighbouring tiles are also TopSnowTile with at least the same height as this one
		// Otherwise we get horrible artifacts from the non-manifold geometry created. Fixes bug #8506
		if ( ( level->getTile(x,y,z) == Tile_SPU::topSnow_Id ) && ( face >= 2 ) )
		{
			int h0 = level->getData(x,y,z) & HEIGHT_MASK;
			int xx = x;
			int yy = y;
			int zz = z;
			// Work out coords of tile who's face we're considering (rather than it's neighbour which is passed in here as x,y,z already
			// offsetting by the face direction)
			switch(face)
			{
			case 2:
				zz += 1;
				break;
			case 3:
				zz -= 1;
				break;
			case 4:
				xx += 1;
				break;
			case 5:
				xx -= 1;
				break;
			default:
				break;
			}
			int h1 = level->getData(xx,yy,zz) & HEIGHT_MASK;
			if( h0 >= h1 ) return false;
		}
		//        if (m == this.material) return false;
		return Tile_SPU::shouldRenderFace(level, x, y, z, face);

	}
};