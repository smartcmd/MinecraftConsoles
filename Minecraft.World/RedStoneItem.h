#pragma once
using namespace std;

#include "Item.h"

 class RedStoneItem : public Item 
 {
 public:
	 RedStoneItem(int id);

 public:
	 virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
};