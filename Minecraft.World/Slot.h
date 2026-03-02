#pragma once

class Container;

class Slot
{
private:
	int slot;

public:
	shared_ptr<Container> container;

public:
	int index;
	int x, y;

	Slot(shared_ptr<Container> container, int slot, int x, int y);
	virtual ~Slot() {}

	void onQuickCraft(shared_ptr<ItemInstance> picked, shared_ptr<ItemInstance> original);

protected:
	virtual void onQuickCraft(shared_ptr<ItemInstance> picked, int count);
	virtual void checkTakeAchievements(shared_ptr<ItemInstance> picked);

public:
	void swap(Slot *other);
	virtual void onTake(shared_ptr<Player> player, shared_ptr<ItemInstance> carried);
	virtual bool mayPlace(shared_ptr<ItemInstance> item);
	virtual shared_ptr<ItemInstance> getItem();
	virtual bool hasItem();
	virtual void set(shared_ptr<ItemInstance> item);
	virtual void setChanged();
	virtual int getMaxStackSize();
	virtual Icon *getNoItemIcon();
	virtual shared_ptr<ItemInstance> remove(int c);
	virtual bool isAt(shared_ptr<Container> c, int s);
	virtual bool mayPickup(shared_ptr<Player> player);
	virtual bool mayCombine(shared_ptr<ItemInstance> item); // 4J Added
	virtual shared_ptr<ItemInstance> combine(shared_ptr<ItemInstance> item); // 4J Added
};