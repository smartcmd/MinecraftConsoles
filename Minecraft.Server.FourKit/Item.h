#pragma once

#include "ItemStack.h"

ref class Location;

public ref class Item
{
public:
	Item(Location^ location, ItemStack^ itemStack)
		: m_location(location), m_itemStack(itemStack), m_pickupDelay(0)
	{
	}

	Location^ getLocation() { return m_location; }
	ItemStack^ getItemStack() { return m_itemStack; }
	void setItemStack(ItemStack^ stack) { m_itemStack = stack; }
	int getPickupDelay() { return m_pickupDelay; }
	void setPickupDelay(int delay) { m_pickupDelay = delay; }

private:
	Location^ m_location;
	ItemStack^ m_itemStack;
	int m_pickupDelay;
};