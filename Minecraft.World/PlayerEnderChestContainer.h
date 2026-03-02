#pragma once

#include "SimpleContainer.h"

class EnderChestTileEntity;

class PlayerEnderChestContainer : public SimpleContainer
{
private:
	shared_ptr<EnderChestTileEntity> activeChest;

public:
	PlayerEnderChestContainer();

	void setActiveChest(shared_ptr<EnderChestTileEntity> activeChest);
	void setItemsByTag(ListTag<CompoundTag> *enderItemsList);
	ListTag<CompoundTag> *createTag();
	bool stillValid(shared_ptr<Player> player);
	void startOpen();
	void stopOpen();
};