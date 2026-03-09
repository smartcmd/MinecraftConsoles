#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.damagesource.h"
#include "WeighedRandom.h"
#include "EnchantmentHelper.h"

Random EnchantmentHelper::random;

int EnchantmentHelper::getEnchantmentLevel(int enchantmentId, shared_ptr<ItemInstance> piece)
{
	if (piece == nullptr)
	{
		return 0;
	}
	ListTag<CompoundTag> *enchantmentTags = piece->getEnchantmentTags();
	if (enchantmentTags == nullptr)
	{
		return 0;
	}
	for (int i = 0; i < enchantmentTags->size(); i++)
	{
		int type = enchantmentTags->get(i)->getShort((wchar_t *)ItemInstance::TAG_ENCH_ID);
		int level = enchantmentTags->get(i)->getShort((wchar_t *)ItemInstance::TAG_ENCH_LEVEL);

		if (type == enchantmentId)
		{
			return level;
		}
	}
	return 0;
}

unordered_map<int, int> *EnchantmentHelper::getEnchantments(shared_ptr<ItemInstance> item)
{
	unordered_map<int, int> *result = new unordered_map<int, int>();
	ListTag<CompoundTag> *list = item->id == Item::enchantedBook_Id ? Item::enchantedBook->getEnchantments(item) : item->getEnchantmentTags();

	if (list != nullptr)
	{
		for (int i = 0; i < list->size(); i++)
		{
			int type = list->get(i)->getShort((wchar_t *)ItemInstance::TAG_ENCH_ID);
			int level = list->get(i)->getShort((wchar_t *)ItemInstance::TAG_ENCH_LEVEL);

			result->insert( unordered_map<int, int>::value_type(type, level));
		}
	}

	return result;
}

void EnchantmentHelper::setEnchantments(unordered_map<int, int> *enchantments, shared_ptr<ItemInstance> item)
{
	ListTag<CompoundTag> *list = new ListTag<CompoundTag>();

	//for (int id : enchantments.keySet())
	for ( const auto& it : *enchantments )
	{
		int id = it.first;
		CompoundTag *tag = new CompoundTag();

		tag->putShort((wchar_t *)ItemInstance::TAG_ENCH_ID, static_cast<short>(id));
		tag->putShort((wchar_t *)ItemInstance::TAG_ENCH_LEVEL, static_cast<short>((int)it.second));

		list->add(tag);

		if (item->id == Item::enchantedBook_Id)
		{
			Item::enchantedBook->addEnchantment(item, new EnchantmentInstance(id, it.second));
		}
	}

	if (list->size() > 0)
	{
		if (item->id != Item::enchantedBook_Id)
		{
			item->addTagElement(L"ench", list);
		}
	}
	else if (item->hasTag())
	{
		item->getTag()->remove(L"ench");
	}
}

int EnchantmentHelper::getEnchantmentLevel(int enchantmentId, ItemInstanceArray inventory)
{
	if (inventory.data == nullptr) return 0;
	int bestLevel = 0;
	//for (ItemInstance piece : inventory)
	for(unsigned int i = 0; i < inventory.length; ++i)
	{
		int newLevel = getEnchantmentLevel(enchantmentId, inventory[i]);
		if (newLevel > bestLevel)
		{
			bestLevel = newLevel;
		}
	}
	return bestLevel;
}

void EnchantmentHelper::runIterationOnItem(EnchantmentIterationMethod &method, shared_ptr<ItemInstance> piece)
{
	if (piece == nullptr)
	{
		return;
	}
	ListTag<CompoundTag> *enchantmentTags = piece->getEnchantmentTags();
	if (enchantmentTags == nullptr)
	{
		return;
	}
	for (int i = 0; i < enchantmentTags->size(); i++)
	{
		int type = enchantmentTags->get(i)->getShort((wchar_t *)ItemInstance::TAG_ENCH_ID);
		int level = enchantmentTags->get(i)->getShort((wchar_t *)ItemInstance::TAG_ENCH_LEVEL);

		if (Enchantment::enchantments[type] != nullptr)
		{
			method.doEnchantment(Enchantment::enchantments[type], level);
		}
	}
}

void EnchantmentHelper::runIterationOnInventory(EnchantmentIterationMethod &method, ItemInstanceArray inventory)
{
	//for (ItemInstance piece : inventory)
	for(unsigned int i = 0; i < inventory.length; ++i)
	{
		runIterationOnItem(method, inventory[i]);
	}
}

void EnchantmentHelper::GetDamageProtectionIteration::doEnchantment(Enchantment *enchantment, int level)
{
	sum += enchantment->getDamageProtection(level, source);
}

EnchantmentHelper::GetDamageProtectionIteration EnchantmentHelper::getDamageProtectionIteration;

