#pragma once

#include "ItemStack.h"

ref class Location;

public ref class Item
{
public:
	Item(Location^ location, ItemStack^ itemStack)
		: m_location(location), m_itemStack(itemStack)
	{
	}

	Location^ getLocation() { return m_location; }
	ItemStack^ getItemStack() { return m_itemStack; }

private:
	Location^ m_location;
	ItemStack^ m_itemStack;
};