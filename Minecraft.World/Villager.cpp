#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.item.trading.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "..\Minecraft.Client\Textures.h"
#include "Villager.h"
#include "AbstractContainerMenu.h"

unordered_map<int, pair<int,int> > Villager::MIN_MAX_VALUES;
unordered_map<int, pair<int,int> > Villager::MIN_MAX_PRICES;

void Villager::_init(int profession)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	setProfession(profession);
	setSize(.6f, 1.8f);

	villageUpdateInterval = 0;
	inLove = false;
	chasing = false;
	village = weak_ptr<Village>();

	tradingPlayer = weak_ptr<Player>();
	offers = NULL;
	updateMerchantTimer = 0;
	addRecipeOnUpdate = false;
	riches = 0;
	lastPlayerTradeName = L"";
	rewardPlayersOnFirstVillage = false;
	baseRecipeChanceMod = 0.0f;

	getNavigation()->setCanOpenDoors(true);
	getNavigation()->setAvoidWater(true);

	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new AvoidPlayerGoal(this, typeid(Zombie), 8, 0.6, 0.6));
	goalSelector.addGoal(1, new TradeWithPlayerGoal(this));
	goalSelector.addGoal(1, new LookAtTradingPlayerGoal(this));
	goalSelector.addGoal(2, new MoveIndoorsGoal(this));
	goalSelector.addGoal(3, new RestrictOpenDoorGoal(this));
	goalSelector.addGoal(4, new OpenDoorGoal(this, true));
	goalSelector.addGoal(5, new MoveTowardsRestrictionGoal(this, 0.6));
	goalSelector.addGoal(6, new MakeLoveGoal(this));
	goalSelector.addGoal(7, new TakeFlowerGoal(this));
	goalSelector.addGoal(8, new PlayGoal(this, 0.32));
	goalSelector.addGoal(9, new InteractGoal(this, typeid(Player), 3, 1.f));
	goalSelector.addGoal(9, new InteractGoal(this, typeid(Villager), 5, 0.02f));
	goalSelector.addGoal(9, new RandomStrollGoal(this, 0.6));
	goalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8));
}

Villager::Villager(Level *level) : AgableMob(level)
{
	_init(0);
}

Villager::Villager(Level *level, int profession) : AgableMob(level)
{
	_init(profession);
}

Villager::~Villager()
{
	delete offers;
}

void Villager::registerAttributes()
{
	AgableMob::registerAttributes();

	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
}

bool Villager::useNewAi()
{
	return true;
}

void Villager::serverAiMobStep()
{
	if (--villageUpdateInterval <= 0)
	{
		level->villages->queryUpdateAround(Mth::floor(x), Mth::floor(y), Mth::floor(z));
		villageUpdateInterval = 70 + random->nextInt(50);

		shared_ptr<Village> _village = level->villages->getClosestVillage(Mth::floor(x), Mth::floor(y), Mth::floor(z), Villages::MaxDoorDist);
		village = _village;
		if (_village == NULL) clearRestriction();
		else
		{
			Pos *center = _village->getCenter();
			restrictTo(center->x, center->y, center->z, (int)((float)_village->getRadius() * 0.6f));
			if (rewardPlayersOnFirstVillage)
			{
				rewardPlayersOnFirstVillage = false;
				_village->rewardAllPlayers(5);
			}
		}
	}

	if (!isTrading() && updateMerchantTimer > 0)
	{
		updateMerchantTimer--;
		if (updateMerchantTimer <= 0)
		{
			if (addRecipeOnUpdate)
			{
				// improve max uses for all obsolete recipes
				if (offers->size() > 0)
				{
					//for (MerchantRecipe recipe : offers)
					for(auto& recipe : *offers)
					{
						if (recipe->isDeprecated())
						{
							recipe->increaseMaxUses(random->nextInt(6) + random->nextInt(6) + 2);
						}
					}
				}
				addOffers(1);
				addRecipeOnUpdate = false;

				if (village.lock() != NULL && !lastPlayerTradeName.empty())
				{
					level->broadcastEntityEvent(shared_from_this(), EntityEvent::VILLAGER_HAPPY);
					village.lock()->modifyStanding(lastPlayerTradeName, 1);
				}
			}
			addEffect(new MobEffectInstance(MobEffect::regeneration->id, SharedConstants::TICKS_PER_SECOND * 10, 0));
		}
	}

	AgableMob::serverAiMobStep();
}

