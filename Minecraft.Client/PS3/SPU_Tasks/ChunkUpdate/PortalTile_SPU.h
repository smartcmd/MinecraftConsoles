#pragma once
#include "HalfTransparentTile_SPU.h"

class PortalTile_SPU : public HalfTransparentTile_SPU
{
public:
	PortalTile_SPU(int id): HalfTransparentTile_SPU(id) {}
    virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		if (level->getTile(x - 1, y, z) == id || level->getTile(x + 1, y, z) == id)
		{
			float xr = 8 / 16.0f;
			float yr = 2 / 16.0f;
			this->setShape(0.5f - xr, 0, 0.5f - yr, 0.5f + xr, 1, 0.5f + yr);
		}
		else
		{
			float xr = 2 / 16.0f;
			float yr = 8 / 16.0f;
			this->setShape(0.5f - xr, 0, 0.5f - yr, 0.5f + xr, 1, 0.5f + yr);
		}

	}
    virtual bool isSolidRender(bool isServerLevel = false) { return false; }
    virtual bool isCubeShaped() { return false; }
    virtual bool shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face)
	{
		if (level->getTile(x, y, z) == id) return false;

		bool w = level->getTile(x - 1, y, z) == id && level->getTile(x - 2, y, z) != id;
		bool e = level->getTile(x + 1, y, z) == id && level->getTile(x + 2, y, z) != id;

		bool n = level->getTile(x, y, z - 1) == id && level->getTile(x, y, z - 2) != id;
		bool s = level->getTile(x, y, z + 1) == id && level->getTile(x, y, z + 2) != id;

		bool we = w || e;
		bool ns = n || s;

		if (we && face == 4) return true;
		if (we && face == 5) return true;
		if (ns && face == 2) return true;
		if (ns && face == 3) return true;

		return false;
	}

    virtual int getRenderLayer() { return 1; }
};