/**
* Fetches the protection value for enchanted items.
* 
* @param inventory
* @param source
* @return
*/
int EnchantmentHelper::getDamageProtection(ItemInstanceArray armor, DamageSource *source)
{
	getDamageProtectionIteration.sum = 0;
	getDamageProtectionIteration.source = source;

	runIterationOnInventory(getDamageProtectionIteration, armor);

	if (getDamageProtectionIteration.sum > 25)
	{
		getDamageProtectionIteration.sum = 25;
	}
	// enchantment protection is on the scale of 0 to 25, where 20 or more
	// will nullify nearly all damage (there will be damage spill)
	return ((getDamageProtectionIteration.sum + 1) >> 1) + random.nextInt((getDamageProtectionIteration.sum >> 1) + 1);
}

void EnchantmentHelper::GetDamageBonusIteration::doEnchantment(Enchantment *enchantment, int level)
{
	sum += enchantment->getDamageBonus(level, target);
}


EnchantmentHelper::GetDamageBonusIteration EnchantmentHelper::getDamageBonusIteration;

/**
* 
* @param inventory
* @param target
* @return
*/
float EnchantmentHelper::getDamageBonus(shared_ptr<LivingEntity> source, shared_ptr<LivingEntity> target)
{

	getDamageBonusIteration.sum = 0;
	getDamageBonusIteration.target = target;

	runIterationOnItem(getDamageBonusIteration, source->getCarriedItem() );

	return getDamageBonusIteration.sum;
}

int EnchantmentHelper::getKnockbackBonus(shared_ptr<LivingEntity> source, shared_ptr<LivingEntity> target)
{
	return getEnchantmentLevel(Enchantment::knockback->id, source->getCarriedItem() );
}

int EnchantmentHelper::getFireAspect(shared_ptr<LivingEntity> source)
{
	return getEnchantmentLevel(Enchantment::fireAspect->id, source->getCarriedItem());
}

int EnchantmentHelper::getOxygenBonus(shared_ptr<LivingEntity> source)
{
	return getEnchantmentLevel(Enchantment::drownProtection->id, source->getEquipmentSlots() );
}

int EnchantmentHelper::getDiggingBonus(shared_ptr<LivingEntity> source)
{
	return getEnchantmentLevel(Enchantment::diggingBonus->id, source->getCarriedItem() );
}

int EnchantmentHelper::getDigDurability(shared_ptr<LivingEntity> source)
{
	return getEnchantmentLevel(Enchantment::digDurability->id, source->getCarriedItem() );
}

bool EnchantmentHelper::hasSilkTouch(shared_ptr<LivingEntity> source)
{
	return getEnchantmentLevel(Enchantment::untouching->id, source->getCarriedItem() ) > 0;
}

int EnchantmentHelper::getDiggingLootBonus(shared_ptr<LivingEntity> source)
{
	return getEnchantmentLevel(Enchantment::resourceBonus->id, source->getCarriedItem() );
}

int EnchantmentHelper::getKillingLootBonus(shared_ptr<LivingEntity> source)
{
	return getEnchantmentLevel(Enchantment::lootBonus->id, source->getCarriedItem() );
}

bool EnchantmentHelper::hasWaterWorkerBonus(shared_ptr<LivingEntity> source)
{
	return getEnchantmentLevel(Enchantment::waterWorker->id, source->getEquipmentSlots() ) > 0;
}

int EnchantmentHelper::getArmorThorns(shared_ptr<LivingEntity> source)
{
	return getEnchantmentLevel(Enchantment::thorns->id, source->getEquipmentSlots());
}

shared_ptr<ItemInstance> EnchantmentHelper::getRandomItemWith(Enchantment *enchantment, shared_ptr<LivingEntity> source)
{
	ItemInstanceArray items = source->getEquipmentSlots();
	for(unsigned int i = 0; i < items.length; ++i)
	{
		shared_ptr<ItemInstance> item = items[i];
		if (item != nullptr && getEnchantmentLevel(enchantment->id, item) > 0)
		{
			return item;
		}
	}

	return nullptr;
}

/**
* 
* @param random
* @param slot
*            The table slot, 0-2
* @param bookcases
*            How many book cases that are found around the table.
* @param itemInstance
*            Which item that is being enchanted.
* @return The enchantment cost, 0 means unchantable, 50 is max.
*/
int EnchantmentHelper::getEnchantmentCost(Random *random, int slot, int bookcases, shared_ptr<ItemInstance> itemInstance)
{
	Item *item = itemInstance->getItem();
	int itemValue = item->getEnchantmentValue();

	if (itemValue <= 0)
	{
		// not enchantable
		return 0;
	}

	// 4J Stu - Updated function to 1.3 version for TU7
	if (bookcases > 15)
	{
		bookcases = 15;
	}

	int selected = random->nextInt(8) + 1 + (bookcases >> 1) + random->nextInt(bookcases + 1);
	if (slot == 0)
	{
		return max((selected / 3), 1);
	}
	if (slot == 1)
	{
		return max(selected, bookcases * 2);
	}
	return selected;
}