bool Villager::mobInteract(shared_ptr<Player> player)
{
	// [EB]: Truly dislike this code but I don't see another easy way
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	bool holdingSpawnEgg = item != NULL && item->id == Item::spawnEgg_Id;

	if (!holdingSpawnEgg && isAlive() && !isTrading() && !isBaby())
	{
		if (!level->isClientSide)
		{
			// note: stop() logic is controlled by trading ai goal
			setTradingPlayer(player);

			// 4J-JEV: Villagers in PC game don't display professions.
			player->openTrading(dynamic_pointer_cast<Merchant>(shared_from_this()), getDisplayName() );
		}
		return true;
	}
	return AgableMob::mobInteract(player);
}

void Villager::defineSynchedData()
{
	AgableMob::defineSynchedData();
	entityData->define(DATA_PROFESSION_ID, 0);
}

void Villager::addAdditonalSaveData(CompoundTag *tag)
{
	AgableMob::addAdditonalSaveData(tag);
	tag->putInt(L"Profession", getProfession());
	tag->putInt(L"Riches", riches);
	if (offers != NULL)
	{
		tag->putCompound(L"Offers", offers->createTag());
	}
}

void Villager::readAdditionalSaveData(CompoundTag *tag)
{
	AgableMob::readAdditionalSaveData(tag);
	setProfession(tag->getInt(L"Profession"));
	riches = tag->getInt(L"Riches");
	if (tag->contains(L"Offers"))
	{
		CompoundTag *compound = tag->getCompound(L"Offers");
		delete offers;
		offers = new MerchantRecipeList(compound);
	}
}

bool Villager::removeWhenFarAway()
{
	return false;
}

int Villager::getAmbientSound()
{
	 if(isTrading())
	{
		return eSoundType_MOB_VILLAGER_HAGGLE;
	}
	return eSoundType_MOB_VILLAGER_IDLE;
}

int Villager::getHurtSound()
{
	return eSoundType_MOB_VILLAGER_HIT;
}

int Villager::getDeathSound()
{
	return eSoundType_MOB_VILLAGER_DEATH;
}

void Villager::setProfession(int profession)
{
	entityData->set(DATA_PROFESSION_ID, profession);
}

int Villager::getProfession()
{
	return entityData->getInteger(DATA_PROFESSION_ID);
}

bool Villager::isInLove()
{
	return inLove;
}

void Villager::setInLove(bool inLove)
{
	this->inLove = inLove;
}

void Villager::setChasing(bool chasing)
{
	this->chasing = chasing;
}

bool Villager::isChasing()
{
	return chasing;
}

void Villager::setLastHurtByMob(shared_ptr<LivingEntity> mob)
{
	AgableMob::setLastHurtByMob(mob);
	shared_ptr<Village> _village = village.lock();
	if (_village != NULL && mob != NULL)
	{
		_village->addAggressor(mob);

		if ( mob->instanceof(eTYPE_PLAYER) )
		{
			int amount = -1;
			if (isBaby())
			{
				amount = -3;
			}
			_village->modifyStanding( dynamic_pointer_cast<Player>(mob)->getName(), amount );
			if (isAlive())
			{
				level->broadcastEntityEvent(shared_from_this(), EntityEvent::VILLAGER_ANGRY);
			}
		}
	}
}

void Villager::die(DamageSource *source)
{
	shared_ptr<Village> _village = village.lock();
	if (_village != NULL)
	{
		shared_ptr<Entity> sourceEntity = source->getEntity();
		if (sourceEntity != NULL)
		{
			if ( sourceEntity->instanceof(eTYPE_PLAYER) )
			{
				_village->modifyStanding( dynamic_pointer_cast<Player>(sourceEntity)->getName(), -2 );
			}
			else if ( sourceEntity->instanceof(eTYPE_ENEMY) )
			{
				_village->resetNoBreedTimer();
			}
		}
		else if (sourceEntity == NULL)
		{
			// if the villager was killed by the world (such as lava or falling), blame
			// the nearest player by not reproducing for a while
			shared_ptr<Player> nearestPlayer = level->getNearestPlayer(shared_from_this(), 16.0f);
			if (nearestPlayer != NULL)
			{
				_village->resetNoBreedTimer();
			}
		}
	}

	// Make the gui close if the villager die while trading
    if (auto currentTrader = tradingPlayer.lock())
    {
        if (currentTrader->containerMenu != nullptr)
        {
            auto menu = currentTrader->containerMenu;
            menu->removed(currentTrader);
        }

        tradingPlayer.reset();
    }

	AgableMob::die(source);
}

