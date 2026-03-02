#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.locale.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "Item.h"
#include "ItemInstance.h"
#include "HtmlString.h"

const wstring ItemInstance::ATTRIBUTE_MODIFIER_FORMAT = L"#.###";

const wchar_t *ItemInstance::TAG_ENCH_ID = L"id";
const wchar_t *ItemInstance::TAG_ENCH_LEVEL = L"lvl";

void ItemInstance::_init(int id, int count, int auxValue)
{
	this->popTime = 0;
	this->id = id;
	this->count = count;
	this->auxValue = auxValue;
	this->tag = NULL;
	this->frame = nullptr;
	// 4J-PB - for trading menu
	this->m_bForceNumberDisplay=false;
}

ItemInstance::ItemInstance(Tile *tile)
{
	_init(tile->id, 1, 0);
}

ItemInstance::ItemInstance(Tile *tile, int count)
{
	_init(tile->id, count, 0);
}
// 4J-PB - added
ItemInstance::ItemInstance(MapItem *item, int count)
{
	_init(item->id, count, 0);
}

ItemInstance::ItemInstance(Tile *tile, int count, int auxValue)
{
	_init(tile->id, count, auxValue);
}

ItemInstance::ItemInstance(Item *item)
{
	_init(item->id, 1, 0);
}



ItemInstance::ItemInstance(Item *item, int count)
{
	_init(item->id, count, 0);
}

ItemInstance::ItemInstance(Item *item, int count, int auxValue)
{
	_init(item->id, count, auxValue);
}

ItemInstance::ItemInstance(int id, int count, int damage)
{
	_init(id,count,damage);
	if (auxValue < 0)
	{
		auxValue = 0;
	}
}

shared_ptr<ItemInstance> ItemInstance::fromTag(CompoundTag *itemTag)
{
	shared_ptr<ItemInstance> itemInstance = shared_ptr<ItemInstance>(new ItemInstance());
	itemInstance->load(itemTag);
	return itemInstance->getItem() != NULL ? itemInstance : nullptr;
}

ItemInstance::~ItemInstance()
{
	if(tag != NULL) delete tag;
}

shared_ptr<ItemInstance> ItemInstance::remove(int count)
{
	shared_ptr<ItemInstance> ii = shared_ptr<ItemInstance>( new ItemInstance(id, count, auxValue) );
	if (tag != NULL) ii->tag = (CompoundTag *) tag->copy();
	this->count -= count;

	// 4J Stu Fix for duplication glitch, make sure that item count is in range
	if(this->count <= 0)
	{
		this->count = 0;
	}
	return ii;
}

Item *ItemInstance::getItem() const
{
	return Item::items[id];
}

Icon *ItemInstance::getIcon()
{
	return getItem()->getIcon(shared_from_this());
}

int ItemInstance::getIconType()
{
	return getItem()->getIconType();
}

bool ItemInstance::useOn(shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	return getItem()->useOn(shared_from_this(), player, level, x, y, z, face, clickX, clickY, clickZ, bTestUseOnOnly);
}

float ItemInstance::getDestroySpeed(Tile *tile)
{
	return getItem()->getDestroySpeed(shared_from_this(), tile);
}

bool ItemInstance::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return getItem()->TestUse( itemInstance, level, player);
}

shared_ptr<ItemInstance> ItemInstance::use(Level *level, shared_ptr<Player> player)
{
	return getItem()->use(shared_from_this(), level, player);
}

shared_ptr<ItemInstance> ItemInstance::useTimeDepleted(Level *level, shared_ptr<Player> player)
{
	return getItem()->useTimeDepleted(shared_from_this(), level, player);
}

CompoundTag *ItemInstance::save(CompoundTag *compoundTag)
{
	compoundTag->putShort(L"id", (short) id);
	compoundTag->putByte(L"Count", (byte) count);
	compoundTag->putShort(L"Damage", (short) auxValue);
	if (tag != NULL) compoundTag->put(L"tag", tag->copy());
	return compoundTag;
}

