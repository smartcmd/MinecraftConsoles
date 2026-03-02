#pragma once
using namespace std;

#include "ComplexItem.h"

class MapItemSavedData;

class MapItem : public ComplexItem 
{
public:
	static const int IMAGE_WIDTH = 128;
	static const int IMAGE_HEIGHT = 128;

public: // 4J Stu - Was protected in Java, but then we can't access it where we need it
	MapItem(int id);

	static shared_ptr<MapItemSavedData> getSavedData(short idNum, Level *level);
	shared_ptr<MapItemSavedData> getSavedData(shared_ptr<ItemInstance> itemInstance, Level *level);
	void update(Level *level, shared_ptr<Entity> player, shared_ptr<MapItemSavedData> data);
	virtual void inventoryTick(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Entity> owner, int slot, bool selected);
	virtual void onCraftedBy(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	shared_ptr<Packet> getUpdatePacket(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};
