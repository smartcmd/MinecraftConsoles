#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "Slot.h"
#include "AbstractContainerMenu.h"

// 4J Stu - The java does not have ctor here (being an abstract) but we need one to initialise the member variables
// TODO Make sure all derived classes also call this
AbstractContainerMenu::AbstractContainerMenu()
{
	lastSlots = new vector<shared_ptr<ItemInstance> >();
	slots = new vector<Slot *>();
	containerId = 0;

	changeUid = 0;
	m_bNeedsRendered = false;

	containerListeners = new vector<ContainerListener *>();
}

AbstractContainerMenu::~AbstractContainerMenu()
{
	delete lastSlots;
	for( unsigned int i = 0; i < slots->size(); i++ )
	{
		delete slots->at(i);
	}
	delete slots;
	delete containerListeners;
}

Slot *AbstractContainerMenu::addSlot(Slot *slot)
{
	slot->index = (int)slots->size();
	slots->push_back(slot);
	lastSlots->push_back(nullptr);
	return slot;
}


void AbstractContainerMenu::addSlotListener(ContainerListener *listener)
{
	// TODO 4J Add exceptions
	/*
	if (containerListeners->contains(listener)) {
	throw new IllegalArgumentException("Listener already listening");
	}
	*/
	containerListeners->push_back(listener);


	vector<shared_ptr<ItemInstance> > *items = getItems();
	listener->refreshContainer(this, items);
	delete items;
	broadcastChanges();
}

vector<shared_ptr<ItemInstance> > *AbstractContainerMenu::getItems()
{
	vector<shared_ptr<ItemInstance> > *items = new vector<shared_ptr<ItemInstance> >();
	AUTO_VAR(itEnd, slots->end());
	for (AUTO_VAR(it, slots->begin()); it != itEnd; it++)
	{
		items->push_back((*it)->getItem());
	}
	return items;
}

void AbstractContainerMenu::sendData(int id, int value)
{
	AUTO_VAR(itEnd, containerListeners->end());
	for (AUTO_VAR(it, containerListeners->begin()); it != itEnd; it++)
	{
		(*it)->setContainerData(this, id, value);
	}
}

void AbstractContainerMenu::broadcastChanges()
{
	for (unsigned int i = 0; i < slots->size(); i++)
	{
		shared_ptr<ItemInstance> current = slots->at(i)->getItem();
		shared_ptr<ItemInstance> expected = lastSlots->at(i);
		if (!ItemInstance::matches(expected, current))
		{
			expected = current == NULL ? nullptr : current->copy();
			(*lastSlots)[i] = expected;
			m_bNeedsRendered = true;

			AUTO_VAR(itEnd, containerListeners->end());
			for (AUTO_VAR(it, containerListeners->begin()); it != itEnd; it++)
			{
				(*it)->slotChanged(this, i, expected);
			}
		}
	}
}

bool AbstractContainerMenu::needsRendered()
{
	bool needsRendered = m_bNeedsRendered;
	m_bNeedsRendered = false;

	for (unsigned int i = 0; i < slots->size(); i++)
	{
		shared_ptr<ItemInstance> current = slots->at(i)->getItem();
		shared_ptr<ItemInstance> expected = lastSlots->at(i);
		if (!ItemInstance::matches(expected, current))
		{
			expected = current == NULL ? nullptr : current->copy();
			(*lastSlots)[i] = expected;
			needsRendered = true;
		}
	}

	return needsRendered;
}

bool AbstractContainerMenu::clickMenuButton(shared_ptr<Player> player, int buttonId)
{
	return false;
}

Slot *AbstractContainerMenu::getSlotFor(shared_ptr<Container> c, int index)
{
	AUTO_VAR(itEnd, slots->end());
	for (AUTO_VAR(it, slots->begin()); it != itEnd; it++)
	{
		Slot *slot = *it; //slots->at(i);
		if (slot->isAt(c, index))
		{
			return slot;
		}
	}
	return NULL;
}

Slot *AbstractContainerMenu::getSlot(int index)
{
	return slots->at(index);
}

shared_ptr<ItemInstance> AbstractContainerMenu::quickMoveStack(shared_ptr<Player> player, int slotIndex)
{
	Slot *slot = slots->at(slotIndex);
	if (slot != NULL)
	{
		return slot->getItem();
	}
	return nullptr;
}