void Villager::setTradingPlayer(shared_ptr<Player> player)
{
	tradingPlayer = weak_ptr<Player>(player);
}

shared_ptr<Player> Villager::getTradingPlayer()
{
	return tradingPlayer.lock();
}

bool Villager::isTrading()
{
	return tradingPlayer.lock() != NULL;
}

void Villager::notifyTrade(MerchantRecipe *activeRecipe)
{
	activeRecipe->increaseUses();
	ambientSoundTime = -getAmbientSoundInterval();
	playSound(eSoundType_MOB_VILLAGER_YES, getSoundVolume(), getVoicePitch());

	// when the player buys the latest item, we improve the merchant a little while later
	if (activeRecipe->isSame(offers->at(offers->size() - 1)))
	{
		updateMerchantTimer = SharedConstants::TICKS_PER_SECOND * 2;
		addRecipeOnUpdate = true;
		if (tradingPlayer.lock() != NULL)
		{
			lastPlayerTradeName = tradingPlayer.lock()->getName();
		}
		else
		{
			lastPlayerTradeName = L"";
		}
	}

	if (activeRecipe->getBuyAItem()->id == Item::emerald_Id)
	{
		riches += activeRecipe->getBuyAItem()->count;
	}
}

void Villager::notifyTradeUpdated(shared_ptr<ItemInstance> item)
{
	if (!level->isClientSide && (ambientSoundTime > (-getAmbientSoundInterval() + SharedConstants::TICKS_PER_SECOND)))
	{
		ambientSoundTime = -getAmbientSoundInterval();
		if (item != NULL)
		{
			playSound(eSoundType_MOB_VILLAGER_YES, getSoundVolume(), getVoicePitch());
		}
		else
		{
			playSound(eSoundType_MOB_VILLAGER_NO, getSoundVolume(), getVoicePitch());
		}
	}
}

MerchantRecipeList *Villager::getOffers(shared_ptr<Player> forPlayer)
{
	if (offers == NULL)
	{
		addOffers(1);
	}
	return offers;
}

float Villager::getRecipeChance(float baseChance)
{
	float newChance = baseChance + baseRecipeChanceMod;
	if (newChance > .9f)
	{
		return .9f - (newChance - .9f);
	}
	return newChance;
}

