#pragma once
// 4J Stu Added
// In EnchantmentMenu.java they create an anoymous class while creating the container. I have moved the content
// of that anonymous class to here

#include "SimpleContainer.h"

class EnchantmentMenu;

class EnchantmentContainer : public SimpleContainer
{
private:
	EnchantmentMenu *m_menu;
public:
	EnchantmentContainer(EnchantmentMenu *menu);
	virtual int getMaxStackSize() const;
	virtual void setChanged();
	virtual bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
};