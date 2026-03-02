#pragma once

class AbstractContainerMenu;
class Inventory;

class MenuBackup
{
private:
	unordered_map<short, ItemInstanceArray *> *backups;
	shared_ptr<Inventory> inventory;
	AbstractContainerMenu *menu;

public:
	MenuBackup(shared_ptr<Inventory> inventory, AbstractContainerMenu *menu);

	void save(short changeUid);

	// Cannot use delete as function name as it is a reserved keyword
	void deleteBackup(short changeUid);
	void rollback(short changeUid);
};