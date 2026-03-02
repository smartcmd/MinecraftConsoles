#pragma once

#include "Item.h"

class Player;
class Level;

class BoatItem : public Item
{
public:

	BoatItem(int id);

	virtual bool TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);

	/*
	 * public boolean useOn(ItemInstance instance, Player player, Level level,
	 * int x, int y, int z, int face) { // if (face != 1) return false; int
	 * targetType = level.getTile(x, y, z); if (targetType == Tile.rail.id) {
	 * level.addEntity(new Minecart(level, x + 0.5f, y + 0.5f, z + 0.5f));
	 * instance.count--; return true; } return false; }
	 */
};