void ItemInstance::load(CompoundTag *compoundTag)
{
	popTime = 0;
	id = compoundTag->getShort(L"id");
	count = compoundTag->getByte(L"Count");
	auxValue = compoundTag->getShort(L"Damage");
	if (auxValue < 0)
	{
		auxValue = 0;
	}
	if (compoundTag->contains(L"tag"))
	{
		delete tag;
		tag = (CompoundTag *)compoundTag->getCompound(L"tag")->copy();
	}
}

int ItemInstance::getMaxStackSize() const
{
	return getItem()->getMaxStackSize();
}

bool ItemInstance::isStackable()
{
	return getMaxStackSize() > 1 && (!isDamageableItem() || !isDamaged());
}

bool ItemInstance::isDamageableItem()
{
	return Item::items[id]->getMaxDamage() > 0;
}

/**
* Returns true if this item type only can be stacked with items that have
* the same auxValue data.
*
* @return
*/

bool ItemInstance::isStackedByData()
{
	return Item::items[id]->isStackedByData();
}

bool ItemInstance::isDamaged()
{
	return isDamageableItem() && auxValue > 0;
}

int ItemInstance::getDamageValue()
{
	return auxValue;
}

int ItemInstance::getAuxValue() const
{
	return auxValue;
}

void ItemInstance::setAuxValue(int value)
{
	auxValue = value;
	if (auxValue < 0)
	{
		auxValue = 0;
	}
}

int ItemInstance::getMaxDamage()
{
	return Item::items[id]->getMaxDamage();
}

bool ItemInstance::hurt(int dmg, Random *random)
{
	if (!isDamageableItem())
	{
		return false;
	}

	if (dmg > 0)
	{
		int level = EnchantmentHelper::getEnchantmentLevel(Enchantment::digDurability->id, shared_from_this());

		int drop = 0;
		for (int y = 0; level > 0 && y < dmg; y++)
		{
			if (DigDurabilityEnchantment::shouldIgnoreDurabilityDrop(shared_from_this(), level, random))
			{
				drop++;
			}
		}
		dmg -= drop;

		if (dmg <= 0) return false;
	}

	auxValue += dmg;

	return auxValue > getMaxDamage();
}

void ItemInstance::hurtAndBreak(int dmg, shared_ptr<LivingEntity> owner)
{
	shared_ptr<Player> player = dynamic_pointer_cast<Player>(owner);
	if (player != NULL && player->abilities.instabuild) return;
	if (!isDamageableItem()) return;

	if (hurt(dmg, owner->getRandom()))
	{
		owner->breakItem(shared_from_this());

		count--;
		if (player != NULL)
		{
			//player->awardStat(Stats::itemBroke[id], 1);
			if (count == 0 && dynamic_cast<BowItem *>( getItem() ) != NULL)
			{
				player->removeSelectedItem();
			}
		}
		if (count < 0) count = 0;
		auxValue = 0;
	}
}

void ItemInstance::hurtEnemy(shared_ptr<LivingEntity> mob, shared_ptr<Player> attacker)
{
	//bool used =
	Item::items[id]->hurtEnemy(shared_from_this(), mob, attacker);
}

void ItemInstance::mineBlock(Level *level, int tile, int x, int y, int z, shared_ptr<Player> owner)
{
	//bool used =
	Item::items[id]->mineBlock( shared_from_this(), level, tile, x, y, z, owner);
}

bool ItemInstance::canDestroySpecial(Tile *tile)
{
	return Item::items[id]->canDestroySpecial(tile);
}

bool ItemInstance::interactEnemy(shared_ptr<Player> player, shared_ptr<LivingEntity> mob)
{
	return Item::items[id]->interactEnemy(shared_from_this(), player, mob);
}

shared_ptr<ItemInstance> ItemInstance::copy() const
{
	shared_ptr<ItemInstance> copy = shared_ptr<ItemInstance>( new ItemInstance(id, count, auxValue) );
	if (tag != NULL)
	{
		copy->tag = (CompoundTag *) tag->copy();
	}
	return copy;
}