shared_ptr<ItemInstance> AbstractContainerMenu::clicked(int slotIndex, int buttonNum, int clickType, shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> clickedEntity = nullptr;
	shared_ptr<Inventory> inventory = player->inventory;

	if ((clickType == CLICK_PICKUP || clickType == CLICK_QUICK_MOVE) && (buttonNum == 0 || buttonNum == 1))
	{
		if (slotIndex == CLICKED_OUTSIDE)
		{
			if (inventory->getCarried() != NULL)
			{
				if (slotIndex == CLICKED_OUTSIDE)
				{
					if (buttonNum == 0)
					{
						player->drop(inventory->getCarried());
						inventory->setCarried(nullptr);
					}
					if (buttonNum == 1)
					{
						player->drop(inventory->getCarried()->remove(1));
						if (inventory->getCarried()->count == 0) inventory->setCarried(nullptr);
					}

				}
			}
		}
		else if (clickType == CLICK_QUICK_MOVE)
		{
			Slot *slot = slots->at(slotIndex);
			if(slot != NULL && slot->mayPickup(player))
			{
				shared_ptr<ItemInstance> piiClicked = quickMoveStack(player, slotIndex);
				if (piiClicked != NULL)
				{
					//int oldSize = piiClicked->count; // 4J - Commented 1.8.2 and replaced with below
					int oldType = piiClicked->id;

					clickedEntity = piiClicked->copy();

					if (slot != NULL)
					{
						if (slot->getItem() != NULL && slot->getItem()->id == oldType)
						{
							// 4J Stu - Brought forward loopClick from 1.2 to fix infinite recursion bug in creative
							loopClick(slotIndex, buttonNum, true, player);
						}
					}
				}
			}
		}
		else
		{
			if (slotIndex < 0) return nullptr;

			Slot *slot = slots->at(slotIndex);
			if (slot != NULL)
			{
				shared_ptr<ItemInstance> clicked = slot->getItem();
				shared_ptr<ItemInstance> carried = inventory->getCarried();

				if (clicked != NULL)
				{
					clickedEntity = clicked->copy();
				}

				if (clicked == NULL)
				{
					if (carried != NULL && slot->mayPlace(carried))
					{
						int c = buttonNum == 0 ? carried->count : 1;
						if (c > slot->getMaxStackSize())
						{
							c = slot->getMaxStackSize();
						}
						slot->set(carried->remove(c));
						if (carried->count == 0)
						{
							inventory->setCarried(nullptr);
						}
					}
				}
				// 4J Added for dyable armour and combinining damaged items
				else if (buttonNum == 1 && mayCombine(slot, carried))
				{
					shared_ptr<ItemInstance> combined = slot->combine(carried);
					if(combined != NULL)
					{
						slot->set(combined);
						if(!player->abilities.instabuild) carried->remove(1);
						if (carried->count == 0)
						{
							inventory->setCarried(nullptr);
						}
					}
				}
				else if (slot->mayPickup(player))
				{
					if (carried == NULL)
					{
						// pick up to empty hand
						int c = buttonNum == 0 ? clicked->count : (clicked->count + 1) / 2;
						shared_ptr<ItemInstance> removed = slot->remove(c);

						inventory->setCarried(removed);
						if (clicked->count == 0)
						{
							slot->set(nullptr);
						}
						slot->onTake(player, inventory->getCarried());
					}
					else if (slot->mayPlace(carried))
					{
						// put down and/or pick up
						if (clicked->id != carried->id || clicked->getAuxValue() != carried->getAuxValue() || !ItemInstance::tagMatches(clicked, carried))
						{
							// no match, replace
							if (carried->count <= slot->getMaxStackSize())
							{
								slot->set(carried);
								inventory->setCarried(clicked);
							}
						}
						else
						{
							// match, attempt to fill slot
							int c = buttonNum == 0 ? carried->count : 1;
							if (c > slot->getMaxStackSize() - clicked->count)
							{
								c = slot->getMaxStackSize() - clicked->count;
							}
							if (c > carried->getMaxStackSize() - clicked->count)
							{
								c = carried->getMaxStackSize() - clicked->count;
							}
							carried->remove(c);
							if (carried->count == 0)
							{
								inventory->setCarried(nullptr);
							}
							clicked->count += c;
						}
					}
					else
					{
						// pick up to non-empty hand
						if (clicked->id == carried->id && carried->getMaxStackSize() > 1 && (!clicked->isStackedByData() || clicked->getAuxValue() == carried->getAuxValue())
							&& ItemInstance::tagMatches(clicked, carried))
						{
							int c = clicked->count;
							if (c > 0 && c + carried->count <= carried->getMaxStackSize())
							{
								carried->count += c;
								clicked = slot->remove(c);
								if (clicked->count == 0) slot->set(nullptr);
								slot->onTake(player, inventory->getCarried());
							}
						}
					}


				}
				slot->setChanged();
			}
		}
	}
	else if (clickType == CLICK_SWAP && buttonNum >= 0 && buttonNum < 9)
	{
		Slot *slot = slots->at(slotIndex);
		if (slot->mayPickup(player))
		{
			shared_ptr<ItemInstance> current = inventory->getItem(buttonNum);
			bool canMove = current == NULL || (slot->container == inventory && slot->mayPlace(current));
			int freeSlot = -1;

			if (!canMove)
			{
				freeSlot = inventory->getFreeSlot();
				canMove |= freeSlot > -1;
			}

			if (slot->hasItem() && canMove)
			{
				shared_ptr<ItemInstance> taking = slot->getItem();
				inventory->setItem(buttonNum, taking);

				if ((slot->container == inventory && slot->mayPlace(current)) || current == NULL)
				{
					slot->remove(taking->count);
					slot->set(current);
					slot->onTake(player, taking);
				}
				else if (freeSlot > -1)
				{
					inventory->add(current);
					slot->remove(taking->count);
					slot->set(nullptr);
					slot->onTake(player, taking);
				}
			}
			else if (!slot->hasItem() && current != NULL && slot->mayPlace(current))
			{
				inventory->setItem(buttonNum, nullptr);
				slot->set(current);
			}
		}
	}
	else if (clickType == CLICK_CLONE && player->abilities.instabuild && inventory->getCarried() == NULL && slotIndex >= 0)
	{
		Slot *slot = slots->at(slotIndex);
		if (slot != NULL && slot->hasItem())
		{
			shared_ptr<ItemInstance> copy = slot->getItem()->copy();
			copy->count = copy->getMaxStackSize();
			inventory->setCarried(copy);
		}
	}
	return clickedEntity;
}

