#pragma once
using namespace std;

#include "Item.h"

class Player;
class Packet;
class Level;

class ComplexItem : public Item
{
protected:
	ComplexItem(int id);

public:
	virtual bool isComplex();
	virtual shared_ptr<Packet> getUpdatePacket(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
};