// 4J Stu - Added this as we need it in the recipe code
ItemInstance *ItemInstance::copy_not_shared() const
{
	ItemInstance *copy = new ItemInstance(id, count, auxValue);
	if (tag != NULL)
	{
		copy->tag = (CompoundTag *) tag->copy();
		if (!copy->tag->equals(tag))
		{
			return copy;
		}
	}
	return copy;
}

// 4J Brought forward from 1.2
bool ItemInstance::tagMatches(shared_ptr<ItemInstance> a, shared_ptr<ItemInstance> b)
{
	if (a == NULL && b == NULL) return true;
	if (a == NULL || b == NULL) return false;

	if (a->tag == NULL && b->tag != NULL)
	{
		return false;
	}
	if (a->tag != NULL && !a->tag->equals(b->tag))
	{
		return false;
	}
	return true;
}

bool ItemInstance::matches(shared_ptr<ItemInstance> a, shared_ptr<ItemInstance> b)
{
	if (a == NULL && b == NULL) return true;
	if (a == NULL || b == NULL) return false;
	return a->matches(b);
}

bool ItemInstance::matches(shared_ptr<ItemInstance> b)
{
	if (count != b->count) return false;
	if (id != b->id) return false;
	if (auxValue != b->auxValue) return false;
	if (tag == NULL && b->tag != NULL)
	{
		return false;
	}
	if (tag != NULL && !tag->equals(b->tag))
	{
		return false;
	}
	return true;
}

/**
* Checks if this item is the same item as the other one, disregarding the
* 'count' value.
*
* @param b
* @return
*/
bool ItemInstance::sameItem(shared_ptr<ItemInstance> b)
{
	return id == b->id && auxValue == b->auxValue;
}

bool ItemInstance::sameItemWithTags(shared_ptr<ItemInstance> b)
{
	if (id != b->id) return false;
	if (auxValue != b->auxValue) return false;
	if (tag == NULL && b->tag != NULL)
	{
		return false;
	}
	if (tag != NULL && !tag->equals(b->tag))
	{
		return false;
	}
	return true;
}

// 4J Stu - Added this for the one time when we compare with a non-shared pointer
bool ItemInstance::sameItem_not_shared(const ItemInstance *b)
{
	return id == b->id && auxValue == b->auxValue;
}

unsigned int ItemInstance::getUseDescriptionId()
{
	return Item::items[id]->getUseDescriptionId(shared_from_this());
}

unsigned int ItemInstance::getDescriptionId(int iData /*= -1*/)
{
	return Item::items[id]->getDescriptionId(shared_from_this());
}

ItemInstance *ItemInstance::setDescriptionId(unsigned int id)
{
	// 4J Stu - I don't think this function is ever used. It if is, it should probably return shared_from_this()
	assert(false);
	return this;
}

shared_ptr<ItemInstance> ItemInstance::clone(shared_ptr<ItemInstance> item)
{
	return item == NULL ? nullptr : item->copy();
}

wstring ItemInstance::toString()
{
	//return count + "x" + Item::items[id]->getDescriptionId() + "@" + auxValue;

	std::wostringstream oss;
	// 4J-PB - TODO - temp fix until ore recipe issue is fixed
	if(Item::items[id]==NULL)
	{
		oss << std::dec << count << L"x" << L" Item::items[id] is NULL " << L"@" << auxValue;
	}
	else
	{
		oss << std::dec << count << L"x" << Item::items[id]->getDescription(shared_from_this()) << L"@" << auxValue;
	}
	return oss.str();
}

void ItemInstance::inventoryTick(Level *level, shared_ptr<Entity> owner, int slot, bool selected)
{
	if (popTime > 0) popTime--;
	Item::items[id]->inventoryTick(shared_from_this(), level, owner, slot, selected);
}