void Villager::addOffers(int addCount)
{
	MerchantRecipeList *newOffers = new MerchantRecipeList();
	switch (getProfession())
	{
	case PROFESSION_FARMER:
		addItemForTradeIn(newOffers, Item::wheat_Id, random, getRecipeChance(.9f));
		addItemForTradeIn(newOffers, Tile::wool_Id, random, getRecipeChance(.5f));
		addItemForTradeIn(newOffers, Item::chicken_raw_Id, random, getRecipeChance(.5f));
		addItemForTradeIn(newOffers, Item::fish_cooked_Id, random, getRecipeChance(.4f));
		addItemForPurchase(newOffers, Item::bread_Id, random, getRecipeChance(.9f));
		addItemForPurchase(newOffers, Item::melon_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::apple_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::cookie_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::shears_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::flintAndSteel_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::chicken_cooked_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::arrow_Id, random, getRecipeChance(.5f));
		if (random->nextFloat() < getRecipeChance(.5f))
		{
			newOffers->push_back(new MerchantRecipe(shared_ptr<ItemInstance>( new ItemInstance(Tile::gravel, 10) ), shared_ptr<ItemInstance>( new ItemInstance(Item::emerald) ), shared_ptr<ItemInstance>( new ItemInstance(Item::flint_Id, 4 + random->nextInt(2), 0))));
		}
		break;
	case PROFESSION_BUTCHER:
		addItemForTradeIn(newOffers, Item::coal_Id, random, getRecipeChance(.7f));
		addItemForTradeIn(newOffers, Item::porkChop_raw_Id, random, getRecipeChance(.5f));
		addItemForTradeIn(newOffers, Item::beef_raw_Id, random, getRecipeChance(.5f));
		addItemForPurchase(newOffers, Item::saddle_Id, random, getRecipeChance(.1f));
		addItemForPurchase(newOffers, Item::chestplate_leather_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::boots_leather_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::helmet_leather_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::leggings_leather_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::porkChop_cooked_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::beef_cooked_Id, random, getRecipeChance(.3f));
		break;
	case PROFESSION_SMITH:
		addItemForTradeIn(newOffers, Item::coal_Id, random, getRecipeChance(.7f));
		addItemForTradeIn(newOffers, Item::ironIngot_Id, random, getRecipeChance(.5f));
		addItemForTradeIn(newOffers, Item::goldIngot_Id, random, getRecipeChance(.5f));
		addItemForTradeIn(newOffers, Item::diamond_Id, random, getRecipeChance(.5f));

		addItemForPurchase(newOffers, Item::sword_iron_Id, random, getRecipeChance(.5f));
		addItemForPurchase(newOffers, Item::sword_diamond_Id, random, getRecipeChance(.5f));
		addItemForPurchase(newOffers, Item::hatchet_iron_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::hatchet_diamond_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::pickAxe_iron_Id, random, getRecipeChance(.5f));
		addItemForPurchase(newOffers, Item::pickAxe_diamond_Id, random, getRecipeChance(.5f));
		addItemForPurchase(newOffers, Item::shovel_iron_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::shovel_diamond_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::hoe_iron_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::hoe_diamond_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::boots_iron_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::boots_diamond_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::helmet_iron_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::helmet_diamond_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::chestplate_iron_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::chestplate_diamond_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::leggings_iron_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::leggings_diamond_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::boots_chain_Id, random, getRecipeChance(.1f));
		addItemForPurchase(newOffers, Item::helmet_chain_Id, random, getRecipeChance(.1f));
		addItemForPurchase(newOffers, Item::chestplate_chain_Id, random, getRecipeChance(.1f));
		addItemForPurchase(newOffers, Item::leggings_chain_Id, random, getRecipeChance(.1f));
		break;
	case PROFESSION_LIBRARIAN:
		addItemForTradeIn(newOffers, Item::paper_Id, random, getRecipeChance(.8f));
		addItemForTradeIn(newOffers, Item::book_Id, random, getRecipeChance(.8f));
		//addItemForTradeIn(newOffers, Item::writtenBook_Id, random, getRecipeChance(0.3f));
		addItemForPurchase(newOffers, Tile::bookshelf_Id, random, getRecipeChance(.8f));
		addItemForPurchase(newOffers, Tile::glass_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::compass_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::clock_Id, random, getRecipeChance(.2f));

		if (random->nextFloat() < getRecipeChance(0.07f))
		{
			Enchantment *enchantment = Enchantment::validEnchantments[random->nextInt(Enchantment::validEnchantments.size())];
			int level = Mth::nextInt(random, enchantment->getMinLevel(), enchantment->getMaxLevel());
			shared_ptr<ItemInstance> book = Item::enchantedBook->createForEnchantment(new EnchantmentInstance(enchantment, level));
			int cost = 2 + random->nextInt(5 + (level * 10)) + 3 * level;

			newOffers->push_back(new MerchantRecipe(shared_ptr<ItemInstance>(new ItemInstance(Item::book)), shared_ptr<ItemInstance>(new ItemInstance(Item::emerald, cost)), book));
		}
		break;
	case PROFESSION_PRIEST:
		addItemForPurchase(newOffers, Item::eyeOfEnder_Id, random, getRecipeChance(.3f));
		addItemForPurchase(newOffers, Item::expBottle_Id, random, getRecipeChance(.2f));
		addItemForPurchase(newOffers, Item::redStone_Id, random, getRecipeChance(.4f));
		addItemForPurchase(newOffers, Tile::glowstone_Id, random, getRecipeChance(.3f));
		{
			int enchantItems[] = {
				Item::sword_iron_Id, Item::sword_diamond_Id, Item::chestplate_iron_Id, Item::chestplate_diamond_Id, Item::hatchet_iron_Id, Item::hatchet_diamond_Id, Item::pickAxe_iron_Id,
				Item::pickAxe_diamond_Id
			};
			for (unsigned int i = 0; i < 8; ++i)
			{
				int id = enchantItems[i];
				if (random->nextFloat() < getRecipeChance(.05f))
				{
					newOffers->push_back(new MerchantRecipe(shared_ptr<ItemInstance>(new ItemInstance(id, 1, 0)),
						shared_ptr<ItemInstance>(new ItemInstance(Item::emerald, 2 + random->nextInt(3), 0)),
						EnchantmentHelper::enchantItem(random, shared_ptr<ItemInstance>(new ItemInstance(id, 1, 0)), 5 + random->nextInt(15))));
				}
			}
		}
		break;
	}

	if (newOffers->empty())
	{
		addItemForTradeIn(newOffers, Item::goldIngot_Id, random, 1.0f);
	}

	// shuffle the list to make it more interesting
    std::shuffle(newOffers->begin(), newOffers->end(), std::mt19937{std::random_device{}()});

	if (offers == NULL)
	{
		offers = new MerchantRecipeList();
	}
	for (int i = 0; i < addCount && i < newOffers->size(); i++)
	{
		if( offers->addIfNewOrBetter(newOffers->at(i)))
		{
			// 4J Added so we can delete newOffers
			newOffers->erase(newOffers->begin() + i);
		}
	}
	delete newOffers;
}