// 4J Stu - Brought forward from 1.2 to fix infinite recursion bug in creative
void AbstractContainerMenu::loopClick(int slotIndex, int buttonNum, bool quickKeyHeld, shared_ptr<Player> player)
{
	clicked(slotIndex, buttonNum, CLICK_QUICK_MOVE, player);
}

bool AbstractContainerMenu::mayCombine(Slot *slot, shared_ptr<ItemInstance> item)
{
	return false;
}

void AbstractContainerMenu::removed(shared_ptr<Player> player)
{
	shared_ptr<Inventory> inventory = player->inventory;
	if (inventory->getCarried() != NULL)
	{
		player->drop(inventory->getCarried());
		inventory->setCarried(nullptr);
	}
}

void AbstractContainerMenu::slotsChanged()// 4J used to take a shared_ptr<Container> but wasn't using it, so removed to simplify things
{
	broadcastChanges();
}

bool AbstractContainerMenu::isPauseScreen()
{
	return false;
}

void AbstractContainerMenu::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	getSlot(slot)->set(item);
}

void AbstractContainerMenu::setAll(ItemInstanceArray *items)
{
	for (unsigned int i = 0; i < items->length; i++) 
	{
		getSlot(i)->set( (*items)[i] );
	}
}

void AbstractContainerMenu::setData(int id, int value)
{
}

short AbstractContainerMenu::backup(shared_ptr<Inventory> inventory)
{
	changeUid++;
	return changeUid;
}

bool AbstractContainerMenu::isSynched(shared_ptr<Player> player)
{
	return !(unSynchedPlayers.find(player) != unSynchedPlayers.end());
}

void AbstractContainerMenu::setSynched(shared_ptr<Player> player, bool synched)
{
	if (synched)
	{
		AUTO_VAR(it, unSynchedPlayers.find(player));

		if(it != unSynchedPlayers.end()) unSynchedPlayers.erase( it );
	}
	else
	{
		unSynchedPlayers.insert(player);
	}
}

// 4J Stu - Brought a few changes in this function forward from 1.2 to make it return a bool
bool AbstractContainerMenu::moveItemStackTo(shared_ptr<ItemInstance> itemStack, int startSlot, int endSlot, bool backwards)
{
	bool anythingChanged = false;

	int destSlot = startSlot;
	if (backwards)
	{
		destSlot = endSlot - 1;
	}

	// find stackable slots first
	if (itemStack->isStackable())
	{
		while (itemStack->count > 0 && ((!backwards && destSlot < endSlot) || (backwards && destSlot >= startSlot)))
		{

			Slot *slot = slots->at(destSlot);
			shared_ptr<ItemInstance> target = slot->getItem();
			if (target != NULL && target->id == itemStack->id && (!itemStack->isStackedByData() || itemStack->getAuxValue() == target->getAuxValue())
				&& ItemInstance::tagMatches(itemStack, target) )
			{
				int totalStack = target->count + itemStack->count;
				if (totalStack <= itemStack->getMaxStackSize())
				{
					itemStack->count = 0;
					target->count = totalStack;
					slot->setChanged();
					anythingChanged = true;
				}
				else if (target->count < itemStack->getMaxStackSize())
				{
					itemStack->count -= (itemStack->getMaxStackSize() - target->count);
					target->count = itemStack->getMaxStackSize();
					slot->setChanged();
					anythingChanged = true;
				}
			}

			if (backwards)
			{
				destSlot--;
			}
			else
			{
				destSlot++;
			}
		}
	}

	// find empty slot
	if (itemStack->count > 0)
	{
		if (backwards)
		{
			destSlot = endSlot - 1;
		}
		else
		{
			destSlot = startSlot;
		}
		while ((!backwards && destSlot < endSlot) || (backwards && destSlot >= startSlot))
		{
			Slot *slot = slots->at(destSlot);
			shared_ptr<ItemInstance> target = slot->getItem();

			if (target == NULL)
			{
				slot->set(itemStack->copy());
				slot->setChanged();
				itemStack->count = 0;
				anythingChanged = true;
				break;
			}

			if (backwards)
			{
				destSlot--;
			}
			else
			{
				destSlot++;
			}
		}
	}
	return anythingChanged;
}

bool AbstractContainerMenu::isOverrideResultClick(int slotNum, int buttonNum)
{
	return false;
}
