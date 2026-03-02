#pragma once

#include "Item.h"

class ArmorItem : public Item
{
public:
	static const int SLOT_HEAD = 0;
	static const int SLOT_TORSO = 1;
	static const int SLOT_LEGS = 2;
	static const int SLOT_FEET = 3;
	static const eMinecraftColour DEFAULT_LEATHER_COLOR = eMinecraftColour_Armour_Default_Leather_Colour;

private:
	static const int healthPerSlot[];
	static const wstring LEATHER_OVERLAYS[] ;

public:
	static const wstring TEXTURE_EMPTY_SLOTS[];

public:
	class ArmorMaterial
	{
	public:
		static const int clothArray[];
		static const int chainArray[];
		static const int ironArray[];
		static const int goldArray[];
		static const int diamondArray[];
	public:
		static const ArmorMaterial *CLOTH;
		static const ArmorMaterial *CHAIN;
		static const ArmorMaterial *IRON;
		static const ArmorMaterial *GOLD;
		static const ArmorMaterial *DIAMOND;

	private:
		int durabilityMultiplier;
		int *slotProtections;
		int enchantmentValue;

		// 4J Stu - Had to make this public but was private
		// We shouldn't be creating these except the static initialisation
	public:
		ArmorMaterial(int durabilityMultiplier, const int slotProtections[], int enchantmentValue);
		~ArmorMaterial();

	public:
		int getHealthForSlot(int slot) const;
		int getDefenseForSlot(int slot) const;
		int getEnchantmentValue() const;
		int getTierItemId() const;
	};

	const int slot;
	const int defense;
	const int modelIndex;

private:
	const ArmorMaterial *armorType;
	Icon *overlayIcon;
	Icon *iconEmpty;

public:
	ArmorItem(int id, const ArmorMaterial *armorType, int icon, int slot);

	//@Override
	int getColor(shared_ptr<ItemInstance> item, int spriteLayer);

	//@Override
	bool hasMultipleSpriteLayers();

	virtual int getEnchantmentValue();

	const ArmorMaterial *getMaterial();
	bool hasCustomColor(shared_ptr<ItemInstance> item);
	int getColor(shared_ptr<ItemInstance> item);

	//@Override
	Icon *getLayerIcon(int auxValue, int spriteLayer);
	void clearColor(shared_ptr<ItemInstance> item);
	void setColor(shared_ptr<ItemInstance> item, int color);

	bool isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem);

	//@Override
	void registerIcons(IconRegister *iconRegister);

	static Icon *getEmptyIcon(int slot);
};