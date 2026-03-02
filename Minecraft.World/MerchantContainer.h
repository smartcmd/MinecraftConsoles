#pragma once

#include "Container.h"
#include "ArrayWithLength.h"

class Merchant;
class Player;
class MerchantRecipe;

class MerchantContainer : public Container
{
private:
	shared_ptr<Merchant> merchant;
	ItemInstanceArray items;
	shared_ptr<Player> player;
	MerchantRecipe *activeRecipe;
	int selectionHint;

public:
	MerchantContainer(shared_ptr<Player> player, shared_ptr<Merchant> villager);
	~MerchantContainer();

	unsigned int getContainerSize();
	shared_ptr<ItemInstance> getItem(unsigned int slot);
	shared_ptr<ItemInstance> removeItem(unsigned int slot, int count);

private:
	bool isPaymentSlot(int slot);

public:
	shared_ptr<ItemInstance> removeItemNoUpdate(int slot);
	void setItem(unsigned int slot, shared_ptr<ItemInstance> item);
	int getName();
	int getMaxStackSize();
	bool stillValid(shared_ptr<Player> player);
	void startOpen();
	void stopOpen();
	void setChanged();
	void updateSellItem();
	MerchantRecipe *getActiveRecipe();
	void setSelectionHint(int selectionHint);
};