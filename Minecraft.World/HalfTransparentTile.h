#pragma once
#include "Tile.h"

class ChunkRebuildData;

class HalfTransparentTile : public Tile
{
	friend class ChunkRebuildData;
private:
	bool allowSame;
	wstring texture;
protected:
	HalfTransparentTile(int id, const wstring &tex, Material *material, bool allowSame);
public:
    virtual bool isSolidRender(bool isServerLevel = false);
    virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
    virtual bool blocksLight();
	virtual void registerIcons(IconRegister *iconRegister);
};
