#pragma once
#include "BaseEntityTile.h"

class Player;

class NoteBlockTile : public BaseEntityTile
{
public:
	NoteBlockTile(int id);
	virtual void neighborChanged(Level *level, int x, int y, int z, int type);
	virtual bool TestUse();
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
	virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
	virtual shared_ptr<TileEntity> newTileEntity(Level *level);
	virtual bool triggerEvent(Level *level, int x, int y, int z, int i, int note);
};