shared_ptr<ItemInstance> EnchantmentHelper::enchantItem(Random *random, shared_ptr<ItemInstance> itemInstance, int enchantmentCost)
{
	vector<EnchantmentInstance *> *newEnchantment = EnchantmentHelper::selectEnchantment(random, itemInstance, enchantmentCost);
	bool isBook = itemInstance->id == Item::book_Id;

	if (isBook) itemInstance->id = Item::enchantedBook_Id;

	if ( newEnchantment )
	{
		for ( EnchantmentInstance *e : *newEnchantment )
		{
			if ( e )
			{
				if (isBook)
				{
					Item::enchantedBook->addEnchantment(itemInstance, e);
				}
				else
				{
					itemInstance->enchant(e->enchantment, e->level);
				}
				delete e;
			}
		}
		delete newEnchantment;
	}
	return itemInstance;
}

/**
* 
* @param random
* @param itemInstance
* @param enchantmentCost
* @return
*/
vector<EnchantmentInstance *> *EnchantmentHelper::selectEnchantment(Random *random, shared_ptr<ItemInstance> itemInstance, int enchantmentCost)
{
	// withdraw bonus from item
	Item *item = itemInstance->getItem();
	int itemBonus = item->getEnchantmentValue();

	if (itemBonus <= 0)
	{
		return nullptr;
	}
	// 4J Stu - Update function to 1.3 version for TU7
	itemBonus /= 2;
	itemBonus = 1 + random->nextInt((itemBonus >> 1) + 1) + random->nextInt((itemBonus >> 1) + 1);

	int enchantmentValue = itemBonus + enchantmentCost;

	// the final enchantment cost will have another random span of +- 15%
	float deviation = (random->nextFloat() + random->nextFloat() - 1.0f) * .15f;
	int realValue = static_cast<int>((float)enchantmentValue * (1.0f + deviation) + .5f);
	if (realValue < 1)
	{
		realValue = 1;
	}

	vector<EnchantmentInstance *> *results = nullptr;

	unordered_map<int, EnchantmentInstance *> *availableEnchantments = getAvailableEnchantmentResults(realValue, itemInstance);
	if (availableEnchantments && !availableEnchantments->empty())
	{
		vector<WeighedRandomItem *> values;
		for( auto& it : *availableEnchantments )
		{
			values.push_back(it.second);
		}
		EnchantmentInstance *instance = static_cast<EnchantmentInstance *>(WeighedRandom::getRandomItem(random, &values));
		values.clear();

		if (instance)
		{
			results = new vector<EnchantmentInstance *>();
			results->push_back( instance->copy() ); // 4J Stu - Inserting a copy so we can clear memory from the availableEnchantments collection

			int bonusChance = realValue;
			while (random->nextInt(50) <= bonusChance)
			{

				// remove incompatible enchantments from previous result
				//final Iterator<Integer> mapIter = availableEnchantments.keySet().iterator();
				//while (mapIter.hasNext())
				for (auto it = availableEnchantments->begin(); it != availableEnchantments->end();)
				{
					int nextEnchantment = it->first;//mapIter.next();
					bool valid = true;
					for ( const EnchantmentInstance *current : *results )
					{
						if (!current->enchantment->isCompatibleWith(Enchantment::enchantments[nextEnchantment]))
						{
							valid = false;
							break;
						}
					}
					if (!valid)
					{
						//mapIter.remove();
						delete it->second;
						it = availableEnchantments->erase(it);
					}
					else
					{
						++it;
					}
				}

				if (!availableEnchantments->empty())
				{					
					for (auto& it : *availableEnchantments )
					{
						values.push_back(it.second);
					}
					EnchantmentInstance *nextInstance = static_cast<EnchantmentInstance *>(WeighedRandom::getRandomItem(random, &values));
					values.clear();
					results->push_back( nextInstance->copy() ); // 4J Stu - Inserting a copy so we can clear memory from the availableEnchantments collection
				}

				bonusChance >>= 1;
			}
		}
	}
	if( availableEnchantments )
	{
		for (auto& it : *availableEnchantments )
		{
			delete it.second;
		}
		delete availableEnchantments;
	}

	return results;
}

unordered_map<int, EnchantmentInstance *> *EnchantmentHelper::getAvailableEnchantmentResults(int value, shared_ptr<ItemInstance> itemInstance)
{
	Item *item = itemInstance->getItem();
	unordered_map<int, EnchantmentInstance *> *results = nullptr;

	bool isBook = itemInstance->id == Item::book_Id;

	//for (Enchantment e : Enchantment.enchantments)
	for(unsigned int i = 0; i < Enchantment::enchantments.length; ++i)
	{
		Enchantment *e = Enchantment::enchantments[i];
		if (e == nullptr)
		{
			continue;
		}

		// Only picks "normal" enchantments, no specialcases
		if (!e->category->canEnchant(item) && !isBook)
		{
			continue;
		}

		for (int level = e->getMinLevel(); level <= e->getMaxLevel(); level++)
		{
			if (value >= e->getMinCost(level) && value <= e->getMaxCost(level))
			{
				if (results == nullptr)
				{
					results = new unordered_map<int, EnchantmentInstance *>();
				}
				auto it = results->find(e->id);
				if(it != results->end())
				{
					delete it->second;
				}
				(*results)[e->id] = new EnchantmentInstance(e, level);
			}
		}
	}

	return results;
}