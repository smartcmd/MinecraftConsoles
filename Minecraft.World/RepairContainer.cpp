#include "stdafx.h"
#include "AnvilMenu.h"
#include "RepairContainer.h"

RepairContainer::RepairContainer(AnvilMenu *menu, int name, bool customName, int size) : SimpleContainer(name, L"", customName, size)
{
	m_menu = menu;
}

void RepairContainer::setChanged()
{
	SimpleContainer::setChanged();
	m_menu->slotsChanged(shared_from_this());
}

bool RepairContainer::canPlaceItem(int slot, shared_ptr<ItemInstance> item)
{
	return true;
}