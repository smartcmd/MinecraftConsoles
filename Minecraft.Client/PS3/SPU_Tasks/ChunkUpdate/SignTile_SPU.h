#pragma once

#include "EntityTile_SPU.h"


class SignTile_SPU : public EntityTile_SPU
{
public:
	SignTile_SPU(int id) : EntityTile_SPU(id) {}
	bool onGround()
	{
		if(id == wallSign_Id)
			return false;
		// sign_Id
		return true;
	}

	Icon_SPU *getTexture(int face, int data){	return TileRef_SPU(wood_Id)->getTexture(face);	}
	void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = nullptr)	// 4J added forceData, forceEntity param
	{
		if (onGround()) return;
		int face = level->getData(x, y, z);
		float h0 = (4 + 0.5f) / 16.0f;
		float h1 = (12 + 0.5f) / 16.0f;
		float w0 = 0 / 16.0f;
		float w1 = 16 / 16.0f;
		float d0 = 2 / 16.0f;
		setShape(0, 0, 0, 1, 1, 1);
		if (face == 2) setShape(w0, h0, 1 - d0, w1, h1, 1);
		if (face == 3) setShape(w0, h0, 0, w1, h1, d0);
		if (face == 4) setShape(1 - d0, h0, w0, 1, h1, w1);
		if (face == 5) setShape(0, h0, w0, d0, h1, w1);
	}

	int getRenderShape() { 	return Tile_SPU::SHAPE_INVISIBLE; }
	bool isCubeShaped() { return false; }
	bool isSolidRender(bool isServerLevel = false) { return false; }
};