void Villager::overrideOffers(MerchantRecipeList *recipeList)
{
}


void Villager::staticCtor()
{
	MIN_MAX_VALUES[Item::coal_Id] = pair<int,int>(16, 24);
	MIN_MAX_VALUES[Item::ironIngot_Id] = pair<int,int>(8, 10);
	MIN_MAX_VALUES[Item::goldIngot_Id] = pair<int,int>(8, 10);
	MIN_MAX_VALUES[Item::diamond_Id] = pair<int,int>(4, 6);
	MIN_MAX_VALUES[Item::paper_Id] = pair<int,int>(24, 36);
	MIN_MAX_VALUES[Item::book_Id] = pair<int,int>(11, 13);
	//MIN_MAX_VALUES.insert(Item::writtenBook_Id, pair<int,int>(1, 1));
	MIN_MAX_VALUES[Item::enderPearl_Id] = pair<int,int>(3, 4);
	MIN_MAX_VALUES[Item::eyeOfEnder_Id] = pair<int,int>(2, 3);
	MIN_MAX_VALUES[Item::porkChop_raw_Id] = pair<int,int>(14, 18);
	MIN_MAX_VALUES[Item::beef_raw_Id] = pair<int,int>(14, 18);
	MIN_MAX_VALUES[Item::chicken_raw_Id] = pair<int,int>(14, 18);
	MIN_MAX_VALUES[Item::fish_cooked_Id] = pair<int,int>(9, 13);
	MIN_MAX_VALUES[Item::seeds_wheat_Id] = pair<int,int>(34, 48);
	MIN_MAX_VALUES[Item::seeds_melon_Id] = pair<int,int>(30, 38);
	MIN_MAX_VALUES[Item::seeds_pumpkin_Id] = pair<int,int>(30, 38);
	MIN_MAX_VALUES[Item::wheat_Id] = pair<int,int>(18, 22);
	MIN_MAX_VALUES[Tile::wool_Id] = pair<int,int>(14, 22);
	MIN_MAX_VALUES[Item::rotten_flesh_Id] = pair<int,int>(36, 64);

	MIN_MAX_PRICES[Item::flintAndSteel_Id] = pair<int,int>(3, 4);
	MIN_MAX_PRICES[Item::shears_Id] = pair<int,int>(3, 4);
	MIN_MAX_PRICES[Item::sword_iron_Id] = pair<int,int>(7, 11);
	MIN_MAX_PRICES[Item::sword_diamond_Id] = pair<int,int>(12, 14);
	MIN_MAX_PRICES[Item::hatchet_iron_Id] = pair<int,int>(6, 8);
	MIN_MAX_PRICES[Item::hatchet_diamond_Id] = pair<int,int>(9, 12);
	MIN_MAX_PRICES[Item::pickAxe_iron_Id] = pair<int,int>(7, 9);
	MIN_MAX_PRICES[Item::pickAxe_diamond_Id] = pair<int,int>(10, 12);
	MIN_MAX_PRICES[Item::shovel_iron_Id] = pair<int,int>(4, 6);
	MIN_MAX_PRICES[Item::shovel_diamond_Id] = pair<int,int>(7, 8);
	MIN_MAX_PRICES[Item::hoe_iron_Id] = pair<int,int>(4, 6);
	MIN_MAX_PRICES[Item::hoe_diamond_Id] = pair<int,int>(7, 8);
	MIN_MAX_PRICES[Item::boots_iron_Id] = pair<int,int>(4, 6);
	MIN_MAX_PRICES[Item::boots_diamond_Id] = pair<int,int>(7, 8);
	MIN_MAX_PRICES[Item::helmet_iron_Id] = pair<int,int>(4, 6);
	MIN_MAX_PRICES[Item::helmet_diamond_Id] = pair<int,int>(7, 8);
	MIN_MAX_PRICES[Item::chestplate_iron_Id] = pair<int,int>(10, 14);
	MIN_MAX_PRICES[Item::chestplate_diamond_Id] = pair<int,int>(16, 19);
	MIN_MAX_PRICES[Item::leggings_iron_Id] = pair<int,int>(8, 10);
	MIN_MAX_PRICES[Item::leggings_diamond_Id] = pair<int,int>(11, 14);
	MIN_MAX_PRICES[Item::boots_chain_Id] = pair<int,int>(5, 7);
	MIN_MAX_PRICES[Item::helmet_chain_Id] = pair<int,int>(5, 7);
	MIN_MAX_PRICES[Item::chestplate_chain_Id] = pair<int,int>(11, 15);
	MIN_MAX_PRICES[Item::leggings_chain_Id] = pair<int,int>(9, 11);
	MIN_MAX_PRICES[Item::bread_Id] = pair<int,int>(-4, -2);
	MIN_MAX_PRICES[Item::melon_Id] = pair<int,int>(-8, -4);
	MIN_MAX_PRICES[Item::apple_Id] = pair<int,int>(-8, -4);
	MIN_MAX_PRICES[Item::cookie_Id] = pair<int,int>(-10, -7);
	MIN_MAX_PRICES[Tile::glass_Id] = pair<int,int>(-5, -3);
	MIN_MAX_PRICES[Tile::bookshelf_Id] = pair<int,int>(3, 4);
	MIN_MAX_PRICES[Item::chestplate_leather_Id] = pair<int,int>(4, 5);
	MIN_MAX_PRICES[Item::boots_leather_Id] = pair<int,int>(2, 4);
	MIN_MAX_PRICES[Item::helmet_leather_Id] = pair<int,int>(2, 4);
	MIN_MAX_PRICES[Item::leggings_leather_Id] = pair<int,int>(2, 4);
	MIN_MAX_PRICES[Item::saddle_Id] = pair<int,int>(6, 8);
	MIN_MAX_PRICES[Item::expBottle_Id] = pair<int,int>(-4, -1);
	MIN_MAX_PRICES[Item::redStone_Id] = pair<int,int>(-4, -1);
	MIN_MAX_PRICES[Item::compass_Id] = pair<int,int>(10, 12);
	MIN_MAX_PRICES[Item::clock_Id] = pair<int,int>(10, 12);
	MIN_MAX_PRICES[Tile::glowstone_Id] = pair<int,int>(-3, -1);
	MIN_MAX_PRICES[Item::porkChop_cooked_Id] = pair<int,int>(-7, -5);
	MIN_MAX_PRICES[Item::beef_cooked_Id] = pair<int,int>(-7, -5);
	MIN_MAX_PRICES[Item::chicken_cooked_Id] = pair<int,int>(-8, -6);
	MIN_MAX_PRICES[Item::eyeOfEnder_Id] = pair<int,int>(7, 11);
	MIN_MAX_PRICES[Item::arrow_Id] = pair<int,int>(-12, -8);
}

