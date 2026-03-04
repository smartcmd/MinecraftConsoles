#include "stdafx.h"
#include "net.minecraft.world.inventory.h"
#include "EnchantmentContainer.h"

EnchantmentContainer::EnchantmentContainer(EnchantmentMenu *menu) : SimpleContainer(IDS_ENCHANT, L"", false, 1), m_menu( menu )
{
}

int EnchantmentContainer::getMaxStackSize() const
{
	return 1;
}

void EnchantmentContainer::setChanged()
{
	SimpleContainer::setChanged();
	m_menu->slotsChanged(); // Remove this param as it's not needed
}

bool EnchantmentContainer::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}