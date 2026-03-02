#pragma once

#include "Item.h"

class FireworksChargeItem : public Item
{
private:
	Icon *overlay;

public:
	FireworksChargeItem(int id);

	virtual Icon *getLayerIcon(int auxValue, int spriteLayer);
	virtual int getColor(shared_ptr<ItemInstance> item, int spriteLayer);
	virtual bool hasMultipleSpriteLayers();

	static Tag *getExplosionTagField(shared_ptr<ItemInstance> instance, const wstring &field);

	virtual void appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced);

	static void appendHoverText(CompoundTag *expTag, vector<HtmlString> *lines);

	virtual void registerIcons(IconRegister *iconRegister);
};