/**
* Adds a merchant recipe that trades items for a single ruby.
*
* @param list
* @param itemId
* @param random
* @param likelyHood
*/
void Villager::addItemForTradeIn(MerchantRecipeList *list, int itemId, Random *random, float likelyHood)
{
	if (random->nextFloat() < likelyHood)
	{
		list->push_back(new MerchantRecipe(getItemTradeInValue(itemId, random), Item::emerald));
	}
}

shared_ptr<ItemInstance> Villager::getItemTradeInValue(int itemId, Random *random)
{
	return shared_ptr<ItemInstance>(new ItemInstance(itemId, getTradeInValue(itemId, random), 0));
}

int Villager::getTradeInValue(int itemId, Random *random)
{
    auto it = MIN_MAX_VALUES.find(itemId);
    if (it == MIN_MAX_VALUES.end())
	{
		return 1;
	}
	pair<int, int> minMax = it->second;
	if (minMax.first >= minMax.second)
	{
		return minMax.first;
	}
	return minMax.first + random->nextInt(minMax.second - minMax.first);
}

/**
* Adds a merchant recipe that trades rubies for an item. If the cost is
* negative, one ruby will give several of that item.
*
* @param list
* @param itemId
* @param random
* @param likelyHood
*/
void Villager::addItemForPurchase(MerchantRecipeList *list, int itemId, Random *random, float likelyHood)
{
	if (random->nextFloat() < likelyHood)
	{
		int purchaseCost = getPurchaseCost(itemId, random);
		shared_ptr<ItemInstance> rubyItem;
		shared_ptr<ItemInstance> resultItem;
		if (purchaseCost < 0)
		{
			rubyItem = shared_ptr<ItemInstance>( new ItemInstance(Item::emerald_Id, 1, 0) );
			resultItem = shared_ptr<ItemInstance>( new ItemInstance(itemId, -purchaseCost, 0) );
		}
		else
		{
			rubyItem = shared_ptr<ItemInstance>( new ItemInstance(Item::emerald_Id, purchaseCost, 0) );
			resultItem = shared_ptr<ItemInstance>( new ItemInstance(itemId, 1, 0) );
		}
		list->push_back(new MerchantRecipe(rubyItem, resultItem));
	}
}