void ItemInstance::onCraftedBy(Level *level, shared_ptr<Player> player, int craftCount)
{
	// 4J Stu Added for tutorial callback
	player->onCrafted(shared_from_this());

	player->awardStat(
		GenericStats::itemsCrafted(id),
		GenericStats::param_itemsCrafted(id, auxValue, craftCount)
		);

	Item::items[id]->onCraftedBy(shared_from_this(), level, player);
}

bool ItemInstance::equals(shared_ptr<ItemInstance> ii)
{
	return id == ii->id && count == ii->count && auxValue == ii->auxValue;
}

int ItemInstance::getUseDuration()
{
	return getItem()->getUseDuration(shared_from_this());
}

UseAnim ItemInstance::getUseAnimation()
{
	return getItem()->getUseAnimation(shared_from_this());
}

void ItemInstance::releaseUsing(Level *level, shared_ptr<Player> player, int durationLeft)
{
	getItem()->releaseUsing(shared_from_this(), level, player, durationLeft);
}

// 4J Stu - Brought forward these functions for enchanting/game rules
bool ItemInstance::hasTag()
{
	return tag != NULL;
}

CompoundTag *ItemInstance::getTag()
{
	return tag;
}

ListTag<CompoundTag> *ItemInstance::getEnchantmentTags()
{
	if (tag == NULL)
	{
		return NULL;
	}
	return (ListTag<CompoundTag> *) tag->get(L"ench");
}

void ItemInstance::setTag(CompoundTag *tag)
{
	delete this->tag;
	this->tag = tag;
}

wstring ItemInstance::getHoverName()
{
	wstring title = getItem()->getHoverName(shared_from_this());

	if (tag != NULL && tag->contains(L"display"))
	{
		CompoundTag *display = tag->getCompound(L"display");

		if (display->contains(L"Name"))
		{
			title = display->getString(L"Name");
		}
	}

	return title;
}

void ItemInstance::setHoverName(const wstring &name)
{
	if (tag == NULL) tag = new CompoundTag();
	if (!tag->contains(L"display")) tag->putCompound(L"display", new CompoundTag());
	tag->getCompound(L"display")->putString(L"Name", name);
}

void ItemInstance::resetHoverName()
{
	if (tag == NULL) return;
	if (!tag->contains(L"display")) return;
	CompoundTag *display = tag->getCompound(L"display");
	display->remove(L"Name");

	if (display->isEmpty())
	{
		tag->remove(L"display");

		if (tag->isEmpty())
		{
			setTag(NULL);
		}
	}
}

bool ItemInstance::hasCustomHoverName()
{
	if (tag == NULL) return false;
	if (!tag->contains(L"display")) return false;
	return tag->getCompound(L"display")->contains(L"Name");
}

