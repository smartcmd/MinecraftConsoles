#pragma once
#include "EntityTile.h"

class ChunkRebuildData;
class EnchantmentTableTile : public EntityTile
{
	friend class ChunkRebuildData;
public:
	static const wstring TEXTURE_SIDE;
	static const wstring TEXTURE_TOP;
	static const wstring TEXTURE_BOTTOM;

private:
	Icon *iconTop;
	Icon *iconBottom;

public:
	EnchantmentTableTile(int id);
	
    virtual void updateDefaultShape(); // 4J Added override
	bool isCubeShaped();
    void animateTick(Level *level, int x, int y, int z, Random *random);
    bool isSolidRender(bool isServerLevel = false);
    Icon *getTexture(int face, int data);
    shared_ptr<TileEntity> newTileEntity(Level *level);
    bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	//@Override
	void registerIcons(IconRegister *iconRegister);
};