int Villager::getPurchaseCost(int itemId, Random *random)
{
    auto it = MIN_MAX_PRICES.find(itemId);
    if (it == MIN_MAX_PRICES.end())
	{
		return 1;
	}
	pair<int, int> minMax = it->second;
	if (minMax.first >= minMax.second)
	{
		return minMax.first;
	}
	return minMax.first + random->nextInt(minMax.second - minMax.first);
}

void Villager::handleEntityEvent(byte id)
{
	if (id == EntityEvent::LOVE_HEARTS)
	{
		addParticlesAroundSelf(eParticleType_heart);
	}
	else if (id == EntityEvent::VILLAGER_ANGRY)
	{
		addParticlesAroundSelf(eParticleType_angryVillager);
	}
	else if (id == EntityEvent::VILLAGER_HAPPY)
	{
		addParticlesAroundSelf(eParticleType_happyVillager);
	}
	else
	{
		AgableMob::handleEntityEvent(id);
	}
}

void Villager::addParticlesAroundSelf(ePARTICLE_TYPE particle)
{
	for (int i = 0; i < 5; i++)
	{
		double xa = random->nextGaussian() * 0.02;
		double ya = random->nextGaussian() * 0.02;
		double za = random->nextGaussian() * 0.02;
		level->addParticle(particle, x + random->nextFloat() * bbWidth * 2 - bbWidth, y + 1.0f + random->nextFloat() * bbHeight, z + random->nextFloat() * bbWidth * 2 - bbWidth, xa, ya, za);
	}
}

MobGroupData *Villager::finalizeMobSpawn(MobGroupData *groupData, int extraData /*= 0*/) // 4J Added extraData param
{
	groupData = AgableMob::finalizeMobSpawn(groupData);

	setProfession(level->random->nextInt(PROFESSION_MAX));

	return groupData;
}

void Villager::setRewardPlayersInVillage()
{
	rewardPlayersOnFirstVillage = true;
}

shared_ptr<AgableMob> Villager::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to villagers that can be bred
	if(level->canCreateMore(GetType(), Level::eSpawnType_Breed) )
	{
		shared_ptr<Villager> villager = shared_ptr<Villager>(new Villager(level));
		villager->finalizeMobSpawn(NULL);
		return villager;
	}
	else
	{
		return nullptr;
	}
}

bool Villager::canBeLeashed()
{
	return false;
}

wstring Villager::getDisplayName()
{
	if (hasCustomName()) return getCustomName();

	int name = IDS_VILLAGER;
	switch(getProfession())
	{
	case PROFESSION_FARMER:
		name = IDS_VILLAGER_FARMER;
		break;
	case PROFESSION_LIBRARIAN:
		name = IDS_VILLAGER_LIBRARIAN;
		break;
	case PROFESSION_PRIEST:
		name = IDS_VILLAGER_PRIEST;
		break;
	case PROFESSION_SMITH:
		name = IDS_VILLAGER_SMITH;
		break;
	case PROFESSION_BUTCHER:
		name = IDS_VILLAGER_BUTCHER;
		break;
	};
	return app.GetString(name);
}