vector<HtmlString> *ItemInstance::getHoverText(shared_ptr<Player> player, bool advanced)
{
	vector<HtmlString> *lines = new vector<HtmlString>();
	Item *item = Item::items[id];
	HtmlString title = HtmlString(getHoverName());

	if (hasCustomHoverName())
	{
		title.italics = true;
	}

	// 4J: This is for showing aux values, not useful in console version
	/*
	if (advanced)
	{
		wstring suffix = L"";

		if (title.length() > 0)
		{
			title += L" (";
			suffix = L")";
		}

		if (isStackedByData())
		{
			title += String.format("#%04d/%d%s", id, auxValue, suffix);
		}
		else
		{
			title += String.format("#%04d%s", id, suffix);
		}
	}
	else if (!hasCustomHoverName() && id == Item::map_Id)
	*/

	/*if (!hasCustomHoverName() && id == Item::map_Id)
	{
		title.text += L" #" + _toString(auxValue);
	}*/

	lines->push_back(title);

	item->appendHoverText(shared_from_this(), player, lines, advanced);

	if (hasTag())
	{
		ListTag<CompoundTag> *list = getEnchantmentTags();
		if (list != NULL)
		{
			for (int i = 0; i < list->size(); i++)
			{
				int type = list->get(i)->getShort((wchar_t *)TAG_ENCH_ID);
				int level = list->get(i)->getShort((wchar_t *)TAG_ENCH_LEVEL);

				if (Enchantment::enchantments[type] != NULL)
				{
					wstring unformatted = L"";
					lines->push_back(Enchantment::enchantments[type]->getFullname(level));
				}
			}
		}

		if (tag->contains(L"display"))
		{
			//CompoundTag *display = tag->getCompound(L"display");

			//if (display->contains(L"color"))
			//{
			//	if (advanced)
			//	{
			//		wchar_t text [256];
			//		swprintf(text, 256, L"Color: LOCALISE #%08X", display->getInt(L"color"));
			//		lines->push_back(HtmlString(text));
			//	}
			//	else
			//	{
			//		lines->push_back(HtmlString(L"Dyed LOCALISE", eMinecraftColour_NOT_SET, true));
			//	}
			//}

			// 4J: Lore isn't in use in game
			/*if (display->contains(L"Lore"))
			{
				ListTag<StringTag> *lore = (ListTag<StringTag> *) display->getList(L"Lore");
				if (lore->size() > 0)
				{
					for (int i = 0; i < lore->size(); i++)
					{
						//lines->push_back(ChatFormatting::DARK_PURPLE + "" + ChatFormatting::ITALIC + lore->get(i)->data);
						lines->push_back(lore->get(i)->data);
					}
				}
			}*/
		}
	}

	attrAttrModMap *modifiers = getAttributeModifiers();

	if (!modifiers->empty())
	{
		// New line
		lines->push_back(HtmlString(L""));

		// Modifier descriptions
		for (AUTO_VAR(it, modifiers->begin()); it != modifiers->end(); ++it)
		{
			// 4J: Moved modifier string building to AttributeModifier
			lines->push_back(it->second->getHoverText(it->first));
		}
	}

	// Delete modifiers map
	for (AUTO_VAR(it, modifiers->begin()); it != modifiers->end(); ++it)
	{
		AttributeModifier *modifier = it->second;
		delete modifier;
	}
	delete modifiers;

	if (advanced)
	{
		if (isDamaged())
		{
			wstring damageStr = L"Durability: LOCALISE " + _toString<int>((getMaxDamage()) - getDamageValue()) + L" / " + _toString<int>(getMaxDamage());
			lines->push_back(HtmlString(damageStr));
		}
	}

	return lines;
}

// 4J Added
vector<HtmlString> *ItemInstance::getHoverTextOnly(shared_ptr<Player> player, bool advanced)
{
	vector<HtmlString> *lines = new vector<HtmlString>();
	Item *item = Item::items[id];

	item->appendHoverText(shared_from_this(), player, lines, advanced);

	if (hasTag())
	{
		ListTag<CompoundTag> *list = getEnchantmentTags();
		if (list != NULL)
		{
			for (int i = 0; i < list->size(); i++)
			{
				int type = list->get(i)->getShort((wchar_t *)TAG_ENCH_ID);
				int level = list->get(i)->getShort((wchar_t *)TAG_ENCH_LEVEL);

				if (Enchantment::enchantments[type] != NULL)
				{
					wstring unformatted = L"";
					lines->push_back(Enchantment::enchantments[type]->getFullname(level));
				}
			}
		}
	}
	return lines;
}

bool ItemInstance::isFoil()
{
	return getItem()->isFoil(shared_from_this());
}

const Rarity *ItemInstance::getRarity()
{
	return getItem()->getRarity(shared_from_this());
}

bool ItemInstance::isEnchantable()
{
	if (!getItem()->isEnchantable(shared_from_this())) return false;
	if (isEnchanted()) return false;
	return true;
}

void ItemInstance::enchant(const Enchantment *enchantment, int level)
{
	if (tag == NULL) this->setTag(new CompoundTag());
	if (!tag->contains(L"ench")) tag->put(L"ench", new ListTag<CompoundTag>(L"ench"));

	ListTag<CompoundTag> *list = (ListTag<CompoundTag> *) tag->get(L"ench");
	CompoundTag *ench = new CompoundTag();
	ench->putShort((wchar_t *)TAG_ENCH_ID, (short) enchantment->id);
	ench->putShort((wchar_t *)TAG_ENCH_LEVEL, (byte) level);
	list->add(ench);
}

