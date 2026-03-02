#pragma once

#include "SimpleContainer.h"

class AnvilMenu;

class RepairContainer : public SimpleContainer, public enable_shared_from_this<RepairContainer>
{
private:
	AnvilMenu *m_menu;

public:
	RepairContainer(AnvilMenu *menu, int name, bool customName, int size);
	void setChanged();
	bool canPlaceItem(int slot, shared_ptr<ItemInstance> item);
};