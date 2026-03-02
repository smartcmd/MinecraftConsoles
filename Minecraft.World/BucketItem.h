#pragma once

#include "Item.h"

class Player;
class Level;

class BucketItem : public Item
{
private:
	int content;

public:
	BucketItem(int id, int content);

	virtual bool TestUse(Level *level, shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);

	// TU9
	bool emptyBucket(Level *level, double x, double y, double z, int xt, int yt, int zt);

	/*
	 * public boolean useOn(ItemInstance instance, Player player, Level level,
	 * int x, int y, int z, int face) { if (content == 0) { } else { if (face ==
	 * 0) y--; if (face == 1) y++; if (face == 2) z--; if (face == 3) z++; if
	 * (face == 4) x--; if (face == 5) x++; int targetType = level.getTile(x, y,
	 * z); if (targetType == 0) { level.setTile(x, y, z, content); }
	 * player->inventory.items[player->inventory.selected] = new
	 * ItemInstance(Item.bucket_empty); } return true; }
	 */
};