#pragma once
using namespace std;

#include "UseAnim.h"
#include "com.mojang.nbt.h"
#include "Attribute.h"

class Entity;
class Level;
class Player;
class Mob;
class LivingEntity;
class CompoundTag;
class Enchantment;
class Rarity;
class AttributeModifier;
class Random;
// 4J-PB - added
class MapItem;
class ItemFrame;
class Icon;
class HtmlString;

// 4J Stu - While this is not really an abstract class, we don't want to make new instances of it,
// mainly because there are too many ctors and that doesn't fit well into out macroisation setup
class ItemInstance: public enable_shared_from_this<ItemInstance>
{
public:
	static const wstring ATTRIBUTE_MODIFIER_FORMAT;
	static const wchar_t *TAG_ENCH_ID;
	static const wchar_t *TAG_ENCH_LEVEL;

	int count;
	int popTime;
	int id;

	// 4J Stu - Brought forward for enchanting/game rules
	CompoundTag *tag;

	/**
	 * This was previously the damage value, but is now used for different stuff
	 * depending on item / tile. Use the getter methods to make sure the value
	 * is interpreted correctly.
	 */
private:
	int auxValue;
	// 4J-PB - added for trading menu
	bool m_bForceNumberDisplay;

	void _init(int id, int count, int auxValue);

	// TU9
	shared_ptr<ItemFrame> frame;

public:
	ItemInstance(Tile *tile);
	ItemInstance(Tile *tile, int count);
	ItemInstance(Tile *tile, int count, int auxValue);
	ItemInstance(Item *item);
	// 4J-PB - added
	ItemInstance(MapItem *item, int count);

	ItemInstance(Item *item, int count);
	ItemInstance(Item *item, int count, int auxValue);
	ItemInstance(int id, int count, int damage);

	static shared_ptr<ItemInstance> fromTag(CompoundTag *itemTag);
private:
	ItemInstance()	{ _init(-1,0,0); }

public:
	~ItemInstance();
	shared_ptr<ItemInstance> remove(int count);

	Item *getItem() const;
	Icon *getIcon();
	int getIconType();
	bool useOn(shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	float getDestroySpeed(Tile *tile);
	bool TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	shared_ptr<ItemInstance> use(Level *level, shared_ptr<Player> player);
	shared_ptr<ItemInstance> useTimeDepleted(Level *level, shared_ptr<Player> player);
	CompoundTag *save(CompoundTag *compoundTag);
	void load(CompoundTag *compoundTag);
	int getMaxStackSize() const;
	bool isStackable();
	bool isDamageableItem();
	bool isStackedByData();
	bool isDamaged();
	int getDamageValue();
	int getAuxValue() const;
	void setAuxValue(int value);
	int getMaxDamage();
	bool hurt(int dmg, Random *random);
	void hurtAndBreak(int dmg, shared_ptr<LivingEntity> owner);
	void hurtEnemy(shared_ptr<LivingEntity> mob, shared_ptr<Player> attacker);
	void mineBlock(Level *level, int tile, int x, int y, int z, shared_ptr<Player> owner);
	bool canDestroySpecial(Tile *tile);
	bool interactEnemy(shared_ptr<Player> player, shared_ptr<LivingEntity> mob);
	shared_ptr<ItemInstance> copy() const;
	ItemInstance *copy_not_shared() const; // 4J Stu - Added for use in recipes
	static bool tagMatches(shared_ptr<ItemInstance> a, shared_ptr<ItemInstance> b); // 4J Brought forward from 1.2
	static bool matches(shared_ptr<ItemInstance> a, shared_ptr<ItemInstance> b);

	// 4J-PB
	int GetCount() {return count;}
	void ForceNumberDisplay(bool bForce) {m_bForceNumberDisplay=bForce;} // to force the display of 0 and 1 on the required trading items when you have o or 1 of the item
	bool GetForceNumberDisplay() {return m_bForceNumberDisplay;} // to force the display of 0 and 1 on the required trading items when you have o or 1 of the item

private:
	bool matches(shared_ptr<ItemInstance> b);

public:
	bool sameItem(shared_ptr<ItemInstance> b);
	bool sameItemWithTags(shared_ptr<ItemInstance> b); //4J Added
	bool sameItem_not_shared(const ItemInstance *b); // 4J Stu - Added this for the one time I need it
	virtual unsigned int getUseDescriptionId(); // 4J Added
	virtual unsigned int getDescriptionId(int iData = -1);
	virtual ItemInstance *setDescriptionId(unsigned int id);
	static shared_ptr<ItemInstance> clone(shared_ptr<ItemInstance> item);
	wstring toString();
	void inventoryTick(Level *level, shared_ptr<Entity> owner, int slot, bool selected);
	void onCraftedBy(Level *level, shared_ptr<Player> player, int craftCount);
	bool equals(shared_ptr<ItemInstance> ii);

	int getUseDuration();
	UseAnim getUseAnimation();
	void releaseUsing(Level *level, shared_ptr<Player> player, int durationLeft);

	// 4J Stu - Brought forward these functions for enchanting/game rules
	bool hasTag();
	CompoundTag *getTag();
	ListTag<CompoundTag> *getEnchantmentTags();
	void setTag(CompoundTag *tag);
	wstring getHoverName();
	void setHoverName(const wstring &name);
	void resetHoverName();
	bool hasCustomHoverName();
	vector<HtmlString> *getHoverText(shared_ptr<Player> player, bool advanced);
	vector<HtmlString> *getHoverTextOnly(shared_ptr<Player> player, bool advanced); // 4J Added
	bool isFoil();
	const Rarity *getRarity();
	bool isEnchantable();
	void enchant(const Enchantment *enchantment, int level);
	bool isEnchanted();
	void addTagElement(wstring name, Tag *tag);
	bool mayBePlacedInAdventureMode();
	bool isFramed();
	void setFramed(shared_ptr<ItemFrame> frame);
	shared_ptr<ItemFrame> getFrame();
	int getBaseRepairCost();
	void setRepairCost(int cost);
	attrAttrModMap *getAttributeModifiers();

	// 4J Added
	void set4JData(int data);
	int get4JData();
	bool hasPotionStrengthBar();
	int GetPotionStrength();
};