bool ItemInstance::isEnchanted()
{
	if (tag != NULL && tag->contains(L"ench")) return true;
	return false;
}

void ItemInstance::addTagElement(wstring name, Tag *tag)
{
	if (this->tag == NULL)
	{
		setTag(new CompoundTag());
	}
	this->tag->put((wchar_t *)name.c_str(), tag);
}

bool ItemInstance::mayBePlacedInAdventureMode()
{
	return getItem()->mayBePlacedInAdventureMode();
}

bool ItemInstance::isFramed()
{
	return frame != NULL;
}

void ItemInstance::setFramed(shared_ptr<ItemFrame> frame)
{
	this->frame = frame;
}

shared_ptr<ItemFrame> ItemInstance::getFrame()
{
	return frame;
}

int ItemInstance::getBaseRepairCost()
{
	if (hasTag() && tag->contains(L"RepairCost"))
	{
		return tag->getInt(L"RepairCost");
	}
	else
	{
		return 0;
	}
}

void ItemInstance::setRepairCost(int cost)
{
	if (!hasTag()) tag = new CompoundTag();
	tag->putInt(L"RepairCost", cost);
}

attrAttrModMap *ItemInstance::getAttributeModifiers()
{
	attrAttrModMap *result = NULL;

	if (hasTag() && tag->contains(L"AttributeModifiers"))
	{
		result = new attrAttrModMap();
		ListTag<CompoundTag> *entries = (ListTag<CompoundTag> *) tag->getList(L"AttributeModifiers");

		for (int i = 0; i < entries->size(); i++)
		{
			CompoundTag *entry = entries->get(i);
			AttributeModifier *attribute = SharedMonsterAttributes::loadAttributeModifier(entry);

			// 4J Not sure why but this is a check that the attribute ID is not empty
			/*if (attribute->getId()->getLeastSignificantBits() != 0 && attribute->getId()->getMostSignificantBits() != 0)
			{*/
			result->insert(std::pair<eATTRIBUTE_ID, AttributeModifier *>(static_cast<eATTRIBUTE_ID>(entry->getInt(L"ID")), attribute));
			/*}*/
		}
	}
	else
	{
		result = getItem()->getDefaultAttributeModifiers();
	}

	return result;
}

void ItemInstance::set4JData(int data)
{
	if(tag == NULL && data == 0) return;
	if (tag == NULL) this->setTag(new CompoundTag());

	if (tag->contains(L"4jdata"))
	{
		IntTag *dataTag = (IntTag *)tag->get(L"4jdata");
		dataTag->data = data;
	}
	else if(data != 0)
	{
		tag->put(L"4jdata", new IntTag(L"4jdata",data));
	}
}

int ItemInstance::get4JData()
{
	if(tag == NULL || !tag->contains(L"4jdata")) return 0;
	else
	{
		IntTag *dataTag = (IntTag *)tag->get(L"4jdata");
		return dataTag->data;
	}
}
// 4J Added - to show strength on potions
bool ItemInstance::hasPotionStrengthBar()
{
	// exclude a bottle of water from this
	if((id==Item::potion_Id) && (auxValue !=0))// && (!MACRO_POTION_IS_AKWARD(auxValue))) 4J-PB leaving the bar on an awkward potion so we can differentiate it from a water bottle
	{
		return true;
	}

	return false;
}

int ItemInstance::GetPotionStrength()
{
	if(MACRO_POTION_IS_INSTANTDAMAGE(auxValue) || MACRO_POTION_IS_INSTANTHEALTH(auxValue) )
	{
		// The two instant potions don't have extended versions
		return (auxValue&MASK_LEVEL2)>>5;
	}
	else
	{
		return (auxValue&MASK_LEVEL2EXTENDED)>>5;
	}
}