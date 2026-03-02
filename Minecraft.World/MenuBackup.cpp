#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "AbstractContainerMenu.h"
#include "Slot.h"
#include "MenuBackup.h"

MenuBackup::MenuBackup(shared_ptr<Inventory> inventory, AbstractContainerMenu *menu)
{
	backups = new unordered_map<short, ItemInstanceArray *>();

	this->inventory = inventory;
	this->menu = menu;
}

void MenuBackup::save(short changeUid)
{
	ItemInstanceArray *backup = new ItemInstanceArray( (int)menu->slots->size() + 1 );
	(*backup)[0] = ItemInstance::clone(inventory->getCarried());
	for (unsigned int i = 0; i < menu->slots->size(); i++)
	{
		(*backup)[i + 1] = ItemInstance::clone(menu->slots->at(i)->getItem());
	}
	// TODO Is unordered_map use correct?
	// Was backups.put(changeUid, backup);
	(*backups)[changeUid] = backup;
}

// Cannot use delete as function name as it is a reserved keyword
void MenuBackup::deleteBackup(short changeUid)
{
	// TODO Is the unordered_map use correct?
	// 4J Was backups.remove(changeUid);
	backups->erase(changeUid);
}

void MenuBackup::rollback(short changeUid)
{
	ItemInstanceArray *backup = backups->at(changeUid);
	backups->clear();
	inventory->setCarried( (*backup)[0] );
	for (unsigned int i = 0; i < menu->slots->size(); i++)
	{
		menu->slots->at(i)->set( (*backup)[i + 1] );
	}

}