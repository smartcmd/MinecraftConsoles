#pragma once
// 4J Stu Added
// In EnchantmentMenu.java they create an anoymous class while creating some slot. I have moved the content
// of that anonymous class to here

#include "Slot.h"

class Container;

class EnchantmentSlot : public Slot
{
public:
	EnchantmentSlot(shared_ptr<Container> container, int id, int x, int y) : Slot(container,id, x, y) {}
	virtual bool mayPlace(shared_ptr<ItemInstance> item) {return true;}
	virtual bool mayCombine(shared_ptr<ItemInstance> item) {return false;} // 4J Added
};