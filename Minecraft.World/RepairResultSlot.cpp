#include "stdafx.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.player.h"
#include "RepairResultSlot.h"

RepairResultSlot::RepairResultSlot(RepairMenu *menu, int xt, int yt, int zt, shared_ptr<Container> container, int slot, int x, int y) : Slot(container, slot, x, y)
{
	m_menu = menu;
	this->xt = xt;
	this->yt = yt;
	this->zt = zt;
}

bool RepairResultSlot::mayPlace(shared_ptr<ItemInstance> item)
{
	return false;
}

bool RepairResultSlot::mayPickup(shared_ptr<Player> player)
{
	return (player->abilities.instabuild || player->experienceLevel >= m_menu->cost) && (m_menu->cost > 0 && hasItem());
}

void RepairResultSlot::onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried)
{
	if (!player->abilities.instabuild) player->withdrawExperienceLevels(m_menu->cost);
	m_menu->repairSlots->setItem(RepairMenu::INPUT_SLOT, nullptr);
	if (m_menu->repairItemCountCost > 0)
	{
		shared_ptr<ItemInstance> addition = m_menu->repairSlots->getItem(RepairMenu::ADDITIONAL_SLOT);
		if (addition != NULL && addition->count > m_menu->repairItemCountCost)
		{
			addition->count -= m_menu->repairItemCountCost;
			m_menu->repairSlots->setItem(RepairMenu::ADDITIONAL_SLOT, addition);
		}
		else
		{
			m_menu->repairSlots->setItem(RepairMenu::ADDITIONAL_SLOT, nullptr);
		}
	}
	else
	{
		m_menu->repairSlots->setItem(RepairMenu::ADDITIONAL_SLOT, nullptr);
	}
	m_menu->cost = 0;

	if (!player->abilities.instabuild && !m_menu->level->isClientSide && m_menu->level->getTile(xt, yt, zt) == Tile::anvil->id && player->getRandom()->nextFloat() < 0.12f)
	{
		int data = m_menu->level->getData(xt, yt, zt);
		int dir = data & 0x3;
		int dmg = data >> 2;

		if (++dmg > 2)
		{
			m_menu->level->setTile(xt, yt, zt, 0);
			m_menu->level->levelEvent(LevelEvent::SOUND_ANVIL_BROKEN, xt, yt, zt, 0);
		}
		else
		{
			m_menu->level->setData(xt, yt, zt, dir | (dmg << 2));
			m_menu->level->levelEvent(LevelEvent::SOUND_ANVIL_USED, xt, yt, zt, 0);
		}
	}
	else if (!m_menu->level->isClientSide)
	{
		m_menu->level->levelEvent(LevelEvent::SOUND_ANVIL_USED, xt, yt, zt, 0);
	}
}

bool RepairResultSlot::mayCombine(shared_ptr<ItemInstance> second)
{
	return false;
}