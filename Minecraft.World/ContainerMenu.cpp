#include "stdafx.h"
#include "Container.h"
#include "net.minecraft.world.item.h"
#include "Slot.h"
#include "GenericStats.h"
#include "../Minecraft.Client/StatsCounter.h"
#include "../Minecraft.Client/Minecraft.h"
#include "../Minecraft.Client/LocalPlayer.h"
#include "ContainerMenu.h"

ContainerMenu::ContainerMenu(shared_ptr<Container> inventory, shared_ptr<Container> container) : AbstractContainerMenu()
{
	this->container = container;
	containerRows = container->getContainerSize() / 9;
	container->startOpen();

	int yo = (containerRows - 4) * 18;

	for (int y = 0; y < containerRows; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			addSlot(new Slot(container, x + y * 9, 8 + x * 18, 18 + y * 18));
		}
	}

	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			addSlot(new Slot(inventory, x + y * 9 + 9, 8 + x * 18, 103 + y * 18 + yo));
		}
	}
	for (int x = 0; x < 9; x++)
	{
		addSlot(new Slot(inventory, x, 8 + x * 18, 161 + yo));
	}
}

bool ContainerMenu::stillValid(shared_ptr<Player> player)
{
	return container->stillValid(player);
}

shared_ptr<ItemInstance> ContainerMenu::quickMoveStack(shared_ptr<Player> player, int slotIndex)
{
	shared_ptr<ItemInstance> clicked = nullptr;
	Slot *slot = slots.at(slotIndex);
	if (slot != nullptr && slot->hasItem())
	{
		shared_ptr<ItemInstance> stack = slot->getItem();
		clicked = stack->copy();

		if (slotIndex < containerRows * 9)
		{
			if(!moveItemStackTo(stack, containerRows * 9, static_cast<int>(slots.size()), true))
			{
				// 4J Stu - Brought forward from 1.2
				return nullptr;
			}
		}
		else
		{
			if(!moveItemStackTo(stack, 0, containerRows * 9, false))
			{
				// 4J Stu - Brought forward from 1.2
				return nullptr;
			}
		}
		if (stack->count == 0)
		{
			slot->set(nullptr);
		}
		else
		{
			slot->setChanged();
		}
	}
	return clicked;
}

void ContainerMenu::removed(shared_ptr<Player> player)
{
	AbstractContainerMenu::removed(player);
	container->stopOpen();
}

shared_ptr<Container> ContainerMenu::getContainer()
{
	return container;
}

shared_ptr<ItemInstance> ContainerMenu::clicked(int slotIndex, int buttonNum, int clickType, shared_ptr<Player> player, bool looped) // 4J Added looped param
{
	shared_ptr<ItemInstance> out = AbstractContainerMenu::clicked(slotIndex, buttonNum, clickType, player, looped);

#ifdef _EXTENDED_ACHIEVEMENTS
	shared_ptr<LocalPlayer> localPlayer = dynamic_pointer_cast<LocalPlayer>(player);

	if (localPlayer != nullptr)	// 4J-JEV: For "Chestful o'Cobblestone" achievement.
	{
		int cobblecount = 0;
		for (int i = 0; i < container->getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> item = container->getItem(i);
			if ( (item != nullptr) && (item->id == Tile::cobblestone_Id) )
			{
				cobblecount += item->GetCount();
			}
		}

		// 4J-JEV: This check performed on XboxOne servers, for other platforms check here.
#ifndef _DURANGO
		StatsCounter *sc = Minecraft::GetInstance()->stats[localPlayer->GetXboxPad()];
		int minedCount = sc->getTotalValue(GenericStats::blocksMined(Tile::stone_Id)) + sc->getTotalValue(GenericStats::blocksMined(Tile::cobblestone_Id));
		if (cobblecount >= 1728 && minedCount >= 1728 )
#endif
		{
			localPlayer->awardStat(GenericStats::chestfulOfCobblestone(),GenericStats::param_chestfulOfCobblestone(cobblecount));
		}
	}
#endif

	return out;
}
