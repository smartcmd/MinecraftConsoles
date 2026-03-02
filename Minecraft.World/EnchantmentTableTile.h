#pragma once
#include "BaseEntityTile.h"

class ChunkRebuildData;
class EnchantmentTableTile : public BaseEntityTile
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
	virtual bool isCubeShaped();
	virtual void animateTick(Level *level, int x, int y, int z, Random *random);
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual Icon *getTexture(int face, int data);
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance);
	virtual void registerIcons(IconRegister *iconRegister);
};
